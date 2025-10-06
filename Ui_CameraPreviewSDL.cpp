#include "stdafx.h"
#include "CDebug.h"
#include "Ui_CameraPreviewSDL.h"
#include "LarStringConversion.h"
// 调试控制台句柄
extern HANDLE ConsoleHandle;
Ui_CameraPreviewSDL* Ui_CameraPreviewSDL::Instance = nullptr;
bool Ui_CameraPreviewSDL::s_IsRunning = false;

Ui_CameraPreviewSDL::Ui_SDLMenuButton::Ui_SDLMenuButton(const SDL_Rect& rect, const std::wstring& text,
    std::function<void()> callback)
    : m_Rect_Area(rect), m_Str_Text(text), m_Func_Callback(callback),
    m_Bool_Hovered(false), m_SDL_TextTexture(nullptr)
{
    // 构造函数初始化按钮属性
}

Ui_CameraPreviewSDL::Ui_SDLMenuButton::~Ui_SDLMenuButton()
{
    // 清理文本纹理资源
    if (m_SDL_TextTexture)
    {
        SDL_DestroyTexture(m_SDL_TextTexture);
        m_SDL_TextTexture = nullptr;
    }
}

void Ui_CameraPreviewSDL::Ui_SDLMenuButton::Render(SDL_Renderer* renderer, TTF_Font* font)
{
    // 安全检查
    if (!renderer) return;
    //绘画按钮
    if (m_Bool_Click)
    {// 按下状态颜色
        SDL_SetRenderDrawColor(renderer, 1, 192, 131, 255);
    }
    else if (m_Bool_Hovered)
    {// 悬停状态颜色
        SDL_SetRenderDrawColor(renderer, 11, 188, 164, 255);
    }
    else
    {// 普通状态颜色
        SDL_SetRenderDrawColor(renderer, 27, 150, 191, 255);
    }
    SDL_RenderFillRect(renderer, &m_Rect_Area);

    //绘画文本
    if (font && !m_Str_Text.empty())
    {
        // 如果文本纹理不存在，创建它
        if (!m_SDL_TextTexture)
        {
            // 转换为UTF-8编码，因为SDL_ttf使用UTF-8
            std::string utf8Text;
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, NULL, 0, NULL, NULL);
            if (utf8Size > 0)
            {
                utf8Text.resize(utf8Size);
                WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, &utf8Text[0], utf8Size, NULL, NULL);
            }

            // 创建文字表面
            SDL_Color textColor = { 255, 255, 255, 255 }; // 白色文本
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, utf8Text.c_str(), textColor);

            if (textSurface)
            {
                // 创建文字纹理
                m_SDL_TextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_FreeSurface(textSurface);
            }
        }

        // 如果文本纹理存在，渲染它
        if (m_SDL_TextTexture)
        {
            // 获取文本纹理大小
            int textWidth, textHeight;
            SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);

            // 计算文本在按钮中的居中位置
            SDL_Rect destRect =
            {
                m_Rect_Area.x + (m_Rect_Area.w - textWidth) / 2,
                m_Rect_Area.y + (m_Rect_Area.h - textHeight) / 2,
                textWidth,
                textHeight
            };

            // 渲染文本
            SDL_RenderCopy(renderer, m_SDL_TextTexture, NULL, &destRect);
        }
    }
}

bool Ui_CameraPreviewSDL::Ui_SDLMenuButton::IsPointInside(int x, int y) const
{
    // 检查坐标是否在按钮区域内
    return (x >= m_Rect_Area.x && x < m_Rect_Area.x + m_Rect_Area.w &&
        y >= m_Rect_Area.y && y < m_Rect_Area.y + m_Rect_Area.h);
}

bool Ui_CameraPreviewSDL::Ui_SDLMenuButton::UpdateHoverState(int x, int y)
{
    // 仅更新悬停状态，不触发回调
    m_Bool_Hovered = IsPointInside(x, y);
    return m_Bool_Hovered;
}

void Ui_CameraPreviewSDL::Ui_SDLMenuButton::UpdateClickState(bool isClick)
{
    m_Bool_Click = isClick;
}

void Ui_CameraPreviewSDL::Ui_SDLMenuButton::Click()
{
    // 执行点击回调
    if (m_Func_Callback) {
        m_Func_Callback();
    }
}

