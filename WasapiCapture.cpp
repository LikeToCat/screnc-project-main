#include "stdafx.h"
#include "WasapiCapture.h"
#include "CDebug.h"
#include "LarStringConversion.h"

static const PROPERTYKEY PKEY_Device_FriendlyName =
{ { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 14 };
static const PROPERTYKEY PKEY_Device_DeviceDesc =
{ { 0xa45c254e, 0xdf1c, 0x4efd, { 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 2 };

WasapiCapture* WasapiCapture::m_Interfance_Ins = nullptr;

WasapiCapture* WasapiCapture::GetInstance()
{
    if (!m_Interfance_Ins)
    {
        m_Interfance_Ins = new WasapiCapture;
    }
    return m_Interfance_Ins;
}

void WasapiCapture::ReleaseInstance()
{
    if (m_Interfance_Ins)
    {
        delete m_Interfance_Ins;
        m_Interfance_Ins = nullptr;
    }
}

WasapiCapture::WasapiCapture()
{
    m_AVFrame_buffer = av_frame_alloc();
}

WasapiCapture::~WasapiCapture()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 开始执行析构函数");
    StopCapture();
    CleanUp();
    m_Interfance_Ins = nullptr;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 析构函数执行完成");
}

bool WasapiCapture::Init(AudioSampleRate sampleRate, AudioFmt audioFmt, const std::wstring& deviceName)
{
    if (m_bool_WasapiInitialized.load())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用Init");
        return false;
    }
    m_Enum_SampleRate = sampleRate;
    m_Enum_format = audioFmt;


    if (!InitWASAPI(deviceName))//初始化wasapi
    {
        DB(ConsoleHandle, L"wasapi初始化wasapi失败");
        return false;
    }
    if (!InitSampleRateConvertCtx())//初始化采样率转换器
    {
        DB(ConsoleHandle, L"wasapi初始化采样率转换器");
        return false;
    }
    if (!InitFormatConvertCtx())//初始化格式转换器
    {
        DB(ConsoleHandle, L"wasapi初始化格式转换器");
        return false;
    }
    m_bool_WasapiInitialized.store(true);
    return true;
}

bool WasapiCapture::InitSampleRateConvertCtx()
{
    int iRet = 0;
    m_SwrCtx_SampleRate = swr_alloc();
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
    AVChannelLayout ch_layout;
    av_channel_layout_default(&ch_layout, m_Int_RawChannels);
    av_opt_set_chlayout(m_SwrCtx_SampleRate, "in_chlayout", &ch_layout, 0);
    av_opt_set_chlayout(m_SwrCtx_SampleRate, "out_chlayout", &ch_layout, 0);
    av_channel_layout_uninit(&ch_layout);
#else
    uint64_t ch_layout = av_get_default_channel_layout(m_Int_RawChannels);
    av_opt_set_int(m_SwrCtx_SampleRate, "in_channel_layout", ch_layout, 0);
    av_opt_set_int(m_SwrCtx_SampleRate, "out_channel_layout", ch_layout, 0);
#endif
    av_opt_set_int(m_SwrCtx_SampleRate, "in_sample_rate", m_Int_RawSampleRate, 0);
    av_opt_set_int(m_SwrCtx_SampleRate, "out_sample_rate", (int)m_Enum_SampleRate, 0);
    av_opt_set_sample_fmt(m_SwrCtx_SampleRate, "in_sample_fmt", (AVSampleFormat)m_Enum_oriFormat, 0);
    av_opt_set_sample_fmt(m_SwrCtx_SampleRate, "out_sample_fmt", (AVSampleFormat)m_Enum_oriFormat, 0);
    iRet = swr_init(m_SwrCtx_SampleRate);
    DEBUG_CONSOLE_FMT(ConsoleHandle,
        L"采样率转换器配置: in_fmt=%d, out_fmt=%d, in_rate=%d, out_rate=%d",
        (AVSampleFormat)m_Enum_oriFormat, (AVSampleFormat)m_Enum_oriFormat, (int)m_Int_RawSampleRate, (int)m_Enum_SampleRate);
    return iRet < 0 ? false : true;
}

bool WasapiCapture::InitFormatConvertCtx()
{
    int iRet = 0;
    m_SwrCtx_Format = swr_alloc();
    AVSampleFormat outformat =
        ((m_Enum_format == WasapiCapture::AudioFmt::MP4) || (m_Enum_format == WasapiCapture::AudioFmt::FLV)) ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16;
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
    AVChannelLayout ch_layout;
    av_channel_layout_default(&ch_layout, m_Int_RawChannels);
    av_opt_set_chlayout(m_SwrCtx_Format, "in_chlayout", &ch_layout, 0);
    av_opt_set_chlayout(m_SwrCtx_Format, "out_chlayout", &ch_layout, 0);
    av_channel_layout_uninit(&ch_layout);
#else
    uint64_t ch_layout = av_get_default_channel_layout(m_Int_RawChannels);
    av_opt_set_int(m_SwrCtx_Format, "in_channel_layout", ch_layout, 0);
    av_opt_set_int(m_SwrCtx_Format, "out_channel_layout", ch_layout, 0);
#endif
    av_opt_set_int(m_SwrCtx_Format, "in_sample_rate", (int)m_Enum_SampleRate, 0);
    av_opt_set_int(m_SwrCtx_Format, "out_sample_rate", (int)m_Enum_SampleRate, 0);
    av_opt_set_sample_fmt(m_SwrCtx_Format, "in_sample_fmt", (AVSampleFormat)m_Enum_oriFormat, 0);
    av_opt_set_sample_fmt(m_SwrCtx_Format, "out_sample_fmt", outformat, 0);
    iRet = swr_init(m_SwrCtx_Format);

    DEBUG_CONSOLE_FMT(ConsoleHandle,
        L"采样率转换器配置: in_fmt=%d, out_fmt=%d, in_rate=%d, out_rate=%d",
        (AVSampleFormat)m_Enum_oriFormat, outformat, (int)m_Enum_SampleRate, (int)m_Enum_SampleRate);
    return iRet < 0 ? false : true;
}

bool WasapiCapture::InitWASAPI(const std::wstring& deviceName)
{
    HRESULT hr;
    // 创建事件句柄
    m_hAudioEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hAudioEvent)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建事件句柄失败");
        return false;
    }

    // 以多线程（MTA）模式初始化 COM
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化 COM (MTA) 失败");
        return false;
    }
    // 进程级 COM 安全初始化（只需调用一次，忽略已“太晚”错误）
    hr = CoInitializeSecurity(
        nullptr, -1, nullptr, nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IDENTIFY,
        nullptr, EOAC_NONE, nullptr
    );
    if (FAILED(hr) && hr != RPC_E_TOO_LATE)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化 COM 安全失败");
        return false;
    }

    // 枚举并收集所有可用设备
    std::vector<CaptureDeviceInfo> devices;
    if (!getCaptureDevicesInfo(&devices) || devices.empty())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"枚举音频设备失败或无可用设备");
        CleanUp();
        return false;
    }

    // 按优先级准备尝试列表：用户指定设备 -> 默认设备 -> 其他设备
    std::vector<std::wstring> tryIds;
    // 用户指定
    if (!deviceName.empty())
    {
        for (auto& info : devices)
        {
            if (info.deviceName == deviceName)
            {
                tryIds.push_back(info.deviceId);
                break;
            }
        }
    }
    // 默认
    for (auto& info : devices)
    {
        if (info.isDefault &&
            (tryIds.empty() || tryIds[0] != info.deviceId))
        {
            tryIds.push_back(info.deviceId);
            break;
        }
    }
    // 其余设备
    for (auto& info : devices)
    {
        if (std::find(tryIds.begin(), tryIds.end(), info.deviceId) == tryIds.end())
        {
            tryIds.push_back(info.deviceId);
        }
    }

    // 创建设备枚举器
    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator
    );
    if (FAILED(hr))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建设备枚举器失败");
        CleanUp();
        return false;
    }

    // 依次尝试每个设备 ID
    bool success = false;
    for (auto& devId : tryIds)
    {
        IMMDevice* pDevice = nullptr;
        hr = pEnumerator->GetDevice(devId.c_str(), &pDevice);
        if (FAILED(hr))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"尝试获取设备失败，跳过");
            continue;
        }

        IAudioClient* pAudioClient = nullptr;
        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
            nullptr, (void**)&pAudioClient);
        if (FAILED(hr))
        {
            pDevice->Release();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"Activate IAudioClient 失败，尝试下一个设备");
            continue;
        }

        // 获取混音格式
        WAVEFORMATEX* pwfx = nullptr;
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr) || !SetCaptureCParam(pwfx))
        {
            CoTaskMemFree(pwfx);
            pAudioClient->Release();
            pDevice->Release();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"GetMixFormat 或 SetCaptureCParam 失败，尝试下一个设备");
            continue;
        }

        // 初始化音频客户端
        hr = pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            0, 0, pwfx, nullptr
        );
        if (FAILED(hr))
        {
            CoTaskMemFree(pwfx);
            pAudioClient->Release();
            pDevice->Release();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"Initialize AudioClient 失败，尝试下一个设备");
            continue;
        }

        // 设置事件句柄
        hr = pAudioClient->SetEventHandle(m_hAudioEvent);
        if (FAILED(hr))
        {
            CoTaskMemFree(pwfx);
            pAudioClient->Release();
            pDevice->Release();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"SetEventHandle 失败!，尝试下一个设备");
            continue;
        }

        // 获取捕获客户端
        IAudioCaptureClient* pCaptureClient = nullptr;
        hr = pAudioClient->GetService(
            __uuidof(IAudioCaptureClient),
            (void**)&pCaptureClient
        );
        if (FAILED(hr))
        {
            CoTaskMemFree(pwfx);
            pAudioClient->Release();
            pDevice->Release();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"GetService IAudioCaptureClient 失败!，尝试下一个设备");
            continue;
        }

        // 保存接口，结束循环
        m_Interface_IMMDeviceEnumerator = pEnumerator;
        m_Interface_IMMDevice = pDevice;
        m_Interface_IAudioClient = pAudioClient;
        m_Interface_IAudioCaptureClient = pCaptureClient;
        m_WaveFormatEx_CaptureFmt = pwfx;
        success = true;
        break;
    }

    if (!success)
    {
        CleanUp();
        DEBUG_CONSOLE_STR(ConsoleHandle, L"所有设备初始化失败");
        return false;
    }
    return true;
}

