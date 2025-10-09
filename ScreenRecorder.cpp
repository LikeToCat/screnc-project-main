#include "stdafx.h"
#include "CDebug.h"
#include "ScreenRecorder.h"
#include "LarStringConversion.h"
#include "CompatibleManager.h"
#include "WindowHandleManager.h"
#include "theApp.h"
#include "GlobalFunc.h"

#define CompatibleManagerIns CompatibleManager::GetInstance()

extern HANDLE ConsoleHandle;
ScreenRecorder* ScreenRecorder::instance = nullptr;
bool ScreenRecorder::m_InsIsRecording = false;
std::atomic<bool> ScreenRecorder::s_isPreheating{ false };
std::atomic<bool> ScreenRecorder::s_abortPreheat{ false };
std::atomic<bool> ScreenRecorder::s_isInsCreateByPreheat{ false };
ScreenRecorder* ScreenRecorder::GetInstance()
{
    //如果预热还未完成
    if (s_isPreheating.load(std::memory_order_acquire) && !s_isInsCreateByPreheat)
    {
        s_abortPreheat.store(true, std::memory_order_release); // 发出中止信号
        ReleaseInstance();
        for (int i = 0; i < 200; ++i)
        {
            if (!s_isPreheating.load(std::memory_order_acquire))
                break;
            Sleep(5);
        }
    }
    if (!instance) 
    {
        instance = new ScreenRecorder;
    }
    return instance;
}

void ScreenRecorder::ReleaseInstance()
{
    if (instance)
    {
        s_abortPreheat.store(true, std::memory_order_release);
        if (!instance->stopRecording())
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"停止录制失败");
        }
        delete instance;
        instance = nullptr;
    }
}

void ScreenRecorder::Preheat()
{
    // 已在预热或已收到中止指令则直接返回
    bool expected = false;
    if (!s_isPreheating.compare_exchange_strong(expected, true))
        return;
    s_abortPreheat.store(false, std::memory_order_release);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[预热] 开始假录制预热...");

    //获取实例（此时不会触发上面 GetInstance 的打断逻辑）
    s_isInsCreateByPreheat.store(true);
    ScreenRecorder* rec = GetInstance();
    if (!rec)
    {
        s_isPreheating.store(false);
        return;
    }

    // 设置最小代价参数
    rec->SetScreenRecordParam(
        ScreenRecorder::Rs_360P,
        ScreenRecorder::StandardDefinition,
        ScreenRecorder::MP4,
        ScreenRecorder::EncodingPreset::Fast,
        ScreenRecorder::RecordMode::None,
        AudioSampleRate::Hz_44100,
        AudioBitRate::Kbps_64,
        1);
    rec->SetRecordMouse(false);
    rec->SetVideoEncoder("");
    rec->SetSystemAudioVolume(0.0f);
    rec->SetMicroVolume(0.0f);

    //构建一个临时文件路径
    wchar_t tempPath[MAX_PATH]{ 0 };
    wchar_t tempFile[MAX_PATH]{ 0 };
    GetTempPathW(MAX_PATH, tempPath);
    GetTempFileNameW(tempPath, L"SRP", 0, tempFile);
    {
        wchar_t renamed[MAX_PATH];
        lstrcpynW(renamed, tempFile, MAX_PATH);
        wchar_t* dot = wcsrchr(renamed, L'.');
        if (dot) wcscpy_s(dot, MAX_PATH - (dot - renamed), L".mp4");
        MoveFileW(tempFile, renamed);
        lstrcpynW(tempFile, renamed, MAX_PATH);
    }
    std::string tempFileUtf8 = GlobalFunc::ConvertToUtf8(tempFile);
    bool started = rec->startRecording(tempFileUtf8.c_str());
    if (!started)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[预热] 启动假录制失败(可能无需预热或编码器缺失)，直接结束。");
        rec->stopRecording();
        ReleaseInstance();
        s_isPreheating.store(false);
        DeleteFileW(tempFile);
        return;
    }

    // 简短等待500ms，等待完成后释放实例
    const int totalWaitMs = 500;
    int elapsed = 0;
    while (elapsed < totalWaitMs && !s_abortPreheat.load(std::memory_order_acquire))
    {
        Sleep(50);
        elapsed += 50;
    }
    rec->stopRecording(); 
    DeleteFileW(tempFile);
    DEBUG_CONSOLE_STR(ConsoleHandle, s_abortPreheat.load()
        ? L"[预热] 被外部请求打断，提前结束。"
        : L"[预热] 完成。");
    s_isPreheating.store(false, std::memory_order_release);
    ReleaseInstance();
}

ScreenRecorder::ScreenRecorder()
{
    m_bool_isAudioInitSuccess = false;
    m_bool_isMicroInitSuccess = false;
    m_bool_isScreenCaptureSuccess = false;
    m_bool_isGdiCaptureSuccess = false;
}

ScreenRecorder::~ScreenRecorder()
{
    if (!stopRecording())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"stopRecording资源释放失败");
    }
    CleanCaptureInterface();//清理捕获器资源 
    if (errorBuf) {
        delete[] errorBuf;
        errorBuf = nullptr;
    }
    m_Map_RecordCallBackFunc.clear();
    std::map<RecordCallBack, std::function<void()>>().swap(m_Map_RecordCallBackFunc);
}
//123312312555556
void ScreenRecorder::SetWindowRecordParam(HWND hWnd, ResolutionRatio resolutionRatio, VideoQuality videoQuality, VideoFormat videoFmt, EncodingPreset EncodingPreset, RecordMode audioCaptureMode, AudioSampleRate audioSampleRate, AudioBitRate audioBitrate, int frameRate)
{
    if (m_Bool_AreaRecord || m_Bool_RestartRecord || m_Bool_ScreenRecord)
    {//如果某一个标志为真，则提示非法调用
        DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用SetWindowRecordParam");
        return;
    }

    m_IsRecording = false;
    m_frameRate = frameRate;
    m_isInit = false;
    m_frameCount = 0;
    m_formatCtx = nullptr;
    m_CodecCtx = nullptr;
    m_VideosStream = nullptr;
    m_frame = nullptr;
    m_swsContext = nullptr;
    m_frameSize = 0;
    errorBuf = nullptr;
    m_VideoFmt = videoFmt;
    m_VideoQulity = videoQuality;
    m_Rs = resolutionRatio;
    m_EncodingPreset = EncodingPreset;
    m_AudioSampleRate = audioSampleRate;
    m_AudioBitRate = audioBitrate;
    m_RecordMode = audioCaptureMode;
    m_gdiCapture = nullptr;
    m_audioCapture = nullptr;
    m_microCapture = nullptr;
#ifdef TARGET_WIN10
    m_dxgiCapture = nullptr;
#elif TARGET_WIN7
    m_ScreenCapture = nullptr;
#endif 
    m_Hwnd = hWnd;//要录制的应用程序句柄

    //获取应用程序窗口的大小
    CRect WindowRect;
    if (::IsIconic(m_Hwnd))// 对于最小化窗口，获取其原始尺寸
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if (::GetWindowPlacement(m_Hwnd, &wp))
        {
            // rcNormalPosition包含窗口在正常状态下的位置和尺寸
            RECT rcNormal = wp.rcNormalPosition;
            m_width = rcNormal.right - rcNormal.left;
            m_height = rcNormal.bottom - rcNormal.top;
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取最小化窗口 \"%s\" 的原始分辨率: %d x %d",
                WindowHandleManager::GetWindowTitle(m_Hwnd).c_str(), m_width, m_height);
        }
    }
    else
    {
        GetWindowRect(m_Hwnd, WindowRect);
        m_width = WindowRect.Width();
        m_height = WindowRect.Height();
    }
    // 确保宽度和高度是偶数
    bool isAdjust = false;
    if (m_width % 2 != 0)
    {
        m_width--;
        isAdjust = true;
    }
    if (m_height % 2 != 0)
    {
        m_height--;
        isAdjust = true;
    }
    if (isAdjust)
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"发现奇数尺寸，调整录制尺寸为: %dx%d（确保兼容编码器）", m_width, m_height);

    //自动估算需要的码率
    m_bitRate = GetVideoQualityMbps(resolutionRatio, videoQuality);
    errorBuf = new char[256];
    memset(errorBuf, 0, 256);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"视频录制对象初始化完成:\n实际分辨率:%dx%d\n实际比特率:%.1f == %.1fMbps\n目标帧率:%dFPS\n目标格式:%S\n声音与麦克风:%s\n",
        //录制基本参数打印
        m_width, m_height, m_bitRate, m_bitRate / (double)1000000, m_frameRate,
        //目标格式打印
        m_VideoFmt == MP4 ? "mp4" :
        (m_VideoFmt == AVI ? "avi" :
            (m_VideoFmt == FLV ? "flv" : "unknow format")),
        //额外选项打印
        audioCaptureMode == RecordMode::SystemSound ? L"录制声音" :
        (audioCaptureMode == RecordMode::Microphone ? L"录制麦克风" :
            (audioCaptureMode == RecordMode::Both ? L"录制声音和麦克风" : L"录制静音视频"))
    );

    //创建GDI捕获器
    if (!m_gdiCapture)
    {
        m_gdiCapture = new HandleCapture(m_Hwnd, m_frameRate);
        if (!m_gdiCapture) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建GDI捕获器失败");
        }
    }

    //创建FFmpeg麦克风音频捕获器
    if (!m_microCapture)
    {
        m_microCapture = (
            ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
            new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                (MircroFmt)m_VideoFmt) :
            nullptr
            );
    }

    //重置所有录制标志并设置某一录制标志
    m_Bool_AreaRecord = false;
    m_Bool_WindowRecord = true;
    m_Bool_ScreenRecord = false;
}

void ScreenRecorder::SetAreaRecordParam(int left, int top, int right, int bottom, ResolutionRatio resolutionRatio, VideoQuality videoQuality, VideoFormat videoFmt, EncodingPreset EncodingPreset, RecordMode audioCaptureMode, AudioSampleRate audioSampleRate, AudioBitRate audioBitrate, int frameRate)
{
    if (m_Bool_AreaRecord || m_Bool_RestartRecord || m_Bool_ScreenRecord)
    {//如果某一个标志为真，则提示非法调用
        DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用SetAreaRecordParam");
        return;
    }
    DBFMT(ConsoleHandle, L"区域录制录制传递的参数:left%d ,top%d, right%d, bottom%d",
        left, top, right, bottom);

    m_IsRecording = false;
    m_frameRate = frameRate;
    m_isInit = false;
    m_frameCount = 0;
    m_formatCtx = nullptr;
    m_CodecCtx = nullptr;
    m_VideosStream = nullptr;
    m_frame = nullptr;
    m_swsContext = nullptr;
    m_frameSize = 0;
    errorBuf = nullptr;
    m_VideoFmt = videoFmt;
    m_VideoQulity = videoQuality;
    m_Rs = resolutionRatio;
    m_EncodingPreset = EncodingPreset;
    m_AudioSampleRate = audioSampleRate;
    m_AudioBitRate = audioBitrate;
    m_RecordMode = audioCaptureMode;
    m_gdiCapture = nullptr;
#ifdef TARGET_WIN10
    m_dxgiCapture = nullptr;
#elif TARGET_WIN7
    m_ScreenCapture = nullptr;
#endif 

    //获取用户电脑物理分辨率
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    //计算选区录制的参数
    m_Left = (left % 2) == 0 ? left : (--left);
    m_Top = (top % 2) == 0 ? top : (--top);
    m_Right = (right % 2) == 0 ? right : (--right);
    m_Bottom = (bottom % 2) == 0 ? bottom : (--bottom);
    m_width = m_Right - m_Left;
    m_height = m_Bottom - m_Top;
    bool IsAdjust = false;
    if (m_width % 2 != 0)  // 确保宽度和高度是偶数 (YUV420P格式要求)
    {
        IsAdjust = true;
        m_width--;  // 减1使宽度变为偶数
    }
    if (m_height % 2 != 0)
    {
        IsAdjust = true;
        m_height--;  // 减1使高度变为偶数
    }
    if (IsAdjust == true)
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"调整后的捕获分辨率: %d x %d (确保为偶数)", m_width, m_height);

    m_bitRate = GetVideoQualityMbps(resolutionRatio, videoQuality);
    errorBuf = new char[256];
    memset(errorBuf, 0, 256);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"视频录制对象初始化完成:\n实际分辨率:%dx%d\n实际比特率:%.1f == %.1fMbps\n目标帧率:%dFPS\n目标格式:%S\n声音与麦克风:%s\n",
        //录制基本参数打印
        m_width, m_height, m_bitRate, m_bitRate / (double)1000000, m_frameRate,
        //目标格式打印
        m_VideoFmt == MP4 ? "mp4" :
        (m_VideoFmt == AVI ? "avi" :
            (m_VideoFmt == FLV ? "flv" : "unknow format")),
        //额外选项打印
        audioCaptureMode == RecordMode::SystemSound ? L"录制声音" :
        (audioCaptureMode == RecordMode::Microphone ? L"录制麦克风" :
            (audioCaptureMode == RecordMode::Both ? L"录制声音和麦克风" : L"录制静音视频"))
    );

#ifdef TARGET_WIN10
    //创建DXGI捕获器(选区录制)
    m_Left = (float)left / screenWidth;
    m_Top = (float)top / screenHeight;
    m_Right = (float)(right) / screenWidth;
    m_Bottom = (float)(bottom) / screenHeight;
    if (!m_dxgiCapture)
    {
        m_dxgiCapture = new DXGICapture(m_width, m_height, frameRate, m_Left, m_Top, m_Right, m_Bottom);
        if (!m_dxgiCapture)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建DXGI捕获器失败");
        }
    }
#elif defined  TARGET_WIN7
    //创建GDI捕获器(选区录制)
    if (!m_ScreenCapture)
    {
        m_ScreenCapture = new GDICapture(m_Left, m_Top, m_Right, m_Bottom, m_frameRate);
        if (!m_ScreenCapture)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建GDI捕获器失败");
        }
    }
#endif

    //创建FFmpeg麦克风音频捕获器
    if (!m_microCapture)
    {
        m_microCapture = (
            ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
            new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                (MircroFmt)m_VideoFmt) :
            nullptr
            );
    }

    DBFMT(ConsoleHandle, L"Screnc初始化录制区域,left%d,top%d,right%d,bottom%d",
        (int)m_Left, (int)m_Top, (int)m_Right, (int)m_Bottom);

    //重置所有录制标志并设置某一录制标志
    m_Bool_AreaRecord = true;
    m_Bool_RestartRecord = false;
    m_Bool_ScreenRecord = false;
}

void ScreenRecorder::SetScreenRecordParam(ResolutionRatio resolutionRatio, VideoQuality videoQuality, VideoFormat videoFmt, EncodingPreset EncodingPreset, RecordMode audioCaptureMode, AudioSampleRate audioSampleRate, AudioBitRate audioBitrate, int frameRate)
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    if (m_Bool_AreaRecord || m_Bool_RestartRecord || m_Bool_ScreenRecord)
    {//如果某一个标志为真，则提示非法调用
        DEBUG_CONSOLE_STR(ConsoleHandle, L"非法调用SetScreenRecordParam");
        return;
    }

    m_IsRecording = false;
    m_frameRate = frameRate;
    m_isInit = false;
    m_frameCount = 0;
    m_formatCtx = nullptr;
    m_CodecCtx = nullptr;
    m_VideosStream = nullptr;
    m_frame = nullptr;
    m_swsContext = nullptr;
    m_frameSize = 0;
    errorBuf = nullptr;
    m_VideoFmt = videoFmt;
    m_VideoQulity = videoQuality;
    m_Rs = resolutionRatio;
    m_EncodingPreset = EncodingPreset;
    m_AudioSampleRate = audioSampleRate;
    m_AudioBitRate = audioBitrate;
    m_RecordMode = audioCaptureMode;
    m_gdiCapture = nullptr;
