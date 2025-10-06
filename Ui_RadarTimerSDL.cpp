#include "stdafx.h"
#include "CDebug.h"
#include "Ui_RadarTimerSDL.h"
#include "Ui_AreaRecordingSDL.h"
#include "Ui_CameraPreviewSDL.h"
#include <math.h>
#include <SDL2_gfxPrimitives.h>
#define _USE_MATH_DEFINES
extern HANDLE ConsoleHandle;

// 调试控制台句柄
extern HANDLE ConsoleHandle;
Ui_RadarTimerSDL* Ui_RadarTimerSDL::s_Instance = nullptr;
bool Ui_RadarTimerSDL::s_IsRunning = false;
Ui_RadarTimerSDL::Ui_RadarTimerSDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr),
    m_TTF_Font(nullptr),
    m_TTF_LargeFont(nullptr),
    m_Texture_CountdownText(nullptr),
    m_Rect_CountdownText({ 0, 0, 0, 0 }),
    m_Bool_Running(false),
    m_Bool_SDLInitialized(false),
    m_Bool_TTFInitialized(false),
    m_Uint32_LastTime(0),
    m_Int_CountdownValue(10),
    m_Int_LastCountdown(10),
    m_Int_LastCountdownValue(-1),
    m_Float_AnimationTime(0.0f),
    m_Bool_ShowingAnimation(false),
    m_Scale(1.0f),
    m_Func_CountdownCallback(nullptr),
    m_IsAutoRelease(false),
    m_SDL_Transparent(nullptr)
{
    m_Font_Brighter = nullptr;
    m_Font_Draker = nullptr;
    m_int_BannerRaduis = 5 * m_Scale;
    // 构造函数初始化类成员
    m_Color_Circle = { 73, 73, 73 };
    m_Color_Rect = { 15,16,16 };
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: Ui_RadarTimerSDL构造");
}

Ui_RadarTimerSDL::~Ui_RadarTimerSDL()
{
    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: Ui_RadarTimerSDL析构");
    Close();
}

Ui_RadarTimerSDL* Ui_RadarTimerSDL::GetInstance()
{
    if (!s_Instance)
    {
        s_Instance = new Ui_RadarTimerSDL;
    }
    return s_Instance;
}

void Ui_RadarTimerSDL::ReleaseInstance()
{
    if (s_Instance)
    {
        delete s_Instance;
        s_Instance = nullptr;
    }
}

bool Ui_RadarTimerSDL::Initialize(const CRect& DisplayArea, float Scale)
{
    // 初始化成员
    m_Scale = Scale;
    m_Int_WindowWidth = DisplayArea.Width();
    m_Int_WindowHeight = DisplayArea.Height();
    m_Rect_WindowArea = DisplayArea;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL倒计时窗口]: 初始化区域 左=%d 上=%d 宽=%d 高=%d",
        DisplayArea.left, DisplayArea.top, DisplayArea.Width(), DisplayArea.Height());

    if (DisplayArea.Width() <= 0 || DisplayArea.Height() <= 0)// 验证区域尺寸是否有效
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 错误 - 区域尺寸无效");
        return false;
    }

    // 初始化时间
    m_Uint32_LastTime = SDL_GetTicks();
    return true;
}

bool Ui_RadarTimerSDL::InitializeSDL()
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    if (SDL_WasInit(SDL_INIT_VIDEO)) // 检查SDL是否已经初始化
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL视频子系统已初始化");
        m_Bool_SDLInitialized = true;
        return true;
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    { // 只初始化视频子系统，避免不必要的初始化
        DebugSDLError(L"[SDL倒计时窗口]:SDL初始化失败:");
        return false;
    }
    m_Bool_SDLInitialized = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL初始化成功");
    return true;
}

bool Ui_RadarTimerSDL::InitializeTTF()
{
    if (m_Bool_TTFInitialized)
    { // 如果TTF已初始化，直接返回成功
        return true;
    }
    if (TTF_Init() == -1)   // 初始化SDL_ttf库
    {
        DebugSDLError(L"[SDL倒计时窗口]: TTF初始化失败:");
        return false;
    }
    m_Bool_TTFInitialized = true;

    // 加载字体
    if (!LoadFonts())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 字体加载失败");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: TTF初始化成功");
    return true;
}

