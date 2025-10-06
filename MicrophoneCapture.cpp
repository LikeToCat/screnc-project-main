#include "stdafx.h"
#include "CDebug.h"
#include "LarStringConversion.h"
#include "MicrophoneCapture.h"
#include "DeviceManager.h"
#define DeviceManagerIns DeviceManager::GetInstance()

MicrophoneCapture::MicrophoneCapture(
    MircroSampleRate sampleRate,
    MircroBitRate bitRate,
    MircroFmt microFmt)
    : m_sampleRate(sampleRate)
    , m_birRate(bitRate)
    , m_microFmt(microFmt)
{
    m_errBuf = new char[256];
    memset(m_errBuf, 0, 256);
    m_pMicro_Frame = av_frame_alloc();
}

MicrophoneCapture::~MicrophoneCapture()
{
    StopCapture();
    if (m_pMicro_Frame)
    {
        av_frame_free(&m_pMicro_Frame);
        m_pMicro_Frame = nullptr;
    }
    if (m_errBuf)
    {
        delete[] m_errBuf;
        m_errBuf = nullptr;
    }
}

bool MicrophoneCapture::Init(std::wstring MicroDevice)
{
    m_isInitSuccess = true;
    if (!InitMicroCapture(MicroDevice))
    {//初始化音频捕获ffmpeg组件
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化麦克风捕获失败");
        m_isInitSuccess = false;
        return false;
    }
    if (!InitMicroResample())
    {//初始化音频采样率转换器ffmpeg组件
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化麦克风采样率转换器失败");
        m_isInitSuccess = false;
        return false;
    }
    if (!InitMicroResampleInner())
    {//初始化音频格式转换器ffmpeg组件
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化麦克风格式转换器失败");
        m_isInitSuccess = false;
        return false;
    }
    //初始化音频缓冲区
    m_pBuffer1_Fifo = av_audio_fifo_alloc(//初始化原始麦克风音频缓冲区
        (AVSampleFormat)m_pMicroDecode_Ctx->streams[0]->codecpar->format,
        m_pMicroDecode_Ctx->streams[0]->codecpar->channels,
        3000 * 1024
    );
    m_pBuffer2_Fifo = av_audio_fifo_alloc(//初始化音频采样率转换后的音频缓冲区
        (AVSampleFormat)m_pMicroDecode_Ctx->streams[0]->codecpar->format,
        m_pMicroDecode_Ctx->streams[0]->codecpar->channels,
        3000 * 1024
    );
    m_pBuffer3_Fifo = av_audio_fifo_alloc(
        ((m_microFmt == MircroFmt::MP4)|| (m_microFmt == MircroFmt::FLV)) ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16,
        m_pMicroDecode_CodecCtx->channels,
        3000 * 1024
    );
    return m_isInitSuccess;
}

bool MicrophoneCapture::StartCapture()
{
    if (!m_isInitSuccess || m_IsRecording) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用StartCapture");
        return false;
    }
    m_IsRecording = true;

    //开启麦克风音频捕获线程
    m_MircroDecode_Thread = std::thread(&MicrophoneCapture::DecodeMicroFrameThread, this);
    //开启麦克风采样率转换线程
    m_MircroResample1_Thread = std::thread(&MicrophoneCapture::MicroSampleRateThread, this);
    //开启麦克风音频格式转换线程
    m_MircroResample2_Thread = std::thread(&MicrophoneCapture::MircroFormatThread, this);
    return true;
}