#ifdef TARGET_WIN10
    m_dxgiCapture = nullptr;
#elif TARGET_WIN7
    m_ScreenCapture = nullptr;
#endif 

    //根据枚举初始化分辨率数值,码率值
    ResolutionRatioParam rsParam = GetRsParamByEnum(resolutionRatio);
    m_width = rsParam.width;
    m_height = rsParam.height;
    m_bitRate = GetVideoQualityMbps(resolutionRatio, videoQuality);
    errorBuf = new char[256];
    memset(errorBuf, 0, 256);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"视频录制对象初始化完成:\n实际分辨率:%dx%d\n实际比特率:%.1f == %.1fMbps\n目标帧率:%dFPS\n目标格式:%S\n声音与麦克风:%s\n",
        //录制基本参数打印
        m_width, m_height, m_bitRate, m_bitRate / (double)1000000, m_frameRate,
        //目标格式打印
        m_VideoFmt == MP4 ? "mp4" :
        (m_VideoFmt == AVI ? "avi" :
            (m_VideoFmt == FLV ? "flv" : "unknow format")),
        //额外选项打印
        audioCaptureMode == RecordMode::SystemSound ? L"录制声音" :
        (audioCaptureMode == RecordMode::Microphone ? L"录制麦克风" :
            (audioCaptureMode == RecordMode::Both ? L"录制声音和麦克风" : L"录制静音视频"))
    );
#ifdef TARGET_WIN10
    if (!m_dxgiCapture)
    {
        m_dxgiCapture = new DXGICapture(m_width, m_height, frameRate);
        if (!m_dxgiCapture)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建DXGI捕获器失败");
        }
    }
#elif defined(TARGET_WIN7) 
    //创建GDI捕获器
    if (!m_ScreenCapture)
    {
        if (m_width != screenWidth || m_height != screenHeight)
        {
            m_ScreenCapture = new GDICapture(m_width, m_height, m_frameRate);
        }
        else
        {
            m_ScreenCapture = new GDICapture(frameRate);
        }
        if (!m_ScreenCapture)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建GDI捕获器失败");
        }
    }
#endif

    //创建FFmpeg麦克风音频捕获器
    if (!m_microCapture)
    {
        m_microCapture = (
            ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
            new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                (MircroFmt)m_VideoFmt) :
            nullptr
            );
    }

    //重置所有录制标志并设置某一录制标志
    m_Bool_AreaRecord = false;
    m_Bool_WindowRecord = false;
    m_Bool_ScreenRecord = true;
}

void ScreenRecorder::SetSystemAudioVolume(float Volume)
{
    m_SystemAudioVolume = Volume;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"系统音频音量设置为: %.2f", m_SystemAudioVolume);
}

void ScreenRecorder::SetMicroVolume(float Volume)
{
    m_MicroVolume = Volume;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"麦克风音量设置为: %.2f", m_MicroVolume);
}

void ScreenRecorder::SetVideoFps(int fps) {
    m_frameRate = fps;
}

void ScreenRecorder::SetVideoResolution(int width, int height) {
    m_width = width;
    m_height = height;
}

void ScreenRecorder::SetVideoQuality(bool isQualityPriority) {
    m_IsQualityPriority = isQualityPriority;
}

void ScreenRecorder::SetAudioSampleRate(AudioSampleRate sampleRate) {
    m_AudioSampleRate = sampleRate;
}

void ScreenRecorder::SetAudioBitrate(AudioBitRate bitrate) {
    m_AudioBitRate = bitrate;
}

void ScreenRecorder::SetAudioCaptureDevice(CString deviceName)
{
    m_wstr_Audio = deviceName;
}

void ScreenRecorder::SetMicroDeviceName(CString deviceName)
{
    m_wstr_MicroDevice = deviceName;
}

void ScreenRecorder::SetOnlyAudioRecord(bool IsOnlyAudioRecord)
{
    m_Bool_OnlyAudioRecord = IsOnlyAudioRecord;
}

void ScreenRecorder::SetRecordMouse(bool IsRecordMouse)
{
    m_IsRecordMouse = IsRecordMouse;
}

bool ScreenRecorder::startRecording(const char* outputfilePath)
{
    if (m_IsRecording) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"录制失败，已经开始录制");
        return false;
    }
#ifdef TARGET_WIN7
    if (m_ScreenCapture)//设置是否需要将鼠标录制进去
        m_ScreenCapture->SetRecordCursor(m_IsRecordMouse);
#endif 
    if (m_gdiCapture)//设置是否需要将鼠标录制进去
        m_gdiCapture->SetRecordCursor(m_IsRecordMouse);

    // 初始化FFmpeg
    if (!InitFFmpeg(outputfilePath))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化ffmpeg失败");
        return false;
    }
    if (m_bool_IsTextFilterActive)
    {//如果启用文字水印滤镜
        if (!InitTextImageFilter(m_str_pathUtf8))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化文字水印滤镜失败");
            m_bool_IsTextFilterActive.store(false);
            //不退出
        }
    }

    // 初始化音频捕获
    if (m_wstr_Audio != L"无设备可用" && (m_RecordMode == RecordMode::SystemSound || m_RecordMode == RecordMode::Both))
    {
        m_audioCapture = WasapiCapture::GetInstance();//创建Wasapi音频捕获器
        if (m_audioCapture && (m_audioCapture->Init(
            (WasapiCapture::AudioSampleRate)m_AudioSampleRate,
            (WasapiCapture::AudioFmt)m_VideoFmt,
            m_wstr_Audio
        )))
        {
            m_bool_isAudioInitSuccess = true;
            DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化音频捕获成功");
        }
        else
        {
            m_bool_isAudioInitSuccess = false;
            DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化音频捕获失败");
            //return false;
        }
    }

    // 初始化麦克风捕获
    if (m_wstr_MicroDevice != L"无设备可用" && (m_RecordMode == RecordMode::Microphone || m_RecordMode == RecordMode::Both))
    {
        if (m_microCapture && (m_microCapture->Init(m_wstr_MicroDevice)))
        {
            m_bool_isMicroInitSuccess = true;
            DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化FFmpeg初始化麦克风捕获成功");
        }
        else
        {
            m_bool_isMicroInitSuccess = false;
            DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化FFmpeg初始化麦克风捕获失败");
            //return false;
        }
    }

    // 清空队列
    while (!m_videoQueue.empty()) m_videoQueue.pop();
    while (!m_audioQueue.empty()) m_audioQueue.pop();

    // 设置基准时间
    m_baseTime = av_gettime();
    m_lastAudioPts = 0;
    m_lastVideoPts = 0;
    m_frameCount = 0;
    // m_stats.StartRecording();//录制状态监控线程

    // 初始化录制大小统计
    m_bytesWritten.store(0);
    m_lastSizeCallbackTime = std::chrono::steady_clock::now();

    // 开始各个捕获线程
    //视频帧捕获线程
    m_IsRecording = true;

    if (!m_Bool_OnlyAudioRecord)
    {
#ifdef TARGET_WIN10
        if (m_dxgiCapture)
        {//DXGI桌面捕获线程
            m_dxgiCapture->SetCaptureCursor(m_IsRecordMouse);
            m_bool_isScreenCaptureSuccess = m_dxgiCapture->StartCaptureThread();
        }
#elif TARGET_WIN7
        if (m_ScreenCapture)
        {//GDI桌面捕获线程
            m_bool_isScreenCaptureSuccess = m_ScreenCapture->StartCapture();
        }
#endif
        else if (m_gdiCapture)
        {//GDI应用程序窗口捕获线程
            m_bool_isGdiCaptureSuccess = m_gdiCapture->StartCapture();
        }
        else
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！GDI应用程序窗口捕获线程开启失败，无法进行录制");
            stopRecording();
            return false;
        }
    }

    //音频帧捕获线程
    if (m_bool_isAudioInitSuccess && m_audioCapture)m_audioCapture->StartCapture();//音频捕获线程
    if (m_bool_isMicroInitSuccess && m_microCapture)m_microCapture->StartCapture();//麦克风捕获线程

    // 启动三个帧处理线程(音频帧编码，视频帧编码，音视频混合和同步线程)
#ifdef TARGET_WIN10
    if (((m_bool_isScreenCaptureSuccess && m_dxgiCapture) || (m_bool_isGdiCaptureSuccess && m_gdiCapture))
        && !m_Bool_OnlyAudioRecord)
        m_videoCapureThread = std::thread(&ScreenRecorder::VideoCaptureThreadFunc, this);//视频帧获取并处理线程
#elif TARGET_WIN7
    if (((m_bool_isScreenCaptureSuccess && m_ScreenCapture) || (m_bool_isGdiCaptureSuccess && m_gdiCapture))
        && !m_Bool_OnlyAudioRecord)
        m_videoCapureThread = std::thread(&ScreenRecorder::VideoCaptureThreadFunc, this);//视频帧获取并处理线程
#endif

    if ((m_bool_isAudioInitSuccess && m_audioCapture) || (m_bool_isMicroInitSuccess && m_microCapture))
        m_audioProcessThread = std::thread(&ScreenRecorder::AudioProcessThreadFunc, this);//音频帧获取并处理线程

#ifdef TARGET_WIN10
    if ((!m_bool_isScreenCaptureSuccess || !m_dxgiCapture) &&
        (!m_bool_isGdiCaptureSuccess || !m_gdiCapture) &&
        (!m_bool_isAudioInitSuccess || !m_audioCapture) &&
        (!m_bool_isMicroInitSuccess || !m_microCapture))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"严重错误:没有任何捕获器被初始化，录制失败!");
        return false;
    }
#elif TARGET_WIN7
    if ((!m_bool_isScreenCaptureSuccess || !m_ScreenCapture) &&
        (!m_bool_isGdiCaptureSuccess || !m_gdiCapture) &&
        (!m_bool_isAudioInitSuccess || !m_audioCapture) &&
        (!m_bool_isMicroInitSuccess || !m_microCapture))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"严重错误:没有任何捕获器被初始化，录制失败!");
        return false;
    }
#endif

    m_mixingThread = std::thread(&ScreenRecorder::MixingThreadFunc, this);//音视频同步处理线程 
    DEBUG_CONSOLE_STR(ConsoleHandle, L"所有捕获和编码线程正常开启，开始录制");

    if (!m_Bool_IsHasRestartSince)//若之前没有重启过，开启重启线程并挂起
    {
        m_Thread_RestartRecord = std::thread(&ScreenRecorder::RestartRecordThread, this);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"重启线程开启");
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"视频将被保存至:%s", LARSC::c2w(outputfilePath));
    m_InsIsRecording = true;
    return true;
}

bool ScreenRecorder::stopRecording()
{
    if (!m_IsRecording)
    {
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"开始停止录制过程...");

    //m_stats.StopRecording();
    //如果没有重启过
    m_IsRecording = false;
    if (!m_Bool_IsHasRestartSince)
    {//先唤醒还在挂起的重启线程结束
        std::lock_guard<std::mutex> lock(m_Mutex_RestartRecord);
        m_CV_RestartRecord.notify_all();
    }
    if (m_Thread_RestartRecord.joinable())
    {
        m_Thread_RestartRecord.join();
    }

    // 停止捕获
    if (!m_Bool_OnlyAudioRecord)//如果不是只录制音频
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"停止屏幕捕获");
#ifdef TARGET_WIN10
        if (m_dxgiCapture)
            m_dxgiCapture->StopCaptureThread();
#elif defined TARGET_WIN7
        if (m_ScreenCapture)
            m_ScreenCapture->StopCapture();
#endif 
        if (m_gdiCapture)
            m_gdiCapture->StopCapture();
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"停止音频捕获");
    if (m_audioCapture)//m_audioCapture是一个接口实例，需要释放，同理再次调用StartCapture需要创建并获取接口实例
    {
        m_audioCapture->StopCapture();
        m_audioCapture->ReleaseInstance();
        m_audioCapture = nullptr;
    }
    if (m_microCapture)
        m_microCapture->StopCapture();

    DEBUG_CONSOLE_STR(ConsoleHandle, L"等待捕获线程结束...");

    // 确保混合线程处理完所有队列中的帧
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"等待混合线程处理剩余帧... (视频队列: %d, 音频队列: %d)",
        m_videoQueue.size(), m_audioQueue.size());
    if (m_mixingThread.joinable()) m_mixingThread.join();

    // 等待捕获线程结束
    if (m_videoCapureThread.joinable()) m_videoCapureThread.join();
    if (m_audioProcessThread.joinable()) m_audioProcessThread.join();

    // 清理音视频同步队列
    while (!m_videoQueue.empty()) {
        VideoPacket& packet = m_videoQueue.front();
        av_packet_unref(&packet.pkt);
        m_videoQueue.pop();
    }
    while (!m_audioQueue.empty()) {
        AudioPacket& packet = m_audioQueue.front();
        av_packet_unref(&packet.pkt);
        m_audioQueue.pop();
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"清理FFmpeg资源");
    // 清理FFmpeg资源
    CleanUpFFmpeg();

    //DEBUG_CONSOLE_STR_LONGBUFER(ConsoleHandle, m_stats.GenerateSummary().c_str());

    //如果是硬件编码过程发生了错误并重启回退到了软件编码，则弹框提示
    if (m_Bool_IsHasRestartSince && m_EncoderFailureHandler.IsFallbackActive())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"在开启录制后，硬件编码发生了异常，进行了重启回退软件编码器");
        AfxMessageBox(L"录制已保存，但显卡硬件编码发生异常，请更新显卡驱动程序以获得更好体验");
    }
    m_InsIsRecording = false;
    return true;
}