bool Ui_RadarTimerSDL::LoadFonts()
{
    // 获取系统字体路径
    wchar_t windowsDir[MAX_PATH];
    GetWindowsDirectoryW(windowsDir, MAX_PATH);
    std::wstring fontPath = std::wstring(windowsDir) + L"\\Fonts\\msyhbd.ttc"; // 使用微软雅黑粗体

    // 转换为UTF-8编码
    std::string utf8Path;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0)
    {
        utf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
    }

    // 尝试加载粗体 - 如果不成功则尝试常规字体
    m_TTF_LargeFont = TTF_OpenFont(utf8Path.c_str(), 170 * (int)m_Scale);

    if (!m_TTF_LargeFont) 
    {
        fontPath = std::wstring(windowsDir) + L"\\Fonts\\msyh.ttc"; // 尝试常规微软雅黑
        utf8Size = WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0)
        {
            utf8Path.resize(utf8Size);
            WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
        }

        m_TTF_LargeFont = TTF_OpenFont(utf8Path.c_str(), 180 * (int)m_Scale);
    }

    // 加载普通字体
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16 * (int)m_Scale);

    if (!m_TTF_Font || !m_TTF_LargeFont)
    {
        // 尝试加载备用字体
        std::wstring backupFontPath = std::wstring(windowsDir) + L"\\Fonts\\simhei.ttf"; // 备用黑体字体
        utf8Size = WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0)
        {
            utf8Path.resize(utf8Size);
            WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
        }

        if (!m_TTF_Font)
            m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16 * (int)m_Scale);

        if (!m_TTF_LargeFont)
            m_TTF_LargeFont = TTF_OpenFont(utf8Path.c_str(), 180 * (int)m_Scale);

        if (!m_TTF_Font || !m_TTF_LargeFont)
        {
            const char* ttfError = TTF_GetError();
            wchar_t wTtfError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wTtfError, _countof(wTtfError), ttfError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL倒计时窗口]: 字体加载失败: %s", wTtfError);
            return false;
        }
    }

    // 设置字体样式
    TTF_SetFontStyle(m_TTF_LargeFont, TTF_STYLE_NORMAL);

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 字体加载成功");
    return true;
}

void Ui_RadarTimerSDL::DebugSDLError(wchar_t error[256])
{
    const char* sdlError = SDL_GetError();
    wchar_t wSdlError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s: %s", error, wSdlError);
}

bool Ui_RadarTimerSDL::CreateSDLWindow()
{
    // 确保SDL已初始化
    if (!m_Bool_SDLInitialized)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 无法创建窗口 - SDL未初始化");
        return false;
    }

    Uint32 windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS |
        SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_SKIP_TASKBAR;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 创建SDL窗口");
    m_SDL_Window = SDL_CreateWindow(
        "CountdownWindow",    // 窗口标题
        m_Rect_WindowArea.left, m_Rect_WindowArea.top,         // 位置
        m_Rect_WindowArea.Width(), m_Rect_WindowArea.Height(),     // 大小
        windowFlags        // 标志
    );
    if (!m_SDL_Window)
    { // 获取SDL错误消息
        DebugSDLError(L"[SDL倒计时窗口]: 窗口创建失败:");
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 窗口创建成功");

    // 创建硬件渲染器，带垂直同步
    m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!m_SDL_Renderer)
    {
        // 尝试创建软件渲染器作为后备方案
        m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_SOFTWARE);
        if (!m_SDL_Renderer)
        {
            DebugSDLError(L"[SDL倒计时窗口]: 渲染器创建失败:");
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
            return false;
        }
    }

    if (!COMAPI::SDL::InitWindowOpacity(
        m_SDL_Renderer, m_SDL_Window, &m_SDL_Transparent,
        m_Rect_WindowArea.Width(), m_Rect_WindowArea.Height(),
        &m_Pixbuffer, &m_Hwnd
    ))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"倒计时窗口透明化失败!");
        return false;
    }
    return true;
}