void WasapiCapture::CleanUp()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 开始执行资源清理");
    //ffmpeg资源释放
    if (m_AudioFifo_fifo1) {
        av_audio_fifo_free(m_AudioFifo_fifo1);
        m_AudioFifo_fifo1 = nullptr;
    }
    if (m_AudioFifo_fifo2) {
        av_audio_fifo_free(m_AudioFifo_fifo2);
        m_AudioFifo_fifo2 = nullptr;
    }
    if (m_SwrCtx_SampleRate) {
        swr_free(&m_SwrCtx_SampleRate);
    }
    if (m_SwrCtx_Format) {
        swr_free(&m_SwrCtx_Format);
    }
    if (m_AVFrame_buffer) {
        av_frame_free(&m_AVFrame_buffer);
    }

    //wasapi资源释放
    if (m_WaveFormatEx_CaptureFmt)
    {
        CoTaskMemFree(m_WaveFormatEx_CaptureFmt);
    }
    if (m_Interface_IAudioCaptureClient)
    {
        m_Interface_IAudioCaptureClient->Release();
    }
    if (m_Interface_IAudioClient)
    {
        m_Interface_IAudioClient->Stop();
        m_Interface_IAudioClient->Release();
    }
    if (m_Interface_IMMDevice)
    {
        m_Interface_IMMDevice->Release();
    }
    if (m_Interface_IMMDeviceEnumerator)
    {
        m_Interface_IMMDeviceEnumerator->Release();
    }
    if (m_bool_WasapiInitialized.load())
    {
        CoUninitialize();
    }

    //关闭事件句柄
    if (m_hAudioEvent)
    {
        CloseHandle(m_hAudioEvent);
        m_hAudioEvent = NULL;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 资源清理完成");
}