bool ScreenRecorder::InitFFmpeg(const char* outputfilePath)
{
    //如果是选区录制，则转换为软件编码
    if (m_Bool_AreaRecord)
    {
        m_VideoEncoder = "";
    }

    av_log_set_level(AV_LOG_ERROR);
    m_str_outputfile = outputfilePath;//保存录制的位置
    // 若已经初始化过，则先清理
    if (m_isInit) {
        CleanUpFFmpeg();
    }

    // 根据枚举选择编码的视频格式
    const char* formatName = nullptr;
    switch (m_VideoFmt)
    {
    case ScreenRecorder::MP4:
        formatName = "mp4";
        break;
    case ScreenRecorder::AVI:
        formatName = "avi";
        break;
    case ScreenRecorder::FLV:
        formatName = "flv";
        break;
    default:
        formatName = "mp4"; // 默认使用MP4
        break;
    }

    // 创建输出上下文
    int ret = avformat_alloc_output_context2(&m_formatCtx, NULL, formatName, outputfilePath);
    if (ret < 0) {
        av_strerror(ret, errorBuf, 256);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建输出上下文失败");
        return false;
    }

    // 查找编码器
    const AVCodec* codec = nullptr;
    bool isHardwareEncoder = m_VideoEncoder == "" ? false : true;
    if (isHardwareEncoder)
    {// 用户设置使用硬件编码器
        codec = avcodec_find_encoder_by_name(m_VideoEncoder.c_str());
        if (!codec)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"硬件编码器查找失败，将使用软件编码器");
            isHardwareEncoder = false;
        }
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"使用硬件编码器:%s", LARSC::s2ws(m_VideoEncoder).c_str());
    }
    else
    {// 用户设置使用软件编码器
        if (!codec)
        {
            codec = avcodec_find_encoder(AV_CODEC_ID_H264);
            if (!codec)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"软件编码器查找失败，当前软件不支持在此电脑上运行");
                return false;
            }
            DEBUG_CONSOLE_STR(ConsoleHandle, L"使用软件H264编码器H264");
        }
    }

    // 创建视频流
    m_VideosStream = avformat_new_stream(m_formatCtx, codec);
    if (!m_VideosStream)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建视频流失败");
        return false;
    }
    m_VideosStream->id = m_formatCtx->nb_streams - 1; // 赋值视频流的id下标
    m_VideoStreamIndex = 0;

    // 创建编码器上下文
    m_CodecCtx = avcodec_alloc_context3(codec);
    if (!m_CodecCtx)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建编码器上下文失败");
        return false;
    }

    // 指定编码视频的基本参数
    m_CodecCtx->width = m_width;
    m_CodecCtx->height = m_height;
    if (m_frameRate > 120)//特殊处理高帧率录制
    {
        if (m_VideoEncoder != "h264_nvenc" || m_VideoEncoder == "")
        {   //h264_qsv,h264_amf,软件编码器,这些硬件编码器最多支持120fps
            m_CodecCtx->time_base = AVRational{ 1, 120 };
            m_CodecCtx->framerate = AVRational{ 120, 1 };
        }
        else
        {
            m_CodecCtx->time_base = AVRational{ 1, m_frameRate };
            m_CodecCtx->framerate = AVRational{ m_frameRate, 1 };
        }
    }
    else
    {
        m_CodecCtx->time_base = AVRational{ 1, m_frameRate };
        m_CodecCtx->framerate = AVRational{ m_frameRate, 1 };
    }

    if (m_frameRate == 180)
        m_CodecCtx->bit_rate = m_bitRate * 30;
    else if (m_frameRate >= 120 && m_frameRate < 180)
        m_CodecCtx->bit_rate = m_bitRate * 20;
    else if (m_frameRate > 65)
        m_CodecCtx->bit_rate = m_bitRate * 14;
    else if (m_frameRate > 40 && m_frameRate < 65)
        m_CodecCtx->bit_rate = m_bitRate * 8;
    else if (m_frameRate <= 30 && m_frameRate > 10)
        m_CodecCtx->bit_rate = m_bitRate * 4.5;
    else
        m_CodecCtx->bit_rate = m_bitRate * 2.5;
    m_CodecCtx->max_b_frames = 0;
    if (m_VideoEncoder == "h264_qsv" || m_VideoEncoder == "h264_nvenc" || m_VideoEncoder == "h264_amf")
    {
        m_CodecCtx->pix_fmt = AV_PIX_FMT_NV12;
    }
    else
        m_CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    if (m_formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        m_CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    m_formatCtx->flags |= AVFMT_FLAG_GENPTS;
    m_VideosStream->time_base = m_CodecCtx->time_base;// 设置时间基和帧率

    //打开编码器
    ret = -1;
    INT8 OriPresest = m_EncodingPreset;
    if (isHardwareEncoder && ret < 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"开始测试可用的硬件编码器预设");
        INT8 preset = (INT8)m_EncodingPreset;
        while (preset >= 0 && ret < 0)
        {
            AVDictionary* options = nullptr;
            m_EncodingPreset = (EncodingPreset)preset;
            SetEncodeParam(m_EncodingPreset);
            ret = avcodec_open2(m_CodecCtx, codec, &options);
            if (options && av_dict_count(options) > 0)
            {// 检查是否有未识别的选项
                const AVDictionaryEntry* t = nullptr;
                while ((t = av_dict_get(options, "", t, AV_DICT_IGNORE_SUFFIX))) {
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"未识别的编码器选项: %s", LARSC::c2w(t->key));
                }
            }
            av_dict_free(&options);// 释放选项字典
            if (ret < 0)
                preset--;;
        }
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"通过测试并选择的编码参数预设:%s",
            preset == EncodingPreset::Fast ? L"视频帧率优先" :
            (preset == EncodingPreset::Medium) ? L"平衡" :
            (preset == EncodingPreset::Slow) ? L"视频品质优先" :
            L"未通过测试，使用软件编码器");
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"用户原本选择的编码器参数预设:%s",
            OriPresest == EncodingPreset::Fast ? L"视频帧率优先" :
            (OriPresest == EncodingPreset::Medium) ? L"平衡" :
            (OriPresest == EncodingPreset::Slow) ? L"视频品质优先" :
            L"未通过测试，使用软件编码器");
    }
    if (ret < 0)
    {//打开软件编码器
        char errBuf[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"使用软件编码器");

        // 重新初始化为软件编码器
        avcodec_free_context(&m_CodecCtx);
        codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!codec)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"找不到软件H264编码器");
            return false;
        }

        m_CodecCtx = avcodec_alloc_context3(codec);
        if (!m_CodecCtx)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建软件编码器上下文失败");
            return false;
        }

        // 重新设置基本参数
        m_CodecCtx->width = m_width;
        m_CodecCtx->height = m_height;
        m_CodecCtx->time_base = AVRational{ 1, m_frameRate };
        m_CodecCtx->framerate = AVRational{ m_frameRate, 1 };
        m_CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        m_CodecCtx->gop_size = 25;
        if (m_frameRate == 180)
            m_CodecCtx->bit_rate = m_bitRate * 56;
        else if (m_frameRate >= 120 && m_frameRate < 180)
            m_CodecCtx->bit_rate = m_bitRate * 38;
        else if (m_frameRate > 65)
            m_CodecCtx->bit_rate = m_bitRate * 36;
        else if (m_frameRate > 40 && m_frameRate < 65)
            m_CodecCtx->bit_rate = m_bitRate * 22;
        else if (m_frameRate <= 30 && m_frameRate > 10)
            m_CodecCtx->bit_rate = m_bitRate * 15;
        else
            m_CodecCtx->bit_rate = m_bitRate * 2.5;
        m_CodecCtx->max_b_frames = 0;
        if (m_formatCtx->oformat->flags & AVFMT_GLOBALHEADER) {
            m_CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
        if (m_VideoFmt == FLV)//对于flv流媒体格式，需要而外的设置一些信息，保证音视频同步
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到FLV格式，应用特殊同步设置");
            AVRational flv_time_base = { 1, 1000 };
            m_CodecCtx->time_base = flv_time_base;
            m_VideosStream->time_base = flv_time_base;
            m_CodecCtx->max_b_frames = 0;
            m_CodecCtx->qmin = 22;
            m_CodecCtx->qmax = 35;
            m_CodecCtx->max_qdiff = 4;
            m_CodecCtx->qcompress = 0.8;
            m_CodecCtx->keyint_min = 10;
        }

        //编码器设置
        AVDictionary* soft_options = nullptr;
        av_opt_set(m_CodecCtx->priv_data, "preset", "superfast", 0);
        av_opt_set(m_CodecCtx->priv_data, "tune", "zerolatency", 0);
        av_opt_set(m_CodecCtx->priv_data, "prifile", "main", 0);
        av_opt_set(m_CodecCtx->priv_data, "b-pyramid", "none", 0);
        if (m_VideoFmt == ScreenRecorder::FLV)//对于flv流媒体格式，需要而外的设置一些信息，保证flv格式视频符合正常标准
        {
            av_dict_set(&soft_options, "crf", "28", 0);
            av_dict_set(&soft_options, "level", "3.0", 0);
            av_dict_set(&soft_options, "subq", "4", 0);
            av_dict_set(&soft_options, "g", "15", 0);
            ret = avcodec_open2(m_CodecCtx, codec, &soft_options);
            av_dict_free(&soft_options);
        }
        else
        {
            ret = avcodec_open2(m_CodecCtx, codec, nullptr);
        }
        if (ret < 0)
        {
            av_strerror(ret, errBuf, AV_ERROR_MAX_STRING_SIZE);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"打开软件编码器也失败,无法录制!: %s", LARSC::c2w(errBuf));
            return false;
        }
    }

    // 更新视频流参数
    m_VideosStream->time_base = m_CodecCtx->time_base;

    // 复制编码器参数到视频流中的解码器
    ret = avcodec_parameters_from_context(m_VideosStream->codecpar, m_CodecCtx);
    if (ret < 0) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"复制编码器参数到视频流中的解码器失败");
        return false;
    }

    // 获取每帧的大小
    m_frameSize = av_image_get_buffer_size(m_CodecCtx->pix_fmt, m_CodecCtx->width, m_CodecCtx->height, 1);

    // 初始化视频帧转换上下文
    m_swsContext = sws_getContext(m_CodecCtx->width, m_CodecCtx->height, AV_PIX_FMT_BGRA,
        m_CodecCtx->width, m_CodecCtx->height,
        m_CodecCtx->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
    if (!m_swsContext) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"屏幕捕获数据转换失败");
        return false;
    }

    //------------------------------音频


    // 音频部分
    m_AudioStream = avformat_new_stream(m_formatCtx, NULL);
    m_AudioStreamIndex = 1;
    if (!m_AudioStream) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建音频流失败");
        return false;
    }

    // 根据视频格式，选择合适的音频编码器
    const AVCodec* audioCodec = avcodec_find_encoder(
        ((m_VideoFmt == VideoFormat::MP4) || (m_VideoFmt == VideoFormat::FLV)) ? AV_CODEC_ID_AAC : AV_CODEC_ID_ADPCM_MS
    );

    if (!audioCodec) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"查找合适的音频编码器失败");
        return false;
    }

    // 通过音频编码器分配音频解码器上下文
    m_pCodecCtx_Audio = avcodec_alloc_context3(audioCodec);
    if (!m_pCodecCtx_Audio) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"创建音频解码器上下文失败");
        return false;
    }

    // 设置音频编码器参数
    m_pCodecCtx_Audio->sample_rate = (int)m_AudioSampleRate;
    m_pCodecCtx_Audio->bit_rate = (int)m_AudioBitRate;
    m_pCodecCtx_Audio->sample_fmt = (
        ((m_VideoFmt == VideoFormat::MP4) || (m_VideoFmt == VideoFormat::FLV)) ? AV_SAMPLE_FMT_FLTP : AV_SAMPLE_FMT_S16
        );
    m_pCodecCtx_Audio->time_base = AVRational{ 1, (int)m_AudioSampleRate };
    m_pCodecCtx_Audio->frame_size = m_VideoFmt == VideoFormat::MP4 ? 1024 : 1012;
    m_AudioStream->time_base = m_pCodecCtx_Audio->time_base;
    if (m_VideoFmt == ScreenRecorder::FLV)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到FLV格式，应用特殊同步设置");
        AVRational flv_time_base = { 1, 1000 };
        m_CodecCtx->time_base = flv_time_base;
        m_VideosStream->time_base = flv_time_base;
        m_pCodecCtx_Audio->time_base = flv_time_base;
        m_AudioStream->time_base = flv_time_base;
        m_pCodecCtx_Audio->bit_rate = 96000;
    }

    // 显式设置通道数为2
    m_pCodecCtx_Audio->channel_layout = AV_CH_LAYOUT_STEREO;
    m_pCodecCtx_Audio->channels = 2;

    //高比特率可能会因为用户电脑性能问题导致音频编码落后于视频，这不可避免,因此我进行一些编码设置速度上的优化
    AVDictionary* audio_opts = nullptr;
    if ((int)m_AudioBitRate > 128000)
    {
        // 对AAC编码器优化
        if (m_VideoFmt == VideoFormat::MP4)
        {
            av_dict_set(&audio_opts, "compression_level", "0", 0); // 最快的压缩级别
            av_dict_set(&audio_opts, "profile", "aac_low", 0);     // 使用低复杂度配置文件

            // 对AAC编码器特殊优化
            m_pCodecCtx_Audio->flags |= AV_CODEC_FLAG_QSCALE;     // 允许可变比特率提高速度
            m_pCodecCtx_Audio->global_quality = 3 * FF_QP2LAMBDA; // 较低质量但速度更快
        }
    }
    if (m_VideoFmt == VideoFormat::AVI)
    { // 如果导出AVI格式视频，使用的ADPCM音频编码器需要额外设置一个参数
        m_pCodecCtx_Audio->block_align = 2048;
    }
    if (m_VideoFmt == VideoFormat::FLV)
    { // FLV格式的特殊音频编码设置
        av_dict_set(&audio_opts, "strict", "experimental", 0);
        av_dict_set(&audio_opts, "profile", "aac_low", 0);
        av_dict_set(&audio_opts, "strict", "experimental", 0);
        av_dict_set(&audio_opts, "profile", "aac_low", 0);
        av_dict_set(&audio_opts, "compression_level", "9", 0);
    }

    // 设置流时间基
    AVRational audioStreamTimeBase = { 1, (int)m_AudioSampleRate };
    m_formatCtx->streams[m_AudioStreamIndex]->time_base = audioStreamTimeBase;
    // 初始化和绑定音频编码器上下文
    ret = avcodec_open2(m_pCodecCtx_Audio, audioCodec, &audio_opts);
    if (ret < 0) {
        CHECK_FFMPEG_ERROR(ret, L"初始化音频解码器失败");
    }
    av_dict_free(&audio_opts);

    // 音频编码器上下文和输出上下文参数同步
    ret = avcodec_parameters_from_context(m_formatCtx->streams[m_AudioStreamIndex]->codecpar, m_pCodecCtx_Audio);
    if (ret < 0) {
        CHECK_FFMPEG_ERROR(ret, L"音频解码器上下文和输出上下文参数同步失败");
    }

    // 打开输出文件
    if (!(m_formatCtx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&m_formatCtx->pb, outputfilePath, AVIO_FLAG_WRITE);
        if (ret < 0) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"打开输出文件失败");
            return false;
        }
    }
    // 写入文件头
    if (m_VideoFmt == ScreenRecorder::FLV)
    { // 优化FLV写入设置
        av_opt_set_int(m_formatCtx, "flush_packets", 1, AV_OPT_SEARCH_CHILDREN);
        m_formatCtx->max_delay = 500000; // 500ms
        av_opt_set(m_formatCtx, "flvflags", "no_duration_filesize", AV_OPT_SEARCH_CHILDREN);
        av_opt_set_int(m_formatCtx->pb, "flush_packets", 1, AV_OPT_SEARCH_CHILDREN);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"已应用FLV格式特殊设置以改善音视频同步与文件大小");
    }
    ret = avformat_write_header(m_formatCtx, NULL);
    if (ret < 0) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"写入文件头失败");
        return false;
    }

    m_isInit = true;
    return true;
}

void ScreenRecorder::FlushEncoder(AVCodecContext* enc_ctx, int stream_index)
{
    if (!enc_ctx)
        return;

    int ret;
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    // 发送一个NULL帧表示结束
    ret = avcodec_send_frame(enc_ctx, nullptr);

    // 接收所有剩余编码包
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, &packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret >= 0) {
            packet.stream_index = stream_index;
            av_write_frame(m_formatCtx, &packet);
        }
        av_packet_unref(&packet);
    }
}