Ui_CameraPreviewSDL::Ui_CameraPreviewSDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr),
    m_TTF_Font(nullptr),
    m_Bool_Running(false),
    m_Bool_SDLInitialized(false),
    m_Bool_TTFInitialized(false)
{
    // 构造函数初始化类成员
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: Ui_CameraPreviewSDL构造");
}

Ui_CameraPreviewSDL::~Ui_CameraPreviewSDL()
{
    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: Ui_CameraPreviewSDL析构");
    Close();

    if (m_AVFrame_Camera)
    {
        av_frame_free(&m_AVFrame_Camera);
        m_AVFrame_Camera = nullptr;
    }
    if (m_SwsCtx_Format)
    {
        sws_freeContext(m_SwsCtx_Format);
        m_SwsCtx_Format = nullptr;
    }
    if (m_Texture_Camera)
    {
        SDL_DestroyTexture(m_Texture_Camera);
        m_Texture_Camera = nullptr;
    }
    if (m_uint8_RenderFrameBuffer)
    {
        free(m_uint8_RenderFrameBuffer);
        m_uint8_RenderFrameBuffer = nullptr;
    }
}

bool Ui_CameraPreviewSDL::InitializeTTF()
{
    if (m_Bool_TTFInitialized)
    { // 如果TTF已初始化，直接返回成功
        return true;
    }
    if (TTF_Init() == -1)   // 初始化SDL_ttf库
    {
        DebugSDLError(L"[SDL摄像头预览窗口]: TTF初始化失败:");
        return false;
    }
    m_Bool_TTFInitialized = true;

    // 加载字体
    if (!CreateFonts())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 字体加载失败");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: TTF初始化成功");
    return true;
}

bool Ui_CameraPreviewSDL::CreateFonts()
{
    // 获取系统字体路径
    wchar_t windowsDir[MAX_PATH];
    GetWindowsDirectoryW(windowsDir, MAX_PATH);
    std::wstring fontPath = std::wstring(windowsDir) + L"\\Fonts\\msyh.ttc"; // 使用微软雅黑字体

    // 转换为UTF-8编码
    std::string utf8Path;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        utf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
    }

    // 加载字体，字号16
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16);
    if (!m_TTF_Font)
    {
        // 尝试加载备用字体
        std::wstring backupFontPath = std::wstring(windowsDir) + L"\\Fonts\\simhei.ttf"; // 备用黑体字体
        utf8Size = WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0)
        {
            utf8Path.resize(utf8Size);
            WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
        }
        m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16);
        if (!m_TTF_Font)
        {
            const char* ttfError = TTF_GetError();
            wchar_t wTtfError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wTtfError, _countof(wTtfError), ttfError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL摄像头预览窗口]: 字体加载失败: %s", wTtfError);
            return false;
        }
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 字体加载成功");
    return true;
}

Ui_CameraPreviewSDL* Ui_CameraPreviewSDL::GetInstance()
{
    if (!Instance)
    {
        Instance = new Ui_CameraPreviewSDL;
    }
    return Instance;
}

void Ui_CameraPreviewSDL::ReleaseInstance()
{
    if (Instance)
    {
        delete Instance;
        Instance = nullptr;
    }
}

bool Ui_CameraPreviewSDL::IsInsExist()
{
    return Instance == nullptr ? false : true;
}

bool Ui_CameraPreviewSDL::IsRunning()
{
    return s_IsRunning;
}

bool Ui_CameraPreviewSDL::SetWindowParam(const CRect& DisplayArea, CameraOptions cameraOption, float Scale)
{
    if (DisplayArea.Width() <= 0 || DisplayArea.Height() <= 0)// 验证录制区域尺寸是否有效
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 错误 - 窗口区域尺寸无效");
        return false;
    }

    // 初始化成员
    m_Scale = Scale;
    m_Rect_WindowArea = DisplayArea;
    m_Int_WindowWidth = DisplayArea.Width();
    m_Int_WindowHeight = DisplayArea.Height();
    m_Int_FrameRate = cameraOption.fps;
    m_AVFrame_Camera = av_frame_alloc();
    m_Struct_CameraOpt = cameraOption;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL摄像头预览窗口]: 初始化窗口区域 左=%d 上=%d 宽=%d 高=%d",
        DisplayArea.left, DisplayArea.top, DisplayArea.Width(), DisplayArea.Height());
    return true;
}