bool WasapiCapture::StartCapture()
{
    if (!m_bool_WasapiInitialized.load())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用StartCapture未经过初始化");
        return false;
    }
    //设置线程标志
    m_bool_IsCapturing.store(true);

    m_Thread_Capture = std::thread(&WasapiCapture::Capture, this);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"开启WasapiCapture接口捕获");

    m_Thread_SampleRateConvert = std::thread(&WasapiCapture::SampleRateConvert, this);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"开启WasapiCapture接口采样率转换线程");
    return true;
}

bool WasapiCapture::StopCapture()
{
    if (!m_bool_IsCapturing.load())
    {
        return true;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 开始停止捕获");
    m_bool_IsCapturing.store(false);

    // 通知所有等待重置的线程
    {
        std::lock_guard<std::mutex> resetLock(m_Mutex_Reset);
        m_bool_NeedReset.store(false);
        m_CV_ResetComplete.notify_all();
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 等待捕获线程结束");
    if (m_Thread_Capture.joinable())
    {
        m_Thread_Capture.join();
    }
    if (m_Thread_SampleRateConvert.joinable())
    {
        m_Thread_SampleRateConvert.join();
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 捕获线程已结束");
    return true;
}

void WasapiCapture::Capture()
{
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[调试] Capture线程开始执行，线程ID:%s ",
        std::to_wstring(GetCurrentThreadId()).c_str());
    // 开启音频捕获
    HRESULT hr = m_Interface_IAudioClient->Start();
    if (FAILED(hr)) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"启动音频捕获失败");
        return;
    }
    m_bool_IsCapturing.store(true);
    const int nb_Samples = (m_Enum_format == AudioFmt::MP4) ? 1024 : 1012;// 确定采样样本数

    if (!m_AudioFifo_fifo1)
    { // 创建音频FIFO缓冲区（如果还未创建） 
        m_AudioFifo_fifo1 = av_audio_fifo_alloc(
            (AVSampleFormat)m_Enum_oriFormat,
            m_Int_RawChannels,
            3000 * 1024  // 预分配多个块的空间
        );
    }

    //创建FFmpeg期望的数据指针
    uint8_t** frameData = new uint8_t * [m_Int_RawChannels];
    for (size_t i = 0; i < m_Int_RawChannels; i++)
    {
        frameData[i] = new uint8_t[nb_Samples * m_Int_bytePerSample];
    }

    BYTE* pData = nullptr;        //指向存储获取的音频帧内存的指针
    UINT32 frames = 0;            //获取的音频帧数量
    UINT32 Availableframes = 0;   //获取的音频帧数量
    DWORD flags = 0;              //接收标志
    bool firstDataAfterReset = false; // 标记是否为重置后的第一帧数据
    bool discontinuityHandled = false; // 标记是否已处理过数据不连续

    while (m_bool_IsCapturing.load())
    {
        DWORD Result = WaitForSingleObject(m_hAudioEvent, 100);
        if (Result == WAIT_OBJECT_0)
        {
            hr = m_Interface_IAudioCaptureClient->GetNextPacketSize(&frames);
            if (FAILED(hr))
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[WASAPI捕获线程]:获取音频数据包大小失败");
                continue;
            }
            if (frames > 0)//有可用数据
            {// 获取音频数据
                hr = m_Interface_IAudioCaptureClient->GetBuffer(&pData,
                    &Availableframes, &flags, nullptr, nullptr
                );//获取实际可用数据
                if (FAILED(hr))
                {
                    WCHAR errorMsg[256];
                    swprintf_s(errorMsg, L"[WASAPI捕获线程]:获取音频数据失败，错误码：0x%08X", hr);
                    DEBUG_CONSOLE_STR(ConsoleHandle, errorMsg);
                    break;
                }

                // 检查是否有数据不连续标志并且尚未处理
                if ((flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) && !discontinuityHandled)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[WASAPI捕获线程]:检测到音频数据不连续！");
                    discontinuityHandled = true; // 标记已处理
                    ResetAudioPipeline();        // 重置整个音频管线
                    firstDataAfterReset = true;  // 标记下一帧为重置后的第一帧

                    // 首先释放当前缓冲区，因为我们将重置整个客户端
                    m_Interface_IAudioCaptureClient->ReleaseBuffer(Availableframes);

                    // 稍微延迟以确保重置完成
                    Sleep(10);
                    continue; // 跳过当前帧，等待重置后的新数据
                }

                // 如果是重置后的第一帧有效数据，完成重置过程
                if (firstDataAfterReset && !(flags & AUDCLNT_BUFFERFLAGS_SILENT))
                {
                    CompleteAudioReset();
                    firstDataAfterReset = false;
                    discontinuityHandled = false; // 重置标志，允许处理未来的不连续性
                }

                //计算实际处理音频帧数
                UINT32 framesToProcess = min(Availableframes, nb_Samples);
                int64_t frameDuration = (1000000 * framesToProcess) / m_Int_RawSampleRate;//捕获的音频帧播放时的持续时间
                if (framesToProcess > 0)
                {
                    //将数据存储到FFmpeg期望的数据指针中
                    if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0)
                        memset(frameData[0], 0, framesToProcess * m_Int_RawChannels * m_Int_bytePerSample);
                    else
                        memcpy(frameData[0], pData, framesToProcess * m_Int_RawChannels * m_Int_bytePerSample);

                    // 只有当不在重置过程中才写入数据
                    if (!m_bool_IsInResetState.load())
                    {
                        int WriteSize;
                        { //写入一单位音频数据块到缓冲区
                            std::lock_guard<std::mutex> lock(m_Mutex_fifo1);
                            WriteSize = av_audio_fifo_write(m_AudioFifo_fifo1,
                                (void**)frameData, framesToProcess);
                        }
                        if (WriteSize < 0)
                        {
                            DEBUG_CONSOLE_STR(ConsoleHandle, L"[WASAPI捕获线程]:写入一帧数据到缓冲区失败");
                        }
                    }
                }
                m_Interface_IAudioCaptureClient->ReleaseBuffer(framesToProcess);
            }
        }
        else if (Result == WAIT_TIMEOUT)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[WasapiCapture捕获线程]:等待wasapi环形缓冲区填充超时，填入静音数据");
            continue;
        }
        else
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[WasapiCapture捕获线程]:等待wasapi环形缓冲区填充失败");
            continue;
        }
    }

    // 清理和停止
    for (size_t i = 0; i < m_Int_RawChannels; i++)
    {
        delete[] frameData[i];
    }
    delete[] frameData;
    m_Interface_IAudioCaptureClient->GetNextPacketSize(&frames);
    m_Interface_IAudioCaptureClient->ReleaseBuffer(frames);
    m_Interface_IAudioClient->Stop();
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[调试] Capture线程即将退出，线程ID:%s ",
        std::to_wstring(GetCurrentThreadId()).c_str());
}