void Ui_RadarTimerSDL::DrawPerfectAntiAliasedCircle(int centerX, int centerY, int radius, SDL_Color fillColor, SDL_Color borderColor, int borderWidth)
{
    // 创建一个更大的临时表面用于像素级绘制
    int bufferSize = (radius + borderWidth + 4) * 2;

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, bufferSize, bufferSize, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) return;

    // 锁定表面进行像素级操作
    SDL_LockSurface(surface);

    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4; // 转换为像素数

    int bufferCenter = bufferSize / 2;

    // 使用超采样进行抗锯齿
    const int sampleCount = 4; // 每个像素采样4x4=16次
    const float sampleOffset = 1.0f / sampleCount;

    for (int y = 0; y < bufferSize; y++) {
        for (int x = 0; x < bufferSize; x++) {
            float totalAlpha = 0.0f;
            SDL_Color finalColor = fillColor;
            bool isBorder = false;

            // 对每个像素进行超采样
            for (int sy = 0; sy < sampleCount; sy++) {
                for (int sx = 0; sx < sampleCount; sx++) {
                    float sampleX = x + (sx + 0.5f) * sampleOffset;
                    float sampleY = y + (sy + 0.5f) * sampleOffset;

                    float dx = sampleX - bufferCenter;
                    float dy = sampleY - bufferCenter;
                    float distance = sqrtf(dx * dx + dy * dy);

                    float alpha = 0.0f;

                    if (distance <= radius - borderWidth) {
                        // 内部填充区域
                        alpha = 1.0f;
                        finalColor = fillColor;
                    }
                    else if (distance <= radius) {
                        // 边框区域
                        alpha = 1.0f;
                        finalColor = borderColor;
                        isBorder = true;
                    }
                    else if (distance <= radius + 1.0f) {
                        // 外边缘平滑过渡
                        alpha = 1.0f - (distance - radius);
                        finalColor = borderColor;
                        isBorder = true;
                    }

                    totalAlpha += alpha;
                }
            }

            // 计算最终透明度
            totalAlpha /= (sampleCount * sampleCount);

            if (totalAlpha > 0.0f) {
                Uint8 finalAlpha = (Uint8)(totalAlpha * 255);
                Uint32 pixelValue = SDL_MapRGBA(surface->format,
                    finalColor.r, finalColor.g, finalColor.b, finalAlpha);
                pixels[y * pitch + x] = pixelValue;
            }
            else {
                pixels[y * pitch + x] = 0; // 完全透明
            }
        }
    }

    SDL_UnlockSurface(surface);

    // 创建纹理
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_SDL_Renderer, surface);
    SDL_FreeSurface(surface);

    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        SDL_Rect destRect = {
            centerX - bufferSize / 2,
            centerY - bufferSize / 2,
            bufferSize,
            bufferSize
        };

        SDL_RenderCopy(m_SDL_Renderer, texture, nullptr, &destRect);
        SDL_DestroyTexture(texture);
    }
}

void Ui_RadarTimerSDL::Close()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 正在清理SDL资源");
    m_Bool_Running = false;
    if (m_Thread_WindowThread.joinable())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 等待窗口线程结束...");
        try {
            m_Thread_WindowThread.join();
        }
        catch (const std::system_error& e) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 线程结束出错");
        }
    }
    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");

    //清除圆形纹理
    if (m_CircleTexture) {
        SDL_DestroyTexture(m_CircleTexture);
        m_CircleTexture = nullptr;
    }

    // 清理倒计时纹理
    if (m_Texture_CountdownText) {
        SDL_DestroyTexture(m_Texture_CountdownText);
        m_Texture_CountdownText = nullptr;
    }

    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");
    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");

    // 清理字体资源
    if (m_TTF_LargeFont)
    {
        TTF_CloseFont(m_TTF_LargeFont);
        m_TTF_LargeFont = nullptr;
    }

    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");

    if (m_TTF_Font)
    {
        TTF_CloseFont(m_TTF_Font);
        m_TTF_Font = nullptr;
    }

    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");

    // 清理渲染器
    if (m_SDL_Renderer)
    {
        SDL_DestroyRenderer(m_SDL_Renderer);
        m_SDL_Renderer = nullptr;
    }

    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");

    // 清理窗口
    if (m_SDL_Window)
    {
        SDL_DestroyWindow(m_SDL_Window);
        m_SDL_Window = nullptr;
    }

    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");

    if (m_Bool_TTFInitialized)
    {
        TTF_Quit();
        m_Bool_TTFInitialized = false;
    }

    DB(ConsoleHandle, L"[SDL倒计时窗口]:窗口线程已结束");
    m_Bool_SDLInitialized = false;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL资源清理完成");
}