bool Ui_CameraPreviewSDL::Init()
{
    SetAdaptAdjustPointRect();    //设置调整点位置
    if (!m_AVFrame_Camera)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]:帧分配失败");
        return false;
    }
    if (!InitializeSDL()) // 分步初始化，先初始化SDL
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL初始化失败");
        return false;
    }
    if (!InitializeTTF())  // 初始化TTF
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: TTF初始化失败");
        return false;
    }
    if (!CreateSDLWindow())// 然后创建SDL窗口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口创建失败");
        return false;
    }
    if (!InitCameraCapture(m_Struct_CameraOpt))// 初始化捕获器接口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 捕获器接口创建失败");
        return false;
    }
    //CreateButtons(); // 创建界面按钮
    CreateSystemCursor();//创建鼠标光标资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口初始化成功");
    return true;
}

bool Ui_CameraPreviewSDL::InitializeSDL()
{
    if (SDL_WasInit(SDL_INIT_VIDEO)) // 检查SDL是否已经初始化
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL视频子系统已初始化");
        m_Bool_SDLInitialized = true;
        return true;
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    { // 只初始化视频子系统，避免不必要的初始化
        DebugSDLError(L"[SDL摄像头预览窗口]:SDL初始化失败:");
        return false;
    }
    m_Bool_SDLInitialized = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL初始化成功");
    return true;
}

bool Ui_CameraPreviewSDL::InitCameraCapture(CameraOptions cameraOption)
{
    m_Interface_CameraCapture = CameraCapture::GetInstance();//获取捕获器接口
    if (m_Interface_CameraCapture)
    {//初始化捕获器
        return m_Interface_CameraCapture->Init(cameraOption);
    }
    return false;
}

bool Ui_CameraPreviewSDL::InitSwsCtx(AVPixelFormat pixfmt)
{
    //如果是重新初始化，则释放之前的资源
    if (m_SwsCtx_Format)
    {
        sws_freeContext(m_SwsCtx_Format);
        m_SwsCtx_Format = nullptr;
    }
    if (m_uint8_RenderFrameBuffer)
    {
        free(m_uint8_RenderFrameBuffer);
        m_uint8_RenderFrameBuffer = nullptr;
    }

    //初始化根式转换器
    if (!m_SwsCtx_Format)
    {
        m_SwsCtx_Format = sws_getContext(
            m_int_OriFrameWidth, m_int_OriFrameHeight, pixfmt,             // 源格式
            m_int_OriFrameWidth, m_int_OriFrameHeight, AV_PIX_FMT_BGR32,   // 目标格式 (SDL使用的RGBA格式)
            SWS_BILINEAR, NULL, NULL, NULL);
        if (!m_SwsCtx_Format)
            return false;
    }
    //分配原始帧缓冲区内存
    if (!m_uint8_RenderFrameBuffer)
    {
        m_int_RenderFrameLinesize = m_int_OriFrameWidth * 4;
        m_uint8_RenderFrameBuffer = (uint8_t*)malloc(m_int_RenderFrameLinesize * m_int_OriFrameHeight);
        if (!m_uint8_RenderFrameBuffer)
            return false;
    }
    return true;
}

bool Ui_CameraPreviewSDL::InitFrameTexture(SDL_Renderer* render)
{
    if (!m_Texture_Camera)
    {
        m_Texture_Camera = SDL_CreateTexture(render,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            m_int_OriFrameWidth,
            m_int_OriFrameHeight);
        if (m_Texture_Camera)
            return true;
    }
    return false;
}

void Ui_CameraPreviewSDL::DebugSDLError(wchar_t error[256])
{
    const char* sdlError = SDL_GetError();
    wchar_t wSdlError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s: %s", error, wSdlError);
}