void WasapiCapture::SampleRateConvert()
{
    // 确定采样块大小
    const int blockSize = (((m_Enum_format == AudioFmt::MP4) || m_Enum_format == AudioFmt::FLV)) ? 1024 : 1012;
    if (!m_AudioFifo_fifo2)
    { // 创建第二个音频FIFO缓冲区（如果还未创建）
        m_AudioFifo_fifo2 = av_audio_fifo_alloc(
            (AVSampleFormat)m_Enum_oriFormat,
            m_Int_RawChannels,
            3000 * 1024  // 预分配多个块的空间
        );
    }

    //分配转换前后的帧结构
    AVFrame* oriFrame = av_frame_alloc();
    AVFrame* dstFrame = av_frame_alloc();

#define UnrefFrameBuffer(oriFrame,dstFrame)\
    av_frame_unref(oriFrame);\
    av_frame_unref(dstFrame);

    //开始音频重采样
    m_bool_IsCapturing.store(true);
    while (m_bool_IsCapturing.load())
    {
        // 检查是否需要等待重置完成
        if (m_bool_NeedReset.load())
        {
            // 等待重置完成
            std::unique_lock<std::mutex> resetLock(m_Mutex_Reset);
            m_CV_ResetComplete.wait(resetLock, [this]() {
                return !m_bool_NeedReset.load() || !m_bool_IsCapturing.load();
                });

            // 如果捕获已停止，退出循环
            if (!m_bool_IsCapturing.load())
            {
                break;
            }

            DEBUG_CONSOLE_STR(ConsoleHandle, L"[采样率转换线程]: 重置完成，恢复处理");
        }

        Sleep(10);
        {
            std::lock_guard<std::mutex> lock1(m_Mutex_fifo1);
            if (av_audio_fifo_size(m_AudioFifo_fifo1) < blockSize)
            { // 从fifo1读取数据
                continue;
            }
        }

        //创建转换前的帧缓冲区
        uint64_t ch_layout = av_get_default_channel_layout(m_Int_RawChannels);
        oriFrame->nb_samples = blockSize;
        oriFrame->sample_rate = m_Int_RawSampleRate;
        oriFrame->format = (AVSampleFormat)m_Enum_oriFormat;
        oriFrame->channel_layout = ch_layout;
        oriFrame->channels = m_Int_RawChannels;
        if (av_frame_get_buffer(oriFrame, 32) < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] SampleRateConvert函数，无法分配oriFrame帧缓冲区内存");
            continue;
        }

        //创建转换后的帧缓冲区
        dstFrame->sample_rate = (int)m_Enum_SampleRate;
        dstFrame->format = (AVSampleFormat)m_Enum_oriFormat;
        dstFrame->channel_layout = ch_layout;
        dstFrame->channels = m_Int_RawChannels;
        dstFrame->nb_samples = av_rescale_rnd(
            oriFrame->nb_samples,
            dstFrame->sample_rate,
            oriFrame->sample_rate,
            AV_ROUND_UP
        );
        if (av_frame_get_buffer(dstFrame, 32) < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] SampleRateConvert函数，无法分配dstFrame帧缓冲区内存");
        }

        //判断是否可以从原始帧缓冲区中数据读取一帧数据
        if (av_audio_fifo_size(m_AudioFifo_fifo1) < oriFrame->nb_samples)
        {//数据不足，循环到下一次捕获
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试]原始帧缓冲区数据不足，循环到下一次捕获");
            UnrefFrameBuffer(oriFrame, dstFrame);
            continue;
        }

        //从原始帧缓冲区中读取一帧数据
        int readSize;
        {
            std::lock_guard<std::mutex> lock2(m_Mutex_fifo1);
            readSize = av_audio_fifo_read(m_AudioFifo_fifo1, (void**)oriFrame->data, blockSize);
        }
        if (readSize < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[采样率转换线程]:读取原始帧缓冲区失败");
            UnrefFrameBuffer(oriFrame, dstFrame);
            continue;
        }

        // 执行采样率转换
        dstFrame->nb_samples = swr_convert(
            m_SwrCtx_SampleRate,
            &(dstFrame->data[0]), dstFrame->nb_samples,
            (const uint8_t**)oriFrame->data, oriFrame->nb_samples
        );
        if (dstFrame->nb_samples < 0) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[采样率转换线程]: 转换失败");
            av_frame_unref(oriFrame);
            continue;
        }

        //判断存储采样率转换后的缓冲区是否有足够的空间
        if (av_audio_fifo_space(m_AudioFifo_fifo2) < dstFrame->nb_samples)
        {//没有足够的空间可以存放数据，丢弃当前帧
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试]存储采样率转换后的缓冲区空间不足，丢弃了一帧音频数据");
            UnrefFrameBuffer(oriFrame, dstFrame);
            continue;
        }

        // 写入到存储采样率转换后的缓冲区
        int writeSize;
        {
            std::lock_guard<std::mutex> lock2(m_Mutex_fifo2);
            writeSize = av_audio_fifo_write(m_AudioFifo_fifo2, (void**)dstFrame->data, dstFrame->nb_samples);
            // 移除条件变量通知，改为直接轮询
        }
        if (writeSize < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[采样率转换线程]: 写入采样率转换后的缓冲区失败");
            UnrefFrameBuffer(oriFrame, dstFrame);
            continue;
        }
        if (!m_bool_IsCapturing.load()) break;
        UnrefFrameBuffer(oriFrame, dstFrame);
    }
    if (oriFrame) av_frame_free(&oriFrame);
    if (dstFrame) av_frame_free(&dstFrame);
    return;
}

