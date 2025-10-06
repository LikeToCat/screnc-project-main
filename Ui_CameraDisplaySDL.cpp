#include "stdafx.h"
#include "CDebug.h"
#include "Ui_CameraDisplaySDL.h"
#include "LarStringConversion.h"
#include "theApp.h"

// 调试控制台句柄
extern HANDLE ConsoleHandle;
Ui_CameraDisplaySDL* Ui_CameraDisplaySDL::s_instance = nullptr;
std::mutex Ui_CameraDisplaySDL::s_Mutex_Instance;

Ui_CameraDisplaySDL::Ui_CameraDisplaySDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr)
{
    m_InterFaceState_State.m_Bool_UiWindowRunning = false;        // 窗口是否正在运行
    m_InterFaceState_State.m_Bool_Renderering = false;            // 停止帧渲染线程
    m_InterFaceState_State.m_Bool_SDLInitialized = false;         // SDL是否已初始化
    m_InterFaceState_State.m_Bool_IsPauseRendering = false;       // 是否暂停帧渲染线程
    m_InterFaceState_State.m_Bool_IsRecording = false;            // 是否正在录制
    m_InterFaceState_State.m_Bool_IsEncodeFrameAvailable = false; // 编码帧是否可用
    m_AVFrame_EncodeFrame = av_frame_alloc();
    // 构造函数初始化类成员
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL主线程]: Ui_CameraDisplaySDL构造");
}

Ui_CameraDisplaySDL::~Ui_CameraDisplaySDL()
{
    // 确保停止录制
    if (m_InterFaceState_State.m_Bool_IsRecording.load()) {
        StopRecording();
    }

    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: Ui_CameraDisplaySDL析构");
    CloseWindow();

    av_frame_free(&m_AVFrame_EncodeFrame);
}

void Ui_CameraDisplaySDL::LogSDLError(const wchar_t* prefix) {
    const char* sdlError = SDL_GetError();
    wchar_t wSdlError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s: %s", prefix, wSdlError);
}

Ui_CameraDisplaySDL* Ui_CameraDisplaySDL::GetInstance()
{
    std::lock_guard<std::mutex> lock(s_Mutex_Instance);
    if (!s_instance)
    {
        s_instance = new Ui_CameraDisplaySDL;
    }
    return s_instance;
}

void Ui_CameraDisplaySDL::ReleaseInstance()
{
    std::lock_guard<std::mutex> lock(s_Mutex_Instance);
    if (s_instance)
    {
        delete s_instance;
        s_instance = nullptr;
    }
}

bool Ui_CameraDisplaySDL::Initialize(
    const CRect& recordArea,
    std::string deviceName,
    std::string desc,
    const int dstframeRate,
    std::string srcfmt,
    int32_t srcwidth,
    int32_t srcheight,
    int32_t SampleRate,
    int32_t BitRate,
    std::string videoEncoder
)
{
    // 保存渲染参数
    m_Rect_CameraDisplayArea = recordArea;
    m_Int_frameRate = dstframeRate;
    m_Int_FrameWidth = srcwidth;
    m_Int_FrameHeight = srcheight;
    m_string_fmt = srcfmt;
    m_string_desc = desc;
    m_string_device = deviceName;
    m_Int_SampleRate = SampleRate > 0 ? SampleRate : 44100;
    m_Int_BitRate = BitRate > 0 ? BitRate : 128;
    ///m_string_videoEncoder = ""; // 保存编码器名称

    // 如果指定了编码器，测试其是否可用
    if (!m_string_videoEncoder.empty()) 
    {
        bool encoderAvailable = TestEncoderAvailability(m_string_videoEncoder.c_str());
        if (encoderAvailable) {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 指定的编码器 %s 可用",
                LARSC::c2w(m_string_videoEncoder.c_str()));
        }
        else 
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 警告 - 指定的编码器 %s 不可用，将回退到软件编码器",
                LARSC::c2w(m_string_videoEncoder.c_str()));
            m_string_videoEncoder = ""; // 清空编码器名称，表示使用默认软件编码器
        }
    }
    else
    {
        DB(ConsoleHandle, L"未指定编码器，准备使用软件编码器");
    }
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 初始化区域 左=%d 上=%d 宽=%d 高=%d",
        recordArea.left, recordArea.top, recordArea.Width(), recordArea.Height());
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 摄像头参数:格式=%s,宽x高:%dx%d,帧率:%d",
        LARSC::s2ws(m_string_fmt).c_str(), m_Int_FrameWidth, m_Int_FrameHeight, m_Int_frameRate);

    // 验证录制区域尺寸是否有效
    if (recordArea.Width() <= 0 || recordArea.Height() <= 0) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 错误 - 初始化区域尺寸无效");
        return false;
    }

    // 分步初始化，先初始化SDL
    if (!InitializeSDL()) 
    {
        m_InterFaceState_State.m_Bool_SDLInitialized.store(false);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: SDL初始化失败");
        return false;
    }
    // 初始化图像格式转换器
    if (!InitialSwsCtx()) 
    {
        m_InterFaceState_State.m_Bool_SDLInitialized.store(false);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: ffmpeg格式转换器初始化失败");
    }

    m_InterFaceState_State.m_Bool_SDLInitialized.store(true);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: SDL窗口初始化成功");
    return true;
}

void Ui_CameraDisplaySDL::RunSDLWindow()
{
    RunUiEventThread();
    RunFrameRendererThread();
}

void Ui_CameraDisplaySDL::MinimalSDLWindow()
{
    SDL_MinimizeWindow(m_SDL_Window.get());
}

void Ui_CameraDisplaySDL::HideSDLWindow()
{
    SDL_HideWindow(m_SDL_Window.get());
}

void Ui_CameraDisplaySDL::ShowSDLWindow()
{
    SDL_ShowWindow(m_SDL_Window.get());
}

void Ui_CameraDisplaySDL::RestoreSDLWindow()
{
    //恢复窗口，确保在最前面，同时恢复渲染渲染线程
    SDL_RestoreWindow(m_SDL_Window.get());
    SDL_RaiseWindow(m_SDL_Window.get());
}

void Ui_CameraDisplaySDL::UpdateSDLWindowPos(const CRect& newPosition)
{
    if (m_SDL_Window.get())
    {
        SDL_SetWindowPosition(m_SDL_Window.get(), newPosition.left, newPosition.top);
        SDL_SetWindowSize(m_SDL_Window.get(), newPosition.Width(), newPosition.Height());
    }
}

bool Ui_CameraDisplaySDL::InitialSwsCtx()
{
    if (m_string_fmt == "mjpeg")
    {
        // 初始化视频格式转换器(源mjpeg->YUV420P)
        const AVPixelFormat mjpegfmt[]
        {
            AV_PIX_FMT_YUVJ420P,
            AV_PIX_FMT_YUVJ422P,
            AV_PIX_FMT_YUVJ444P
        };
        for (AVPixelFormat fmt : mjpegfmt)
        {
            SwsContext* mjpegSwsCtx = sws_getContext(
                m_Int_FrameWidth, m_Int_FrameHeight,
                fmt,
                m_Int_FrameWidth, m_Int_FrameHeight,
                AV_PIX_FMT_YUV420P,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
            if (!mjpegSwsCtx)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]:视频格式转换器(源mjpeg->YUV420P)创建失败");
                return false;
            }
            m_Map_SwsCtx.emplace(fmt, std::shared_ptr<SwsContext>(mjpegSwsCtx, [](SwsContext* pSwsCtx)
                {
                    if (pSwsCtx)
                    {
                        sws_freeContext(pSwsCtx);
                    }
                }));
        }
    }
    else if (m_string_fmt == "yuyv422")
    {
        // 初始化视频格式转换器(源yuyv->YUV420P)
        const AVPixelFormat yuyvfmt = AV_PIX_FMT_YUYV422;
        SwsContext* yuyvSwsCtx = sws_getContext(
            m_Int_FrameWidth, m_Int_FrameHeight,
            yuyvfmt,
            m_Int_FrameWidth, m_Int_FrameHeight,
            AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
        if (!yuyvSwsCtx)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]:视频格式转换器(源yuyv->YUV420P)创建失败");
            return false;
        }
        m_Map_SwsCtx.emplace(yuyvfmt, std::shared_ptr<SwsContext>(yuyvSwsCtx, [](SwsContext* pSwsCtx)
            {
                if (pSwsCtx)
                {
                    sws_freeContext(pSwsCtx);
                }
            }));
    }
    return true;
}