bool Ui_CameraPreviewSDL::CreateSDLWindow()
{
    // 确保SDL已初始化
    if (!m_Bool_SDLInitialized)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 无法创建窗口 - SDL未初始化");
        return false;
    }

    // 计算初始窗口位置和大小
    int left = max(0, m_Rect_WindowArea.left);
    int top = max(0, m_Rect_WindowArea.top);
    int width = max(100, min(GetSystemMetrics(SM_CXSCREEN) - left, m_Rect_WindowArea.Width()));
    int height = max(100, min(GetSystemMetrics(SM_CYSCREEN) - top, m_Rect_WindowArea.Height()));

    Uint32 windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 创建SDL窗口");
    m_SDL_Window = SDL_CreateWindow(
        "DeviceWindow",    // 窗口标题
        left, top,         // 位置
        width, height,     // 大小
        windowFlags        // 标志
    );
    if (!m_SDL_Window)
    { // 获取SDL错误消息
        DebugSDLError(L"[SDL摄像头预览窗口]: 窗口创建失败:");
        return false;
    }
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL摄像头预览窗口]: 窗口创建成功，位置=%d,%d 大小=%d,%d",
        left, top, width, height);

    // 创建硬件渲染器 - 这种情况下工作更可靠
    m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_SDL_Renderer) {
        // 尝试创建软件渲染器作为后备方案
        m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_SOFTWARE);
        if (!m_SDL_Renderer)
        {
            DebugSDLError(L"[SDL摄像头预览窗口]: 渲染器创建失败:");
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
            return false;
        }
    }

    //设置为顶层的圆形窗口
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(m_SDL_Window, &wmInfo))
    { // 设置扩展窗口样式 - 永远在最上面
        HWND hwnd = wmInfo.info.win.window;
        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        exStyle |= WS_EX_TOPMOST;
        SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE);// 额外设置窗口位置
    }
    if (!CreateWindowRoundShape())//设置圆形窗口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"设置为圆形窗口失败");
        return false;
    }
    return true;
}

void Ui_CameraPreviewSDL::Close()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 正在清理SDL资源");

    m_Bool_Running = false;
    if (m_Thread_WindowThread.joinable())
    {
        m_Thread_WindowThread.join();
    }

    // 先清空按钮容器，防止后续访问无效的指针
    m_Btn_ControlButtons.clear();

    //清理捕获器
    if (m_Interface_CameraCapture)
    {
        m_Interface_CameraCapture->ReleaseInstance();
    }

    // 清理字体资源
    if (m_TTF_Font)
    {
        TTF_CloseFont(m_TTF_Font);
        m_TTF_Font = nullptr;
    }

    // 清理渲染器
    if (m_SDL_Renderer)
    {
        SDL_DestroyRenderer(m_SDL_Renderer);
        m_SDL_Renderer = nullptr;
    }

    // 清理窗口
    if (m_SDL_Window)
    {
        SDL_DestroyWindow(m_SDL_Window);
        m_SDL_Window = nullptr;
    }

    // 关闭TTF
    if (m_Bool_TTFInitialized)
    {
        TTF_Quit();
        m_Bool_TTFInitialized = false;
    }

    m_Bool_SDLInitialized = false;
    //清理鼠标资源
    if (m_SDLCursor_default)
    {
        SDL_FreeCursor(m_SDLCursor_default);
    }
    if (m_SDLCursor_lr)
    {
        SDL_FreeCursor(m_SDLCursor_lr);
    }
    if (m_SDLCursor_tb)
    {
        SDL_FreeCursor(m_SDLCursor_tb);
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL资源清理完成");
}

void Ui_CameraPreviewSDL::CreateButtons()
{
    //获取窗口宽高，按钮高度   
    int WindowWidth, WindowHeight, BtnHeight, BtnWidth, WindowSpacing;
    SDL_GetWindowSize(m_SDL_Window, &WindowWidth, &WindowHeight);

    // 取消解绑按钮
    float CancaleBindingWidth = 161 * m_Scale;
    float CancaleBindingHeight = 40 * m_Scale;
    float CancaleBindingY = 0.803 * WindowHeight;
    float CancaleBindingX = (WindowWidth - CancaleBindingWidth) / 2;
    m_Btn_ControlButtons.emplace_back(
        SDL_Rect{ (int)CancaleBindingX, (int)CancaleBindingY,
                (int)CancaleBindingWidth, (int)CancaleBindingHeight },
        L"按钮",
        [this]()
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 点击了按钮");
            if (m_Func_OnCancelBindingRecord)
            {
                m_Func_OnCancelBindingRecord();
            }
        }
    );
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL摄像头预览窗口]: 创建了%d个按钮", (int)m_Btn_ControlButtons.size());
}