bool WasapiCapture::QueryFrame(void** frameData, int* frameSize)
{
    if (!m_bool_IsCapturing.load() || !m_bool_WasapiInitialized.load())
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"非法调用QueryFrame,状态:是否开启了捕获:%s，WASAPI是否正确初始化:%s",
            m_bool_IsCapturing ? L"开启了捕获" : L"未开启捕获", m_bool_WasapiInitialized ? L"正确初始化" : L"未正确初始化");
        return false;
    }

    // 确定所需采样数量
    int requiredSamples =
        (((m_Enum_format == WasapiCapture::AudioFmt::MP4) || (m_Enum_format == WasapiCapture::AudioFmt::FLV)) ?
            1024 : 1012);

    // 检查缓冲区是否有足够的数据
    bool hasEnoughData = false;
    {
        std::lock_guard<std::mutex> lock(m_Mutex_fifo2);
        hasEnoughData = (av_audio_fifo_size(m_AudioFifo_fifo2) >= requiredSamples);
    }

    // 准备输出帧
    av_frame_unref(m_AVFrame_buffer);
    m_AVFrame_buffer->nb_samples = requiredSamples;
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
    AVChannelLayout ch_layout;
    av_channel_layout_default(&ch_layout, m_Int_RawChannels);
    av_channel_layout_copy(&m_AVFrame_buffer->ch_layout, &ch_layout);
