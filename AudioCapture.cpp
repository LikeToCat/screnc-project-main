#include "stdafx.h"
#include "CDebug.h"
#include "AudioCapture.h"
#include "LarStringConversion.h"


extern HANDLE ConsoleHandle;

AudioCapture::AudioCapture(AudioSampleRate sampleRate, AudioBitRate bitRate, AudioFmt audioFmt)
	: m_pFormatCtx_AudioInner(NULL)
	, m_pReadCodecCtx_AudioInner(NULL)
	, m_sampleRate(sampleRate)
	, m_bitRate(bitRate)
	, m_pAudioInnerResampleCtx(NULL)
	, m_pAudioInnerFifo(NULL)
	, m_pAudioInnerResampleFifo(NULL)
	, m_InitSuccess(false)
	, m_IsRecord(false)
	, m_pAudioFrame(nullptr)
	, m_pAudioConvertCtx(nullptr)
	, m_frameCount(0)
	, m_audioFmt(audioFmt)
{
	INIT_ERROR_HANDLING(ConsoleHandle);
	avdevice_register_all();
	m_pAudioFrame = av_frame_alloc();
}

AudioCapture::~AudioCapture()
{
	StopCapture();
	if(m_pAudioFrame)
		av_frame_free(&m_pAudioFrame);
}

bool AudioCapture::Init()
{
	m_InitSuccess = true;
	if (!InitAudioCapture()) {//初始化音频捕获器
		m_InitSuccess = false;
		return false;
	}
	if (!InitAudioInnerResampleCtx()) {//初始化音频采样率转换器
		m_InitSuccess = false;
		return false;
	}
	if (!InitAudioConvertCtx()) {//初始化音频格式转换器
		m_InitSuccess = false;
		return false;
	}
	//分配存储捕获到的原始音频数据的缓冲区大小
	m_pAudioInnerFifo = av_audio_fifo_alloc(
		(AVSampleFormat)m_pFormatCtx_AudioInner->streams[0]->codecpar->format,
		m_pFormatCtx_AudioInner->streams[0]->codecpar->channels,
		3000 * 1024
	);
	//分配存储重采样处理后的音频数据
	m_pAudioInnerResampleFifo = av_audio_fifo_alloc(
		(AVSampleFormat)m_pFormatCtx_AudioInner->streams[0]->codecpar->format,
		m_pFormatCtx_AudioInner->streams[0]->codecpar->channels,
		3000 * 1024
	);
	CHECK_CONDITION_ERROR(m_pAudioInnerResampleFifo, L"错误！分配存储重采样处理后的音频数据缓冲区失败!");
	CHECK_CONDITION_ERROR(m_pAudioInnerFifo, L"错误！分配原始音频数据缓冲区失败!");

	return m_InitSuccess;
}

bool AudioCapture::StartCapture()
{
	if (m_IsRecord) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"音频已经开始捕获，调用StartCapture失败");
		return false;
	}
	m_IsRecord = true;
	if (!m_InitSuccess) {
		CHECK_CONDITION_ERROR(nullptr, L"音频捕获调用失败！可能是初始化失败或者未进行初始化");
	}
	m_hAudioInnerCapture = CreateThread(NULL, 0, AudioInnerCaptureProc, this, 0, NULL);//音频捕获线程开启
	m_hAudioInnerResample = CreateThread(NULL, 0, AudioInnerResampleProc, this, 0, NULL);//音频重采样线程开启
	return true;
}

bool AudioCapture::CaptureFrame(void** frameData, int* frameSize)
{
	return QueryFrame(frameData, frameSize);
}

bool AudioCapture::StopCapture()
{
	if (!m_IsRecord) {
		return false;
	}
	m_IsRecord = false;
	m_frameCount = 0;
	// 等待线程自然终止（最多等待5秒）
	if (m_hAudioInnerCapture != NULL) {
		DWORD waitResult = WaitForSingleObject(m_hAudioInnerCapture, 5000);
		if (waitResult == WAIT_TIMEOUT) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"警告：音频捕获线程未在预期时间内终止");
		}
		CloseHandle(m_hAudioInnerCapture);
		m_hAudioInnerCapture = NULL;
	}
	if (m_hAudioInnerResample != NULL) {
		DWORD waitResult = WaitForSingleObject(m_hAudioInnerResample, 5000);
		if (waitResult == WAIT_TIMEOUT) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"警告：音频重采样线程未在预期时间内终止");
		}
		CloseHandle(m_hAudioInnerResample);
		m_hAudioInnerResample = NULL;
	}

	// 释放音频处理相关资源
	if (m_pAudioConvertCtx != NULL) {
		swr_free(&m_pAudioConvertCtx);
		m_pAudioConvertCtx = NULL;
	}
	if (m_pAudioInnerFifo != NULL) {
		av_audio_fifo_free(m_pAudioInnerFifo);
		m_pAudioInnerFifo = NULL;
	}
	if (m_pAudioInnerResampleFifo != NULL) {
		av_audio_fifo_free(m_pAudioInnerResampleFifo);
		m_pAudioInnerResampleFifo = NULL;
	}
	if (m_pAudioInnerResampleCtx != NULL) {
		swr_free(&m_pAudioInnerResampleCtx);
		m_pAudioInnerResampleCtx = NULL;
	}
	if (m_pReadCodecCtx_AudioInner != NULL) {
		avcodec_free_context(&m_pReadCodecCtx_AudioInner);
		m_pReadCodecCtx_AudioInner = NULL;
	}
	if (m_pFormatCtx_AudioInner != NULL) {
		avformat_close_input(&m_pFormatCtx_AudioInner);
		m_pFormatCtx_AudioInner = NULL;
	}

	DEBUG_CONSOLE_STR(ConsoleHandle, L"音频捕获已停止，资源已释放");
	return true;
}