bool MicrophoneCapture::StopCapture()
{
    if (!m_IsRecording || !m_isInitSuccess) {
        return false;
    }

    // 先设置停止标志
    m_IsRecording = false;

    // 确保线程完全退出
    if (m_MircroDecode_Thread.joinable()) {
        m_MircroDecode_Thread.join();
    }
    if (m_MircroResample1_Thread.joinable()) {
        m_MircroResample1_Thread.join();
    }
    if (m_MircroResample2_Thread.joinable()) {
        m_MircroResample2_Thread.join();
    }

    // 使用互斥锁保护资源释放
    {
        std::lock_guard<std::mutex> lock1(m_Buffer1_Mutex);
        std::lock_guard<std::mutex> lock2(m_Buffer2_Mutex);
        std::lock_guard<std::mutex> lock3(m_Buffer3_Mutex);

        // 首先释放较高级别的资源
        if (m_pBuffer1_Fifo) {
            av_audio_fifo_free(m_pBuffer1_Fifo);
            m_pBuffer1_Fifo = nullptr;
        }
        if (m_pBuffer2_Fifo) {
            av_audio_fifo_free(m_pBuffer2_Fifo);
            m_pBuffer2_Fifo = nullptr;
        }
        if (m_pBuffer3_Fifo) {
            av_audio_fifo_free(m_pBuffer3_Fifo);
            m_pBuffer3_Fifo = nullptr;
        }

        // 然后释放转换上下文
        if (m_pMicroSampleRate_SwrCtx) {
            swr_free(&m_pMicroSampleRate_SwrCtx);
            m_pMicroSampleRate_SwrCtx = nullptr;
        }
        if (m_pMicroFormat_SwrCtx) {
            swr_free(&m_pMicroFormat_SwrCtx);
            m_pMicroFormat_SwrCtx = nullptr;
        }

        // 最后释放解码器和输入上下文
        if (m_pMicroDecode_CodecCtx) {
            avcodec_free_context(&m_pMicroDecode_CodecCtx);
            m_pMicroDecode_CodecCtx = nullptr;
        }
        if (m_pMicroDecode_Ctx) {
            avformat_close_input(&m_pMicroDecode_Ctx);
            m_pMicroDecode_Ctx = nullptr;
        }
    }

    return true;
}

bool MicrophoneCapture::CaptureMicroFrame(void** microFrame, int* captureSize)
{
    return QueryFrame(microFrame, captureSize);
}

bool MicrophoneCapture::InitMicroCapture(std::wstring MicroDevice)
{
    m_wstr_MicroDevice = MicroDevice;
    if (m_wstr_MicroDevice == L"无可用设备")
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"无可用麦克风设备，无法录制麦克风");
        return false;
    }

    //获取用户选择的设备的设备描述
    std::string Desc, DeviceName;
    auto MicroDevices = DeviceManager::GetInstance().GetMicrophoneDevices();
    for (auto microDevice : MicroDevices)
    {
        if (microDevice.nameW == m_wstr_MicroDevice)
        {
            Desc = microDevice.alternateName;
            DeviceName = microDevice.nameA;
            break;
        }
    }
    if (Desc.empty())
    {
        return false;
    }

    //构造打开格式
    std::string OpenDeviceDesc = "audio=" + Desc;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"使用麦克风设备:%s",
        LARSC::s2ws(DeviceName).c_str());
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"使用麦克风设备的设备标识符:%s", LARSC::s2ws(OpenDeviceDesc).c_str());

    //打开dshow设备
#ifdef TARGET_WIN10
    const AVInputFormat* inputFormat = av_find_input_format("dshow");
    if (!inputFormat)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！找不到dshow设备，可能是windows版本不兼容");
        return false;
    }
#elif defined TARGET_WIN7
    AVInputFormat* inputFormat = av_find_input_format("dshow");
    if (!inputFormat)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！找不到dshow设备，可能是windows版本不兼容");
        return false;
    }