void ScreenRecorder::UpdateRecordArea(const CRect& RecordRect)
{
    if (!m_IsRecording || !m_isInit) {
        return;
    }

    //限制更新速率200ms
    auto nowtime = std::chrono::steady_clock::now();
    static auto lastime = nowtime - std::chrono::milliseconds(100);
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(nowtime - lastime).count();
    if (diff < 100)
        return;
    lastime = std::chrono::steady_clock::now();

    //更新录制区域参数
    m_width = RecordRect.Width();
    m_height = RecordRect.Height();
    int left, right, top, bottom;
    left = RecordRect.left;
    right = RecordRect.right;
    top = RecordRect.top;
    bottom = RecordRect.bottom;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]:更新录制区域参数:%d,%d,%d,%d",
        left, top, m_width, m_height);
    bool IsAdjust = false;
    if (m_width % 2 != 0)  // 确保宽度和高度是偶数 (YUV420P格式要求)
    {
        IsAdjust = true;
        m_width--;  // 减1使宽度变为偶数
    }
    if (m_height % 2 != 0)
    {
        IsAdjust = true;
        m_height--;  // 减1使高度变为偶数
    }
    if (IsAdjust == true)
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"调整后的捕获分辨率: %d x %d (确保为偶数)", m_width, m_height);

    std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);//暂停视频编码
    //更新编码参数
    if (m_CodecCtx)
    {
        m_CodecCtx->width = m_width;
        m_CodecCtx->height = m_height;
#ifdef TARGET_WIN10
        if (!m_dxgiCapture->UpdateCaptureRegion(left, top, m_width, m_height))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]:更新DXGI捕获选项失败");
        }
#elif defined TARGET_WIN7
        if (!m_ScreenCapture->UpdateRecordArea(left, top))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]:更新GDI捕获选项失败");
        }
#endif
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]:更新录制区域完成");
}

void ScreenRecorder::SetRecordCallBack(RecordCallBack CallBackType, std::function<void()> funcCallBack)
{
    m_Map_RecordCallBackFunc.emplace(CallBackType, funcCallBack);
}

void ScreenRecorder::CleanUpFFmpeg()
{
    if (!m_isInit) {
        return;
    }

    // 刷新视频编码器
    if (m_CodecCtx) {
        FlushEncoder(m_CodecCtx, m_VideoStreamIndex);
    }

    // 刷新音频编码器
    if (m_pCodecCtx_Audio) {
        FlushEncoder(m_pCodecCtx_Audio, m_AudioStreamIndex);
    }

    // 写入文件尾
    if (m_formatCtx) {
        av_write_trailer(m_formatCtx);
    }
    //关闭解码器
    if (m_CodecCtx) {
        avcodec_free_context(&m_CodecCtx);
    }
    //释放帧
    if (m_frame) {
        av_frame_free(&m_frame);
    }
    //释放音频解码器
    if (m_pCodecCtx_Audio) {
        avcodec_free_context(&m_pCodecCtx_Audio);
    }
    //释放缩放上下文
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }
    //关闭输出文件
    if (m_formatCtx) {
        if (!(m_formatCtx->oformat->flags & AVFMT_NOFILE)) {
            avio_closep(&m_formatCtx->pb);
        }
        avformat_free_context(m_formatCtx);
        m_formatCtx = nullptr;
    }
}

void ScreenRecorder::CleanCaptureInterface()
{
    if (!m_Bool_OnlyAudioRecord)//如果不是只录制声音
    {   //视频捕获器释放
#ifdef TARGET_WIN10
        if (m_dxgiCapture && m_dxgiCapture->IsCaptureThreadRunning())
        {
            m_dxgiCapture->StopCaptureThread();
        }
        if (m_dxgiCapture)
        {
            delete m_dxgiCapture;
            m_dxgiCapture = nullptr;
        }
#elif defined TARGET_WIN7
        if (m_ScreenCapture)
        {
            delete m_ScreenCapture;
            m_ScreenCapture = nullptr;
        }
#endif
        if (m_gdiCapture)
        {
            delete m_gdiCapture;
            m_gdiCapture = nullptr;
        }
    }
    //音频捕获器释放
    if (m_audioCapture)
    {
        m_audioCapture->ReleaseInstance();
        m_audioCapture = nullptr;
    }
    if (m_microCapture)
    {
        delete m_microCapture;
        m_microCapture = nullptr;
    }
}

bool ScreenRecorder::TestEncoderAvailability(const char* encoderName)
{
    if (!encoderName || strlen(encoderName) == 0) {
        return false;
    }

    const AVCodec* codec = avcodec_find_encoder_by_name(encoderName);
    return (codec != nullptr);
}

void ScreenRecorder::SetEncodeParam(EncodingPreset encodePreset)
{
    if (m_VideoEncoder == "h264_nvenc")
    {//英伟达硬件编码器
        if (encodePreset == EncodingPreset::Fast)
        {//
        }
        else if (encodePreset == EncodingPreset::Medium)
        {//平衡

        }
        else if (encodePreset == EncodingPreset::Slow)
        {//高质量

        }
    }
    else
    {//其他硬件编码器
        //什么都不做，让编码器自己选择(等待后续测试后可扩展)
    }

    if (m_VideoFmt == VideoFormat::FLV)
    { //什么都不做，让编码器自己选择(等待后续测试后可扩展)

    }
    return;
}

ResolutionRatioParam ScreenRecorder::GetRsParamByEnum(ResolutionRatio resolutionRatio)
{
    ResolutionRatioParam param{ 0,0 };
    switch (resolutionRatio)
    {
    case ScreenRecorder::Rs_SameAsScreen:
        param.width = GetSystemMetrics(SM_CXSCREEN);
        param.height = GetSystemMetrics(SM_CYSCREEN);
        return param;
        break;
    case ScreenRecorder::Rs_360P:
        param.width = 640;
        param.height = 360;
        return param;
        break;
    case ScreenRecorder::Rs_480P:
        param.width = 854;
        param.height = 480;
        return param;
        break;
    case ScreenRecorder::Rs_720P:
        param.width = 1280;
        param.height = 720;
        return param;
        break;
    case ScreenRecorder::Rs_1080P:
        param.width = 1920;
        param.height = 1080;
        return param;
        break;
    case ScreenRecorder::Rs_2K:
        param.width = 2560;
        param.height = 1440;
        return param;
        break;
    case ScreenRecorder::Rs_4k:
        param.width = 3840;
        param.height = 2160;
        return param;
        break;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"获取屏幕分辨率失败！错误码:ScreenRecorder509");
    return param;
}

double ScreenRecorder::GetOriginQualityByFullScreen()
{
    // 获取当前屏幕分辨率
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 根据屏幕分辨率确定合适的比特率(Mbps)
    double bitrateMbps = 0.0;
    if (screenHeight <= 360) {  // 360p或更低
        bitrateMbps = 2.5;
    }
    else if (screenHeight <= 480) {// 480p
        bitrateMbps = 4.0;
    }
    else if (screenHeight <= 720) { // 720p
        bitrateMbps = 8.0;
    }
    else if (screenHeight <= 1080) {// 1080p
        bitrateMbps = 13.0;
    }
    else if (screenHeight <= 1440) { // 2K/1440p
        bitrateMbps = 25.0;
    }
    else if (screenHeight <= 2160) { // 4K/2160p
        bitrateMbps = 65.0;
    }

    // 记录日志
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"检测到屏幕分辨率: %dx%d\n原画质量比特率: %.1f Mbps\n",
        screenWidth, screenHeight, bitrateMbps);

    return bitrateMbps * 1000000;
}

double ScreenRecorder::GetVideoQualityMbps(ResolutionRatioParam rsParam, VideoQuality videoQuality)
{
    //定义标准分辨率及其对应的原画比特率 (Mbps)
    struct ResolutionBitrate {
        int pixels;       // 总像素数 (width * height)
        double bitrate;   // 对应的原画比特率 (Mbps)
        const char* name; // 分辨率名称，用于日志
    };
    const ResolutionBitrate standardResolutions[] = {
        { 640 * 360,    3.0, "360p" },
        { 854 * 480,    5.0, "480p" },
        { 1280 * 720,   10.0, "720p" },
        { 1920 * 1080,  20.0, "1080p" },
        { 2560 * 1440,  35.0, "1440p" },
        { 3840 * 2160,  100.0, "4K" },
        { 7680 * 4320,  350.0, "8K" }
    };

    //计算输入分辨率的总像素数
    int inputPixels = rsParam.width * rsParam.height;
    if (inputPixels <= 0) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"无效分辨率参数，使用默认值 10 Mbps");
        return 10.0; // 默认为720p的比特率
    }

    // 查找最接近的标准分辨率
    double originalBitrate = 0.0;
    const char* resName = "custom";

    // 如果小于最低分辨率，使用最低标准
    if (inputPixels < standardResolutions[0].pixels) {
        originalBitrate = standardResolutions[0].bitrate;
        resName = standardResolutions[0].name;
    }
    // 如果超过最高分辨率，使用最高标准
    else if (inputPixels >= standardResolutions[6].pixels) {
        originalBitrate = standardResolutions[6].bitrate;
        resName = standardResolutions[6].name;
    }
    // 否则查找区间
    else {
        for (int i = 0; i < 6; i++) {
            if (inputPixels >= standardResolutions[i].pixels &&
                inputPixels < standardResolutions[i + 1].pixels) {

                // 确定在该区间的位置 (0.0 - 1.0)
                double position = static_cast<double>(inputPixels - standardResolutions[i].pixels) /
                    (standardResolutions[i + 1].pixels - standardResolutions[i].pixels);

                // 线性插值计算适合的比特率
                originalBitrate = standardResolutions[i].bitrate +
                    position * (standardResolutions[i + 1].bitrate - standardResolutions[i].bitrate);

                // 确定接近哪个标准分辨率用于日志
                if (position < 0.5) {
                    resName = standardResolutions[i].name;
                }
                else {
                    resName = standardResolutions[i + 1].name;
                }

                break;
            }
        }
    }

    // 根据视频质量调整比特率
    double finalBitrate = 0.0;
    const wchar_t* qualityName = L"";
    switch (videoQuality) {
    case Origin:
        finalBitrate = originalBitrate;
        qualityName = L"原画";
        break;
    case SuperDefinition:
        finalBitrate = originalBitrate * 0.8;
        qualityName = L"超清";
        break;
    case HighDefinition:
        finalBitrate = originalBitrate * 0.7;
        qualityName = L"高清";
        break;
    case StandardDefinition:
        finalBitrate = originalBitrate * 0.2;
        qualityName = L"标清";
        break;
    default:
        finalBitrate = originalBitrate;
        qualityName = L"未知";
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle,
        L"采用线性插值算法:分辨率: %dx%d\n 视频质量: %s\n 计算比特率: %.2f Mbps\n",
        rsParam.width, rsParam.height, qualityName, finalBitrate);
    return finalBitrate;
}

double ScreenRecorder::GetVideoQualityMbps(ResolutionRatio resolutionRatio, VideoQuality videoQuality)
{
    double birRate = 0;
    if (resolutionRatio == ResolutionRatio::Rs_SameAsScreen) {
        ResolutionRatioParam ratioPararm;
        ratioPararm.height = GetSystemMetrics(SM_CYSCREEN);
        ratioPararm.width = GetSystemMetrics(SM_CXSCREEN);
        birRate = GetVideoQualityMbps(ratioPararm, videoQuality) * 16000;
    }
    else {
        birRate = (double)resolutionRatio * ((double)videoQuality / 10);
    }
    return birRate;
}

int64_t ScreenRecorder::GetCurrentPtsUsec()
{
    return av_gettime() - m_baseTime;
}

void ScreenRecorder::VideoCaptureThreadFunc()
{
#ifdef TARGET_WIN10
    if (m_dxgiCapture)
        DXGIVideoFrameEncode();
#elif defined TARGET_WIN7
    if (m_ScreenCapture)
        GDIScreenVideoFrameEncode();
#endif // TARGET_WIN10
    else if (m_gdiCapture)
        GdiVideoFrameEncode();
}

void ScreenRecorder::AudioProcessThreadFunc()
{
    if (m_bool_isAudioInitSuccess && !m_bool_isMicroInitSuccess)//仅录制系统声音
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"启动系统音频编码");
        AudioFrameEncode();
    }
    else if (m_bool_isMicroInitSuccess && !m_bool_isAudioInitSuccess)//仅录制麦克风声音
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"启动麦克风音频编码");
        MicroFrameEncode();
    }
    else if (m_bool_isMicroInitSuccess && m_bool_isAudioInitSuccess)//混合录制两者
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"启动麦克风音频系统音频混合编码");
        AudioMicroMixEncode();
    }
}