bool AudioCapture::InitAudioCapture()
{
	//查找并打开音频捕获器
#ifdef TARGET_WIN10
	const AVInputFormat* pInputFmt = av_find_input_format("dshow");
#elif defined TARGET_WIN7
	AVInputFormat* pInputFmt = av_find_input_format("dshow");
#endif
	const char* pDeviceCapture = "audio=virtual-audio-capturer";
	CHECK_CONDITION_ERROR(pInputFmt, L"打开输入格式失败");
	int ret = avformat_open_input(&m_pFormatCtx_AudioInner, pDeviceCapture, pInputFmt, NULL);
	if (ret < 0)
		CHECK_FFMPEG_ERROR(ret, L"打开音频并获取失败");

	//查找流信息，获取并分配解码器上下文
	ret = avformat_find_stream_info(m_pFormatCtx_AudioInner, NULL);
	if (ret < 0)
		CHECK_FFMPEG_ERROR(ret, L"查找流信息失败");
	if (m_pFormatCtx_AudioInner->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
		CHECK_CONDITION_ERROR(nullptr, L"错误！查找不到音频流");
	}
	const AVCodec* codec = avcodec_find_decoder(m_pFormatCtx_AudioInner->streams[0]->codecpar->codec_id);
	CHECK_CONDITION_ERROR(codec, L"错误！查找不到解码器");
	m_pReadCodecCtx_AudioInner = avcodec_alloc_context3(codec);

	//设置解码器的一些参数
	m_pReadCodecCtx_AudioInner->sample_rate = m_pFormatCtx_AudioInner->streams[0]->codecpar->sample_rate;
	m_pReadCodecCtx_AudioInner->sample_fmt = (AVSampleFormat)m_pFormatCtx_AudioInner->streams[0]
		->codecpar->format;
	m_pReadCodecCtx_AudioInner->channel_layout = AV_CH_LAYOUT_STEREO;
	m_pReadCodecCtx_AudioInner->channels = 2;

	//初始化解码器并与解码器上下文绑定
	ret = avcodec_open2(m_pReadCodecCtx_AudioInner, codec, NULL);
	if (ret < 0)
		CHECK_FFMPEG_ERROR(ret, L"初始化解码器并与解码器上下文绑定失败");

	//解码器参数更新给ffmpeg上下文(保证参数一致)
	avcodec_parameters_from_context(m_pFormatCtx_AudioInner->streams[0]->codecpar, m_pReadCodecCtx_AudioInner);
	return true;
}