#endif // TARGET_WIN10

    //打开设备
    AVDictionary* options = nullptr;
    av_dict_set(&options, "avoid_src_mixing", "1", 0);
    m_errNum = avformat_open_input(
        &m_pMicroDecode_Ctx, OpenDeviceDesc.c_str(), inputFormat, &options
    );
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"使用麦克风设备的设备标识符:%s", LARSC::s2ws(OpenDeviceDesc).c_str());
    if (m_errNum < 0)
    {
        av_strerror(m_errNum, m_errBuf, 256);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！打开指定麦克风设备错误:%s", LARSC::c2w(m_errBuf));
        return false;
    }

    //查找流信息，并查找编码器
    m_errNum = avformat_find_stream_info(m_pMicroDecode_Ctx, NULL);
    if (m_errNum < 0) {
        av_strerror(m_errNum, m_errBuf, 256);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！查找流信息失败:%s", LARSC::c2w(m_errBuf));
        return false;
    }
    if (m_pMicroDecode_Ctx->streams[0]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！麦克风设备没有查找到音频流");
        return false;
    }
    const AVCodec* codec = avcodec_find_decoder(m_pMicroDecode_Ctx->streams[0]->codecpar->codec_id);
    if (!codec) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！没有查找的编码器");
        return false;
    }
    //分配解码器上下文
    m_pMicroDecode_CodecCtx = avcodec_alloc_context3(codec);
    if (!m_pMicroDecode_CodecCtx) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！分配解码器上下文失败");
        return false;
    }
    m_pMicroDecode_CodecCtx->sample_fmt = (AVSampleFormat)m_pMicroDecode_Ctx->streams[0]->codecpar->format;
    m_pMicroDecode_CodecCtx->sample_rate = m_pMicroDecode_Ctx->streams[0]->codecpar->sample_rate;
    m_pMicroDecode_CodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    m_pMicroDecode_CodecCtx->channels = 2;
    //初始化解码器上下文
    m_errNum = avcodec_open2(m_pMicroDecode_CodecCtx, codec, NULL);
    if (m_errNum < 0) {
        av_strerror(m_errNum, m_errBuf, 256);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！解码器上下文初始化失败:%s", LARSC::c2w(m_errBuf));
        return false;
    }
    //更新参数参数给设备上下文中的解码器
    m_errNum = avcodec_parameters_from_context(m_pMicroDecode_Ctx->streams[0]->codecpar, m_pMicroDecode_CodecCtx);
    if (m_errNum < 0) {
        av_strerror(m_errNum, m_errBuf, 256);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！分配参数给解码器上下文失败:%s", LARSC::c2w(m_errBuf));
        return false;
    }
    m_isInitSuccess = true;
    return m_isInitSuccess;
}

bool MicrophoneCapture::InitMicroResample()
{
    m_pMicroSampleRate_SwrCtx = swr_alloc();
    av_opt_set_int(m_pMicroSampleRate_SwrCtx, "in_channel_layout", m_pMicroDecode_Ctx->streams[0]->codecpar->channel_layout, 0);
    av_opt_set_int(m_pMicroSampleRate_SwrCtx, "out_channel_layout", m_pMicroDecode_Ctx->streams[0]->codecpar->channel_layout, 0);
    av_opt_set_int(m_pMicroSampleRate_SwrCtx, "in_sample_rate", m_pMicroDecode_Ctx->streams[0]->codecpar->sample_rate, 0);
    av_opt_set_int(m_pMicroSampleRate_SwrCtx, "out_sample_rate", (int)m_sampleRate, 0);
    av_opt_set_sample_fmt(m_pMicroSampleRate_SwrCtx, "in_sample_fmt",
        (AVSampleFormat)m_pMicroDecode_Ctx->streams[0]->codecpar->format, 0);
    av_opt_set_sample_fmt(m_pMicroSampleRate_SwrCtx, "out_sample_fmt",
        (AVSampleFormat)m_pMicroDecode_Ctx->streams[0]->codecpar->format, 0);
    m_errNum = swr_init(m_pMicroSampleRate_SwrCtx);
    return m_errNum < 0 ? false : true;
}