bool Ui_CameraDisplaySDL::InitializeSDL()
{
    // 只初始化视频子系统，避免不必要的初始化
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        LogSDLError(L"[主线程]:初始化视频子系统失败");
        return false;
    }

    // 设置SDL提示，提高在Windows上的兼容性
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // 抗锯齿
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: SDL初始化成功");
    return true;
}

bool Ui_CameraDisplaySDL::CreateSDLWindow()
{
    // 计算初始窗口位置和大小
    int left = max(0, m_Rect_CameraDisplayArea.left);
    int top = max(0, m_Rect_CameraDisplayArea.top);
    int width = max(100, min(GetSystemMetrics(SM_CXSCREEN) - left, m_Rect_CameraDisplayArea.Width()));
    int height = max(100, min(GetSystemMetrics(SM_CYSCREEN) - top, m_Rect_CameraDisplayArea.Height()));

    // 创建窗口
    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 创建SDL摄像头画面渲染窗口");
    SDL_Window* sdlWindow = SDL_CreateWindow(
        "Camera Display",  // 窗口标题
        left, top,         // 位置
        width, height,     // 大小
        windowFlags        // 标志
    );
    if (!sdlWindow) {
        LogSDLError(L"[主线程]：创建窗口失败!");
        return false;
    }
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 窗口创建成功，位置=%d,%d 大小=%d,%d",
        left, top, width, height);
    m_SDL_Window = std::shared_ptr<SDL_Window>(sdlWindow, [](SDL_Window* p)
        {
            if (p)
            {
                SDL_DestroyWindow(p);
            }
        });

    // 创建硬件加速渲染器
    SDL_Renderer* renderer = SDL_CreateRenderer(m_SDL_Window.get(), -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) 
    {
        // 尝试创建软件渲染器作为后备方案
        DB(ConsoleHandle, L"使用软件渲染器");
        renderer = SDL_CreateRenderer(m_SDL_Window.get(), -1, SDL_RENDERER_SOFTWARE);
        if (!renderer)
        {
            LogSDLError(L"[主线程]: 渲染器创建失败:");
            return false;
        }
    }
    m_SDL_Renderer = std::shared_ptr<SDL_Renderer>(renderer, [](SDL_Renderer* p)
        {
            if (p)
            {
                SDL_DestroyRenderer(p);
            }
        });

    SDL_Texture* texture = SDL_CreateTexture(
        m_SDL_Renderer.get(),
        SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING,
        m_Int_FrameWidth,
        m_Int_FrameHeight
    );
    if (!texture)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 渲染器纹理创建失败");
        return false;
    }
    m_SDL_Texture = std::shared_ptr<SDL_Texture>(texture, [](SDL_Texture* p)
        {
            if (p)
            {
                SDL_DestroyTexture(p);
            }
        });

    //设置为顶层窗口,且在应用窗口列表中移除
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(m_SDL_Window.get(), &wmInfo)) {
        HWND hwnd = wmInfo.info.win.window;

        // 设置扩展窗口样式 - 永远在最上面
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_TOPMOST;
        exStyle |= WS_EX_TOOLWINDOW;   // 添加工具窗口样式
        exStyle &= ~WS_EX_APPWINDOW;   // 移除应用窗口样式
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

        // 额外设置窗口位置
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);
    }
    return true;
}

void Ui_CameraDisplaySDL::CloseWindow()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 正在清理SDL资源");
    if (m_InterFaceState_State.m_Bool_IsRecording.load()) 
    {   // 确保停止录制
        StopRecording();
    }
    CameraCapture::GetInstance()->ReleaseInstance();

    //先等待渲染线程结束
    m_InterFaceState_State.m_Bool_Renderering.store(false);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待[SDL窗口视频渲染子线程]结束");
    if (m_InterFaceThread_Thread.m_Thread_RenderFrame.joinable())
    {
        m_InterFaceThread_Thread.m_Thread_RenderFrame.join();
    }

    //等待窗口Ui事件循环结束
    m_InterFaceState_State.m_Bool_UiWindowRunning.store(false);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待[SDL窗口子线程]结束");
    if (m_InterFaceThread_Thread.m_Thread_UiWindow.joinable())
    {
        m_InterFaceThread_Thread.m_Thread_UiWindow.join();
    }

    // 完全清理SDL资源
    if (m_InterFaceState_State.m_Bool_SDLInitialized) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO); // 退出SDL视频子系统      
        if (SDL_WasInit(0) == 0) {// 检查是否还有其他子系统在运行
            SDL_Quit();
        }
        m_InterFaceState_State.m_Bool_SDLInitialized = false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: SDL资源清理完成");
}

void Ui_CameraDisplaySDL::PauseRendering()
{
    {
        std::lock_guard<std::mutex> PasuingLock(m_InterFaceThreadProtect_Protect.m_Mutex_RenderPausing);
        if (!m_InterFaceState_State.m_Bool_IsPauseRendering.load())
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 暂停SDL摄像头渲染");
            CameraCapture* pCCInterface = CameraCapture::GetInstance();
            pCCInterface->PauseCapture();
            m_InterFaceState_State.m_Bool_IsPauseRendering.store(true);
        }
    }
}

void Ui_CameraDisplaySDL::ResumeRendering()
{
    {
        std::lock_guard<std::mutex> PasuingLock(m_InterFaceThreadProtect_Protect.m_Mutex_RenderPausing);
        if (m_InterFaceState_State.m_Bool_IsPauseRendering.load())
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 恢复SDL摄像头渲染");
            m_InterFaceState_State.m_Bool_IsPauseRendering.store(false);
            CameraCapture* pCCInterface = CameraCapture::GetInstance();
            pCCInterface->ResumeCapture();
            m_InterFaceThreadProtect_Protect.m_CV_RenderPasuing.notify_all();//通知渲染线程，可以恢复执行了
        }
    }
}

void Ui_CameraDisplaySDL::RunUiEventThread()
{
    if (!m_InterFaceState_State.m_Bool_SDLInitialized.load())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 非法调用SDL摄像头渲染窗口Run函数！");
        return;
    }
    m_InterFaceThread_Thread.m_Thread_UiWindow = std::thread(&Ui_CameraDisplaySDL::UiLoop, this);
}

void Ui_CameraDisplaySDL::RunFrameRendererThread()
{
    if (!m_InterFaceState_State.m_Bool_SDLInitialized.load())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]:非法调用RunFrameRendererThread，开启线程失败！");
        return;
    }
    m_InterFaceThread_Thread.m_Thread_RenderFrame = std::thread(&Ui_CameraDisplaySDL::Render, this);
}