bool AudioCapture::InitAudioInnerResampleCtx()
{
	int iRet = 0;
	m_pAudioInnerResampleCtx = swr_alloc();
	av_opt_set_int(m_pAudioInnerResampleCtx, "in_channel_layout",
		m_pFormatCtx_AudioInner->streams[0]->codecpar->channel_layout, 0);
	av_opt_set_int(m_pAudioInnerResampleCtx, "out_channel_layout",
		m_pFormatCtx_AudioInner->streams[0]->codecpar->channel_layout, 0);
	av_opt_set_int(m_pAudioInnerResampleCtx, "in_sample_rate",
		m_pFormatCtx_AudioInner->streams[0]->codecpar->sample_rate, 0);
	av_opt_set_int(m_pAudioInnerResampleCtx, "out_sample_rate", (int)m_sampleRate, 0);
	av_opt_set_sample_fmt(m_pAudioInnerResampleCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_sample_fmt(m_pAudioInnerResampleCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

	iRet = swr_init(m_pAudioInnerResampleCtx);
	return iRet < 0 ? false : true;
}

bool AudioCapture::InitAudioConvertCtx()
{
	int iRet = 0;
	m_pAudioConvertCtx = swr_alloc();

	// 使用预定义常量设置立体声通道布局
	av_opt_set_int(m_pAudioConvertCtx, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(m_pAudioConvertCtx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);

	// 其他设置保持不变
	av_opt_set_int(m_pAudioConvertCtx, "in_sample_rate", 44100, 0);
	av_opt_set_int(m_pAudioConvertCtx, "out_sample_rate", 44100, 0);
	av_opt_set_sample_fmt(m_pAudioConvertCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	av_opt_set_sample_fmt(m_pAudioConvertCtx, "out_sample_fmt",
		m_audioFmt == AudioFmt::MP4 ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16,
		0);

	iRet = swr_init(m_pAudioConvertCtx);
	return iRet < 0 ? false : true;
}

DWORD WINAPI AudioCapture::AudioInnerCaptureProc(LPVOID param)
{
	AudioCapture* pAudioCapture = (AudioCapture*)param;
	if (pAudioCapture != NULL) {
		pAudioCapture->AudioInnerCaptureThread();
	}
	else {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！音频捕获线程开启失败，没有获得类对象");
	}
	return 0;
}

DWORD WINAPI AudioCapture::AudioInnerResampleProc(LPVOID param)
{
	AudioCapture* pAudioCapture = (AudioCapture*)param;
	if (pAudioCapture != NULL) {
		pAudioCapture->AudioResampleThread();
	}
	else {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！音频重采样线程开启失败，没有获得类对象");
	}
	return 0;
}

void AudioCapture::AudioInnerCaptureThread()
{
	//从捕获设备中（设备上下文）捕获到数据并解码存入原始音频缓冲区中
	AVFrame* frame = av_frame_alloc();
	AVPacket packet = { 0 };
	while (m_IsRecord) {
		av_packet_unref(&packet);
		int ret = av_read_frame(m_pFormatCtx_AudioInner, &packet);
		if (ret == EAGAIN) {//如果解码器需要更多数据
			continue;
		}
		else if (ret == AVERROR_EOF) {//数据读取完成（理论上永远不会执行）
			break;
		}
		else if (ret < 0) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"读取一帧音频数据失败");
			break;
		}
		ret = avcodec_send_packet(m_pReadCodecCtx_AudioInner, &packet);
		if (ret < 0) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"解码一帧音频数据失败");
			break;
		}
		ret = avcodec_receive_frame(m_pReadCodecCtx_AudioInner, frame);
		if (ret < 0) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"获取解码后的一帧音频数据失败");
			break;
		}
		else if (ret == EAGAIN) {
			continue;
		}
		else if (ret == AVERROR_EOF) {
			break;
		}

		int fifoSpace = av_audio_fifo_space(m_pAudioInnerFifo);
		if (frame->nb_samples <= fifoSpace) {//缓冲区中还有可用空间存储
			{
				std::lock_guard<std::mutex> lock(m_csAudioInnerMutex);
				av_audio_fifo_write(m_pAudioInnerFifo, (void**)frame->data, frame->nb_samples);
			}
		}
		av_packet_unref(&packet);
	}

	av_frame_free(&frame);
	DEBUG_CONSOLE_STR(ConsoleHandle, L"停止捕获音频数据");
}