#else
    m_AVFrame_buffer->channel_layout = av_get_default_channel_layout(m_Int_RawChannels);
    m_AVFrame_buffer->channels = m_Int_RawChannels;
#endif
    m_AVFrame_buffer->sample_rate = (int)m_Enum_SampleRate;
    m_AVFrame_buffer->format =
        (((m_Enum_format == WasapiCapture::AudioFmt::MP4) || (m_Enum_format == WasapiCapture::AudioFmt::FLV))
            ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16);

    if (av_frame_get_buffer(m_AVFrame_buffer, 32) < 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] QueryFrame函数,无法分配m_AVFrame_buffer帧缓冲区内存");
        return false;
    }

    if (hasEnoughData)
    {
        // 处理真实音频数据
        AVFrame* oriFrame = av_frame_alloc();
        if (!oriFrame)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"无法分配原始帧内存");
            return false;
        }

#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
        av_channel_layout_copy(&oriFrame->ch_layout, &ch_layout);
#else
        oriFrame->channel_layout = av_get_default_channel_layout(m_Int_RawChannels);
        oriFrame->channels = m_Int_RawChannels;
#endif
        oriFrame->nb_samples = requiredSamples;
        oriFrame->sample_rate = (int)m_Enum_SampleRate;
        oriFrame->format = (AVSampleFormat)m_Enum_oriFormat;

        if (av_frame_get_buffer(oriFrame, 32) < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] QueryFrame函数,无法分配oriFrame帧缓冲区内存");
            av_frame_free(&oriFrame);
            return false;
        }

        int readSize;
        {
            std::lock_guard<std::mutex> lock(m_Mutex_fifo2);
            readSize = av_audio_fifo_read(m_AudioFifo_fifo2, (void**)oriFrame->data, oriFrame->nb_samples);
        }

        if (readSize < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"读取存储采样率转换后的缓冲区失败");
            av_frame_free(&oriFrame);
            return false;
        }

        // 将读取到的数据进行格式转换
        int nb;
        if (m_AVFrame_buffer->format == AV_SAMPLE_FMT_FLTP)
        {
            nb = swr_convert(
                m_SwrCtx_Format,
                m_AVFrame_buffer->data, m_AVFrame_buffer->nb_samples,
                (const uint8_t**)oriFrame->data, oriFrame->nb_samples
            );
        }
        else if (m_AVFrame_buffer->format == AV_SAMPLE_FMT_S16)
        {
            nb = swr_convert(
                m_SwrCtx_Format,
                &(m_AVFrame_buffer->data[0]), m_AVFrame_buffer->nb_samples,
                (const uint8_t**)oriFrame->data, oriFrame->nb_samples
            );
        }

        if (nb < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"格式转换失败");
            av_frame_free(&oriFrame);
            return false;
        }

        av_frame_free(&oriFrame);
    }
    else
    {
        // 生成静音帧
        if (m_AVFrame_buffer->format == AV_SAMPLE_FMT_FLTP)
        {
            // FLTP格式 (MP4/FLV) - 为每个通道平面填充0.0f
            for (int ch = 0; ch < m_Int_RawChannels; ch++)
            {
                float* buffer = (float*)m_AVFrame_buffer->data[ch];
                memset(buffer, 0, requiredSamples * sizeof(float));
            }
        }
        else if (m_AVFrame_buffer->format == AV_SAMPLE_FMT_S16)
        {
            // S16格式 (AVI) - 填充交错的0值
            int16_t* buffer = (int16_t*)m_AVFrame_buffer->data[0];
            memset(buffer, 0, requiredSamples * m_Int_RawChannels * sizeof(int16_t));
        }
    }

    // 设置返回值
    *frameData = m_AVFrame_buffer;
    *frameSize = m_AVFrame_buffer->nb_samples;
    return true;
}