bool Ui_CameraPreviewSDL::CreateWindowRoundShape()
{
    if (!m_SDL_Window)
        return false;

    //获取窗口信息
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(m_SDL_Window, &wmInfo))
    {
        DebugSDLError(L"[SDL摄像头预览窗口]: 获取窗口信息失败:");
        return false;
    }

    //根据窗口原本大小，创建圆形区域
    int width, height;
    HWND hwnd = wmInfo.info.win.window;// 获取窗口句柄
    SDL_GetWindowSize(m_SDL_Window, &width, &height);  // 获取窗口尺寸
    HRGN hRgn = CreateEllipticRgn(0, 0, width, height);// 创建圆形区域

   // 应用区域到窗口
    if (SetWindowRgn(hwnd, hRgn, TRUE) == 0) 
    {
        DebugSDLError(L"[SDL摄像头预览窗口]: 设置窗口区域失败:");
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 成功设置圆形窗口");
    return true;
}

void Ui_CameraPreviewSDL::CreateSystemCursor()
{
    m_SDLCursor_default = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    m_SDLCursor_lr = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    m_SDLCursor_tb = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
}

void Ui_CameraPreviewSDL::WindowThreadFunc()
{
    if (!Init())//初始化SDL窗口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[摄像头预览SDL窗口]:初始化SDL窗口失败");
        return;
    }

    // 检查窗口和渲染器是否正确初始化
    if (!m_SDL_Window || !m_SDL_Renderer)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 无法运行 - 窗口或渲染器未初始化");
        return;
    }
    SDL_ShowWindow(m_SDL_Window);//显示窗口

    //开启捕获器
    m_Interface_CameraCapture->StartCapture();
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 捕获器开始捕获摄像头");

    
    // 运行SDL窗口的主循环，直到窗口关闭
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口主循环开始运行");
    m_Bool_Running = true;
    s_IsRunning = true;
    while (m_Bool_Running)
    {
        ProcessEvents();
        Render();
        SDL_Delay(8);//120fps
    }
    s_IsRunning = false;

    //关闭捕获器
    m_Interface_CameraCapture->StopCapture();
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口主循环结束");
}

void Ui_CameraPreviewSDL::ProcessEvents()
{
    // 检查窗口是否正确初始化
    if (!m_SDL_Window) {
        m_Bool_Running = false;
        return;
    }

    // 获取当前鼠标位置（在屏幕坐标系中）
    int l_Int_GlobalMouseX, l_Int_GlobalMouseY;
    SDL_GetGlobalMouseState(&l_Int_GlobalMouseX, &l_Int_GlobalMouseY);

    // 处理SDL事件队列中的所有事件
    SDL_Event event;
    bool BtnHovingState = false;
    bool LastBtnHovingState = false;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]:窗口退出");
            m_Bool_Running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                // 检查是否点击了调整点
                AdjustPointType pointType;
                if (IsPointInAdjustPoint(event.button.x, event.button.y, pointType))
                {
                    m_Enum_DraggingPoint = pointType;
                    m_Point_DragStartMousePos = { l_Int_GlobalMouseX, l_Int_GlobalMouseY };

                    // 保存拖拽开始时窗口的位置和大小
                    int x, y;
                    SDL_GetWindowPosition(m_SDL_Window, &x, &y);
                    m_Rect_DragStartWindowRect = { x, y, m_Int_WindowWidth, m_Int_WindowHeight };

                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL摄像头预览窗口]:开始拖拽调整点 %d", (int)pointType);
                }
                else
                {
                    m_Bool_UpdateWindowXY = true;
                    m_Int_dragStartX = l_Int_GlobalMouseX - event.button.x;
                    m_Int_dragStartY = l_Int_GlobalMouseY - event.button.y;
                    m_Int_dragDiffX = event.button.x;
                    m_Int_dragDiffY = event.button.y;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            m_Enum_DraggingPoint = AdjustPointType::None;
            m_Bool_UpdateWindowXY = false;
            break;

        case SDL_MOUSEMOTION:
            if (m_Bool_UpdateWindowXY)//如果是需要拖动窗口的状态
            {
                int diffX = l_Int_GlobalMouseX - m_Int_dragDiffX - m_Int_dragStartX;
                int diffY = l_Int_GlobalMouseY - m_Int_dragDiffY - m_Int_dragStartY;
                m_Rect_WindowArea.left = m_Int_dragStartX + diffX;
                m_Rect_WindowArea.top = m_Int_dragStartY + diffY;
                SDL_SetWindowPosition(
                    m_SDL_Window,
                    m_Rect_WindowArea.left,
                    m_Rect_WindowArea.top
                );
            }
            if (m_Enum_DraggingPoint == AdjustPointType::None)
                SetCursorByPos(event.motion.x, event.motion.y);
            else 
            {
                ResizeWindow(l_Int_GlobalMouseX, l_Int_GlobalMouseY);// 拖拽调整窗口大小
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_LEAVE)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]:鼠标离开SDL窗口");
                m_Bool_IsMouseInWindow.store(false);
            }
            if (event.window.event == SDL_WINDOWEVENT_ENTER)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]:鼠标进入SDL窗口");
                m_Bool_IsMouseInWindow.store(true);
            }
            break;

        case SDL_KEYDOWN:
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]:按键按下");
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 按下ESC键，关闭窗口");
                m_Bool_Running = false;
            }
            break;
        }
    }
}