void ScreenRecorder::MixingThreadFunc()
{
    bool isFlvFormat = (m_VideoFmt == ScreenRecorder::FLV);
    int64_t lastVideoPts = 0;
    int64_t lastAudioPts = 0;

    int videoFrameCount = 0;
    int audioFrameCount = 0;
    int64_t cur_pts_v = 0;
    int64_t cur_pts_a = 0;

    // 平衡系数（控制视频和音频的处理比例）
    const double balance_ratio = 1.0;
    int video_batch = 0;
    int audio_batch = 0;

    // 额外视频帧计数器，用于确保视频不会过早结束
    int extra_video_frames = 0;
    const int MAX_EXTRA_VIDEO_FRAMES = 50; // 最多允许多少额外视频帧

    while (m_IsRecording || !m_videoQueue.empty() || !m_audioQueue.empty())
    {
        bool wrote_frame = false;

        // 检查是否有视频或音频数据可用
        bool hasVideo = false, hasAudio = false;
        int64_t next_video_pts = 0, next_audio_pts = 0;
        int videoQueueSize = 0, audioQueueSize = 0;

        {
            std::lock_guard<std::mutex> videoLock(m_videoMutex);
            hasVideo = !m_videoQueue.empty();
            videoQueueSize = (int)m_videoQueue.size();
            if (hasVideo) {
                next_video_pts = m_videoQueue.front().pts;
            }
        }

        {
            std::lock_guard<std::mutex> audioLock(m_audioMutex);
            hasAudio = !m_audioQueue.empty();
            audioQueueSize = (int)m_audioQueue.size();
            if (hasAudio) {
                next_audio_pts = m_audioQueue.front().pts;
            }
        }

        // 如果没有可用数据且录制已停止，则退出循环
        if (!hasVideo && !hasAudio)
        {
            if (!m_IsRecording) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // 决定处理视频还是音频
        bool process_video = false;

        // 如果只有一种类型的数据可用，则处理该类型
        if (hasVideo && !hasAudio)
        {
            // 如果已经超过了额外视频帧的限制，就不再处理视频
            if (extra_video_frames >= MAX_EXTRA_VIDEO_FRAMES && !m_IsRecording)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
            process_video = true;
        }
        else if (!hasVideo && hasAudio)
        {
            process_video = false;
        }
        // 如果两种数据都有，根据不同策略决定处理顺序
        else if (hasVideo && hasAudio)
        {
            // 先比较时间戳
            int64_t video_timestamp = av_rescale_q(next_video_pts, m_formatCtx->streams[m_VideoStreamIndex]->time_base, AVRational{ 1, AV_TIME_BASE });
            int64_t audio_timestamp = av_rescale_q(next_audio_pts, m_formatCtx->streams[m_AudioStreamIndex]->time_base, AVRational{ 1, AV_TIME_BASE });

            // 基于时间戳比较决定处理顺序
            if (std::abs(video_timestamp - audio_timestamp) > 500000) { // 如果相差超过0.5秒
                // 选择时间戳较早的帧
                process_video = (video_timestamp < audio_timestamp);
            }
            else {
                // 当时间戳接近时，根据队列长度和平衡系数决定
                if (videoQueueSize > audioQueueSize * balance_ratio) {
                    process_video = true; // 视频队列积累较多，优先处理视频
                }
                else if (audioQueueSize > videoQueueSize * balance_ratio) {
                    process_video = false; // 音频队列积累较多，优先处理音频
                }
                else {
                    // 使用批量处理模式，连续处理几个相同类型的帧
                    if (video_batch > 0) {
                        process_video = true;
                        video_batch--;
                    }
                    else if (audio_batch > 0) {
                        process_video = false;
                        audio_batch--;
                    }
                    else {
                        // 开始新的批次
                        if (videoFrameCount <= audioFrameCount) {
                            process_video = true;
                            video_batch = 3; // 处理3个视频帧
                        }
                        else {
                            process_video = false;
                            audio_batch = 3; // 处理3个音频帧
                        }
                    }
                }
            }
        }

        if (process_video && hasVideo)
        {
            // 处理视频帧
            std::lock_guard<std::mutex> lock(m_videoMutex);
            if (!m_videoQueue.empty()) // 再次检查，因为可能在获取锁的过程中队列被修改
            {
                VideoPacket& videoPacket = m_videoQueue.front();

                // 创建一个新的AVPacket来写入
                AVPacket pkt;
                av_init_packet(&pkt);
                av_packet_ref(&pkt, &videoPacket.pkt);

                // 确保设置了正确的流索引和时间戳
                pkt.stream_index = m_VideoStreamIndex;
                cur_pts_v = pkt.pts;
                if (isFlvFormat)
                { // 确保时间戳连续递增
                    if (pkt.pts <= lastVideoPts && lastVideoPts > 0)
                    {
                        pkt.pts = pkt.dts = lastVideoPts + 1;
                    }
                    lastVideoPts = pkt.pts;
                }

                // 写入帧
                int ret = av_write_frame(m_formatCtx, &pkt);
                if (ret < 0) {
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"写入视频帧失败: %d\n", ret);
                }

                //记录录制大小
                if (ret >= 0)
                {
                    m_bytesWritten.fetch_add(pkt.size, std::memory_order_relaxed);
                    if (m_RecordSizeCallback)
                    {
                        auto now = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSizeCallbackTime).count()
                            >= m_sizeCallbackIntervalMs)
                        {
                            m_lastSizeCallbackTime = now;
                            double mb = m_bytesWritten.load(std::memory_order_relaxed) / (1024.0 * 1024.0);
                            m_RecordSizeCallback(mb);
                        }
                    }
                }

                av_packet_unref(&pkt);
                av_packet_unref(&videoPacket.pkt);
                m_videoQueue.pop();

                videoFrameCount++;
                wrote_frame = true;

                // 如果音频已经结束，记录额外的视频帧
                if (!m_IsRecording && m_audioQueue.empty()) {
                    extra_video_frames++;
                }

                DEBUG_CONSOLE_FMT(ConsoleHandle, L"写入视频帧 #%d, PTS: %lld\n",
                    videoFrameCount, cur_pts_v);


            }
        }
        else if (!process_video && hasAudio)
        {
            // 处理音频帧
            std::lock_guard<std::mutex> lock(m_audioMutex);
            if (!m_audioQueue.empty()) // 再次检查，因为可能在获取锁的过程中队列被修改
            {
                AudioPacket& audioPacket = m_audioQueue.front();

                // 创建一个新的AVPacket来写入
                AVPacket pkt;
                av_init_packet(&pkt);
                av_packet_ref(&pkt, &audioPacket.pkt);

                // 确保设置了正确的流索引和时间戳
                pkt.stream_index = m_AudioStreamIndex;
                cur_pts_a = pkt.pts;
                if (isFlvFormat)
                {  // 确保时间戳连续递增
                    if (pkt.pts <= lastAudioPts && lastAudioPts > 0)
                    {
                        pkt.pts = pkt.dts = lastAudioPts + 1;
                    }
                    lastAudioPts = pkt.pts;
                }

                // 写入帧
                int ret = av_write_frame(m_formatCtx, &pkt);
                if (ret < 0) {
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"写入音频帧失败: %d\n", ret);
                }
                if (ret >= 0)
                {
                    m_bytesWritten.fetch_add(pkt.size, std::memory_order_relaxed);
                    if (m_RecordSizeCallback)
                    {
                        auto now = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSizeCallbackTime).count()
                            >= m_sizeCallbackIntervalMs)
                        {
                            m_lastSizeCallbackTime = now;
                            double mb = m_bytesWritten.load(std::memory_order_relaxed) / (1024.0 * 1024.0);
                            m_RecordSizeCallback(mb);
                        }
                    }
                }


                av_packet_unref(&pkt);
                av_packet_unref(&audioPacket.pkt);
                m_audioQueue.pop();

                audioFrameCount++;
                wrote_frame = true;

                DEBUG_CONSOLE_FMT(ConsoleHandle, L"写入音频帧 #%d, PTS: %lld\n",
                    audioFrameCount, cur_pts_a);
            }
        }

        // 如果队列增长过快，控制处理速度
        if (videoQueueSize > 50 || audioQueueSize > 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"混合线程结束，处理了 %d 个视频帧和 %d 个音频帧\n",
        videoFrameCount, audioFrameCount);
}

#ifdef TARGET_WIN10
void ScreenRecorder::DXGIVideoFrameEncode()
{
    char* captureBuffer = new char[m_width * m_height * 4];//图像数据接受缓冲区
    int captureSize = 0;//实际处理捕获到的图像数据大小
    int64_t currentTime = 0;//当前捕获时间
    int64_t pts = 0;//当前帧pts
    int64_t frameDuration = 1000000 / m_frameRate;//每帧持续时间
    int64_t LastFrameTime = 0;//上一帧的时间
    int ret = 0;//错误返回值
    AVFrame* filterFrame = av_frame_alloc();
    av_frame_get_buffer(filterFrame, 0);

    AVFrame* videoFrame;//存储帧
    videoFrame = av_frame_alloc();
    videoFrame->width = m_width;
    videoFrame->height = m_height;
    videoFrame->format = m_CodecCtx->pix_fmt;
    av_frame_get_buffer(videoFrame, 0);

    while (m_IsRecording)
    {
        //如果暂停了录制，则等待标志复原
        while (m_isPause)
        {
            DB(ConsoleHandle, L"DXGIVideoFrameEncode：暂停编码");
            std::unique_lock<std::mutex> lock(m_muxPause);
            m_cvPause.wait(lock);
            DB(ConsoleHandle, L"DXGIVideoFrameEncode：被唤醒");
        }

        if (m_dxgiCapture && m_dxgiCapture->CaptureImage(captureBuffer, &captureSize))
        {
            // 验证捕获数据有效性
            if (captureSize <= 0 || captureSize != m_width * m_height * 4)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                continue;
            }

            //如果捕获到一帧图像
            currentTime = av_gettime() - m_baseTime;
            if (m_VideoFmt == ScreenRecorder::FLV)
            {// FLV使用毫秒级时间戳
                pts = currentTime / 1000;
            }
            else
            {//使用实际捕获时间计算pts
                pts = av_rescale_q(
                    currentTime,
                    AVRational{ 1,1000000 },
                    m_formatCtx->streams[m_VideoStreamIndex]->time_base
                );
            }
            AVPacket packet;//实际帧数据
            av_init_packet(&packet);

            //定义编码器编码需要的帧格式
            av_frame_make_writable(videoFrame);
            videoFrame->pts = pts;//更新帧的显示时间戳

            //将帧构造为FFmpeg需要的帧格式（实际数据格式不变）
            uint8_t* srcBuffer[4] = { reinterpret_cast<uint8_t*>(captureBuffer),0,0,0 };
            int srcLineSize[4] = { m_width * 4,0,0,0 };
            bool scaleSuccess = false;
            {
                std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);
                BOOL exceptionOccurred = FALSE;
                auto oldHandler = SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* exInfo) -> LONG
                    {
                        return EXCEPTION_EXECUTE_HANDLER;
                    });
                try
                {
                    sws_scale(m_swsContext, srcBuffer, srcLineSize,
                        0, videoFrame->height, videoFrame->data, videoFrame->linesize);
                    scaleSuccess = true;
                }
                catch (...)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"捕获到异常，跳过当前帧");
                }
                SetUnhandledExceptionFilter(oldHandler);
            }
            if (!scaleSuccess) // 如果格式转换失败，跳过此帧
            {
                av_packet_unref(&packet);
                continue;
            }

            bool IsHasFilterFrame = false;
            if (m_bool_IsTextFilterActive)
            {//如果需要加入文字水印滤镜
                av_frame_unref(filterFrame);
                ret = av_buffersrc_add_frame_flags(m_AVFCtx_TextIn, videoFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"将帧加入滤镜器失败!:%s", LARSC::c2w(errorBuf));
                }
                ret = av_buffersink_get_frame(m_AVFCtx_TextOut, filterFrame);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"从滤镜器拿出一帧滤镜帧失败!:%s", LARSC::c2w(errorBuf));
                }
                IsHasFilterFrame = ret < 0 ? false : true;
            }

            //编码视频帧
            {
                std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);//如果没有更新窗口录制区域，则拿取锁执行编码帧
                if (m_bool_IsTextFilterActive && IsHasFilterFrame)//如果当前需要给视频增加水印
                    ret = avcodec_send_frame(m_CodecCtx, filterFrame);
                else
                    ret = avcodec_send_frame(m_CodecCtx, videoFrame);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"解码一帧视频帧数据失败!:%s", LARSC::c2w(errorBuf));
                    continue;
                }
                else if (ret == AVERROR(EAGAIN))  continue;
                ret = avcodec_receive_packet(m_CodecCtx, &packet);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取编码后的一帧视频帧数据失败!:%s", LARSC::c2w(errorBuf));
                    continue;
                }
                else if (ret == AVERROR(EAGAIN))  continue;
            }

            //存入视频帧队列，设定编码后的数据包流信息
            VideoPacket VideoPacket = { 0 };
            av_packet_ref(&VideoPacket.pkt, &packet);
            if (m_VideoFmt == ScreenRecorder::FLV)
            {
                VideoPacket.pkt.pts = VideoPacket.pkt.dts = pts;
                VideoPacket.pkt.duration = 1000 / m_frameRate;
            }
            else
            {
                VideoPacket.pkt.pts = VideoPacket.pkt.dts = pts;
                VideoPacket.pkt.duration = 1;
            }
            VideoPacket.pkt.stream_index = m_VideoStreamIndex;
            {
                std::lock_guard<std::mutex> lock(m_videoMutex);
                m_videoQueue.push(VideoPacket);
            }

            //若与上一个帧的间隔时间小于计算当前帧率下每帧的持续时间，则控制帧率，静默当前线程
            currentTime = av_gettime() - m_baseTime;
            if (currentTime - LastFrameTime < frameDuration) {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(frameDuration - (currentTime - LastFrameTime)
                    ));
            }

            //更新当前帧时间，并释放资源
            {
                std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);
                LastFrameTime = av_gettime() - m_baseTime;
                av_packet_unref(&packet);
            }
        }

    }

    av_frame_free(&videoFrame);
    delete[] captureBuffer;
    av_frame_free(&filterFrame);
}
#elif defined TARGET_WIN7
void ScreenRecorder::GDIScreenVideoFrameEncode()
{
    AVFrame* frame = av_frame_alloc();//原始捕获帧缓冲区
    AVFrame* filterFrame = av_frame_alloc();//滤镜帧缓冲区
    int64_t currentTime = 0;//当前捕获时间
    int64_t pts = 0;//当前帧pts
    int64_t frameDuration = 1000000 / m_frameRate;//每帧持续时间
    int64_t LastFrameTime = 0;//上一帧的时间
    int ret = 0;//错误返回值
    while (m_IsRecording)
    {
        if (m_ScreenCapture && m_ScreenCapture->CaptureFrame(frame))
        {
            //如果暂停了录制，则等待标志复原
            while (m_isPause)
            {
                DB(ConsoleHandle, L"GDIScreenVideoFrameEncode：暂停编码");
                std::unique_lock<std::mutex> lock(m_muxPause);
                m_cvPause.wait(lock);
                DB(ConsoleHandle, L"GDIScreenVideoFrameEncode：被唤醒");
            }

            //如果捕获到一帧图像
            //获取当前的pts
            currentTime = av_gettime() - m_baseTime;
            if (m_VideoFmt == ScreenRecorder::FLV)
            {   // FLV使用毫秒级时间戳
                pts = currentTime / 1000;
            }
            else
            {//使用实际捕获时间计算pts
                pts = av_rescale_q(
                    currentTime,
                    AVRational{ 1,1000000 },
                    m_formatCtx->streams[m_VideoStreamIndex]->time_base
                );
            }

            AVFrame* videoFrame;//存储帧
            AVPacket packet;//实际帧数据
            av_init_packet(&packet);

            //定义编码器编码需要的帧格式
            videoFrame = av_frame_alloc();
            videoFrame->width = m_width;
            videoFrame->height = m_height;
            videoFrame->format = m_CodecCtx->pix_fmt;
            videoFrame->pts = pts;//更新帧的显示时间戳
            av_frame_get_buffer(videoFrame, 32);

            // 高频率选项下动态进行录制区域改变时，因为高帧率下区频繁域改变时会导致编码器内部状态频繁的发生改变或暂时异常
            // (硬件编码器还没有反映过来)，但过程很短，不需要暂停，但继续编码，当前帧可能在这里引发系统级异常，
            // 这对实际录制效果影响约等于0， 所以我在这里忽略这个异常并丢弃当前帧
            bool scaleSuccess = false;
            {
                std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);
                BOOL exceptionOccurred = FALSE;
                auto oldHandler = SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* exInfo) -> LONG
                    {
                        return EXCEPTION_EXECUTE_HANDLER;
                    });
                try
                {
                    sws_scale(m_swsContext, frame->data, frame->linesize,
                        0, videoFrame->height, videoFrame->data, videoFrame->linesize);
                    scaleSuccess = true;
                }
                catch (...)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"捕获到异常，跳过当前帧");
                }
                SetUnhandledExceptionFilter(oldHandler);

            }
            if (!scaleSuccess) // 如果格式转换失败，跳过此帧
            {
                av_frame_free(&videoFrame);
                av_packet_unref(&packet);
                continue;
            }

            bool IsHasFilterFrame = false;
            if (m_bool_IsTextFilterActive)
            {//如果需要加入文字水印滤镜
                av_frame_unref(filterFrame);
                ret = av_buffersrc_add_frame_flags(m_AVFCtx_TextIn, videoFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"将帧加入滤镜器失败!:%s", LARSC::c2w(errorBuf));
                }
                ret = av_buffersink_get_frame(m_AVFCtx_TextOut, filterFrame);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"从滤镜器拿出一帧滤镜帧失败!:%s", LARSC::c2w(errorBuf));
                }
                IsHasFilterFrame = ret < 0 ? false : true;
            }

            //编码视频帧
            {
                std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);//如果没有更新窗口录制区域，则拿取锁执行编码帧
                if (IsHasFilterFrame)
                    ret = avcodec_send_frame(m_CodecCtx, filterFrame);
                else
                    ret = avcodec_send_frame(m_CodecCtx, videoFrame);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"解码一帧视频帧数据失败!:%s", LARSC::c2w(errorBuf));
                    //将错误交给错误处理器判断是否需要回退软件编码
                    if (m_VideoEncoder != "" && m_EncoderFailureHandler.ShouldFallbackToSoftware(ret, errorBuf))
                    {
                        m_EncoderFailureHandler.SetFallbackActive(true);//设置录制状态为回退状态
                        DEBUG_CONSOLE_STR(ConsoleHandle, 
                            L"[视频编码线程]:硬件编码器异常，唤醒重启线程开始重启为软件编码录制");
                        std::lock_guard<std::mutex> lock(m_Mutex_RestartRecord);
                        m_Bool_RestartRecord.store(true);
                        m_CV_RestartRecord.notify_all();//通知唤醒重启线程
                        break;//结束循环编码
                    }
                    continue;
                }
                else if (ret == AVERROR(EAGAIN))
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"Frame EAGAIN.....");
                    continue;
                }
                ret = avcodec_receive_packet(m_CodecCtx, &packet);
                if (ret == AVERROR(EAGAIN))
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"Packet EAGAIN.....");
                }
                else if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取编码后的一帧视频帧数据失败!:%s", LARSC::c2w(errorBuf));
                    av_frame_unref(frame);
                    av_frame_unref(videoFrame);
                    continue;
                }
            }

            //存入视频帧队列，设定编码后的数据包流信息
            VideoPacket VideoPacket = { 0 };
            av_packet_ref(&VideoPacket.pkt, &packet);
            if (m_VideoFmt == ScreenRecorder::FLV)
            {
                VideoPacket.pkt.pts = VideoPacket.pkt.dts = pts;
                VideoPacket.pkt.duration = 1000 / m_frameRate;
            }
            else
            {
                VideoPacket.pkt.pts = VideoPacket.pkt.dts = pts;
                VideoPacket.pkt.duration = 1;
            }
            VideoPacket.pkt.stream_index = m_VideoStreamIndex;
            {
                std::lock_guard<std::mutex> lock(m_videoMutex);
                m_videoQueue.push(VideoPacket);
            }

            //若与上一个帧的间隔时间小于计算当前帧率下每帧的持续时间，则控制帧率，静默当前线程
            currentTime = av_gettime() - m_baseTime;
            if (currentTime - LastFrameTime < frameDuration)
            {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(frameDuration - (currentTime - LastFrameTime)
                    ));
            }

            //更新当前帧时间，并释放资源
            {
                std::lock_guard<std::mutex> lock(m_UpdateRecordAreaMutex);
                LastFrameTime = av_gettime() - m_baseTime;
                av_frame_free(&videoFrame);
                av_frame_unref(frame);
                av_packet_unref(&packet);
            }
        }
    }
    av_frame_free(&frame);
}
#endif