bool WasapiCapture::SetCaptureCParam(WAVEFORMATEX* pwfx)
{
    // 确定捕获帧的格式
    if (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        m_Enum_oriFormat = (WasapiCapture::OriAudioFmt)AV_SAMPLE_FMT_FLT;
    }
    else if (pwfx->wFormatTag == WAVE_FORMAT_PCM)
    {
        if (pwfx->wBitsPerSample == 16)
        {
            m_Enum_oriFormat = (WasapiCapture::OriAudioFmt)AV_SAMPLE_FMT_S16;
        }
        else if (pwfx->wBitsPerSample == 32)
        {
            m_Enum_oriFormat = (WasapiCapture::OriAudioFmt)AV_SAMPLE_FMT_S32;
        }
        else
        {
            CleanUp();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"不支持的音频位深");
            return false;
        }
    }
    else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        WAVEFORMATEXTENSIBLE* pwfxext = (WAVEFORMATEXTENSIBLE*)pwfx;
        if (IsEqualGUID(pwfxext->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
        {
            m_Enum_oriFormat = (WasapiCapture::OriAudioFmt)AV_SAMPLE_FMT_FLT;
        }
        else if (IsEqualGUID(pwfxext->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))
        {
            if (pwfx->wBitsPerSample == 16)
            {
                m_Enum_oriFormat = (WasapiCapture::OriAudioFmt)AV_SAMPLE_FMT_S16;
            }
            else if (pwfx->wBitsPerSample == 32)
            {
                m_Enum_oriFormat = (WasapiCapture::OriAudioFmt)AV_SAMPLE_FMT_S32;
            }
            else
            {
                CleanUp();
                return false;
            }
        }
        else
        {
            CleanUp();
            return false;
        }
    }
    else
    {
        CleanUp();
        DEBUG_CONSOLE_STR(ConsoleHandle, L"不支持的音频格式,waiapi初始化失败");
        return false;
    }

    //确认捕获帧的每个样本所占字节
    switch (m_Enum_oriFormat) {
    case OriAudioFmt::AV_SAMPLE_FMT_S16:
        m_Int_bytePerSample = 2;
        break;
    case OriAudioFmt::AV_SAMPLE_FMT_S32:
    case OriAudioFmt::AV_SAMPLE_FMT_FLT:
        m_Int_bytePerSample = 4;
        break;
    default:
        m_Int_bytePerSample = 2; // 默认值
    }

    m_Int_RawSampleRate = pwfx->nSamplesPerSec;
    m_Int_RawChannels = pwfx->nChannels;
    return true;
}

bool WasapiCapture::getCaptureDevicesInfo(_Out_ std::vector<CaptureDeviceInfo>* vecDevicesInfo)
{
    if (!vecDevicesInfo)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] getCaptureDevicesInfo: 参数无效");
        return false;
    }
    vecDevicesInfo->clear();
    if (!m_Vec_AudioDevices.empty())
    {// 如果已经枚举过设备，直接返回缓存的结果
        *vecDevicesInfo = m_Vec_AudioDevices;
        return true;
    }

    // 初始化COM库(如果已初始化则忽略)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool needUninit = SUCCEEDED(hr) && hr != S_FALSE;

    // 创建设备枚举器
    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建设备枚举器失败");
        if (needUninit) CoUninitialize();
        return false;
    }

    // 获取默认设备ID
    IMMDevice* pDefaultDevice = NULL;
    std::wstring defaultDeviceId;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDefaultDevice);
    if (SUCCEEDED(hr))
    {
        LPWSTR deviceId = NULL;
        hr = pDefaultDevice->GetId(&deviceId);
        if (SUCCEEDED(hr))
        {
            defaultDeviceId = deviceId;
            CoTaskMemFree(deviceId);
        }
        pDefaultDevice->Release();
    }

    // 枚举所有渲染设备(用于系统音频捕获)
    IMMDeviceCollection* pDevices = NULL;
    hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevices);
    if (FAILED(hr))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"枚举音频设备失败");
        pEnumerator->Release();
        if (needUninit) CoUninitialize();
        return false;
    }

    // 获取设备数量
    UINT deviceCount;
    hr = pDevices->GetCount(&deviceCount);
    if (FAILED(hr))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"获取设备数量失败");
        pDevices->Release();
        pEnumerator->Release();
        if (needUninit) CoUninitialize();
        return false;
    }

    // 遍历所有设备
    for (UINT i = 0; i < deviceCount; i++)
    {
        IMMDevice* pDevice = NULL;
        hr = pDevices->Item(i, &pDevice);
        if (SUCCEEDED(hr))
        {
            CaptureDeviceInfo deviceInfo;

            // 获取设备ID
            LPWSTR deviceId = NULL;
            hr = pDevice->GetId(&deviceId);
            if (SUCCEEDED(hr))
            {
                deviceInfo.deviceId = deviceId;
                deviceInfo.isDefault = (deviceInfo.deviceId == defaultDeviceId);
                CoTaskMemFree(deviceId);

                // 获取设备属性
                IPropertyStore* pProps = NULL;
                hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
                if (SUCCEEDED(hr))
                {
                    // 设备友好名称
                    PROPVARIANT varName;
                    PropVariantInit(&varName);
                    hr = pProps->GetValue(reinterpret_cast<const PROPERTYKEY&>(PKEY_Device_FriendlyName), &varName);
                    if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR)
                    {
                        deviceInfo.deviceName = varName.pwszVal;
                    }
                    PropVariantClear(&varName);

                    // 设备描述
                    PROPVARIANT varDesc;
                    PropVariantInit(&varDesc);
                    hr = pProps->GetValue(reinterpret_cast<const PROPERTYKEY&>(PKEY_Device_DeviceDesc), &varDesc);
                    if (SUCCEEDED(hr) && varDesc.vt == VT_LPWSTR)
                    {
                        deviceInfo.deviceDesc = varDesc.pwszVal;
                    }
                    PropVariantClear(&varDesc);
                    pProps->Release();
                }

                DEBUG_CONSOLE_FMT(ConsoleHandle, L"枚举到音频设备:%s\n设备描述符:%s",
                    deviceInfo.deviceName.c_str(), deviceInfo.deviceDesc.c_str());
                vecDevicesInfo->push_back(deviceInfo);
                m_Vec_AudioDevices.push_back(deviceInfo);
            }
            pDevice->Release();
        }
    }

    // 释放资源
    pDevices->Release();
    pEnumerator->Release();
    if (needUninit)
    {
        CoUninitialize();
    }

    return !vecDevicesInfo->empty();
}