void Ui_RadarTimerSDL::Run()
{
    if (m_IsAutoRelease)
    {
        m_Thread_AutoReleaseThread = std::thread(&Ui_RadarTimerSDL::AutoReleaseThread, this);
        m_Thread_AutoReleaseThread.detach();
    }
    m_Thread_WindowThread = std::thread(&Ui_RadarTimerSDL::WindowThread, this);
}

bool Ui_RadarTimerSDL::InitBotBanner()
{
    //加载字体
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_Brighter, L"msyh.ttc", 15 * m_Scale))
    {
        DB(ConsoleHandle, L"InitBotBanner:font load failed!");
        return false;
    }
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_Draker, L"msyh.ttc", 15 * m_Scale))
    {
        DB(ConsoleHandle, L"InitBotBanner:font load failed!");
        return false;
    }

    SDL_Color darkColor{ 170,170,170,255 };
    SDL_Color BrightColor{ 255,255,255,255 };
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer,
        m_Font_Draker,
        L"控制项",
        darkColor,
        &m_txt_Header1.tex,
        &m_txt_Header1.texR.w, &m_txt_Header1.texR.h
    ))
    {
        DB(ConsoleHandle, L"InitBotBanner:font tex load failed!");
        return false;
    }
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer,
        m_Font_Draker,
        L"快捷键",
        BrightColor,
        &m_txt_Header2.tex,
        &m_txt_Header2.texR.w, &m_txt_Header2.texR.h
    ))
    {
        DB(ConsoleHandle, L"InitBotBanner:font tex load failed!");
        return false;
    }
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer,
        m_Font_Draker,
        L"状态",
        darkColor,
        &m_txt_Header3.tex,
        &m_txt_Header3.texR.w, &m_txt_Header3.texR.h
    ))
    {
        DB(ConsoleHandle, L"InitBotBanner:font tex load failed!");
        return false;
    }
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer,
        m_Font_Draker,
        L"开始/结束录制",
        BrightColor,
        &m_txt_StartOrStopRecording.tex,
        &m_txt_StartOrStopRecording.texR.w, &m_txt_StartOrStopRecording.texR.h
    ))
    {
        DB(ConsoleHandle, L"InitBotBanner:font tex load failed!");
        return false;
    }
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer,
        m_Font_Draker,
        L"Alt + B",
        BrightColor,
        &m_txt_StartOrStopRecordingShortKey.tex,
        &m_txt_StartOrStopRecordingShortKey.texR.w, &m_txt_StartOrStopRecordingShortKey.texR.h
    ))
    {
        DB(ConsoleHandle, L"InitBotBanner:font tex load failed!");
        return false;
    }
    if (!COMAPI::SDL::TextLoad(
        m_SDL_Renderer,
        m_Font_Draker,
        L"状态",
        BrightColor,
        &m_txt_StartOrStopRecordingState.tex,
        &m_txt_StartOrStopRecordingState.texR.w, &m_txt_StartOrStopRecordingState.texR.h
    ))
    {
        DB(ConsoleHandle, L"InitBotBanner:font tex load failed!");
        return false;
    }
    m_Rect_Banner.w = 400 * m_Scale;
    m_Rect_Banner.h = 55 * m_Scale;
    m_Rect_Banner.x = (m_Int_WindowWidth - m_Rect_Banner.w) / 2;
    m_Rect_Banner.y = m_Int_WindowHeight - m_Rect_Banner.h - 10 * m_Scale;
    SetTableLayout(m_Rect_Banner.x + 50 * m_Scale, m_Rect_Banner.y, 135 * m_Scale, 30 * m_Scale);
    return true;
}