void ScreenRecorder::GdiVideoFrameEncode()
{
    SwsContext* m_SwsCtx_Scale = nullptr;//窗口捕获时，窗口尺寸发生变化时需要的缩放器上下文
    struct SwsCtxParam
    {
        int srcWidth;
        int srcHeight;
        AVPixelFormat srcFormat;
        int dstWidth;
        int dstHeight;
        AVPixelFormat dstFormat;
        int flags;
    };
    SwsCtxParam swsParam{ 0 };
    AVFrame* ScaleVideoFrame = av_frame_alloc();//缩放后的帧缓冲区
    AVFrame* EncodeFrame = av_frame_alloc();    //格式转换后的帧缓冲区（最终编码帧）
    AVFrame* VideoFrame = av_frame_alloc();//图像数据接受缓冲区
    int captureSize = 0;//实际处理捕获到的图像数据大小
    int64_t currentTime = 0;//当前捕获时间
    int64_t pts = 0;//当前帧pts
    int64_t frameDuration = 1000000 / m_frameRate;//每帧持续时间
    int64_t LastFrameTime = 0;//上一帧的时间
    int ret = 0;//错误返回值
    auto it = m_Map_RecordCallBackFunc.find(WindowRecord_WindowMinimalAndClose);
    bool CallBackRun = (it != m_Map_RecordCallBackFunc.end()) ? true : false;
    while (m_IsRecording)
    {
        //如果暂停了录制，则等待标志复原
        while (m_isPause)
        {
            DB(ConsoleHandle, L"GdiVideoFrameEncode：暂停编码");
            std::unique_lock<std::mutex> lock(m_muxPause);
            m_cvPause.wait(lock);
            DB(ConsoleHandle, L"GdiVideoFrameEncode：被唤醒");
        }

        // 如果设置了窗口异常情况回调
        if (CallBackRun)
        {
            bool isInvalid = !::IsWindow(m_Hwnd);
            bool isMinimized = ::IsIconic(m_Hwnd);
            bool isNotVisible = !::IsWindowVisible(m_Hwnd);
            if (isInvalid || isMinimized || isNotVisible)
            {//进行窗口最小化或关闭或无效时的回调处理
                if (isInvalid)
                    DB(ConsoleHandle, L"[视频编码线程]：_____窗口无效！");
                if (isMinimized)
                    DB(ConsoleHandle, L"[视频编码线程]：_____窗口最小化！");
                if (isNotVisible)
                    DB(ConsoleHandle, L"[视频编码线程]：_____窗口不可见！");
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[视频编码线程]：录制窗口%p异常，准备执行设置的回调",
                    (void*)m_Hwnd);
                it->second();
                break;
            }
        }

        //获取GDI捕获器捕获的帧
        if (m_gdiCapture && m_gdiCapture->CaptureFrame(VideoFrame))
        {//如果捕获到一帧图像
            //获取当前的pts（使用实际捕获时间计算pts）
            currentTime = av_gettime() - m_baseTime;
            pts = av_rescale_q(
                currentTime,
                AVRational{ 1,1000000 },
                m_formatCtx->streams[m_VideoStreamIndex]->time_base
            );
            AVPacket packet;//实际帧数据
            av_init_packet(&packet);


            //编码视频帧
            //如果窗口尺寸发生突变(这里以实际捕获的帧为基准,创建缩放上下文，使适配编码器)
            bool NeedScaled = false;
            if (m_CodecCtx->width != VideoFrame->width || m_CodecCtx->height != VideoFrame->height)
            {
                //分配缩放帧缓冲区
                ScaleVideoFrame->width = m_CodecCtx->width;
                ScaleVideoFrame->height = m_CodecCtx->height;
                ScaleVideoFrame->format = AV_PIX_FMT_BGRA;
                av_frame_get_buffer(ScaleVideoFrame, 32);

                DEBUG_CONSOLE_STR(ConsoleHandle, L"[视频编码线程]:窗口尺寸突变，开始执行缩放");
                NeedScaled = true;
                if (!m_SwsCtx_Scale)//如果之前还没创建过缩放转换器上下文（第一次窗口尺寸突变）
                {
                    //创建缩放参数
                    swsParam.srcWidth = VideoFrame->width;
                    swsParam.srcHeight = VideoFrame->height;
                    swsParam.srcFormat = AV_PIX_FMT_BGRA;
                    swsParam.dstWidth = m_CodecCtx->width;
                    swsParam.dstHeight = m_CodecCtx->height;
                    swsParam.dstFormat = AV_PIX_FMT_BGRA;
                    swsParam.flags = SWS_BICUBIC;

                    //创建缩放转换器上下文
                    m_SwsCtx_Scale = sws_getContext(
                        swsParam.srcWidth, swsParam.srcHeight,
                        swsParam.srcFormat,
                        swsParam.dstWidth, swsParam.dstHeight,
                        swsParam.dstFormat,
                        swsParam.flags, NULL, NULL, NULL
                    );
                }
                else if (swsParam.srcWidth != VideoFrame->width ||
                    swsParam.srcHeight != VideoFrame->height)//之前创建过转换器上下文(窗口尺寸再次发生变化)，需要创建新的
                {
                    sws_freeContext(m_SwsCtx_Scale);
                    m_SwsCtx_Scale = nullptr;
                    //创建缩放参数
                    swsParam.srcWidth = VideoFrame->width;
                    swsParam.srcHeight = VideoFrame->height;
                    swsParam.srcFormat = AV_PIX_FMT_BGRA;
                    swsParam.dstWidth = m_CodecCtx->width;
                    swsParam.dstHeight = m_CodecCtx->height;
                    swsParam.dstFormat = AV_PIX_FMT_BGRA;
                    swsParam.flags = SWS_BICUBIC;

                    //创建缩放转换器上下文
                    m_SwsCtx_Scale = sws_getContext(
                        swsParam.srcWidth, swsParam.srcHeight,
                        swsParam.srcFormat,
                        swsParam.dstWidth, swsParam.dstHeight,
                        swsParam.dstFormat,
                        swsParam.flags, NULL, NULL, NULL
                    );
                }

                //进行帧的缩放处理
                int nb = sws_scale(
                    m_SwsCtx_Scale,
                    (const uint8_t* const*)VideoFrame->data, (const int*)VideoFrame->linesize,
                    0, VideoFrame->height,
                    (uint8_t* const*)ScaleVideoFrame->data, (const int*)ScaleVideoFrame->linesize
                );
                if (nb < 0)
                {
                    char errorBuf[256]{ 0 };
                    av_strerror(nb, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[视频编码线程]:窗口尺寸变化时，帧的缩放失败！错误：%s", LARSC::c2w(errorBuf));
                }
            }

            //创建最终编码帧
            EncodeFrame->width = m_CodecCtx->width;
            EncodeFrame->height = m_CodecCtx->height;
            EncodeFrame->format = m_CodecCtx->pix_fmt;
            av_frame_get_buffer(EncodeFrame, 32);

            //进行格式转换
            int nb = 0;
            if (NeedScaled)
            {
                //进行格式转换
                nb = sws_scale(m_swsContext,
                    ScaleVideoFrame->data,
                    ScaleVideoFrame->linesize,
                    0,
                    m_CodecCtx->height,
                    EncodeFrame->data,
                    EncodeFrame->linesize);
            }
            else
            {
                //进行格式转换
                nb = sws_scale(m_swsContext,
                    VideoFrame->data,
                    VideoFrame->linesize,
                    0,
                    m_CodecCtx->height,
                    EncodeFrame->data,
                    EncodeFrame->linesize);
            }
            if (nb < 0)
            {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[视频编码线程]:格式转换失败:%s", LARSC::c2w(errorBuf));
                continue;
            }

            //若进行了缩放转换，则编码缩放帧
            if (NeedScaled)
            {
                ret = avcodec_send_frame(m_CodecCtx, EncodeFrame);
            }
            else
            {
                ret = avcodec_send_frame(m_CodecCtx, EncodeFrame);
            }
            if (ret < 0)
            {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[视频编码线程]:编码一帧视频帧数据失败!:%s", LARSC::c2w(errorBuf));
                //将错误交给错误处理器判断是否需要回退软件编码
                if (m_VideoEncoder != "" && m_EncoderFailureHandler.ShouldFallbackToSoftware(ret, errorBuf))
                {
                    m_EncoderFailureHandler.SetFallbackActive(true);//设置录制状态为回退状态
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[视频编码线程]:硬件编码器异常，唤醒重启线程开始重启为软件编码录制");
                    std::lock_guard<std::mutex> lock(m_Mutex_RestartRecord);
                    m_Bool_RestartRecord.store(true);
                    m_CV_RestartRecord.notify_all();//通知唤醒重启线程
                    break;//结束循环编码
                }
                continue;
            }
            else if (ret == AVERROR(EAGAIN))  continue;

            //打包成数据包
            ret = avcodec_receive_packet(m_CodecCtx, &packet);
            if (ret < 0)
            {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[视频编码线程]:获取编码后的一帧视频帧数据失败!:%s", LARSC::c2w(errorBuf));
                continue;
            }

            //存入视频帧队列，设定编码后的数据包流信息
            VideoPacket VideoPacket = { 0 };
            av_packet_ref(&VideoPacket.pkt, &packet);
            VideoPacket.pkt.pts = VideoPacket.pkt.dts = pts;
            VideoPacket.pkt.duration = 1;
            VideoPacket.pkt.stream_index = m_VideoStreamIndex;
            {
                std::lock_guard<std::mutex> lock(m_videoMutex);
                m_videoQueue.push(VideoPacket);
            }

            //若与上一个帧的间隔时间小于计算当前帧率下每帧的持续时间，则控制帧率，静默当前线程
            currentTime = av_gettime() - m_baseTime;
            if (currentTime - LastFrameTime < frameDuration) {
                std::this_thread::sleep_for(
                    std::chrono::microseconds(frameDuration - (currentTime - LastFrameTime)
                    ));
            }

            //更新当前帧时间，并释放资源
            LastFrameTime = av_gettime() - m_baseTime;
            av_frame_unref(VideoFrame);
            av_frame_unref(ScaleVideoFrame);
            av_frame_unref(EncodeFrame);
            av_packet_unref(&packet);
        }
    }

    if (m_SwsCtx_Scale)
    {
        sws_freeContext(m_SwsCtx_Scale);
    }
    av_frame_free(&VideoFrame);
    av_frame_free(&ScaleVideoFrame);
    av_frame_free(&EncodeFrame);
}