void Ui_CameraDisplaySDL::Render()
{
    //等待窗口创建完成
    {
        std::unique_lock<std::mutex> lock(m_InterFaceThreadProtect_Protect.m_Mutex_WaitUiInital);
        if (!m_InterFaceThreadProtect_Protect.m_CV_WaitUiInital.wait_for(lock, std::chrono::milliseconds(1000), [this]()
            {
                return m_InterFaceState_State.m_Bool_UiWindowRunning.load();
            }))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]:未开启窗口UI事件循环，无法进行渲染!");
            m_InterFaceState_State.m_Bool_Renderering.store(false);
            return;
        }
    }

    //获取摄像头捕获接口，初始化捕获参数
    CameraCapture* pCCInterface = CameraCapture::GetInstance();
    CameraOptions cameraOptions;
    cameraOptions.fps = m_Int_frameRate;
    cameraOptions.vcodec = m_string_fmt;
    cameraOptions.pixelX = m_Int_FrameWidth;
    cameraOptions.pixelY = m_Int_FrameHeight;
    cameraOptions.deviceDesc = m_string_desc;
    cameraOptions.deviceName = m_string_device;
    if (!pCCInterface->Init(cameraOptions))
    {
        m_InterFaceState_State.m_Bool_Renderering.store(false);
        m_InterFaceState_State.m_Bool_UiWindowRunning.store(false);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 摄像头捕获接口初始化失败!");
        return;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 开始");

    // 分配临时视频帧
    AVFrame* videoFrame = av_frame_alloc();
    if (!videoFrame) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 分配视频帧失败");
        m_InterFaceState_State.m_Bool_UiWindowRunning = false;
        m_InterFaceState_State.m_Bool_Renderering = false;
        av_frame_free(&videoFrame);
        return;
    }
    m_AVFrame_DisplayFrame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* pframe)
        {
            av_frame_free(&pframe);
        });

    // 一次性为显示帧分配缓冲
    m_AVFrame_DisplayFrame.get()->format = AV_PIX_FMT_YUV420P;
    m_AVFrame_DisplayFrame.get()->width = m_Int_FrameWidth;
    m_AVFrame_DisplayFrame.get()->height = m_Int_FrameHeight;
    if (!m_DisplayFrameBufferInited) 
    {
        if (av_frame_get_buffer(m_AVFrame_DisplayFrame.get(), 32) < 0)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 分配显示帧缓冲失败");
            m_InterFaceState_State.m_Bool_Renderering.store(false);
            av_frame_free(&videoFrame);
            return;
        }
        m_DisplayFrameBufferInited = true;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 开始渲染循环");
    { // 标记渲染已开始并通知等待线程
        std::lock_guard<std::mutex> lock(m_InterFaceThreadProtect_Protect.m_Mutex_RenderingStarted);
        m_InterFaceState_State.m_Bool_Renderering.store(true);
        m_InterFaceThreadProtect_Protect.m_CV_RenderingStarted.notify_all();
    }

    //开启摄像头捕获
    pCCInterface->StartCapture();
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 使用摄像头捕获功能接口并开启捕获");

    // 根据帧率动态计算帧持续时间
    auto lastFrameTime = std::chrono::steady_clock::now();
    std::chrono::milliseconds frameDuration;
    frameDuration = std::chrono::milliseconds(static_cast<int>(1000.0 / m_Int_frameRate));
    while (m_InterFaceState_State.m_Bool_Renderering.load())
    {
        { //是否暂停渲染
            std::unique_lock<std::mutex> PasuingLock(m_InterFaceThreadProtect_Protect.m_Mutex_RenderPausing);
            while (m_InterFaceState_State.m_Bool_IsPauseRendering.load())
            {
                m_InterFaceThreadProtect_Protect.m_CV_RenderPasuing.wait(PasuingLock);
            } 
        }
        
        // 控制帧率
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - lastFrameTime;
        if (elapsed < frameDuration) 
        {
            auto sleepTime = frameDuration - elapsed;
            std::this_thread::sleep_for(sleepTime);
            continue;
        }
        lastFrameTime = std::chrono::steady_clock::now();//更新当前帧事件

        // 捕获摄像头帧并渲染
        bool frameOk = pCCInterface->CaptureFrame(videoFrame);
        if (!frameOk) 
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 捕获摄像头帧失败");
            continue;
        }

        //// 渲染帧
        // 执行图像格式转换
        auto it = m_Map_SwsCtx.find((AVPixelFormat)(videoFrame->format));
        if (it == m_Map_SwsCtx.end())
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]:找不到对应的图像格式转换器，无法渲染!");
            m_InterFaceState_State.m_Bool_UiWindowRunning = false;
            m_InterFaceState_State.m_Bool_Renderering = false;
            return;
        }
        std::shared_ptr<SwsContext> SwsCtx = it->second;
        //m_AVFrame_DisplayFrame.get()->format = AV_PIX_FMT_YUV420P;
        //m_AVFrame_DisplayFrame.get()->width = m_Int_FrameWidth;
        //m_AVFrame_DisplayFrame.get()->height = m_Int_FrameHeight;
        //av_frame_get_buffer(m_AVFrame_DisplayFrame.get(), 32);
        //DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL窗口视频渲染子线程]：使用转换器<%d> -> <%d>",
        //    it->first, AV_PIX_FMT_YUV420P);
        if (av_frame_make_writable(m_AVFrame_DisplayFrame.get()) < 0)
        {
            // 如果之前被意外释放（buf[0] == nullptr），尝试重新分配一次缓冲
            if (!m_AVFrame_DisplayFrame.get()->buf[0])
            {
                m_AVFrame_DisplayFrame.get()->format = AV_PIX_FMT_YUV420P;
                m_AVFrame_DisplayFrame.get()->width = m_Int_FrameWidth;
                m_AVFrame_DisplayFrame.get()->height = m_Int_FrameHeight;
                if (av_frame_get_buffer(m_AVFrame_DisplayFrame.get(), 32) < 0)
                {
                    static bool s_LogOnce = false;
                    if (!s_LogOnce)
                    {
                        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 重新分配显示帧缓冲失败");
                        s_LogOnce = true;
                    }
                    continue;
                }
            }
            else
            {
                // 缓冲存在但不可写，跳过本帧
                static bool s_LogOnce2 = false;
                if (!s_LogOnce2)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, 
                        L"[SDL窗口视频渲染子线程]: 显示帧缓冲不可写(可能仍被引用) 跳过本帧");
                    s_LogOnce2 = true;
                }
                continue;
            }
        }

        // （保持原来的“像素格式变化才打印转换器”逻辑不变）
        if (m_LastSrcPixelFormat != (int)videoFrame->format)
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 使用转换器<%d> -> <%d>",
                videoFrame->format, AV_PIX_FMT_YUV420P);
            m_LastSrcPixelFormat = (int)videoFrame->format;
        }
        int nb = sws_scale(
            SwsCtx.get(),
            (const uint8_t* const*)videoFrame->data,
            (const int*)videoFrame->linesize,
            0, m_Int_FrameHeight,
            (uint8_t* const*)m_AVFrame_DisplayFrame.get()->data,
            (const int*)m_AVFrame_DisplayFrame.get()->linesize
        );
        if (nb <= 0) 
        {
            char errobuf[256]{ 0 };
            if (nb < 0)
            {
                av_strerror(nb, errobuf, 256);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL窗口视频渲染子线程] 图像帧转换错误: %s", LARSC::c2w(errobuf));
                continue;
            }
            else
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程] 异常：图像帧转换了0行");
                continue;
            }
        }
        // 如果编码线程正在运行，则通知编码线程帧可用
        if (m_InterFaceState_State.m_Bool_IsRecording.load())
        {
            std::lock_guard<std::mutex> lock(m_InterFaceThreadProtect_Protect.m_Mutex_IsEncodeFrameAvailable);
            av_frame_ref(m_AVFrame_EncodeFrame, m_AVFrame_DisplayFrame.get());
            m_InterFaceState_State.m_Bool_IsEncodeFrameAvailable = true;
            m_InterFaceThreadProtect_Protect.m_CV_IsEncodeFrameAvailable.notify_all();
        }

        // 渲染到SDL
        int result = SDL_UpdateYUVTexture(
            m_SDL_Texture.get(), NULL,
            m_AVFrame_DisplayFrame.get()->data[0], m_AVFrame_DisplayFrame.get()->linesize[0],
            m_AVFrame_DisplayFrame.get()->data[1], m_AVFrame_DisplayFrame.get()->linesize[1],
            m_AVFrame_DisplayFrame.get()->data[2], m_AVFrame_DisplayFrame.get()->linesize[2]
        );

        if (result != 0) 
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 更新纹理失败: %s", LARSC::c2w(SDL_GetError()));
           m_InterFaceState_State.m_Bool_UiWindowRunning = false;
           m_InterFaceState_State.m_Bool_Renderering = true;
            continue;
        }

        // 清除渲染器
        result = SDL_RenderClear(m_SDL_Renderer.get());
        if (result != 0)
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL窗口视频渲染子线程] : 清除渲染器失败: %s",
                LARSC::c2w(SDL_GetError()));
            continue;
        }

        // 复制纹理到渲染器
        SDL_Rect RendererRect;
        RendererRect.x = 0;
        RendererRect.y = 0;
        RendererRect.w = m_Rect_CameraDisplayArea.Width();
        RendererRect.h = m_Rect_CameraDisplayArea.Height();
        result = SDL_RenderCopy(m_SDL_Renderer.get(), m_SDL_Texture.get(), NULL, &RendererRect);
        if (result != 0) 
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[渲染线程] 复制纹理失败: %s (矩形: %d,%d,%d,%d)",
                LARSC::c2w(SDL_GetError()),
                RendererRect.x, RendererRect.y, RendererRect.w, RendererRect.h);
            continue;
        }

        // 显示渲染的画面
        SDL_RenderPresent(m_SDL_Renderer.get());
        av_frame_unref(videoFrame);
        //av_frame_unref(m_AVFrame_DisplayFrame.get());
    }
    av_frame_free(&videoFrame);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口视频渲染子线程]: 结束");
}