void WasapiCapture::ResetAudioPipeline()
{
    if (m_bool_IsInResetState.load())
    {
        // 如果当前已经重置，等待重新初始化，避免重复操作
        return;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 检测到音频流不连续，开始完全重置音频处理管线");

    if (m_Interface_IAudioClient)
    {
        m_Interface_IAudioClient->Stop();
    }
    {
        std::lock_guard<std::mutex> lock1(m_Mutex_fifo1);
        if (m_AudioFifo_fifo1)
        {
            av_audio_fifo_reset(m_AudioFifo_fifo1);
        }
    }
    {
        std::lock_guard<std::mutex> lock2(m_Mutex_fifo2);
        if (m_AudioFifo_fifo2)
        {
            av_audio_fifo_reset(m_AudioFifo_fifo2);
        }
    }
    // 释放旧的音频客户端和捕获客户端
    if (m_Interface_IAudioCaptureClient)
    {
        m_Interface_IAudioCaptureClient->Release();
        m_Interface_IAudioCaptureClient = NULL;
    }

    if (m_Interface_IAudioClient)
    {
        m_Interface_IAudioClient->Release();
        m_Interface_IAudioClient = NULL;
    }

    // 释放并重新初始化FFmpeg资源
    if (m_SwrCtx_SampleRate)
    {
        swr_free(&m_SwrCtx_SampleRate);
    }

    if (m_SwrCtx_Format)
    {
        swr_free(&m_SwrCtx_Format);
    }

    // 设置标志位，通知SampleRateConvert函数暂停执行
    m_bool_NeedReset.store(true);

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 音频处理管线已停止，准备重新初始化");

    // 重新初始化WASAPI
    if (m_Interface_IMMDevice)
    {
        // 重新初始化音频客户端
        IAudioClient* pAudioClient = NULL;
        HRESULT hr = m_Interface_IMMDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
            NULL, (void**)&pAudioClient);
        if (SUCCEEDED(hr))
        {
            m_Interface_IAudioClient = pAudioClient;

            // 重新初始化音频客户端
            hr = m_Interface_IAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                0, 0, m_WaveFormatEx_CaptureFmt, NULL);

            if (SUCCEEDED(hr))
            {
                // 设置事件句柄
                hr = m_Interface_IAudioClient->SetEventHandle(m_hAudioEvent);

                // 获取捕获客户端
                if (SUCCEEDED(hr))
                {
                    IAudioCaptureClient* pCaptureClient = NULL;
                    hr = m_Interface_IAudioClient->GetService(
                        __uuidof(IAudioCaptureClient),
                        (void**)&pCaptureClient);

                    if (SUCCEEDED(hr))
                    {
                        m_Interface_IAudioCaptureClient = pCaptureClient;

                        // 重新初始化采样率转换器和格式转换器
                        InitSampleRateConvertCtx();
                        InitFormatConvertCtx();

                        // 启动音频客户端
                        hr = m_Interface_IAudioClient->Start();
                        if (SUCCEEDED(hr))
                        {
                            DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 音频客户端重新初始化并启动成功");
                            m_bool_IsInResetState.store(true);
                        }
                        else
                        {
                            DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] 重新启动音频客户端失败");
                        }
                    }
                    else
                    {
                        DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] 重新获取捕获客户端失败");
                    }
                }
                else
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] 重新设置事件句柄失败");
                }
            }
            else
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] 重新初始化音频客户端失败");
            }
        }
        else
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误] 重新激活音频设备失败");
        }
    }
}

void WasapiCapture::CompleteAudioReset()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 收到新音频数据，完成音频处理管线重置");

    // 重新初始化采样率转换器
    if (m_SwrCtx_SampleRate)
    {
        swr_init(m_SwrCtx_SampleRate);
    }

    // 重新初始化格式转换器
    if (m_SwrCtx_Format)
    {
        swr_init(m_SwrCtx_Format);
    }

    // 重置标志位
    m_bool_NeedReset.store(false);
    m_bool_IsInResetState.store(false);

    // 通知所有等待的线程
    m_CV_ResetComplete.notify_all();

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[调试] 音频处理管线重置完全结束，恢复正常处理");
}