bool MicrophoneCapture::InitMicroResampleInner()
{
    m_pMicroFormat_SwrCtx = swr_alloc();
    av_opt_set_int(m_pMicroFormat_SwrCtx, "in_channel_layout", m_pMicroDecode_Ctx->streams[0]->codecpar->channel_layout, 0);
    av_opt_set_int(m_pMicroFormat_SwrCtx, "out_channel_layout", m_pMicroDecode_Ctx->streams[0]->codecpar->channel_layout, 0);
    av_opt_set_int(m_pMicroFormat_SwrCtx, "in_sample_rate", (int)m_sampleRate, 0);
    av_opt_set_int(m_pMicroFormat_SwrCtx, "out_sample_rate", (int)m_sampleRate, 0);
    av_opt_set_sample_fmt(m_pMicroFormat_SwrCtx, "in_sample_fmt",
        (AVSampleFormat)m_pMicroDecode_Ctx->streams[0]->codecpar->format, 0);
    av_opt_set_sample_fmt(m_pMicroFormat_SwrCtx, "out_sample_fmt",
        ((m_microFmt == MircroFmt::MP4)|| m_microFmt == MircroFmt::FLV) ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16, 0
    );
    m_errNum = swr_init(m_pMicroFormat_SwrCtx);
    return m_errNum < 0 ? false : true;
}