void Ui_CameraDisplaySDL::UiLoop()
{
    // 创建SDL窗口
    if (!CreateSDLWindow()) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口子线程]: SDL窗口创建失败");
        m_InterFaceState_State.m_Bool_UiWindowRunning.store(false);
        m_InterFaceState_State.m_Bool_Renderering.store(false);
        return;
    }

    //开启窗口主循环
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口子线程]: SDL窗口主循环开始运行");
    {//首先通知渲染线程，窗口已经准备好
        std::lock_guard<std::mutex> lock(m_InterFaceThreadProtect_Protect.m_Mutex_WaitUiInital);
        m_InterFaceState_State.m_Bool_UiWindowRunning.store(true);
        m_InterFaceThreadProtect_Protect.m_CV_WaitUiInital.notify_all();
    }
    while (m_InterFaceState_State.m_Bool_UiWindowRunning)
    {
        // 处理SDL事件队列中的所有事件
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
                case SDL_QUIT:
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口子线程]: 窗口退出");
                    m_InterFaceState_State.m_Bool_UiWindowRunning.store(false);
                    m_InterFaceState_State.m_Bool_Renderering.store(false);
                    break;

                case SDL_MOUSEBUTTONDOWN:

                    break;

                case SDL_MOUSEBUTTONUP:

                    break;

                case SDL_MOUSEMOTION:

                    break;
                case SDL_WINDOWEVENT:

                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)//按下了esc键
                    {
                        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口子线程]: 按下esc键，SDL渲染窗口关闭");
                       m_InterFaceState_State.m_Bool_UiWindowRunning.store(false);
                       m_InterFaceState_State.m_Bool_Renderering.store(false);
                    }
                    break;
                }
        }
        SDL_Delay(40); // 30 FPS
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL窗口子线程]: SDL窗口主循环结束");
}

//--------------------
// 摄像头录制相关
//--------------------
bool Ui_CameraDisplaySDL::TestEncoderAvailability(const char* encoderName)
{
    // 尝试查找指定的编码器
    const AVCodec* codec = avcodec_find_encoder_by_name(encoderName);
    if (!codec) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 找不到编码器 %s", LARSC::c2w(encoderName));
        return false;
    }

    // 分配编码器上下文以进行测试
    AVCodecContext* ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 分配编码器上下文失败");
        return false;
    }

    // 设置基本参数以测试编码器
    ctx->codec_id = codec->id;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->width = m_Int_FrameWidth;
    ctx->height = m_Int_FrameHeight;
    ctx->time_base = AVRational{ 1, m_Int_frameRate };
    ctx->framerate = AVRational{ m_Int_frameRate, 1 };
    ctx->bit_rate = 3000000;

    // 使用编码器支持的第一个像素格式
    ctx->pix_fmt = AV_PIX_FMT_YUV420P; // 默认尝试使用YUV420P
    if (codec->pix_fmts) {
        ctx->pix_fmt = codec->pix_fmts[0]; // 使用编码器支持的第一个格式
    }

    // 尝试打开编码器
    int ret = avcodec_open2(ctx, codec, nullptr);

    // 释放资源
    avcodec_free_context(&ctx);

    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 打开编码器 %s 失败: %s",
            LARSC::c2w(encoderName), LARSC::c2w(errbuf));
        return false;
    }

    return true;
}

bool Ui_CameraDisplaySDL::StartRecording(const std::string& outputFilePath, RecordMode mode)
{
    if (m_InterFaceState_State.m_Bool_IsRecording.load()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 已经在录制中，请先停止当前录制");
        return false;
    }

    if (!m_InterFaceState_State.m_Bool_Renderering.load()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 渲染未运行，无法开始录制");
        return false;
    }

    // 设置录制参数和初始化编码器
    if (!SetupRecording(outputFilePath, mode)) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 设置录制失败");
        return false;
    }

    // 标记录制状态
    m_Recording_Context.isActive = true;
    m_InterFaceState_State.m_Bool_IsRecording = true;

    // 启动视频录制线程
    m_InterFaceState_State.m_Bool_IsRecording.store(true);
    m_InterFaceThread_Thread.m_Thread_Recording = std::thread(&Ui_CameraDisplaySDL::RecordingThread, this);

    // 如果需要录制音频，启动音频录制线程
    if (mode == RecordMode::Both) {
        m_InterFaceThread_Thread.m_Thread_AudioRecording = std::thread(&Ui_CameraDisplaySDL::AudioRecordingThread, this);
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 成功开始录制");
    return true;
}