void Ui_CameraPreviewSDL::Render()
{
    RenderCameraFrame();    //渲染摄像头画面
    RenderAdjustPoint();    //渲染四个调整点

    SDL_RenderPresent(m_SDL_Renderer);//渲染最终内容
}

void Ui_CameraPreviewSDL::RenderCameraFrame()
{
    //渲染摄像头画面
    if (m_Interface_CameraCapture->CaptureFrame(m_AVFrame_Camera))
    {//如果捕获到一帧，则绘制一帧摄像头帧
        if (!m_SwsCtx_Format)//先根据原始帧格式，初始化格式转换器
        {
            //原始帧的帧结构保存
            m_AVPixfmt_OriFmt = (AVPixelFormat)m_AVFrame_Camera->format;
            m_int_OriFrameHeight = m_AVFrame_Camera->height;
            m_int_OriFrameWidth = m_AVFrame_Camera->width;
            if (!InitSwsCtx(m_AVPixfmt_OriFmt))//用保存的帧结构进行转换器初始化
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览线程]:初始化格式转换器或分配渲染缓冲区失败!");
                return;
            }
        }
        if (!m_Texture_Camera)//根据原始帧格式，初始化格式转换器
        {
            if (!InitFrameTexture(m_SDL_Renderer))
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览线程]:初始化纹理失败!");
                return;
            }
        }

        //转换AVFrame到RGB格式
        uint8_t* dstSlice[1] = { m_uint8_RenderFrameBuffer };
        int dstStride[1] = { m_int_RenderFrameLinesize };
        int ret = sws_scale(m_SwsCtx_Format,
            (const uint8_t* const*)m_AVFrame_Camera->data,
            m_AVFrame_Camera->linesize,
            0, m_AVFrame_Camera->height,
            dstSlice, dstStride);
        if (ret < 0)
        {
            char errbuf[256]{ 0 };
            av_strerror(ret, errbuf, 256);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL摄像头预览线程]:帧格式转换失败!:%s", LARSC::c2w(errbuf));
        }

        // 更新SDL纹理
        ret = SDL_UpdateTexture(m_Texture_Camera, NULL, m_uint8_RenderFrameBuffer, m_int_RenderFrameLinesize);
        if (ret != 0)
        {
            DebugSDLError(L"SDL摄像头预览线程]:更新SDL纹理失败!");
            return;
        }

        // 画面填充整个SDL窗口
        SDL_Rect displayRect;
        displayRect.x = 0;
        displayRect.y = 0;
        displayRect.w = m_Int_WindowWidth;
        displayRect.h = m_Int_WindowHeight;

        //渲染结果
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览线程]:渲染一帧摄像头内容");
        SDL_RenderCopy(m_SDL_Renderer, m_Texture_Camera, NULL, &displayRect);// 渲染纹理到渲染器
        av_frame_unref(m_AVFrame_Camera);  // 释放当前原始帧资源
    }
    else
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览线程]:等待原始帧超时,渲染黑屏内容");
        SDL_SetRenderDrawColor(m_SDL_Renderer, 16, 23, 24, 255);
        SDL_RenderClear(m_SDL_Renderer);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Ui_CameraPreviewSDL::RenderAdjustPoint()
{
    //渲染四个调整点
    if (m_Bool_IsMouseInWindow.load())
    {
        // 绘画四个调整点
        SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 120, 255, 255);   // 设置调整点的颜色为蓝色
        SDL_RenderFillRect(m_SDL_Renderer, &m_SDLRect_topPoint);    // 上方调整点
        SDL_RenderFillRect(m_SDL_Renderer, &m_SDLRect_bottomPoint); // 下方调整点
        SDL_RenderFillRect(m_SDL_Renderer, &m_SDLRect_leftPoint);   // 左方调整点
        SDL_RenderFillRect(m_SDL_Renderer, &m_SDLRect_rightPoint);  // 右方调整点
    }
}