bool MicrophoneCapture::QueryFrame(void** microFrame, int* captureSize)
{
    //准备编码需要的帧结构
    av_frame_unref(m_pMicro_Frame);
    m_pMicro_Frame->sample_rate = (int)m_sampleRate;
    m_pMicro_Frame->nb_samples = ((m_microFmt == MircroFmt::MP4) || (m_microFmt == MircroFmt::FLV)) ? 1024 : 1012;
    m_pMicro_Frame->format = ((m_microFmt == MircroFmt::MP4) || (m_microFmt == MircroFmt::FLV)) ? 
        AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16;
    m_pMicro_Frame->channel_layout = m_pMicroDecode_CodecCtx->channel_layout;
    m_pMicro_Frame->channels = m_pMicroDecode_CodecCtx->channels;
    av_frame_get_buffer(m_pMicro_Frame, 0);
    int readAble = 0;
    //从最终重采样后音频缓冲区中拿取一帧麦克风音频数据
    while (true)
    {
        if (readAble = av_audio_fifo_size(m_pBuffer3_Fifo) >= m_pMicro_Frame->nb_samples) {
            {
                std::lock_guard<std::mutex> lock(m_Buffer3_Mutex);
                m_errNum = av_audio_fifo_read(
                    m_pBuffer3_Fifo,
                    (void**)m_pMicro_Frame->data,
                    m_pMicro_Frame->nb_samples
                );
            }
            if (m_errNum < 0) {
                av_strerror(m_errNum, m_errBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！拿取一帧麦克风编码所需帧失败:%s", LARSC::c2w(m_errBuf));
                return false;
            }
            *microFrame = m_pMicro_Frame;
            *captureSize = m_pMicro_Frame->nb_samples;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return true;
}

void MicrophoneCapture::DecodeMicroFrameThread()
{
    AVFrame* frame = nullptr;
    frame = av_frame_alloc();
    AVPacket packet = { 0 };
    av_init_packet(&packet);
    while (m_IsRecording.load()) {
        //从设备中读取一帧数据
        m_errNum = av_read_frame(m_pMicroDecode_Ctx, &packet);
        if (m_errNum < 0) {
            av_strerror(m_errNum, m_errBuf, 256);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！从麦克风中读取一帧数据失败:%s", LARSC::c2w(m_errBuf));
            continue;
        }
        m_errNum = avcodec_send_packet(m_pMicroDecode_CodecCtx, &packet);
        if (m_errNum < 0) {
            av_strerror(m_errNum, m_errBuf, 256);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！从麦克风中发送一帧数据到解码器失败:%s", LARSC::c2w(m_errBuf));
            av_packet_unref(&packet);
            continue;
        }
        else if (m_errNum == AVERROR(EAGAIN)) {

            continue;
        }
        m_errNum = avcodec_receive_frame(m_pMicroDecode_CodecCtx, frame);
        if (m_errNum < 0) {
            av_strerror(m_errNum, m_errBuf, 256);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！从麦克风中解码一帧数据失败:%s", LARSC::c2w(m_errBuf));
            av_packet_unref(&packet);
            continue;
        }
        //将读取的帧数据存入缓冲区中
        if (av_audio_fifo_space(m_pBuffer1_Fifo) > frame->nb_samples)
        {
            std::lock_guard<std::mutex> lock(m_Buffer1_Mutex);
            int writeSize = av_audio_fifo_write(m_pBuffer1_Fifo, (void**)frame->data, frame->nb_samples);
            if (writeSize <= 0) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"写入0数据到原始音频缓冲区");
            }
        }
        av_packet_unref(&packet);
        av_frame_unref(frame);
    }
    av_frame_free(&frame);
}

void MicrophoneCapture::MicroSampleRateThread()
{
    AVFrame* Origin_MircroFrame = av_frame_alloc();
    AVFrame* MircroFrame = av_frame_alloc();
    //循环读取原始麦克风音频缓冲区中的内容
    while (m_IsRecording.load())
    {
        //准备采样率转换后的缓冲区帧结构
        MircroFrame->sample_rate = (int)m_sampleRate;
        MircroFrame->format = m_pMicroDecode_CodecCtx->sample_fmt;
        Origin_MircroFrame->channel_layout = m_pMicroDecode_CodecCtx->channel_layout;
        Origin_MircroFrame->channels = m_pMicroDecode_CodecCtx->channels;

        //准备接受原始缓冲区的帧结构  
        Origin_MircroFrame->sample_rate = m_pMicroDecode_CodecCtx->sample_rate;
        Origin_MircroFrame->format = m_pMicroDecode_CodecCtx->sample_fmt;
        MircroFrame->channel_layout = m_pMicroDecode_CodecCtx->channel_layout;
        MircroFrame->channels = m_pMicroDecode_CodecCtx->channels;

        //动态的检查应该读取的数据大小
        int64_t availableSamples = av_audio_fifo_size(m_pBuffer1_Fifo);
        Origin_MircroFrame->nb_samples =
            availableSamples > 1024 ? 1024 : availableSamples;
        MircroFrame->nb_samples = av_rescale_q_rnd(
            Origin_MircroFrame->nb_samples,
            AVRational{ 1,Origin_MircroFrame->sample_rate },
            AVRational{ 1,(int)m_sampleRate },
            AV_ROUND_UP
        );
        av_frame_get_buffer(MircroFrame, 0);
        av_frame_get_buffer(Origin_MircroFrame, 0);
        //从原始麦克风音频数据中拿取数据
        if (Origin_MircroFrame->nb_samples != 0) {
            std::lock_guard<std::mutex> lock(m_Buffer1_Mutex);
            int readSize = av_audio_fifo_read(m_pBuffer1_Fifo, (void**)Origin_MircroFrame->data,
                Origin_MircroFrame->nb_samples);
            if (readSize <= 0) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"从原始音频缓冲区中读取了0数据");
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        //执行采样率转换
        int nb = swr_convert(
            m_pMicroSampleRate_SwrCtx, (uint8_t**)MircroFrame->data,
            MircroFrame->nb_samples,
            (const uint8_t**)Origin_MircroFrame->data,
            Origin_MircroFrame->nb_samples
        );
        if (nb < 0) {
            av_frame_unref(Origin_MircroFrame);
            av_frame_unref(MircroFrame);
            av_strerror(nb, m_errBuf, 256);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！执行音频采样率转换失败:%s", LARSC::c2w(m_errBuf));
            continue;
        }

        //将转换后的帧存入采样率转换后的缓冲区
        if (av_audio_fifo_space(m_pBuffer2_Fifo) > nb) {
            std::lock_guard<std::mutex> lock(m_Buffer2_Mutex);
            int writeSize = av_audio_fifo_write(m_pBuffer2_Fifo, (void**)MircroFrame->data, nb);
            if (writeSize == 0) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"写入了0数据到采样率转换后的音频缓冲区");
            }
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"缓冲区空间不足，无法存储最终采样的麦克风音频数据");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            av_frame_unref(Origin_MircroFrame);
            av_frame_unref(MircroFrame);
            continue;
        }
        av_frame_unref(Origin_MircroFrame);
        av_frame_unref(MircroFrame);
    }
    av_frame_free(&Origin_MircroFrame);
    av_frame_free(&MircroFrame);
}