void Ui_RadarTimerSDL::SetAutoRelease(bool IsAutoRelease)
{
    m_IsAutoRelease = IsAutoRelease;
}

void Ui_RadarTimerSDL::WindowThread()
{
    if (!InitializeSDL()) // 分步初始化，先初始化SDL
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL初始化失败");
        return;
    }
    if (!InitializeTTF())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL初始化字体失败");
        return;
    }
    if (!CreateSDLWindow())// 然后创建SDL窗口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL窗口创建失败");
        return;
    }
    if (!InitBotBanner())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL初始化底部字体失败");
        return;
    }

    // 检查窗口和渲染器是否正确初始化
    if (!m_SDL_Window || !m_SDL_Renderer)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 无法运行 - 窗口或渲染器未初始化");
        return;
    }

    m_Bool_Running = true;
    m_Bool_ShowingAnimation = true;
    m_Float_AnimationTime = 0.0f;

    SDL_ShowWindow(m_SDL_Window);
    // 运行SDL窗口的主循环，直到窗口关闭
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL窗口主循环开始运行");
    s_IsRunning = true;
    bool countdownFinished = false;// 标记倒计时是否已经结束
    // 一秒倒计时定时器
    float countdownTimer = 1.0f;

    while (m_Bool_Running)
    {
        // 处理SDL事件
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                m_Bool_Running = false;
                break;
            }
        }

        // 计算时间增量
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - m_Uint32_LastTime) / 1000.0f;

        // 防止时间间隔过大
        if (deltaTime > 0.1f) deltaTime = 0.016f; // 约为60fps

        m_Uint32_LastTime = currentTime;

        // 倒计时逻辑
        if (m_Int_CountdownValue > 0) 
        {
            countdownTimer -= deltaTime;
            if (countdownTimer <= 0.0f) 
            {
                // 每秒触发一次动画
                m_Int_LastCountdown = m_Int_CountdownValue;
                m_Int_CountdownValue--;
                countdownTimer = 1.0f;

                m_Bool_ShowingAnimation = true;
                m_Float_AnimationTime = 0.0f;

                if (m_Int_CountdownValue == 0) {
                    countdownFinished = true;
                }
            }
        }
        else if (countdownFinished) 
        {
            // 倒计时已经归零，并且已标记为完成
            countdownFinished = false;

            // 调用回调函数（如果设置了的话）
            if (m_Func_CountdownCallback)
            {
                try 
                {
                    m_Func_CountdownCallback();
                    m_Bool_Running = false;
                    break;
                }
                catch (const std::exception& e) 
                {
                    // 捕获并记录可能的异常，防止回调中的异常导致整个程序崩溃
                    const char* what = e.what();
                    wchar_t wWhat[512] = { 0 };
                    size_t convertedChars = 0;
                    mbstowcs_s(&convertedChars, wWhat, _countof(wWhat), what, _TRUNCATE);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL倒计时窗口]: 回调函数异常: %s", wWhat);
                }
                catch (...)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: 回调函数发生未知异常");
                }
            }
        }

        // 更新数字动画 
        if (m_Bool_ShowingAnimation)
        {
            m_Float_AnimationTime += deltaTime; 
            if (m_Float_AnimationTime >= 1.0f)
            {
                m_Bool_ShowingAnimation = false;
                m_Float_AnimationTime = 0.0f;
            }
        }

        Render();
        // 不SDL_Delay，因为使用了PRESENTVSYNC
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL倒计时窗口]: SDL窗口主循环结束");
    s_IsRunning = false;

    if (m_IsAutoRelease)
    {
        std::lock_guard<std::mutex> lock(m_Mutex_ActiveRelease);
        m_Bool_ActiveRelease = true;
        m_CV_ActiveRelease.notify_one();
    }
}