void ScreenRecorder::AudioFrameEncode()
{
    //计算每一帧的时长
    const int64_t FrameDuationTime = (int64_t)(m_pCodecCtx_Audio->frame_size * 1000000) / m_pCodecCtx_Audio->sample_rate;
    AVFrame* pAudioBuffer = av_frame_alloc();//音频缓冲区
    pAudioBuffer->nb_samples = m_VideoFmt == MP4 ? 1024 : 1012;
    av_frame_get_buffer(pAudioBuffer, 0);
    AVPacket packet = { 0 };
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    int captureSize = 0;//获取捕获的大小
    int64_t currentTime = 0;//当前时间
    int64_t nextFrameTime = av_gettime() - m_baseTime;//下一帧的时间
    int64_t pts = 0;//当前pts的时间
    int64_t lastPts = 0;
    bool Captured = false;

    while (m_IsRecording)
    {
        //如果暂停了录制，则等待标志复原
        while (m_isPause)
        {
            DB(ConsoleHandle, L"AudioFrameEncode：暂停编码");
            std::unique_lock<std::mutex> lock(m_muxPause);
            m_cvPause.wait(lock);
            DB(ConsoleHandle, L"AudioFrameEncode：被唤醒");
        }

        //从捕获器中拿取捕获的音频数据
        if (m_audioCapture->CaptureFrame(reinterpret_cast<void**>(&pAudioBuffer), &captureSize))
        {
            Captured = true;
        }
        if (Captured)
        {
            //对捕获的音频音量进行调整（考虑不同格式）
            if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_FLTP)
            {
                int Channels = pAudioBuffer->channels;
                int Samples = pAudioBuffer->nb_samples;
                for (int ch = 0; ch < Channels; ch++)
                {
                    //AV_SAMPLE_FMT_FLTP音频数据实际是float类型的数据，进行转换
                    float* audioData = reinterpret_cast<float*>(pAudioBuffer->data[ch]);
                    for (int i = 0; i < Samples; i++)
                    {
                        //调整音量值
                        audioData[i] *= m_SystemAudioVolume;
                        //放置超出范围
                        if (audioData[i] > 1.0f)audioData[i] = 1.0f;
                        if (audioData[i] < -1.0f)audioData[i] = -1.0f;
                    }
                }
            }
            else if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_S16)
            {
                int Channels = pAudioBuffer->channels;
                int Samples = pAudioBuffer->nb_samples;
                //AV_SAMPLE_FMT_S16实际为int16_t类型数据，进行转换
                int16_t* audioData = reinterpret_cast<int16_t*>(pAudioBuffer->data[0]);
                for (int i = 0; i < Samples * Channels; i++)
                {
                    //获取调整后的音量值(先转成int32_t防止内存溢出)
                    int32_t sample = static_cast<int32_t>(static_cast<float>(audioData[i]) * m_SystemAudioVolume);
                    //防止音量调整过大或过小导致失真
                    if (sample > 32767) sample = 32767;
                    if (sample < -32768) sample = -32768;
                    audioData[i] = static_cast<int16_t>(sample);//转回int16_t存储
                }
            }

            currentTime = av_gettime() - m_baseTime;//上一帧音频完整处理的时间点
            if (currentTime >= nextFrameTime)//如果当前时间点大于或等于这一帧应该处理的时间点
            {
                if (m_VideoFmt == ScreenRecorder::FLV)
                {// 直接计算毫秒级时间戳
                    pts = nextFrameTime / 1000; // 微秒转毫秒
                }
                else
                { //计算当前帧的pts（音频数据帧数据捕获一般是恒定的，采用固定计数计算pts）
                    pts = av_rescale_q(
                        nextFrameTime,
                        AVRational{ 1,1000000 },
                        m_pCodecCtx_Audio->time_base
                    );
                }
                if (pts <= lastPts) {
                    pts = lastPts + m_pCodecCtx_Audio->frame_size;
                }
                //编码
                pAudioBuffer->pts = pts;

                int ret = avcodec_send_frame(m_pCodecCtx_Audio, pAudioBuffer);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码一帧音频帧失败:%s\n", errorBuf);
                    continue;
                }
                av_packet_unref(&packet);
                ret = avcodec_receive_packet(m_pCodecCtx_Audio, &packet);
                if (ret < 0)
                {
                    av_strerror(ret, errorBuf, 256);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取一帧音频帧数据包失败:%s\n", errorBuf);
                    continue;
                }

                AudioPacket audioPacket = { 0 };
                av_packet_ref(&audioPacket.pkt, &packet);
                audioPacket.pkt.stream_index = m_AudioStreamIndex;
                audioPacket.dts = audioPacket.pts = pts;
                if (m_VideoFmt == ScreenRecorder::FLV)
                {// 设置帧持续时间(毫秒)
                    audioPacket.pkt.duration = (m_pCodecCtx_Audio->frame_size * 1000) / m_pCodecCtx_Audio->sample_rate;
                }
                {
                    std::lock_guard<std::mutex> lock(m_audioMutex);
                    m_audioQueue.push(audioPacket);
                }
                lastPts = pts;

                //根据当前帧处理完毕时间，决定追赶或静默
                nextFrameTime += FrameDuationTime;//当前帧的下一帧时间点
                if (currentTime - FrameDuationTime > nextFrameTime) {//如果当前时间点大于了帧的下两帧时间点
                    nextFrameTime = currentTime;//进行追赶，保证同步(丢弃下一帧，从下两帧开始处理)
                }
                if (currentTime < nextFrameTime) {
                    std::this_thread::sleep_for(std::chrono::microseconds(nextFrameTime - currentTime));
                }
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));//等待一会
        }
    }
}

void ScreenRecorder::AudioMicroMixEncode()
{
    // 初始化
    int64_t currentFrameTime = 0;
    int64_t pts = 0, last_pts = 0;
    AVFrame* audioFrame = av_frame_alloc();  // 系统音频帧
    AVFrame* microFrame = av_frame_alloc();  // 麦克风帧
    AVFrame* mixedFrame = av_frame_alloc();  // 混合后的帧
    int64_t frameDurationTime = (int64_t)(m_pCodecCtx_Audio->frame_size * 1000000) / m_pCodecCtx_Audio->sample_rate;

    AVPacket packet = { 0 };
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    int audioCapSize = 0, microCapSize = 0;
    int64_t nextFrameTime = av_gettime() - m_baseTime;
    int64_t encodeCompleteTime = 0;

    while (m_IsRecording)
    {
        //如果暂停了录制，则等待标志复原
        while (m_isPause)
        {
            DB(ConsoleHandle, L"AudioMicroMixEncode：暂停编码");
            std::unique_lock<std::mutex> lock(m_muxPause);
            m_cvPause.wait(lock);
            DB(ConsoleHandle, L"AudioMicroMixEncode：被唤醒");
        }

        // 捕获音频帧，注意每帧都要重新捕获以保持状态一致
        bool hasAudio = m_audioCapture->CaptureFrame(reinterpret_cast<void**>(&audioFrame), &audioCapSize);
        bool hasMicro = m_microCapture->CaptureMicroFrame(reinterpret_cast<void**>(&microFrame), &microCapSize);

        currentFrameTime = av_gettime() - m_baseTime;
        if (currentFrameTime >= nextFrameTime && (hasAudio || hasMicro))
        {
            // 计算PTS，根据格式区分处理方式
            if (m_VideoFmt == ScreenRecorder::FLV)
            {// FLV格式使用毫秒时间戳
                pts = nextFrameTime / 1000;  // 转换为毫秒
                if (last_pts >= pts)
                    pts = last_pts + 1;      // 确保递增
            }
            else
            {// 原有处理方式
                pts = av_rescale_q(
                    nextFrameTime,
                    AVRational{ 1,1000000 },
                    m_pCodecCtx_Audio->time_base
                );
                if (last_pts >= pts)
                    pts = last_pts + m_pCodecCtx_Audio->frame_size;
            }

            bool isSystemAudioSilent = true;
            if (hasAudio && audioFrame && audioFrame->data[0])
            {
                // 根据实际音频格式检测静音
                if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_FLTP)
                {
                    const float* samples = (const float*)audioFrame->data[0];
                    for (int i = 0; i < min(16, audioFrame->nb_samples); i++)
                    {
                        if (fabs(samples[i]) > 0.0001f)
                        {
                            isSystemAudioSilent = false;
                            break;
                        }
                    }
                }
                else if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_S16)
                {
                    const int16_t* samples = (const int16_t*)audioFrame->data[0];
                    for (int i = 0; i < min(16, audioFrame->nb_samples); i++)
                    {
                        if (abs(samples[i]) > 8) // 使用适合S16格式的阈值
                        {
                            isSystemAudioSilent = false;
                            break;
                        }
                    }
                }
            }

            // 准备用于输出的帧
            av_frame_unref(mixedFrame);

            if (isSystemAudioSilent || !hasAudio)
            {
                // 系统静音或无系统音频时，直接使用麦克风数据
                av_frame_ref(mixedFrame, microFrame);

                // 应用音量调整
                if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_FLTP)
                {
                    int channels = mixedFrame->channels;
                    int samples = mixedFrame->nb_samples;
                    for (int ch = 0; ch < channels; ch++)
                    {
                        float* data = (float*)mixedFrame->data[ch];
                        for (int i = 0; i < samples; i++)
                        {
                            data[i] *= m_MicroVolume;
                            if (data[i] > 1.0f) data[i] = 1.0f;
                            if (data[i] < -1.0f) data[i] = -1.0f;
                        }
                    }
                }
                else if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_S16)
                {// S16格式
                    int channels = mixedFrame->channels;
                    int samples = mixedFrame->nb_samples;
                    int16_t* audioData = (int16_t*)mixedFrame->data[0];
                    for (int i = 0; i < samples * channels; i++)
                    {
                        int32_t sample = (int32_t)(audioData[i] * m_MicroVolume);
                        if (sample > 32767) sample = 32767;
                        if (sample < -32768) sample = -32768;
                        audioData[i] = (int16_t)sample;
                    }
                }
            }
            else
            {
                // 混合系统音频和麦克风音频到新帧
                mixedFrame->nb_samples = m_pCodecCtx_Audio->frame_size;
                mixedFrame->format = m_pCodecCtx_Audio->sample_fmt;
                mixedFrame->sample_rate = m_pCodecCtx_Audio->sample_rate;

                // 替换 av_channel_layout_copy
                mixedFrame->channel_layout = m_pCodecCtx_Audio->channel_layout;
                mixedFrame->channels = m_pCodecCtx_Audio->channels;

                av_frame_get_buffer(mixedFrame, 0);

                // 执行混合操作
                if (m_VideoFmt == MP4 || m_VideoFmt == FLV)
                    MixMicroAudioFLTP((void**)&mixedFrame, (void**)&audioFrame, (void**)&microFrame);
                else if (m_VideoFmt == AVI)
                    MixMicroAudioS16((void**)&mixedFrame, (void**)&audioFrame, (void**)&microFrame);
            }

            // 设置完整帧属性
            mixedFrame->pts = pts;
            mixedFrame->sample_rate = m_pCodecCtx_Audio->sample_rate;
            mixedFrame->nb_samples = m_pCodecCtx_Audio->frame_size;
            mixedFrame->format = m_pCodecCtx_Audio->sample_fmt;
            mixedFrame->channel_layout = m_pCodecCtx_Audio->channel_layout;
            mixedFrame->channels = m_pCodecCtx_Audio->channels;

            // 编码帧，与MicroFrameEncode保持相同顺序和处理方式
            int ret = avcodec_send_frame(m_pCodecCtx_Audio, mixedFrame);
            if (ret < 0)
            {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码音频帧失败:%s\n", LARSC::c2w(errorBuf));
                continue;
            }
            ret = avcodec_receive_packet(m_pCodecCtx_Audio, &packet);
            if (ret < 0)
            {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"接收音频包失败:%s\n", errorBuf);
                continue;
            }

            // 处理音频包
            AudioPacket mixPacket;
            av_packet_ref(&mixPacket.pkt, &packet);
            av_packet_unref(&packet);  // 确保释放原始包
            mixPacket.pts = mixPacket.dts = pts;
            if (m_VideoFmt == ScreenRecorder::FLV)   // 根据格式设置不同的duration
                mixPacket.pkt.duration = (m_pCodecCtx_Audio->frame_size * 1000) / m_pCodecCtx_Audio->sample_rate;
            else
                mixPacket.pkt.duration = m_pCodecCtx_Audio->frame_size;
            mixPacket.pkt.stream_index = m_AudioStreamIndex;
            {// 添加到队列
                std::lock_guard<std::mutex> lock(m_audioMutex);
                m_audioQueue.push(mixPacket);
            }

            // 更新时间戳和下一帧时间
            last_pts = pts;
            nextFrameTime += frameDurationTime;
            encodeCompleteTime = av_gettime() - m_baseTime;

            // 时间调整和追赶逻辑，与MicroFrameEncode完全一致
            if (currentFrameTime - frameDurationTime > nextFrameTime)
            {
                nextFrameTime = currentFrameTime;
            }
            if (currentFrameTime < nextFrameTime)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(nextFrameTime - currentFrameTime));
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    av_frame_free(&mixedFrame);
}