void Ui_CameraDisplaySDL::StopRecording()
{
    if (!m_InterFaceState_State.m_Bool_IsRecording.load()) {
        return;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 正在停止录制...");

    // 停止录制线程
    m_Recording_Context.isActive = false;
    m_Recording_Context.packetQueueCV.notify_all();

    // 等待视频录制线程结束
    if (m_InterFaceThread_Thread.m_Thread_Recording.joinable()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待视频录制线程结束...");
        m_InterFaceThread_Thread.m_Thread_Recording.join();
    }

    // 等待音频录制线程结束
    if (m_InterFaceThread_Thread.m_Thread_AudioRecording.joinable()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待音频录制线程结束...");
        m_InterFaceThread_Thread.m_Thread_AudioRecording.join();
    }

    // 正确写入文件尾部
    if (m_Recording_Context.formatContext) {
        std::lock_guard<std::mutex> lock(m_Recording_Context.formatMutex);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 写入文件尾部...");
        av_write_trailer(m_Recording_Context.formatContext);
    }

    // 清理录制资源
    CleanupRecording();

    m_InterFaceState_State.m_Bool_IsRecording = false;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 录制已停止");
}

bool Ui_CameraDisplaySDL::WaitForRenderingToStart(int timeoutMs)
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待渲染线程启动...");

    // 如果渲染已经开始，直接返回成功
    if (m_InterFaceState_State.m_Bool_Renderering.load()) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 渲染线程已经启动");
        return true;
    }

    // 否则等待渲染开始的通知
    std::unique_lock<std::mutex> lock(m_InterFaceThreadProtect_Protect.m_Mutex_RenderingStarted);
    bool result = m_InterFaceThreadProtect_Protect.m_CV_RenderingStarted.wait_for(
        lock,
        std::chrono::milliseconds(timeoutMs),
        [this]() 
        {
            return m_InterFaceState_State.m_Bool_Renderering.load();
        }
    );

    if (result) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 渲染线程已成功启动");
    }
    else 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待渲染线程启动超时");
    }

    return result;
}

bool Ui_CameraDisplaySDL::SetupRecording(const std::string& outputFilePath, RecordMode mode)
{
    m_Recording_Context.outputFilePath = outputFilePath;
    m_Recording_Context.mode = mode;

    // 创建输出格式上下文
    const char* format = nullptr;
    bool isMP4 = false;

    if (outputFilePath.find(".mp4") != std::string::npos) {
        format = "mp4";
        isMP4 = true;
    }
    else if (outputFilePath.find(".avi") != std::string::npos) {
        format = "avi";
    }
    else if (outputFilePath.find(".mkv") != std::string::npos) {
        format = "matroska";
    }
    else {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 不支持的输出文件格式，使用默认MP4格式");
        format = "mp4";
        isMP4 = true;
    }

    // 初始化输出格式上下文
    int ret = avformat_alloc_output_context2(&m_Recording_Context.formatContext, nullptr, format, outputFilePath.c_str());
    if (ret < 0 || !m_Recording_Context.formatContext) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 创建输出格式上下文失败: %s", LARSC::c2w(errbuf));
        return false;
    }

    // MP4特殊处理
    if (isMP4) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 检测到MP4格式，使用兼容性设置");
    }

    // 设置视频流
    if (!SetupVideoStream()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 设置视频流失败");
        CleanupRecording();
        return false;
    }

    // 如果需要录制音频，设置音频流
    if (mode == RecordMode::Both) {
        if (!SetupAudioStream()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 设置音频流失败");
            CleanupRecording();
            return false;
        }
    }

    // 可选：打印格式和流信息
    av_dump_format(m_Recording_Context.formatContext, 0, outputFilePath.c_str(), 1);

    // 打开输出文件
    if (!(m_Recording_Context.formatContext->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&m_Recording_Context.formatContext->pb, outputFilePath.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            char errbuf[256];
            av_strerror(ret, errbuf, sizeof(errbuf));
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 打开输出文件失败: %s", LARSC::c2w(errbuf));
            CleanupRecording();
            return false;
        }
    }

    // MP4需要特殊选项
    AVDictionary* opts = nullptr;
    if (isMP4) {
        av_dict_set(&opts, "movflags", "faststart", 0); // 将metadata放在文件前，便于播放
    }

    // 写入文件头
    ret = avformat_write_header(m_Recording_Context.formatContext, &opts);
    av_dict_free(&opts);

    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 写入文件头失败: %s", LARSC::c2w(errbuf));
        CleanupRecording();
        return false;
    }

    return true;
}

bool Ui_CameraDisplaySDL::SetupVideoStream()
{
    // 查找视频编码器，优先尝试使用指定的硬件编码器
    const AVCodec* videoCodec = nullptr;
    bool isHardwareEncoder = false;

    if (!m_string_videoEncoder.empty())
    {
        // 尝试查找指定的编码器
        videoCodec = avcodec_find_encoder_by_name(m_string_videoEncoder.c_str());
        if (videoCodec)
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 使用硬件编码器: %s",
                LARSC::c2w(m_string_videoEncoder.c_str()));
            isHardwareEncoder = true;
        }
        else 
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle,
                L"[主线程]: 指定硬件编码器 %s 不可用，直接改用软件H264编码器",
                LARSC::c2w(m_string_videoEncoder.c_str()));
            m_string_videoEncoder.clear(); //清空，表示当前实际为软件编码
        }
    }

    // 如果没有指定编码器或指定的编码器不可用，使用默认的软件编码器
    if (!videoCodec)
    {
        videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!videoCodec) 
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 找不到H264编码器");
            return false;
        }
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 使用软件H264编码器 (libx264)");
    }

    // 创建视频流
    AVStream* videoStream = avformat_new_stream(m_Recording_Context.formatContext, nullptr);
    if (!videoStream)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 创建视频流失败");
        return false;
    }
    m_Recording_Context.videoStreamIndex = videoStream->index;

    // 配置视频编码器上下文
    m_Recording_Context.videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!m_Recording_Context.videoCodecContext)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 分配视频编码器上下文失败");
        return false;
    }

    AVCodecContext* ctx = m_Recording_Context.videoCodecContext;
    ctx->codec_id = videoCodec->id;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->width = m_Int_FrameWidth;
    ctx->height = m_Int_FrameHeight;

    // 设置基本必要参数
    ctx->time_base = AVRational{ 1, 90000 };  // e.g. {1,30}
    ctx->framerate = AVRational{ m_Int_frameRate, 1 };
    videoStream->time_base = ctx->time_base;
    ctx->bit_rate = 12000000; 
    ctx->gop_size = m_Int_frameRate;  // 每秒一个 I 帧
    ctx->max_b_frames = 0;

    // 为硬件编码器选择支持的像素格式，软件编码器使用标准YUV420P
    ctx->pix_fmt = AV_PIX_FMT_YUV420P; // 默认
    if (isHardwareEncoder && videoCodec->pix_fmts)
    {
        //优先挑选能直接提供的软格式
        const AVPixelFormat* pf = videoCodec->pix_fmts;
        AVPixelFormat picked = AV_PIX_FMT_NONE;
        while (*pf != AV_PIX_FMT_NONE)
        {
            if (*pf == AV_PIX_FMT_YUV420P || *pf == AV_PIX_FMT_NV12)
            {
                picked = *pf;
                break;
            }
            pf++;
        }

        if (picked != AV_PIX_FMT_NONE)
        {
            ctx->pix_fmt = picked;
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 硬件编码尝试使用兼容像素格式: %d", ctx->pix_fmt);
        }
        else
        {
            //不支持的硬表面格式，直接放弃硬件(后续版本需要分局不同的硬件编码器，分离底层)
            DEBUG_CONSOLE_FMT(ConsoleHandle,
                L"[主线程]: 硬件编码器(%s) 仅列出硬件表面 / 不兼容格式，回退到软件H264",
                LARSC::c2w(m_string_videoEncoder.c_str()));
            m_string_videoEncoder.clear();
            isHardwareEncoder = false;
            ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        }
    }

    // 对于MP4格式，设置全局头标志(必要参数)
    if (m_Recording_Context.formatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // 使用极简参数打开编码器
    AVDictionary* opts = nullptr;

    // 对于软件编码器，需要基本预设
    if (!isHardwareEncoder)
    {
        DB(ConsoleHandle, L"设置软件编码器预设值");
        av_opt_set(ctx->priv_data, "preset", "medium", 0);
        av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
        av_opt_set(ctx->priv_data, "prifile", "main", 0);
        av_opt_set(ctx->priv_data, "b-pyramid", "none", 0);
        av_opt_set(ctx->priv_data, "crf", "23", 0);
    }

    // 尝试打开编码器
    int ret = avcodec_open2(ctx, videoCodec, &opts);
    av_dict_free(&opts);

    if (ret < 0)
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 打开视频编码器失败: %s", LARSC::c2w(errbuf));

        if (isHardwareEncoder)
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle,
                L"[主线程]: 硬件编码器 (%s) 初始化失败，开始回退到软件H264编码器...",
                LARSC::c2w(m_string_videoEncoder.c_str()));
            m_string_videoEncoder.clear(); //清空当前记录的硬件编码器名称，表示已不再使用
            bool fb = SetupVideoStreamFallback();
            if (!fb) 
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 回退到软件编码器失败，无法继续录制");
                return false;
            }
            else
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 已成功回退为软件H264编码器 (libx264)");
                return true; 
            }
        }
        return false;
    }

    // 设置流的时基和帧率
    videoStream->time_base = ctx->time_base;
    videoStream->avg_frame_rate = ctx->framerate;
    videoStream->r_frame_rate = ctx->framerate;

    // 将编码器参数复制到流中
    ret = avcodec_parameters_from_context(videoStream->codecpar, ctx);
    if (ret < 0)
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 复制视频编码器参数失败: %s", LARSC::c2w(errbuf));
        return false;
    }
    return true;
}