void Ui_CameraPreviewSDL::SetCursorByPos(int mouseX,int mouseY)
{
    SDL_Point Point{ mouseX ,mouseY };
    if (SDL_PointInRect(&Point, &m_SDLRect_rightPoint) || SDL_PointInRect(&Point, &m_SDLRect_leftPoint))
    {
        SDL_SetCursor(m_SDLCursor_lr);
    }
    else if (SDL_PointInRect(&Point, &m_SDLRect_topPoint) || SDL_PointInRect(&Point, &m_SDLRect_bottomPoint))
    {
        SDL_SetCursor(m_SDLCursor_tb);
    }
    else
    {
        SDL_SetCursor(m_SDLCursor_default);
    }
}

void Ui_CameraPreviewSDL::SetOnCancelBindingCallback(std::function<void()> callback)
{
    m_Func_OnCancelBindingRecord = callback;
}

void Ui_CameraPreviewSDL::SetRaiseWindow()
{
    SDL_RaiseWindow(m_SDL_Window);
}

void Ui_CameraPreviewSDL::SetAdaptAdjustPointRect()
{
    // 调整点的尺寸
    int pointSize = (int)(10 * m_Scale);

    // 上方调整点
    m_SDLRect_topPoint = {
        m_Int_WindowWidth / 2 - pointSize / 2,
        0,
        pointSize,
        pointSize
    };

    // 下方调整点
    m_SDLRect_bottomPoint = {
        m_Int_WindowWidth / 2 - pointSize / 2,
        m_Int_WindowHeight - pointSize,
        pointSize,
        pointSize
    };

    // 左方调整点
    m_SDLRect_leftPoint = {
        0,
        m_Int_WindowHeight / 2 - pointSize / 2,
        pointSize,
        pointSize
    };

    // 右方调整点
    m_SDLRect_rightPoint = {
        m_Int_WindowWidth - pointSize,
        m_Int_WindowHeight / 2 - pointSize / 2,
        pointSize,
        pointSize
    };
}

void Ui_CameraPreviewSDL::RunWindowThread()
{
    m_Thread_WindowThread = std::thread(&Ui_CameraPreviewSDL::WindowThreadFunc, this);
}

bool Ui_CameraPreviewSDL::IsPointInAdjustPoint(int x, int y, AdjustPointType& outType)
{
    // 检查是否在上方调整点内
    if (x >= m_SDLRect_topPoint.x && x < m_SDLRect_topPoint.x + m_SDLRect_topPoint.w &&
        y >= m_SDLRect_topPoint.y && y < m_SDLRect_topPoint.y + m_SDLRect_topPoint.h)
    {
        outType = AdjustPointType::Top;
        return true;
    }

    // 检查是否在下方调整点内
    if (x >= m_SDLRect_bottomPoint.x && x < m_SDLRect_bottomPoint.x + m_SDLRect_bottomPoint.w &&
        y >= m_SDLRect_bottomPoint.y && y < m_SDLRect_bottomPoint.y + m_SDLRect_bottomPoint.h)
    {
        outType = AdjustPointType::Bottom;
        return true;
    }

    // 检查是否在左方调整点内
    if (x >= m_SDLRect_leftPoint.x && x < m_SDLRect_leftPoint.x + m_SDLRect_leftPoint.w &&
        y >= m_SDLRect_leftPoint.y && y < m_SDLRect_leftPoint.y + m_SDLRect_leftPoint.h)
    {
        outType = AdjustPointType::Left;
        return true;
    }

    // 检查是否在右方调整点内
    if (x >= m_SDLRect_rightPoint.x && x < m_SDLRect_rightPoint.x + m_SDLRect_rightPoint.w &&
        y >= m_SDLRect_rightPoint.y && y < m_SDLRect_rightPoint.y + m_SDLRect_rightPoint.h)
    {
        outType = AdjustPointType::Right;
        return true;
    }

    outType = AdjustPointType::None;
    return false;
}