void ScreenRecorder::MicroFrameEncode()
{
    int64_t currentFrameTime = 0;
    int64_t pts = 0, lastpts = 0;
    AVFrame* audioFrame = av_frame_alloc();
    int64_t frameDuraitonTime = (int64_t)(m_pCodecCtx_Audio->frame_size * 1000000) / m_pCodecCtx_Audio->sample_rate;

    AVPacket packet = { 0 };
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    int captureSize = 0;
    int64_t nextFrameTime = av_gettime() - m_baseTime;
    int64_t encodeCompleteTime = 0;

    while (m_IsRecording)
    {
        //如果暂停了录制，则等待标志复原
        while (m_isPause)
        {
            DB(ConsoleHandle, L"MicroFrameEncode：暂停编码");
            std::unique_lock<std::mutex> lock(m_muxPause);
            m_cvPause.wait(lock);
            DB(ConsoleHandle, L"MicroFrameEncode：被唤醒");
        }

        m_microCapture->CaptureMicroFrame(reinterpret_cast<void**>(&audioFrame), &captureSize);
        currentFrameTime = av_gettime() - m_baseTime;
        if (currentFrameTime >= nextFrameTime && captureSize != 0)
        {
            pts = av_rescale_q(//以固定时间间隔为准计算当前pts
                nextFrameTime,
                AVRational{ 1,1000000 },
                m_pCodecCtx_Audio->time_base
            );
            if (lastpts >= pts) {//保证单调递增（特殊格式要求）
                pts = lastpts + m_pCodecCtx_Audio->frame_size;
            }

            //根据不同格式调整音量大小
            if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_FLTP)
            {
                int Channels = audioFrame->channels;
                int Samples = audioFrame->nb_samples;
                for (int ch = 0; ch < Channels; ch++)
                {
                    float* audioData = reinterpret_cast<float*>(audioFrame->data[ch]);
                    for (int i = 0; i < Samples; i++)
                    {
                        audioData[i] *= m_MicroVolume;
                        if (audioData[i] > 1.0f)audioData[i] = 1.0f;
                        else if (audioData[i] < -1.0f)audioData[i] = -1.0f;
                    }
                }
            }
            else if (m_pCodecCtx_Audio->sample_fmt == AV_SAMPLE_FMT_S16)
            {
                // S16格式
                int channels = audioFrame->channels;
                int samples = audioFrame->nb_samples;
                int16_t* audioData = (int16_t*)audioFrame->data[0];

                for (int i = 0; i < samples * channels; i++) {
                    int32_t sample = (int32_t)(audioData[i] * m_MicroVolume);
                    // 防止超出范围
                    if (sample > 32767) sample = 32767;
                    if (sample < -32768) sample = -32768;
                    audioData[i] = (int16_t)sample;
                }
            }

            //准备帧结构
            audioFrame->pts = pts;
            audioFrame->sample_rate = m_pCodecCtx_Audio->sample_rate;
            audioFrame->nb_samples = m_pCodecCtx_Audio->frame_size;
            audioFrame->format = m_pCodecCtx_Audio->sample_fmt;
            audioFrame->channel_layout = m_pCodecCtx_Audio->channel_layout;
            audioFrame->channels = m_pCodecCtx_Audio->channels;

            //编码帧
            int ret = avcodec_send_frame(m_pCodecCtx_Audio, audioFrame);
            if (ret < 0) {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码一帧音频帧失败:%s\n", errorBuf);
                continue;
            }
            ret = avcodec_receive_packet(m_pCodecCtx_Audio, &packet);
            if (ret < 0) {
                av_strerror(ret, errorBuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"编码一帧音频帧失败:%s\n", errorBuf);
                continue;
            }

            //将包加入到音视频同步处理队列中
            AudioPacket MicroPacket;
            av_packet_ref(&MicroPacket.pkt, &packet);
            av_packet_unref(&packet);
            MicroPacket.pkt.dts = MicroPacket.pkt.pts = pts;
            MicroPacket.pkt.duration = m_pCodecCtx_Audio->frame_size;
            MicroPacket.pkt.stream_index = m_AudioStreamIndex;
            {
                std::lock_guard<std::mutex> lock(m_audioMutex);
                m_audioQueue.push(MicroPacket);
            }

            //更新上一帧pts，和下一帧的处理时间点，并决定是否追赶
            lastpts = pts;
            nextFrameTime += frameDuraitonTime;
            encodeCompleteTime = av_gettime() - m_baseTime;
            if (currentFrameTime - frameDuraitonTime > nextFrameTime) {
                nextFrameTime = currentFrameTime;
            }
            if (currentFrameTime < nextFrameTime) {
                std::this_thread::sleep_for(std::chrono::microseconds(nextFrameTime - currentFrameTime));
            }
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void ScreenRecorder::MixMicroAudioS16(void** dstFrameData, void** srcAudioData, void** srcMicroData)
{
    AVFrame* mixedFrame = (AVFrame*)*dstFrameData;
    AVFrame* audioFrame = (AVFrame*)*srcAudioData;
    AVFrame* microFrame = (AVFrame*)*srcMicroData;

    // 混合比例
    const float audioVolume = m_SystemAudioVolume;
    const float microVolume = m_MicroVolume;

    // 获取数据指针
    int16_t* mixedData = (int16_t*)mixedFrame->data[0];
    int16_t* audioData = (int16_t*)audioFrame->data[0];
    int16_t* microData = (int16_t*)microFrame->data[0];

    // 获取通道数和样本数
    int channels = mixedFrame->channels;
    int samples = mixedFrame->nb_samples;

    // 确保样本数不超过源帧 (修复重复声明)
    samples = (audioFrame->nb_samples < samples) ? audioFrame->nb_samples : samples;
    samples = (microFrame->nb_samples < samples) ? microFrame->nb_samples : samples;

    // 逐样本混合
    for (int i = 0; i < samples * channels; i++)
    {
        // 加权混合并防止溢出
        int32_t mixed = (int32_t)(audioData[i] * audioVolume) +
            (int32_t)(microData[i] * microVolume);

        // 限制在16位有符号整数范围内
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;

        mixedData[i] = (int16_t)mixed;
    }
}

void ScreenRecorder::MixMicroAudioFLTP(void** dstFrameData, void** srcAudioData, void** srcMicroData)
{
    AVFrame* mixedFrame = (AVFrame*)*dstFrameData;
    AVFrame* audioFrame = (AVFrame*)*srcAudioData;
    AVFrame* microFrame = (AVFrame*)*srcMicroData;

    // 混合比例 - 可根据需要调整
    const float audioVolume = m_SystemAudioVolume;  // 系统音频音量
    const float microVolume = m_MicroVolume;  // 麦克风音量

    // 平面格式 - 每个通道有独立数组
    int channels = mixedFrame->channels;
    int samples = mixedFrame->nb_samples;

    // 确保样本数不超过源帧
    samples = (audioFrame->nb_samples < samples) ? audioFrame->nb_samples : samples;
    samples = (microFrame->nb_samples < samples) ? microFrame->nb_samples : samples;

    // 对每个通道分别处理
    for (int ch = 0; ch < channels; ch++)
    {
        float* mixedData = (float*)mixedFrame->data[ch];
        float* audioData = (float*)audioFrame->data[ch];
        float* microData = (float*)microFrame->data[ch];

        // 逐样本混合
        for (int i = 0; i < samples; i++)
        {
            // 加权混合
            float mixed = audioData[i] * audioVolume +
                microData[i] * microVolume;

            // 限制在浮点音频范围内(-1.0到1.0)
            if (mixed > 1.0f) mixed = 1.0f;
            if (mixed < -1.0f) mixed = -1.0f;

            mixedData[i] = mixed;
        }
    }
}

bool ScreenRecorder::RestartRecordThread()
{
    //当重启标志为假，挂起重启线程
    while (!m_Bool_RestartRecord.load() && m_IsRecording)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:重启线程被挂起");
        std::unique_lock<std::mutex> lock(m_Mutex_RestartRecord);
        m_CV_RestartRecord.wait(lock, [this]()
            {
                return (m_Bool_RestartRecord.load()) || (!m_IsRecording);
            });
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:重启线程执行");

    //判断是否录制完毕唤醒的还是需要重启唤醒的线程
    if ((!m_IsRecording) && !(m_Bool_RestartRecord.load()))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:录制结束，重启线程结束");
        return false;//返回false代表没有执行一次完整的重启
    }

    //需要进行重启，开始停止所有录制的线程
    if (!StopRecordToRestart())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:重启未能成功，停止录制失败");
        return false;
    }
    CleanCaptureInterface();//清理捕获器资源

    //获取当前的录制模式(全屏录制？选区录制？声音录制？应用窗口录制?)
    if (m_Bool_WindowRecord)
    {//应用窗口句柄录制
        if (!m_gdiCapture)
        { //创建GDI捕获器
            m_gdiCapture = new HandleCapture(m_Hwnd, m_frameRate);
            if (!m_gdiCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建GDI捕获器失败");
                return false;
            }
        }
        if (!m_microCapture)
        {//创建FFmpeg麦克风音频捕获器
            m_microCapture = (
                ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
                new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                    (MircroFmt)m_VideoFmt) :
                nullptr
                );
        }
    }
    else if (m_Bool_OnlyAudioRecord)
    {//声音录制

#ifdef TARGET_WIN10
        if (!m_dxgiCapture)
        { 
            m_dxgiCapture = new DXGICapture(m_width, m_height, m_frameRate);
            if (!m_dxgiCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建DXGI采集器失败");
                return false;
            }
        }
#elif defined TARGET_WIN7
        if (m_ScreenCapture)
        {
            //创建GDI捕获器
            m_ScreenCapture = new GDICapture(m_frameRate);
            if (!m_ScreenCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建GDI捕获器失败");
                return false;
            }
        }
#endif 
        if (!m_microCapture)
        {//创建FFmpeg麦克风音频捕获器
            m_microCapture = (
                ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
                new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                    (MircroFmt)m_VideoFmt) :
                nullptr
                );
        }
    }
    else if (m_Bool_ScreenRecord)
    {
#ifdef TARGET_WIN10
        if (!m_dxgiCapture)
        { 
            m_dxgiCapture = new DXGICapture(m_width, m_height, m_frameRate);
            if (!m_dxgiCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建DXGI采集器失败");
                return false;
            }
        }
#elif defined TARGET_WIN7
        if (m_ScreenCapture)
        { //创建GDI捕获器
            m_ScreenCapture = new GDICapture(m_frameRate);
            if (!m_ScreenCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建GDI捕获器失败");
                return false;
            }
        }
#endif
        if (!m_microCapture)
        {//创建FFmpeg麦克风音频捕获器
            m_microCapture = (
                ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
                new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                    (MircroFmt)m_VideoFmt) :
                nullptr
                );
        }
    }
    else if (m_Bool_AreaRecord)
    {
#ifdef TARGET_WIN10
        if (!m_dxgiCapture)
        {//创建DXGI捕获器
            m_dxgiCapture = new DXGICapture(m_width, m_height, m_frameRate, m_Left, m_Top, m_Right, m_Bottom);
            if (!m_dxgiCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建DXGI捕获器失败");
                return false;
            }
        }
#elif defined TARGET_WIN7
        if (!m_ScreenCapture)
        {//创建GDI捕获器
            m_ScreenCapture = new GDICapture(m_Left, m_Top, m_Right, m_Bottom, m_frameRate);
            if (!m_ScreenCapture)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:创建GDI捕获器失败");
                return false;
            }
        }
#endif
        if (!m_microCapture)
        { //创建FFmpeg麦克风音频捕获器
            m_microCapture = (
                ((m_RecordMode == RecordMode::Microphone) || (m_RecordMode == RecordMode::Both)) ?
                new MicrophoneCapture((MircroSampleRate)m_AudioSampleRate, (MircroBitRate)m_AudioBitRate,
                    (MircroFmt)m_VideoFmt) :
                nullptr
                );
        }
    }

    //重新初始化以软件编码器的方式开始录制
    m_VideoEncoder = "";
    m_Bool_IsHasRestartSince = true;//设置已经重启过的标志，防止递归重启 
    if (!startRecording(m_str_outputfile.c_str()))
    {
        m_Bool_IsHasRestartSince = false;
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:重启未能成功,开启录制失败");
        return false;
    }
    return true;
}

bool ScreenRecorder::StopRecordToRestart()
{
    if (!m_IsRecording) {
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:开始停止录制过程...");
    m_IsRecording = false;

    // 停止捕获
    if (!m_Bool_OnlyAudioRecord)//如果不是只录制音频
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:停止屏幕捕获");
#ifdef TARGET_WIN10
        if (m_dxgiCapture)
            m_dxgiCapture->StopCaptureThread();
#elif TARGET_WIN7
        if (m_ScreenCapture)
            m_ScreenCapture->StopCapture();
#endif // TARGET_WIN10
        if (m_gdiCapture)
            m_gdiCapture->StopCapture();
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:停止音频捕获");
    if (m_audioCapture)//m_audioCapture是一个接口实例，需要释放，同理再次调用StartCapture需要创建并获取接口实例
    {
        m_audioCapture->StopCapture();
        m_audioCapture->ReleaseInstance();
        m_audioCapture = nullptr;
    }
    if (m_microCapture)
        m_microCapture->StopCapture();

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:等待捕获线程结束...");

    // 确保混合线程处理完所有队列中的帧
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[重启线程]:等待混合线程处理剩余帧... (视频队列: %d, 音频队列: %d)",
        m_videoQueue.size(), m_audioQueue.size());
    if (m_mixingThread.joinable()) m_mixingThread.join();

    // 等待捕获线程结束
    if (m_videoCapureThread.joinable()) m_videoCapureThread.join();
    if (m_audioProcessThread.joinable()) m_audioProcessThread.join();

    // 清理音视频同步队列
    while (!m_videoQueue.empty())
    {
        VideoPacket& packet = m_videoQueue.front();
        av_packet_unref(&packet.pkt);
        m_videoQueue.pop();
    }
    while (!m_audioQueue.empty())
    {
        AudioPacket& packet = m_audioQueue.front();
        av_packet_unref(&packet.pkt);
        m_audioQueue.pop();
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[重启线程]:清理FFmpeg资源");
    // 清理FFmpeg资源
    CleanUpFFmpeg();
    return true;
}

void ScreenRecorder::PauseRecording()
{
    if (!m_isPause.exchange(true)) 
    { 
        m_pauseBeginUsec = av_gettime(); 
        DEBUG_CONSOLE_STR(ConsoleHandle, L"暂停录制：记录时间基校正起点");
    }
}

void ScreenRecorder::ResumeRecording()
{
    int64_t pausedUsec = 0;
    {
        std::lock_guard<std::mutex> lock(m_muxPause);
        if (m_isPause.load() && m_pauseBeginUsec > 0)
        {
            int64_t now = av_gettime();
            pausedUsec = now - m_pauseBeginUsec;
            m_baseTime += pausedUsec;
            m_pauseBeginUsec = 0;
        }
        m_isPause.store(false);
        m_cvPause.notify_all();
    }
    if (pausedUsec > 0) 
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"恢复录制：时间基向前平移 %.3f 秒", pausedUsec / 1000000.0);
}

void ScreenRecorder::SetRecordSizeCallback(std::function<void(double)> cb)
{
    m_RecordSizeCallback = std::move(cb);
}

void ScreenRecorder::SetRecordSizeCallbackInterval(int intervalMs)
{
    if (intervalMs < 50) intervalMs = 50;
    m_sizeCallbackIntervalMs = intervalMs;
}

void ScreenRecorder::SetVideoTextFilter(std::wstring textInfo,
    std::string textSize, std::string fontColor, std::wstring logoPath)
{
    std::string logoPathUtf8 = GlobalFunc::ConvertToUtf8(logoPath);
    std::replace(logoPathUtf8.begin(), logoPathUtf8.end(), '\\', '/');
    logoPathUtf8.insert(1, "\\");

    m_str_textInfo = std::move(textInfo);
    m_str_textSize = std::move(textSize);
    m_str_fontColor = std::move(fontColor);
    m_str_pathUtf8 = std::move(logoPathUtf8);
    m_bool_IsTextFilterActive.store(true);
}

bool ScreenRecorder::InitTextImageFilter(const std::string& logoPathUtf8)
{
    if (!m_bool_IsTextFilterActive.load())
        return false;

    // 新建 graph + buffer & buffersink
    m_AVFGraph_Text = avfilter_graph_alloc();
    std::ostringstream os;
    os << "video_size=" << m_width << "x" << m_height
        << ":pix_fmt=" << m_CodecCtx->pix_fmt
        << ":time_base=1/" << m_frameRate;
    if (avfilter_graph_create_filter(&m_AVFCtx_TextIn,
        avfilter_get_by_name("buffer"),
        "in", os.str().c_str(), NULL, m_AVFGraph_Text) < 0)
        return false;
    if (avfilter_graph_create_filter(&m_AVFCtx_TextOut,
        avfilter_get_by_name("buffersink"),
        "out", NULL, NULL, m_AVFGraph_Text) < 0)
        return false;

    //  把文本和 logo 路径都准备好
    std::string textUtf8 = GlobalFunc::ConvertToUtf8(m_str_textInfo);
    std::string logo = m_str_pathUtf8;

    // 拼 水印 描述
    std::ostringstream fss;
    fss << "movie='" << logoPathUtf8 << "'[logo];" <<
        "[in][logo]overlay=" <<
        "x=(W-w)-W/2 - w:" <<
        "y=10" <<
        "[tmp];" <<
        "[tmp]drawtext=" <<
        "font='Microsoft YaHei'" <<
        ":text='" << textUtf8 << "'" <<
        ":fontcolor=" << m_str_fontColor << "@1.0" <<
        ":fontsize=" << m_str_textSize <<
        ":x=(w-text_w)/2 + 30" <<
        ":y=25" <<
        "[out]";
    std::string filter_descr = fss.str();
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"filter_descr: %hs", filter_descr.c_str());

    //  解析 & 配置
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    outputs->name = av_strdup("in");
    outputs->filter_ctx = m_AVFCtx_TextIn;
    outputs->pad_idx = 0; outputs->next = NULL;
    inputs->name = av_strdup("out");
    inputs->filter_ctx = m_AVFCtx_TextOut;
    inputs->pad_idx = 0; inputs->next = NULL;

    int ret = avfilter_graph_parse_ptr(
        m_AVFGraph_Text, filter_descr.c_str(), &inputs, &outputs, NULL);
    if (ret < 0)
    {
        char err[128];
        av_strerror(ret, err, sizeof(err));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"parse_ptr 错误: %hs", err);
        return false;
    }
    ret = avfilter_graph_config(m_AVFGraph_Text, NULL);
    if (ret < 0)
    {
        char err[128];
        av_strerror(ret, err, sizeof(err));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"config 错误: %hs", err);
        return false;
    }
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    DEBUG_CONSOLE_STR(ConsoleHandle, L"文字+图片水印滤镜初始化完成");
    return true;
}