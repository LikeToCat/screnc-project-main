#include "stdafx.h"
#include "CameraCapture.h"
#include "CDebug.h"
#include "LarStringConversion.h"
extern HANDLE ConsoleHandle;

CameraCapture* CameraCapture::instance = nullptr;
CameraCapture::CameraCapture(CameraOptions cameraOptions)
{
	avdevice_register_all();
	m_IsInitSuccess = false;
	m_IsCapture = false;
	m_VideoFrame = av_frame_alloc();
	if (!m_VideoFrame) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"视频帧结构分配失败");
	}
	m_IsFrameAvailable = false;
	m_IsPausing = false;
	m_OptionParam = cameraOptions;
	memset((void*)m_Buf, 0, 256);
}

CameraCapture::CameraCapture()
{
	avdevice_register_all();
	m_IsInitSuccess = false;
	m_IsCapture = false;
	m_VideoFrame = av_frame_alloc();
	if (!m_VideoFrame) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"视频帧结构分配失败");
	}
	m_IsFrameAvailable = false;
	m_OptionParam = CameraOptions();
}

CameraCapture::~CameraCapture()
{
	StopCapture();
}

CameraCapture* CameraCapture::GetInstance()
{
	if (!instance) {
		instance = new CameraCapture;
	}
	return instance;
}

void CameraCapture::ReleaseInstance()
{
	if (instance) 
	{
		delete instance;
		instance = nullptr;
	}
}

void CameraCapture::StartCapture()
{
	if (m_IsCapture|| !m_IsInitSuccess) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用StartCapture");
		return;
	}
	m_IsCapture = true;
	m_Decode_Thread = std::thread(&CameraCapture::DecodeFrame, this);
}

void CameraCapture::StopCapture()
{
	if (!m_IsCapture) {
		return; // 如果已经停止了，直接返回
	}

	m_IsCapture = false;

	// 等待解码线程结束
	if (m_Decode_Thread.joinable()) {
		m_Decode_Thread.join();
	}

	// 清理帧和编解码器资源
	{
		std::lock_guard<std::mutex> lock(m_FrameBuf_Mutex);
		m_IsFrameAvailable = false;

		if (m_VideoFrame) {
			av_frame_unref(m_VideoFrame);
		}
	}

	// 唤醒任何可能在等待的条件变量
	m_FrameBuf_CV.notify_all();

	// 释放资源
	if (m_pInput_Ctx) {
		avformat_close_input(&m_pInput_Ctx);
		m_pInput_Ctx = nullptr;
	}

	if (m_pDecode_CodecCtx) {
		avcodec_free_context(&m_pDecode_CodecCtx);
		m_pDecode_CodecCtx = nullptr;
	}

	if (m_VideoFrame) {
		av_frame_free(&m_VideoFrame);
		m_VideoFrame = nullptr;
	}
}

bool CameraCapture::Init(CameraOptions cameraOptions)
{
	m_OptionParam = cameraOptions;
	if (!InitInputDevice()) {//初始化输入设备
		return false;
	}
	m_IsInitSuccess = true;
	return true;
}

bool CameraCapture::CaptureFrame(AVFrame* frame)
{
	if (!m_IsInitSuccess || !m_IsCapture) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用CaptureFrame");
		return false;
	}
	
	//判断是否有新帧可用
	std::unique_lock<std::mutex> lock(m_FrameBuf_Mutex);
	if (!m_IsFrameAvailable)
	{
		if (!m_FrameBuf_CV.wait_for(lock, std::chrono::milliseconds(1000),
			[this]() { return m_IsFrameAvailable; }))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[摄像头采集]: 等待帧超时");
			return false;
		}
	}

	//拷贝数据
	int ret = 0;
	ret = av_frame_ref(frame, m_VideoFrame);
	if (ret < 0) 
	{
		av_strerror(ret, m_Buf, 256);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"拷贝帧数据失败: %s", LARSC::c2w(m_Buf));
		return false;
	}
	m_IsFrameAvailable = false;
	return true;
}

void CameraCapture::PauseCapture()
{
	{//暂停捕获线程
		std::lock_guard<std::mutex> PausingLock(m_Pausing_Mutex);
		if (!m_IsPausing.load())
		{
			m_IsPausing.store(true);
		}
	}
}

void CameraCapture::ResumeCapture()
{
	{//恢复捕获线程
		std::lock_guard<std::mutex> PausingLock(m_Pausing_Mutex);
		if (m_IsPausing.load())m_IsPausing.store(false);
		m_Pasuing_CV.notify_all();//通知捕获线程可以继续开始捕获了
	}
}

bool CameraCapture::InitInputDevice()
{
	//打开输入格式
#ifdef TARGET_WIN10
	const AVInputFormat* inputFmt = av_find_input_format("dshow");
	if (!inputFmt)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"dshow打开失败");
		return false;
	}
#elif defined TARGET_WIN7
	AVInputFormat* inputFmt = av_find_input_format("dshow");
	if (!inputFmt)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"dshow打开失败");
		return false;
	}