bool Ui_CameraDisplaySDL::SetupAudioStream()
{
    // 查找音频编码器 - 使用AAC编码器
    const AVCodec* audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!audioCodec) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 找不到AAC编码器");
        return false;
    }

    // 创建音频流
    AVStream* audioStream = avformat_new_stream(m_Recording_Context.formatContext, nullptr);
    if (!audioStream) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 创建音频流失败");
        return false;
    }
    m_Recording_Context.audioStreamIndex = audioStream->index;

    // 配置音频编码器上下文
    m_Recording_Context.audioCodecContext = avcodec_alloc_context3(audioCodec);
    if (!m_Recording_Context.audioCodecContext) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 分配音频编码器上下文失败");
        return false;
    }

    AVCodecContext* ctx = m_Recording_Context.audioCodecContext;
    ctx->codec_id = audioCodec->id;
    ctx->codec_type = AVMEDIA_TYPE_AUDIO;
    ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;  // AAC需要浮点平面格式

    // 使用标准参数
    ctx->bit_rate = 128000;  // 128kbps
    ctx->sample_rate = 44100; // 标准采样率

    // 检查采样率是否支持
    if (audioCodec->supported_samplerates) {
        ctx->sample_rate = audioCodec->supported_samplerates[0]; // 使用第一个支持的采样率
        const int* p = audioCodec->supported_samplerates;
        while (*p) {
            if (*p == 44100) {
                ctx->sample_rate = 44100; // 优先使用44.1kHz
                break;
            }
            p++;
        }
    }

    // 设置声道布局为立体声
    ctx->channel_layout = AV_CH_LAYOUT_STEREO; 
    ctx->channels = 2; 

    // 设置音频时基为采样率
    ctx->time_base = AVRational{ 1, ctx->sample_rate };

    // 对于MP4格式，设置全局头标志
    if (m_Recording_Context.formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // 打开音频编码器
    int ret = avcodec_open2(ctx, audioCodec, nullptr);
    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 打开音频编码器失败: %s", LARSC::c2w(errbuf));
        return false;
    }

    // 确保音频流与编码器时基一致
    audioStream->time_base = ctx->time_base;

    // 将编码器参数复制到流中
    ret = avcodec_parameters_from_context(audioStream->codecpar, ctx);
    if (ret < 0) {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 复制音频编码器参数失败: %s", LARSC::c2w(errbuf));
        return false;
    }

    return true;
}

bool Ui_CameraDisplaySDL::SetupVideoStreamFallback()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 执行回退到软件编码器");

    // 使用软件编码器作为回退
    const AVCodec* videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!videoCodec) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 找不到H264软件编码器进行回退");
        return false;
    }

    // 获取现有的视频流
    AVStream* videoStream = m_Recording_Context.formatContext->streams[m_Recording_Context.videoStreamIndex];

    // 释放原有的编码器上下文
    if (m_Recording_Context.videoCodecContext) 
    {
        avcodec_free_context(&m_Recording_Context.videoCodecContext);
    }

    // 配置视频编码器上下文
    m_Recording_Context.videoCodecContext = avcodec_alloc_context3(videoCodec);
    if (!m_Recording_Context.videoCodecContext) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 分配视频编码器上下文失败");
        return false;
    }

    AVCodecContext* ctx = m_Recording_Context.videoCodecContext;
    ctx->codec_id = videoCodec->id;
    ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ctx->width = m_Int_FrameWidth;
    ctx->height = m_Int_FrameHeight;
    ctx->time_base = AVRational{ 1, 90000 };
    ctx->framerate = AVRational{ m_Int_frameRate, 1 };
    ctx->gop_size = m_Int_frameRate;
    ctx->max_b_frames = 0;
    ctx->bit_rate = 3000000;

    // 软件编码器特定选项
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "preset", "medium", 0);
    av_dict_set(&opts, "profile", "main", 0);
    av_dict_set(&opts, "tune", "fastdecode", 0);
    av_dict_set(&opts, "x264-params", "bframes=0:keyint=30:min-keyint=30", 0);

    // 对于MP4格式，设置全局头标志
    if (m_Recording_Context.formatContext->oformat->flags & AVFMT_GLOBALHEADER) 
    {
        ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // 打开视频编码器
    int ret = avcodec_open2(ctx, videoCodec, &opts);
    if (ret < 0) 
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 回退到软件编码器也失败: %s", LARSC::c2w(errbuf));
        av_dict_free(&opts);
        return false;
    }
    av_dict_free(&opts);

    // 设置流的时基和帧率
    videoStream->time_base = ctx->time_base;
    videoStream->avg_frame_rate = ctx->framerate;
    videoStream->r_frame_rate = ctx->framerate;

    // 将编码器参数复制到流中
    ret = avcodec_parameters_from_context(videoStream->codecpar, ctx);
    if (ret < 0) 
    {
        char errbuf[256];
        av_strerror(ret, errbuf, sizeof(errbuf));
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 复制视频编码器参数失败: %s", LARSC::c2w(errbuf));
        return false;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 成功使用软件H264编码器 (libx264) 进行回退");
    return true;
}