void AudioCapture::AudioResampleThread()
{
	int ret = 0;

	while (true)
	{
		if (av_audio_fifo_size(m_pAudioInnerFifo) >= 1024)
		{
			// 创建输入帧
			AVFrame* frame_audio_inner = av_frame_alloc();
			frame_audio_inner->nb_samples = 1024;
			frame_audio_inner->channel_layout = m_pFormatCtx_AudioInner->streams[0]->codecpar->channel_layout;
			frame_audio_inner->channels = m_pFormatCtx_AudioInner->streams[0]->codecpar->channels;
			frame_audio_inner->format = m_pFormatCtx_AudioInner->streams[0]->codecpar->format;
			frame_audio_inner->sample_rate = m_pFormatCtx_AudioInner->streams[0]->codecpar->sample_rate;
			av_frame_get_buffer(frame_audio_inner, 0);

			// 从原始FIFO读取数据
			{
				std::lock_guard<std::mutex> lock(m_csAudioInnerMutex);
				int readcount = av_audio_fifo_read(
					m_pAudioInnerFifo,
					(void**)frame_audio_inner->data,
					frame_audio_inner->nb_samples);
			}

			// 创建输出重采样帧
			AVFrame* frame_audio_inner_resample = av_frame_alloc();

			// 使用与ULinkRecord完全相同的计算方式
			int iDelaySamples = 0;
			int dst_nb_samples = av_rescale_rnd(
				iDelaySamples + frame_audio_inner->nb_samples,
				(int)m_sampleRate, frame_audio_inner->sample_rate,
				AV_ROUND_UP);

			// 设置重采样帧参数
			frame_audio_inner_resample->nb_samples = 1024; // 固定为1024
			frame_audio_inner_resample->channel_layout = m_pFormatCtx_AudioInner->streams[0]->codecpar->channel_layout;
			frame_audio_inner_resample->channels = m_pFormatCtx_AudioInner->streams[0]->codecpar->channels;
			frame_audio_inner_resample->format = AV_SAMPLE_FMT_S16; // 明确指定为S16
			frame_audio_inner_resample->sample_rate = (int)m_sampleRate;
			av_frame_get_buffer(frame_audio_inner_resample, 0);

			//直接使用指向data[0]的单指针，完全模拟ULinkRecord的处理方式
			uint8_t* out_buffer = (uint8_t*)frame_audio_inner_resample->data[0];

			int nb = swr_convert(
				m_pAudioInnerResampleCtx,
				&out_buffer, 
				dst_nb_samples,
				(const uint8_t**)frame_audio_inner->data,
				frame_audio_inner->nb_samples
			);

			// 写入重采样FIFO
			{
				std::lock_guard<std::mutex> lock(m_csAudioInnerMutex);
				ret = av_audio_fifo_write(
					m_pAudioInnerResampleFifo,
					(void**)frame_audio_inner_resample->data,
					nb // 使用实际转换的样本数
				);
			}

			av_frame_free(&frame_audio_inner);
			av_frame_free(&frame_audio_inner_resample);

			if (!m_IsRecord) {
				if (av_audio_fifo_size(m_pAudioInnerFifo) < 1024) {
					break;
				}
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (!m_IsRecord) {
				break;
			}
		}
	}
}

bool AudioCapture::QueryFrame(void** frameData, int* captureSize)
{
	if ((!m_InitSuccess) || (!m_IsRecord)) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！从音频缓冲区中拿取一帧数据失败");
		return false;
	}

	while (m_IsRecord)
	{
		if (av_audio_fifo_size(m_pAudioInnerResampleFifo) >= 1024)
		{
			// 读取音频数据
			AVFrame* frame_mix = av_frame_alloc();
			frame_mix->nb_samples = (m_audioFmt == AudioFmt::MP4 ? 1024 : 1012);
			frame_mix->channel_layout = m_pFormatCtx_AudioInner->streams[0]->codecpar->channel_layout;
			frame_mix->channels = m_pFormatCtx_AudioInner->streams[0]->codecpar->channels;
			frame_mix->format = AV_SAMPLE_FMT_S16;
			frame_mix->sample_rate = (int)m_sampleRate;

			int ret = av_frame_get_buffer(frame_mix, 0);
			if (ret < 0)
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"申请帧存储原始音频数据失败");
				av_frame_free(&frame_mix);
				return false;
			}	

			// 从FIFO读取
			{
				std::lock_guard<std::mutex> lock(m_csAudioInnerMutex);
				ret = av_audio_fifo_read(
					m_pAudioInnerResampleFifo,
					(void**)frame_mix->data,
					frame_mix->nb_samples
				);
				if (ret < 0) 
				{
					av_frame_free(&frame_mix);
					return false;
				}
			}

			// 准备最终重采样的编码所需帧
			av_frame_unref(m_pAudioFrame);
			m_pAudioFrame->nb_samples = m_audioFmt == AudioFmt::MP4 ? 1024 : 1012;
			m_pAudioFrame->channel_layout = m_pFormatCtx_AudioInner->streams[0]->codecpar->channel_layout;
			m_pAudioFrame->channels = m_pFormatCtx_AudioInner->streams[0]->codecpar->channels;
			m_pAudioFrame->format = (
				m_audioFmt == AudioFmt::MP4 ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16
				);
			m_pAudioFrame->sample_rate = (int)m_sampleRate;

			ret = av_frame_get_buffer(m_pAudioFrame, 0);
			if (ret < 0) 
			{
				av_frame_free(&frame_mix);
				return false;
			}

			uint8_t* audio_buf[2] = { 0 };
			audio_buf[0] = (uint8_t*)m_pAudioFrame->data[0];
			audio_buf[1] = (uint8_t*)m_pAudioFrame->data[1];

			ret = swr_convert(
				m_pAudioConvertCtx,
				audio_buf,
				m_pAudioFrame->nb_samples,
				(const uint8_t**)frame_mix->data,
				frame_mix->nb_samples
			);

			if (ret < 0)
			{
				av_frame_free(&frame_mix);
				DEBUG_CONSOLE_STR(ConsoleHandle, L"重采样编码帧失败");
				return false;
			}

			m_frameCount++;
			*frameData = (void*)m_pAudioFrame;
			*captureSize = m_pAudioFrame->nb_samples;
			av_frame_free(&frame_mix);
			return true;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	return false;
}