void MicrophoneCapture::MircroFormatThread()
{
    AVFrame* frame = av_frame_alloc();//准备接受采样率转换后的帧缓冲区
    AVFrame* Ori_Frame = av_frame_alloc(); //准备接受格式转换后的帧缓冲区
    while (m_IsRecording.load())
    {
        //接受帧结构预设
        Ori_Frame->sample_rate = (int)m_sampleRate;
        Ori_Frame->format = m_pMicroDecode_CodecCtx->sample_fmt;
        Ori_Frame->channel_layout = m_pMicroDecode_CodecCtx->channel_layout;
        Ori_Frame->channels = m_pMicroDecode_CodecCtx->channels;

        //转换帧结构预设
        frame->sample_rate = (int)m_sampleRate;
        frame->format = ((m_microFmt == MircroFmt::MP4) || (m_microFmt == MircroFmt::FLV)) ? 
            AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16;
        frame->channel_layout = m_pMicroDecode_CodecCtx->channel_layout;
        frame->channels = m_pMicroDecode_CodecCtx->channels;

        //检查缓冲区中的数据总量，并选择性动态调整帧大小分配
        int64_t availbleSamples = av_audio_fifo_size(m_pBuffer2_Fifo);
        Ori_Frame->nb_samples = (availbleSamples > 1024 ? 1024 : availbleSamples);
        frame->nb_samples = Ori_Frame->nb_samples;//采样率一样（时间基一样），直接赋值
        av_frame_get_buffer(frame, 0);
        av_frame_get_buffer(Ori_Frame, 0);

        //从缓冲区中读取数据
        if (Ori_Frame->nb_samples != 0) {
            std::lock_guard<std::mutex> lock(m_Buffer2_Mutex);
            int readSize = av_audio_fifo_read(m_pBuffer2_Fifo, (void**)Ori_Frame->data, Ori_Frame->nb_samples);
            if (readSize == 0) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"从采样率转换后的音频缓冲区中读取了0数据");
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        //进行格式转换重采样
        int nb = swr_convert(
            m_pMicroFormat_SwrCtx,
            (uint8_t**)frame->data,
            frame->nb_samples,
            (const uint8_t**)Ori_Frame->data,
            Ori_Frame->nb_samples
        );
        if (nb < 0) {
            av_frame_unref(Ori_Frame);
            av_frame_unref(frame);
            av_strerror(nb, m_errBuf, 256);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"错误！执行音频采样率转换失败:%s", LARSC::c2w(m_errBuf));
            continue;
        }

        //将格式转换后的麦克风音频数据存入另一个缓冲区中
        if (av_audio_fifo_space(m_pBuffer3_Fifo) > nb) {
            std::lock_guard<std::mutex> lock(m_Buffer3_Mutex);
            int writeSize = av_audio_fifo_write(m_pBuffer3_Fifo, (void**)frame->data, nb);
            if (writeSize == 0) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"写入了0数据到最终重采样缓冲区");
            }
            av_frame_unref(Ori_Frame);
            av_frame_unref(frame);
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"缓冲区空间不足，无法存储最终采样的麦克风音频数据");
            av_frame_unref(Ori_Frame);
            av_frame_unref(frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
    }
}