void Ui_CameraDisplaySDL::RecordingThread()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[视频录制线程]: 开始");
    // 计数和统计
    int actualFramesCaptured = 0;
    auto recordingStartTime = std::chrono::steady_clock::now();
    int recordingSeconds = 0;
    int ret = 0;
    // 主录制循环
    while (m_Recording_Context.isActive.load())
    {
        // 更新录制时长统计
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedSinceStart = std::chrono::duration_cast<std::chrono::seconds>(
            currentTime - recordingStartTime).count();

        if (elapsedSinceStart > recordingSeconds)
        {
            recordingSeconds = static_cast<int>(elapsedSinceStart);
            DEBUG_CONSOLE_FMT(ConsoleHandle,
                L"[视频录制线程]: 已录制 %d 秒，捕获 %d 帧，平均帧率 %.1f fps",
                recordingSeconds, actualFramesCaptured,
                static_cast<float>(actualFramesCaptured) / recordingSeconds);
        }

        // 捕获视频帧
        DEBUG_CONSOLE_STR(ConsoleHandle, L"等待编码帧到达...");
        while (!m_InterFaceState_State.m_Bool_IsEncodeFrameAvailable.load())
        {
            std::unique_lock<std::mutex> lock(m_InterFaceThreadProtect_Protect.m_Mutex_IsEncodeFrameAvailable);
            m_InterFaceThreadProtect_Protect.m_CV_IsEncodeFrameAvailable.wait(lock, [this]()
                {
                    return m_InterFaceState_State.m_Bool_IsEncodeFrameAvailable.load();
                });
        }
        DEBUG_CONSOLE_STR(ConsoleHandle, L"新的编码帧到达");
        actualFramesCaptured++;

        // 计算PTS
        auto frameTime = std::chrono::steady_clock::now();
        int64_t elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
            frameTime - recordingStartTime).count();
            m_AVFrame_EncodeFrame->pts = av_rescale_q(
                elapsedUs,
                AVRational{ 1, 1000000 },       // 源：微秒单位
                m_Recording_Context.videoCodecContext->time_base
                );

        // 编码和发送视频帧
        auto codecCtx = m_Recording_Context.videoCodecContext;
        DEBUG_CONSOLE_FMT(ConsoleHandle,
            L"codec: %dx%d pix_fmt=%d time_base=%d/%d\n"
            L"frame: format=%d, w=%d, h=%d, pts=%lld",
            codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
            codecCtx->time_base.num, codecCtx->time_base.den,
            m_AVFrame_EncodeFrame->format, m_AVFrame_EncodeFrame->width, 
            m_AVFrame_EncodeFrame->height, m_AVFrame_EncodeFrame->pts);
        ret = avcodec_send_frame(m_Recording_Context.videoCodecContext, m_AVFrame_EncodeFrame);
        if (ret < 0)
        {
            char errbuf[256];
            av_strerror(ret, errbuf, sizeof(errbuf));
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[视频录制线程]: 发送视频帧失败: %s", LARSC::c2w(errbuf));
        }
        else
        {
            // 接收并写入所有可用的编码帧
            std::lock_guard<std::mutex> lock(m_Recording_Context.formatMutex);
            ReceiveAndWritePackets(m_Recording_Context.videoCodecContext,
                m_Recording_Context.videoStreamIndex, false);
        }
        av_frame_unref(m_AVFrame_EncodeFrame);
        m_InterFaceState_State.m_Bool_IsEncodeFrameAvailable.store(false);
    }

    // 刷新编码器缓冲区
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[视频录制线程]: 刷新视频编码器缓冲区");
    avcodec_send_frame(m_Recording_Context.videoCodecContext, nullptr);
    {
        std::lock_guard<std::mutex> lock(m_Recording_Context.formatMutex);
        ReceiveAndWritePackets(m_Recording_Context.videoCodecContext,
            m_Recording_Context.videoStreamIndex, true);
    }

    // 输出最终统计信息
    float avgFrameRate = (recordingSeconds > 0) ?
        static_cast<float>(actualFramesCaptured) / recordingSeconds : 0;

    DEBUG_CONSOLE_FMT(ConsoleHandle,
        L"[视频录制线程]: 结束，共录制 %d 秒，捕获 %d 视频帧",
        recordingSeconds, actualFramesCaptured);
    DEBUG_CONSOLE_FMT(ConsoleHandle,
        L"[视频录制线程]: 平均帧率 %.1f fps，理论帧率 %d fps",
        avgFrameRate, m_Int_frameRate);
}