#endif

	std::string video_size = std::to_string(m_OptionParam.pixelX) +
		"x" + std::to_string(m_OptionParam.pixelY);
	std::string fps = std::to_string(m_OptionParam.fps);
	std::string desc = "video=" + m_OptionParam.deviceDesc;
	AVDictionary* options = NULL;
	av_dict_set(&options, "video_size", video_size.c_str(), 0);
	av_dict_set(&options, "framerate", fps.c_str(), 0);
	if (m_OptionParam.vcodec == "mjpeg")
	{
		av_dict_set(&options, "input_format", "mjpeg", 0);
	}
	else
	{
		av_dict_set(&options, "pixel_format", m_OptionParam.vcodec.c_str(), 0);
	}
	int ret = avformat_open_input(&m_pInput_Ctx, desc.c_str(), inputFmt, &options);
	av_dict_free(&options);
	if (ret < 0) {
		av_strerror(ret, m_Buf, 256);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"打开设备失败:%s", LARSC::c2w(m_Buf));
		return false;
	}

	//查找视频流信息，并查找并分配解码器上下文
	ret = avformat_find_stream_info(m_pInput_Ctx, NULL);
	if (ret < 0) {
		av_strerror(ret, m_Buf, 256);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"打开设备失败:%s", LARSC::c2w(m_Buf));
		return false;
	}
	m_VideoStreamIndex = -1;
	for (size_t i = 0; i < m_pInput_Ctx->nb_streams; i++)
	{
		if (m_pInput_Ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_VideoStreamIndex = i;
		}
	}
	if (m_VideoStreamIndex == -1) {
		avformat_free_context(m_pInput_Ctx);
		DEBUG_CONSOLE_STR(ConsoleHandle, L"查找摄像头设备视频流失败");
		return false;
	}
	const AVCodec* codec = avcodec_find_decoder(m_pInput_Ctx->streams[m_VideoStreamIndex]->codecpar->codec_id);
	if (!codec) {
		avformat_free_context(m_pInput_Ctx);
		DEBUG_CONSOLE_STR(ConsoleHandle, L"查找解码器上下文失败");
		return false;
	}
	m_pDecode_CodecCtx = avcodec_alloc_context3(codec);
	if (!m_pDecode_CodecCtx) {
		avformat_free_context(m_pInput_Ctx);
		DEBUG_CONSOLE_STR(ConsoleHandle, L"分配解码器上下文失败");
		return false;
	}
	ret = avcodec_parameters_to_context(m_pDecode_CodecCtx, m_pInput_Ctx->streams[m_VideoStreamIndex]->codecpar);
	if (ret < 0) {
		avformat_free_context(m_pInput_Ctx);
		avcodec_free_context(&m_pDecode_CodecCtx);
		av_strerror(ret, m_Buf, 256);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"流参数到解码器参数失败:%s", LARSC::c2w(m_Buf));
		return false;
	}
	ret = avcodec_open2(m_pDecode_CodecCtx, codec, NULL);
	if (ret < 0) {
		avformat_free_context(m_pInput_Ctx);
		avcodec_free_context(&m_pDecode_CodecCtx);
		av_strerror(ret, m_Buf, 256);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"打开初始化解码器失败:%s", LARSC::c2w(m_Buf));
		return false;
	}
	return true;
}

void CameraCapture::DecodeFrame()
{
	//准备解码后的数据包
	AVPacket packet = { 0 };
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;
	int ret = 0;

	while (m_IsCapture.load()) 
	{
		ret = av_read_frame(m_pInput_Ctx, &packet);
		if (ret < 0)
		{
			av_strerror(ret, m_Buf, 256);
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"从设备上下文中获取一帧数据包失败:%s", LARSC::c2w(m_Buf));
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}
		{//是否暂停解码
			std::unique_lock<std::mutex> PausingLock(m_Pausing_Mutex);
			while (m_IsPausing.load() && m_IsCapture.load())
			{
				m_Pasuing_CV.wait(PausingLock);// 等待恢复，避免空转
			}
		}
		if (packet.stream_index == m_VideoStreamIndex) 
		{//如果是视频数据
			ret =avcodec_send_packet(m_pDecode_CodecCtx, &packet);
			if (ret < 0) 
			{
				av_strerror(ret, m_Buf, 256);
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"发送数据包到解码器失败:%s", LARSC::c2w(m_Buf));
				av_packet_unref(&packet);
				continue;
			}
			if (ret >= 0) 
			{//解码到一帧
				std::lock_guard<std::mutex> lock(m_FrameBuf_Mutex);
				av_frame_unref(m_VideoFrame);
				ret = avcodec_receive_frame(m_pDecode_CodecCtx, m_VideoFrame);
				if (ret == 0)
				{
					m_IsFrameAvailable = true;
					m_FrameBuf_CV.notify_one();
				}
				else if (ret == AVERROR(EAGAIN))
				{
					DB(ConsoleHandle, L"编码器需要更多数据....");
				}
				else if (ret == AVERROR_EOF)
				{
					DEBUG_CONSOLE_STR(ConsoleHandle, L"[摄像头解码线程]: 解码到流结束");
				}
				else
				{
					av_strerror(ret, m_Buf, 256);
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"[摄像头解码线程]: 接收解码帧失败: %s", LARSC::c2w(m_Buf));
				}
			}
			else if (ret == AVERROR(EAGAIN))
			{
				continue;
			}
			else if (ret < 0) {
				av_strerror(ret, m_Buf, 256);
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"解码一帧视频帧失败:%s", LARSC::c2w(m_Buf));
				continue;
			}
			
		}
		av_packet_unref(&packet);
	}
	av_packet_unref(&packet);
}