void Ui_RadarTimerSDL::AutoReleaseThread()
{
    while (!m_Bool_ActiveRelease)
    {
        std::unique_lock<std::mutex> lock(m_Mutex_ActiveRelease);
        m_CV_ActiveRelease.wait(lock, [this]()
            {
                return m_Bool_ActiveRelease.load();
            });
    }
    Ui_RadarTimerSDL::ReleaseInstance();
}

void Ui_RadarTimerSDL::UpdateCountdownTexture()
{
    if (!m_TTF_LargeFont || !m_SDL_Renderer) return;

    if (m_Int_CountdownValue != m_Int_LastCountdownValue || !m_Texture_CountdownText) {
        // 如果已存在纹理，先销毁
        if (m_Texture_CountdownText) {
            SDL_DestroyTexture(m_Texture_CountdownText);
            m_Texture_CountdownText = nullptr;
        }

        // 创建新的文本纹理
        std::string text = std::to_string(m_Int_CountdownValue);
        SDL_Color textColor = { 255, 255, 255, 255 }; // 纯白色文本

        // 使用描边渲染以增强可视性
        SDL_Surface* textSurface = TTF_RenderText_Blended(m_TTF_LargeFont, text.c_str(), textColor);

        if (textSurface) {
            m_Texture_CountdownText = SDL_CreateTextureFromSurface(m_SDL_Renderer, textSurface);

            if (m_Texture_CountdownText) {
                // 保存文本尺寸供渲染使用
                m_Rect_CountdownText.w = textSurface->w;
                m_Rect_CountdownText.h = textSurface->h;

                // 确保设置纹理的混合模式
                SDL_SetTextureBlendMode(m_Texture_CountdownText, SDL_BLENDMODE_BLEND);
            }
            else {
                DebugSDLError(L"[SDL倒计时窗口]: 创建倒计时纹理失败:");
            }

            SDL_FreeSurface(textSurface);
        }

        m_Int_LastCountdownValue = m_Int_CountdownValue;
    }
}

void Ui_RadarTimerSDL::Render()
{
    if (!m_SDL_Renderer || !m_SDL_Window) return;
    if (SDL_SetRenderTarget(m_SDL_Renderer, m_SDL_Transparent) != 0) 
    {
        DebugSDLError(L"[SDL倒计时窗口]: 设置渲染目标失败:");
        return;
    }

    // 计算窗口中心位置
    int centerX = m_Int_WindowWidth / 2;
    int centerY = m_Int_WindowHeight / 2;

    // 计算圆形半径
    int radius = (int)(min(m_Int_WindowWidth, m_Int_WindowHeight) * 0.4f);

    // 将倒计时圆形向上移动
    int circleOffsetY = -(int)(radius * 0.2f); // 向上移动20%的半径


    RenderCircle(centerX, centerY + circleOffsetY, radius);     // 绘制圆形和边框 
    RenderCountdown(centerX, centerY + circleOffsetY);          // 绘制倒计时数字
    RederBottomBanner();        //绘画底部提示框

    COMAPI::SDL::UpdateArgbFrameToWindow(
        m_SDL_Window, m_SDL_Renderer, 
        m_SDL_Transparent, &m_Pixbuffer, 
        &m_Hwnd, 185
    );
}

void Ui_RadarTimerSDL::RenderCircle(int centerX, int centerY, int radius)
{
    // 检查是否需要重新创建纹理
    if (!m_CircleTexture || m_LastCircleRadius != radius) {
        if (m_CircleTexture) {
            SDL_DestroyTexture(m_CircleTexture);
        }

        SDL_Color fillColor = { m_Color_Rect.r, m_Color_Rect.g, m_Color_Rect.b, 255 };
        SDL_Color borderColor = { m_Color_Circle.r, m_Color_Circle.g, m_Color_Circle.b, 255 };

        // 使用完美抗锯齿方法
        DrawPerfectAntiAliasedCircle(centerX, centerY, radius, fillColor, borderColor, 3);
        m_LastCircleRadius = radius;
        return; 
    }
}