void Ui_CameraDisplaySDL::AudioRecordingThread()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[音频录制线程]: 开始");

    // 初始化时间戳和帧计数变量
    int64_t currentFrameTime = 0;
    int64_t pts = 0, lastpts = 0;
    AVFrame* audioFrame = av_frame_alloc();

    // 计算帧持续时间（微秒）
    int64_t frameDurationTime = (int64_t)(m_Recording_Context.audioCodecContext->frame_size * 1000000) /
        m_Recording_Context.audioCodecContext->sample_rate;

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: 音频帧持续时间: %lld 微秒, 每帧样本数: %d",
        frameDurationTime, m_Recording_Context.audioCodecContext->frame_size);

    // 初始化音频包
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    int captureSize = 0;

    // 创建麦克风捕获对象
    CString MicroDevice;
    App.m_Dlg_Main.m_Dlg_Config->m_Btn_MicroDevice.GetWindowTextW(MicroDevice);
    std::unique_ptr<MicrophoneCapture> microCapture = std::make_unique<MicrophoneCapture>(
        static_cast<MircroSampleRate>(m_Int_SampleRate),
        static_cast<MircroBitRate>(m_Int_BitRate * 1000),
        MircroFmt::MP4);

    std::wstring MicroDeviceW = MicroDevice;
    if (!microCapture->Init(MicroDeviceW))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[音频录制线程]: 初始化麦克风捕获失败");
        av_frame_free(&audioFrame);
        return;
    }

    microCapture->StartCapture();
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[音频录制线程]: 麦克风捕获启动成功");

    // 等待麦克风启动稳定
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 获取录制开始时间基准
    auto recordingStartTime = std::chrono::steady_clock::now();
    int64_t baseTime = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    // 设置下一帧的处理时间
    int64_t nextFrameTime = 0; // 相对于baseTime的时间
    int64_t encodeCompleteTime = 0;
    int audioFrameCount = 0;

    // 主录制循环
    while (m_Recording_Context.isActive.load())
    {
        // 捕获麦克风音频
        void* audioData;
        bool captureOk = microCapture->CaptureMicroFrame(&audioData, &captureSize);

        // 获取当前时间
        currentFrameTime = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() - baseTime;

        // 当时间达到下一帧时间点且有音频数据时处理
        if (currentFrameTime >= nextFrameTime && captureSize != 0 && captureOk && audioData)
        {
            audioFrameCount++;
            AVFrame* capturedFrame = static_cast<AVFrame*>(audioData);

            // 计算PTS - 使用固定时间间隔
            pts = av_rescale_q(
                nextFrameTime,
                AVRational{ 1, 1000000 },
                m_Recording_Context.audioCodecContext->time_base
            );

            // 确保PTS单调递增
            if (lastpts >= pts) {
                pts = lastpts + m_Recording_Context.audioCodecContext->frame_size;
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: PTS调整为 %lld (确保单调递增)", pts);
            }

            // 根据不同格式调整音量大小
            if (m_Recording_Context.audioCodecContext->sample_fmt == AV_SAMPLE_FMT_FLTP)
            {
                int Channels = capturedFrame->channels; 
                int Samples = capturedFrame->nb_samples;
                for (int ch = 0; ch < Channels; ch++)
                {
                    float* audioData = reinterpret_cast<float*>(capturedFrame->data[ch]);
                    for (int i = 0; i < Samples; i++)
                    {
                        float volumeFactor = 1.0f;
                        audioData[i] *= volumeFactor;
                        if (audioData[i] > 1.0f) audioData[i] = 1.0f;
                        else if (audioData[i] < -1.0f) audioData[i] = -1.0f;
                    }
                }
            }
            else if (m_Recording_Context.audioCodecContext->sample_fmt == AV_SAMPLE_FMT_S16)
            {
                // S16格式处理
                int channels = capturedFrame->channels; 
                int samples = capturedFrame->nb_samples;
                int16_t* audioData = (int16_t*)capturedFrame->data[0];
                float volumeFactor = 1.0f;
                for (int i = 0; i < samples * channels; i++)
                {
                    int32_t sample = (int32_t)(audioData[i] * volumeFactor);
                    if (sample > 32767) sample = 32767;
                    if (sample < -32768) sample = -32768;
                    audioData[i] = (int16_t)sample;
                }
            }

            // 准备帧结构 
            capturedFrame->pts = pts;
            capturedFrame->sample_rate = m_Recording_Context.audioCodecContext->sample_rate;
            capturedFrame->nb_samples = m_Recording_Context.audioCodecContext->frame_size;
            capturedFrame->format = m_Recording_Context.audioCodecContext->sample_fmt;
            capturedFrame->channel_layout = m_Recording_Context.audioCodecContext->channel_layout;
            capturedFrame->channels = m_Recording_Context.audioCodecContext->channels;

            if (audioFrameCount % 100 == 1) {
                DEBUG_CONSOLE_FMT(ConsoleHandle,
                    L"[音频录制线程]: 帧 #%d, PTS: %lld, 时间: %.2f 秒",
                    audioFrameCount, pts, (double)nextFrameTime / 1000000.0);
            }

            // 编码帧
            int ret = avcodec_send_frame(m_Recording_Context.audioCodecContext, capturedFrame);
            if (ret < 0)
            {
                char errbuf[256];
                av_strerror(ret, errbuf, sizeof(errbuf));
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: 发送音频帧失败: %s", LARSC::c2w(errbuf));
                continue;
            }

            ret = avcodec_receive_packet(m_Recording_Context.audioCodecContext, &packet);
            if (ret < 0)
            {
                char errbuf[256];
                av_strerror(ret, errbuf, sizeof(errbuf));
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: 接收音频包失败: %s", LARSC::c2w(errbuf));
                continue;
            }

            // 设置包属性
            packet.stream_index = m_Recording_Context.audioStreamIndex;
            packet.pts = packet.dts = pts;
            packet.duration = m_Recording_Context.audioCodecContext->frame_size;

            // 调整时间戳到流时基
            av_packet_rescale_ts(&packet,
                m_Recording_Context.audioCodecContext->time_base,
                m_Recording_Context.formatContext->streams[m_Recording_Context.audioStreamIndex]->time_base);

            // 写入音频包到文件
            {
                std::lock_guard<std::mutex> lock(m_Recording_Context.formatMutex);
                ret = av_interleaved_write_frame(m_Recording_Context.formatContext, &packet);
                if (ret < 0)
                {
                    char errbuf[256];
                    av_strerror(ret, errbuf, sizeof(errbuf));
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: 写入音频包失败: %s", LARSC::c2w(errbuf));
                }
            }

            av_packet_unref(&packet);

            // 更新时间戳和下一帧时间点
            lastpts = pts;
            nextFrameTime += frameDurationTime;
            encodeCompleteTime = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count() - baseTime;

            // 时间追赶机制：如果处理太慢，跳到当前时间
            if (currentFrameTime - frameDurationTime > nextFrameTime)
            {
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: 时间追赶，从 %lld 到 %lld 微秒",
                    nextFrameTime, currentFrameTime);
                nextFrameTime = currentFrameTime;
            }

            // 如果处理太快，等待到下一帧时间
            if (currentFrameTime < nextFrameTime)
            {
                int64_t waitTime = nextFrameTime - currentFrameTime;
                std::this_thread::sleep_for(std::chrono::microseconds(waitTime));
            }
        }
        else
        {
            // 没有数据或未到时间，短暂等待
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    // 停止和清理
    microCapture->StopCapture();
    av_frame_free(&audioFrame);

    // 刷新音频编码器
    avcodec_send_frame(m_Recording_Context.audioCodecContext, nullptr);
    while (true)
    {
        int ret = avcodec_receive_packet(m_Recording_Context.audioCodecContext, &packet);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
            break;

        if (ret >= 0)
        {
            packet.stream_index = m_Recording_Context.audioStreamIndex;
            av_packet_rescale_ts(&packet,
                m_Recording_Context.audioCodecContext->time_base,
                m_Recording_Context.formatContext->streams[m_Recording_Context.audioStreamIndex]->time_base);

            {
                std::lock_guard<std::mutex> lock(m_Recording_Context.formatMutex);
                av_interleaved_write_frame(m_Recording_Context.formatContext, &packet);
            }

            av_packet_unref(&packet);
        }
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[音频录制线程]: 结束, 共录制 %d 音频帧", audioFrameCount);
}

void Ui_CameraDisplaySDL::ReceiveAndWritePackets(AVCodecContext* codecContext, int streamIndex, bool flush)
{
    AVPacket* pkt = av_packet_alloc();
    int ret = 0;
    int packetCount = 0;

    while (true) {
        ret = avcodec_receive_packet(codecContext, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            char errbuf[256];
            av_strerror(ret, errbuf, sizeof(errbuf));
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[录制线程]: 接收编码包失败: %s", LARSC::c2w(errbuf));
            break;
        }

        // 设置正确的流索引
        pkt->stream_index = streamIndex;

        // 确保时间戳不为空
        if (pkt->pts == AV_NOPTS_VALUE) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[录制线程]: 警告 - 包有无效PTS");
            continue; // 跳过无效时间戳的包
        }

        // 调整时间戳 - 从编码器时基转换到流时基
        av_packet_rescale_ts(pkt, codecContext->time_base,
            m_Recording_Context.formatContext->streams[streamIndex]->time_base);

        // 确保DTS递增且不为负
        if (pkt->dts < 0)
        {
            pkt->dts = 0;
        }

        // 确保PTS >= DTS
        if (pkt->pts < pkt->dts)
        {
            pkt->pts = pkt->dts;
        }

        // 写入包前确保所有字段设置正确
        pkt->pos = -1;  // 字节位置未知

        // 写入包
        ret = av_interleaved_write_frame(m_Recording_Context.formatContext, pkt);
        if (ret < 0)
        {
            char errbuf[256];
            av_strerror(ret, errbuf, sizeof(errbuf));
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[录制线程]: 写入媒体包失败: %s", LARSC::c2w(errbuf));

            // 尝试使用非交叉写入模式
            ret = av_write_frame(m_Recording_Context.formatContext, pkt);
            if (ret < 0) {
                av_strerror(ret, errbuf, sizeof(errbuf));
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[录制线程]: 备用方法写入媒体包也失败: %s", LARSC::c2w(errbuf));
            }
        }

        packetCount++;
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);

    if (flush && packetCount > 0)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[录制线程]: 编码器缓冲区刷新完成，写入了 %d 个包", packetCount);
    }
}

void Ui_CameraDisplaySDL::CleanupRecording()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 正在清理录制资源...");

    // 关闭编码器
    if (m_Recording_Context.videoCodecContext) {
        avcodec_free_context(&m_Recording_Context.videoCodecContext);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 释放视频编码器");
    }

    if (m_Recording_Context.audioCodecContext) {
        avcodec_free_context(&m_Recording_Context.audioCodecContext);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 释放音频编码器");
    }

    // 关闭输出文件
    if (m_Recording_Context.formatContext) {
        // 确保已写入文件尾
        if (m_Recording_Context.formatContext->pb &&
            !(m_Recording_Context.formatContext->oformat->flags & AVFMT_NOFILE)) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 关闭输出文件");
            avio_closep(&m_Recording_Context.formatContext->pb);
        }

        DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 释放格式上下文");
        avformat_free_context(m_Recording_Context.formatContext);
        m_Recording_Context.formatContext = nullptr;
    }

    // 清空队列
    {
        std::lock_guard<std::mutex> lockVideo(m_Recording_Context.videoQueueMutex);
        std::queue<VideoPacket> empty;
        std::swap(m_Recording_Context.videoPacketQueue, empty);
    }

    {
        std::lock_guard<std::mutex> lockAudio(m_Recording_Context.audioQueueMutex);
        std::queue<AudioPacket> empty;
        std::swap(m_Recording_Context.audioPacketQueue, empty);
    }

    m_Recording_Context.videoStreamIndex = -1;
    m_Recording_Context.audioStreamIndex = -1;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 录制资源清理完成");
}