void Ui_CameraPreviewSDL::ResizeWindow(int mouseX, int mouseY)
{
    if (m_Enum_DraggingPoint == AdjustPointType::None)
        return;

    int newWidth = m_Int_WindowWidth;
    int newHeight = m_Int_WindowHeight;
    int newX = m_Rect_WindowArea.left;
    int newY = m_Rect_WindowArea.top;

    // 获取窗口位置
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_SDL_Window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    // 根据拖拽点类型计算新的窗口大小和位置
    switch (m_Enum_DraggingPoint)
    {
    case AdjustPointType::Right:
    {
        // 右侧调整点：固定左上角，向右调整大小
        int deltaX = mouseX - m_Point_DragStartMousePos.x;
        newWidth = m_Rect_DragStartWindowRect.w + deltaX;

        // 保持等比例
        newHeight = newWidth;
        break;
    }

    case AdjustPointType::Bottom:
    {
        // 底部调整点：固定左上角，向下调整大小
        int deltaY = mouseY - m_Point_DragStartMousePos.y;
        newHeight = m_Rect_DragStartWindowRect.h + deltaY;

        // 保持等比例
        newWidth = newHeight;
        break;
    }

    case AdjustPointType::Top:
    {
        // 顶部调整点：固定右下角，向上调整大小
        int deltaY = m_Point_DragStartMousePos.y - mouseY;

        // 计算新高度并保持等比例
        newHeight = m_Rect_DragStartWindowRect.h + deltaY;
        if (newHeight < 100) newHeight = 100; // 最小尺寸限制

        newWidth = newHeight;

        // 调整位置以固定右下角
        newX = m_Rect_DragStartWindowRect.x + m_Rect_DragStartWindowRect.w - newWidth;
        newY = m_Rect_DragStartWindowRect.y + m_Rect_DragStartWindowRect.h - newHeight;
        break;
    }

    case AdjustPointType::Left:
    {
        // 左侧调整点：固定右下角，向左调整大小
        int deltaX = m_Point_DragStartMousePos.x - mouseX;

        // 计算新宽度并保持等比例
        newWidth = m_Rect_DragStartWindowRect.w + deltaX;
        if (newWidth < 100) newWidth = 100; // 最小尺寸限制

        newHeight = newWidth;

        // 调整位置以固定右下角
        newX = m_Rect_DragStartWindowRect.x + m_Rect_DragStartWindowRect.w - newWidth;
        newY = m_Rect_DragStartWindowRect.y + m_Rect_DragStartWindowRect.h - newHeight;
        break;
    }
    }

    // 应用新的窗口大小和位置
    if (newWidth >= 100 && newHeight >= 100) // 最小尺寸限制
    {
        m_Int_WindowWidth = newWidth;
        m_Int_WindowHeight = newHeight;
        m_Rect_WindowArea.left = newX;
        m_Rect_WindowArea.top = newY;
        m_Rect_WindowArea.right = newX + newWidth;
        m_Rect_WindowArea.bottom = newY + newHeight;

        // 更新窗口位置和大小
        SDL_SetWindowSize(m_SDL_Window, newWidth, newHeight);
        SDL_SetWindowPosition(m_SDL_Window, newX, newY);
        CreateWindowRoundShape();  // 重新设置窗口为圆形
        SetAdaptAdjustPointRect(); // 更新调整点位置
    }
}