float Ui_RadarTimerSDL::EaseOutQuad(float t)
{
    return t * (2.0f - t); // 二次方缓出函数
}

void Ui_RadarTimerSDL::SetTableLayout(int x, int y, int interval, int celHeight)
{
    int cellWidth = interval;           //单元格长度
    int cellHeight = celHeight; //单元格高度

    // 第一行
    m_txt_Header1.texR.x = x;
    m_txt_Header1.texR.y = y;
    m_txt_Header2.texR.x = x + cellWidth;
    m_txt_Header2.texR.y = y;
    m_txt_Header3.texR.x = x + cellWidth * 2;
    m_txt_Header3.texR.y = y;

    // 第二行
    int secondRowY = y + cellHeight;
    m_txt_StartOrStopRecording.texR.x = x;
    m_txt_StartOrStopRecording.texR.y = secondRowY;
    m_txt_StartOrStopRecordingShortKey.texR.x = x + cellWidth;
    m_txt_StartOrStopRecordingShortKey.texR.y = secondRowY;
    m_txt_StartOrStopRecordingState.texR.x = x + cellWidth * 2;
    m_txt_StartOrStopRecordingState.texR.y = secondRowY;
}

void Ui_RadarTimerSDL::RenderCountdown(int centerX, int centerY)
{
    // 确保倒计时纹理是最新的
    UpdateCountdownTexture();

    if (m_Texture_CountdownText) {
        // 计算动画参数 - 放大和淡出效果
        float scale = 1.0f;
        Uint8 alpha = 255;

        if (m_Bool_ShowingAnimation)
        {
            // 使用缓动函数使动画更自然
            float easedTime = EaseOutQuad(m_Float_AnimationTime);

            // 在整个动画过程中，数字逐渐放大
            scale = 1.0f + easedTime * 0.5f; // 1.0 -> 1.5倍大小

            // 从一开始就渐渐淡出，确保在1秒结束时完全消失
            alpha = (Uint8)((1.0f - easedTime) * 255);
        }

        // 应用透明度
        SDL_SetTextureAlphaMod(m_Texture_CountdownText, alpha);

        // 计算文本位置和大小 - 居中显示
        int textWidth = (int)(m_Rect_CountdownText.w * scale);
        int textHeight = (int)(m_Rect_CountdownText.h * scale);
        SDL_Rect destRect = {
            centerX - textWidth / 2,
            centerY - textHeight / 2,
            textWidth,
            textHeight
        };

        // 渲染主文本
        SDL_RenderCopy(m_SDL_Renderer, m_Texture_CountdownText, NULL, &destRect);
    }
}

void Ui_RadarTimerSDL::RederBottomBanner()
{
    roundedBoxRGBA(
        m_SDL_Renderer,
        m_Rect_Banner.x, m_Rect_Banner.y,
        m_Rect_Banner.x + m_Rect_Banner.w, m_Rect_Banner.y + m_Rect_Banner.h,
        m_int_BannerRaduis,
        5, 5, 5, 255
    );

    // 渲染底部横幅表格中的所有文本元素
    SDL_RenderCopy(m_SDL_Renderer, m_txt_Header1.tex, nullptr, &m_txt_Header1.texR);
    SDL_RenderCopy(m_SDL_Renderer, m_txt_Header2.tex, nullptr, &m_txt_Header2.texR);
    SDL_RenderCopy(m_SDL_Renderer, m_txt_Header3.tex, nullptr, &m_txt_Header3.texR);
    SDL_RenderCopy(m_SDL_Renderer, m_txt_StartOrStopRecording.tex, 
        nullptr, &m_txt_StartOrStopRecording.texR);
    SDL_RenderCopy(m_SDL_Renderer, m_txt_StartOrStopRecordingShortKey.tex, 
        nullptr, &m_txt_StartOrStopRecordingShortKey.texR);
    SDL_RenderCopy(m_SDL_Renderer, m_txt_StartOrStopRecordingState.tex, 
        nullptr, &m_txt_StartOrStopRecordingState.texR);
}
