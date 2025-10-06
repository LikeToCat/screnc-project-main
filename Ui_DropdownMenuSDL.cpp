#include "stdafx.h"
#include "CDebug.h"
#include "Ui_DropdownMenuSDL.h"
#include "Ui_DropdownMenuSDL.h"

// 调试控制台句柄
extern HANDLE ConsoleHandle;

//========================================================================
// Ui_SDLButton 实现 - 提供录制控制按钮功能
//========================================================================
bool isPointInRect(int px, int py, const SDL_Rect& rect)
{
    return (px >= rect.x) && (px < rect.x + rect.w)
        && (py >= rect.y) && (py < rect.y + rect.h);
}

Ui_SDLMenuButton::Ui_SDLMenuButton(const SDL_Rect& rect, const std::wstring& text,
    std::function<void()> callback)
    : m_Rect_Area(rect), m_Str_Text(text), m_Func_Callback(callback),
    m_Bool_Hovered(false), m_SDL_TextTexture(nullptr)
{
    // 构造函数初始化按钮属性
}

Ui_SDLMenuButton::~Ui_SDLMenuButton()
{
    // 清理文本纹理资源
    if (m_SDL_TextTexture) {
        SDL_DestroyTexture(m_SDL_TextTexture);
        m_SDL_TextTexture = nullptr;
    }
}

void Ui_SDLMenuButton::Render(SDL_Renderer* renderer, TTF_Font* font)
{
    // 安全检查
    if (!renderer) return;

    //===================================================================
    // 绘制按钮背景 - 使用新的背景色RGB(26,31,37)
    //===================================================================
    if (m_Bool_Hovered) {
        // 悬停状态稍微亮一些
        SDL_SetRenderDrawColor(renderer, 64, 65, 70, 255);
    }
    else {
        // 指定的按钮背景色
        SDL_SetRenderDrawColor(renderer, 37, 39, 46, 255);
    }
    SDL_RenderFillRect(renderer, &m_Rect_Area);

    //===================================================================
    // 使用TTF绘制按钮文本
    //===================================================================
    if (font && !m_Str_Text.empty()) {
        // 如果文本纹理不存在，创建它
        if (!m_SDL_TextTexture) {
            // 转换为UTF-8编码，因为SDL_ttf使用UTF-8
            std::string utf8Text;
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, NULL, 0, NULL, NULL);
            if (utf8Size > 0) {
                utf8Text.resize(utf8Size);
                WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, &utf8Text[0], utf8Size, NULL, NULL);
            }

            // 创建文字表面
            SDL_Color textColor = { 255, 255, 255, 255 }; // 白色文本
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, utf8Text.c_str(), textColor);

            if (textSurface) {
                // 创建文字纹理
                m_SDL_TextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_FreeSurface(textSurface);
            }
        }

        // 如果文本纹理存在，渲染它
        if (m_SDL_TextTexture) {
            // 获取文本纹理大小
            int textWidth, textHeight;
            SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);

            // 计算文本在按钮中的居中位置
            SDL_Rect destRect = {
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

bool Ui_SDLMenuButton::IsPointInside(int x, int y) const
{
    // 检查坐标是否在按钮区域内
    return (x >= m_Rect_Area.x && x < m_Rect_Area.x + m_Rect_Area.w &&
        y >= m_Rect_Area.y && y < m_Rect_Area.y + m_Rect_Area.h);
}

void Ui_SDLMenuButton::UpdateHoverState(int x, int y)
{
    // 仅更新悬停状态，不触发回调
    m_Bool_Hovered = IsPointInside(x, y);
}

void Ui_SDLMenuButton::Click()
{
    // 执行点击回调
    if (m_Func_Callback) {
        m_Func_Callback();
    }
}

//========================================================================
// Ui_DropdownMenuSDL 实现 - 提供区域录制界面
//========================================================================

Ui_DropdownMenuSDL::Ui_DropdownMenuSDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr),
    m_TTF_Font(nullptr),
    m_Bool_Running(false),
    m_Bool_SDLInitialized(false),
    m_Bool_TTFInitialized(false),
    m_Rect_Anchor(nullptr),
    m_WindowX(0),
    m_WindowY(0)
{
    // 构造函数初始化类成员
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: Ui_DropdownMenuSDL构造");
}

Ui_DropdownMenuSDL::~Ui_DropdownMenuSDL()
{
    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: Ui_DropdownMenuSDL析构");
    Close();
}

//========================================================================
// TTF相关函数
//========================================================================

bool Ui_DropdownMenuSDL::InitializeTTF()
{
    // 如果TTF已初始化，直接返回成功
    if (m_Bool_TTFInitialized) {
        return true;
    }

    // 初始化SDL_ttf库
    if (TTF_Init() == -1) {
        const char* ttfError = TTF_GetError();
        wchar_t wTtfError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wTtfError, _countof(wTtfError), ttfError, _TRUNCATE);

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: TTF初始化失败: %s", wTtfError);
        return false;
    }

    m_Bool_TTFInitialized = true;

    // 加载字体
    if (!LoadFonts()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 字体加载失败");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
        return false;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: TTF初始化成功");
    return true;
}

bool Ui_DropdownMenuSDL::LoadFonts()
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
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 13 * m_Scale);
    if (!m_TTF_Font) {
        // 尝试加载备用字体
        std::wstring backupFontPath = std::wstring(windowsDir) + L"\\Fonts\\simhei.ttf"; // 备用黑体字体

        utf8Size = WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0) {
            utf8Path.resize(utf8Size);
            WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
        }

        m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16);

        if (!m_TTF_Font) {
            const char* ttfError = TTF_GetError();
            wchar_t wTtfError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wTtfError, _countof(wTtfError), ttfError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: 字体加载失败: %s", wTtfError);
            return false;
        }
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 字体加载成功");
    return true;
}

void Ui_DropdownMenuSDL::GetDpiScale()
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    if (screen == NULL) {
        AfxMessageBox(L"无法获取屏幕 DC。");
        return;
    }
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);

    // 计算缩放因子（基准 DPI 为 96）
    double scaleX = static_cast<double>(dpiX) / 96.0;
    double scaleY = static_cast<double>(dpiY) / 96.0;
    m_Scale = scaleY;
}

//========================================================================
// 初始化与资源管理函数
//========================================================================
Ui_DropdownMenuSDL* Ui_DropdownMenuSDL::s_ins = nullptr;

Ui_DropdownMenuSDL* Ui_DropdownMenuSDL::GetInstance()
{
    if (!s_ins)
        s_ins = new Ui_DropdownMenuSDL;
    return s_ins;
}

void Ui_DropdownMenuSDL::ReleaseInstance()
{
    m_Bool_Running = false;
    if (s_ins)
    {
        delete s_ins;
        s_ins = nullptr;
    }
}

bool Ui_DropdownMenuSDL::IsInsExist()
{
    if (s_ins)
    {
        return s_ins;
    }
    return nullptr;
}

bool Ui_DropdownMenuSDL::Initialize(const CRect& recordArea)
{
    try {
        //初始化DPI系数
        GetDpiScale();
        // 保存录制区域
        m_Rect_RecordArea = recordArea;

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: 初始化录制区域 左=%d 上=%d 宽=%d 高=%d",
            recordArea.left, recordArea.top, recordArea.Width(), recordArea.Height());

        // 验证录制区域尺寸是否有效
        if (recordArea.Width() <= 0 || recordArea.Height() <= 0) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 错误 - 录制区域尺寸无效");
            return false;
        }

        // 分步初始化，先初始化SDL
        if (!InitializeSDL()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL初始化失败");
            return false;
        }

        // 初始化TTF
        if (!InitializeTTF()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: TTF初始化失败");
            return false;
        }

        // 然后创建SDL窗口
        if (!CreateSDLWindow()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口创建失败");
            return false;
        }

        // 创建界面按钮
        CreateButtons();
        m_Bool_Running = true;

        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口初始化成功");
        return true;
    }
    catch (const std::exception& e) {
        // 捕获并记录异常
        char errorMsg[256] = { 0 };
        sprintf_s(errorMsg, "初始化过程中发生异常: %s", e.what());

        // 转换为宽字符以便输出
        wchar_t wErrorMsg[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wErrorMsg, _countof(wErrorMsg), errorMsg, _TRUNCATE);

        DEBUG_CONSOLE_STR(ConsoleHandle, wErrorMsg);
        Close(); // 确保资源被释放
        return false;
    }
    catch (...) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 初始化过程中发生未知异常");
        Close(); // 确保资源被释放
        return false;
    }
}

bool Ui_DropdownMenuSDL::InitializeSDL()
{
    // 检查SDL是否已经初始化
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL视频子系统已初始化，退出初始化");
        m_Bool_SDLInitialized = true;
        return true;
    }

    // 只初始化视频子系统，避免不必要的初始化
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
        // 获取SDL错误消息
        const char* sdlError = SDL_GetError();

        // 转换为宽字符以便输出
        wchar_t wSdlError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: SDL初始化失败: %s", wSdlError);
        return false;
    }
    DB(ConsoleHandle, L"视频子系统初始化完毕");

    // 设置SDL提示，提高在Windows上的兼容性
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // 抗锯齿

    m_Bool_SDLInitialized = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL完整初始化成功");
    return true;
}

bool Ui_DropdownMenuSDL::CreateSDLWindow()
{
    // 确保SDL已初始化
    if (!m_Bool_SDLInitialized) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 无法创建窗口 - SDL未初始化");
        return false;
    }

    try {
        // 清理之前可能存在的窗口和渲染器
        if (m_SDL_Renderer) {
            SDL_DestroyRenderer(m_SDL_Renderer);
            m_SDL_Renderer = nullptr;
        }
        if (m_SDL_Window) {
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
        }

        // 计算初始窗口位置和大小
        int left = max(0, m_Rect_RecordArea.left);
        int top = max(0, m_Rect_RecordArea.top);
        int width = max(100, min(GetSystemMetrics(SM_CXSCREEN) - left, m_Rect_RecordArea.Width()));
        int height = max(100, min(GetSystemMetrics(SM_CYSCREEN) - top, m_Rect_RecordArea.Height()));

        //===================================================================
        // 使用标准窗口标志 
        //===================================================================
        Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS |
            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 创建SDL透明窗口");

        // 创建窗口（使用计算的位置和大小）
        m_SDL_Window = SDL_CreateWindow(
            "Recording Area",  // 窗口标题
            left, top,         // 位置
            width, height,     // 大小
            windowFlags        // 标志
        );

        if (!m_SDL_Window) {
            // 获取SDL错误消息
            const char* sdlError = SDL_GetError();
            wchar_t wSdlError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: 窗口创建失败: %s", wSdlError);
            return false;
        }

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: 窗口创建成功，位置=%d,%d 大小=%d,%d",
            left, top, width, height);

        // 创建软件渲染器渲染器 - 这种情况下工作更可靠
        m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_SOFTWARE);

        if (!m_SDL_Renderer) {
            // 尝试创建硬件渲染器作为后备方案
            m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_ACCELERATED);

            if (!m_SDL_Renderer) {
                const char* sdlError = SDL_GetError();
                wchar_t wSdlError[256] = { 0 };
                size_t convertedChars = 0;
                mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);

                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: 渲染器创建失败: %s", wSdlError);
                SDL_DestroyWindow(m_SDL_Window);
                m_SDL_Window = nullptr;
                return false;
            }
        }

        //设置为顶层窗口
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(m_SDL_Window, &wmInfo)) {
            HWND hwnd = wmInfo.info.win.window;

            // 设置扩展窗口样式 - 永远在最上面
            LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            exStyle |= WS_EX_TOPMOST;
            SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

            // 额外设置窗口位置
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE);
        }
        return true;
    }
    catch (const std::exception& e) {
        // 捕获并记录异常
        char errorMsg[256] = { 0 };
        sprintf_s(errorMsg, "创建窗口过程中发生异常: %s", e.what());

        // 转换为宽字符以便输出
        wchar_t wErrorMsg[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wErrorMsg, _countof(wErrorMsg), errorMsg, _TRUNCATE);

        DEBUG_CONSOLE_STR(ConsoleHandle, wErrorMsg);

        // 清理已创建的资源
        if (m_SDL_Renderer) {
            SDL_DestroyRenderer(m_SDL_Renderer);
            m_SDL_Renderer = nullptr;
        }
        if (m_SDL_Window) {
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
        }
        return false;
    }
    catch (...) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 创建窗口过程中发生未知异常");

        // 清理已创建的资源
        if (m_SDL_Renderer) {
            SDL_DestroyRenderer(m_SDL_Renderer);
            m_SDL_Renderer = nullptr;
        }
        if (m_SDL_Window) {
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
        }
        return false;
    }
}

void Ui_DropdownMenuSDL::Close()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 正在清理SDL资源");

    // 先清空按钮容器，防止后续访问无效的指针
    m_Btn_ControlButtons.clear();

    // 清理字体资源
    if (m_TTF_Font) {
        TTF_CloseFont(m_TTF_Font);
        m_TTF_Font = nullptr;
    }

    // 清理渲染器
    if (m_SDL_Renderer) {
        SDL_DestroyRenderer(m_SDL_Renderer);
        m_SDL_Renderer = nullptr;
    }

    // 清理窗口
    if (m_SDL_Window) {
        SDL_DestroyWindow(m_SDL_Window);
        m_SDL_Window = nullptr;
    }

    // 关闭TTF
    if (m_Bool_TTFInitialized) {
        TTF_Quit();
        m_Bool_TTFInitialized = false;
    }

    m_Bool_SDLInitialized = false;
    s_ins = nullptr;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL资源清理完成");
}

void Ui_DropdownMenuSDL::SetAnchorRect(CRect r)
{
    if (m_Rect_Anchor)
    {
        delete m_Rect_Anchor;
        m_Rect_Anchor = nullptr;
    }
    m_Rect_Anchor = new SDL_Rect();
    m_Rect_Anchor->x = r.left;
    m_Rect_Anchor->y = r.top;
    m_Rect_Anchor->w = r.Width();
    m_Rect_Anchor->h = r.Height();
}

//========================================================================
// 界面控件管理函数
//========================================================================

void Ui_DropdownMenuSDL::CreateButtons()
{
    //获取窗口宽高，按钮高度   
    int WindowWidth, WindowHeight, BtnHeight, BtnWidth,WindowSpacing;
    SDL_GetWindowSize(m_SDL_Window, &WindowWidth, &WindowHeight);
    WindowSpacing = 10;
    BtnHeight = (WindowHeight - WindowSpacing * 2) / 3;
    BtnWidth = WindowWidth - WindowSpacing * 2;


    // 跟随鼠标录制 
    int MouseRecordBtnY = WindowSpacing;
    m_Btn_ControlButtons.emplace_back(
        SDL_Rect{ WindowSpacing, MouseRecordBtnY,
                BtnWidth, BtnHeight },
        L"跟随鼠标录制",
        [this]() {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框（更多按钮）线程]: 点击了跟随鼠标录制按钮");
            if (m_Func_OnMouseRecordRecord) {
                m_Func_OnMouseRecordRecord();
                m_Bool_Running = false;
            }
        }
    );

    // 声音录制 
    int SystemAudioMircroY = MouseRecordBtnY + BtnHeight;
    m_Btn_ControlButtons.emplace_back(
        SDL_Rect{ WindowSpacing, SystemAudioMircroY,
                BtnWidth, BtnHeight },
        L"声音录制",
        [this]() {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 点击了声音录制按钮");
            if (m_Func_OnMicroSystemAudioRecord) {
                m_Func_OnMicroSystemAudioRecord();
                m_Bool_Running = false;
            }
        }
    );

    // 摄像头录制
    int CameraRecordY = SystemAudioMircroY + BtnHeight;
    m_Btn_ControlButtons.emplace_back(
        SDL_Rect{ WindowSpacing, CameraRecordY,
                BtnWidth, BtnHeight },
        L"摄像头录制",
        [this]() {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 点击了摄像头录制按钮");
            if (m_Func_OnCameraRecord) m_Func_OnCameraRecord();
            m_Bool_Running = false;
        }
    );

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL下拉框线程]: 创建了%d个按钮", (int)m_Btn_ControlButtons.size());
}

//========================================================================
// 主循环与事件处理函数
//========================================================================

void Ui_DropdownMenuSDL::Run()
{
    // 检查窗口和渲染器是否正确初始化
    if (!m_SDL_Window || !m_SDL_Renderer) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 无法运行 - 窗口或渲染器未初始化");
        return;
    }

    try {
        // 运行SDL窗口的主循环，直到窗口关闭
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口主循环开始运行");
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"m_Bool_Running:%s", m_Bool_Running ? L"true":L"false");
        while (m_Bool_Running) {
            ProcessEvents();
            Render();
            SDL_Delay(16); // ~60 FPS
        }
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"m_Bool_Running:%s", m_Bool_Running ? L"true":L"false");
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口主循环结束");
    }
    catch (const std::exception& e) {
        // 获取异常消息
        char errorMsg[256] = { 0 };
        sprintf_s(errorMsg, "运行过程中发生异常: %s", e.what());

        // 转换为宽字符以便输出
        wchar_t wErrorMsg[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wErrorMsg, _countof(wErrorMsg), errorMsg, _TRUNCATE);

        DEBUG_CONSOLE_STR(ConsoleHandle, wErrorMsg);
    }
    catch (...) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 运行过程中发生未知异常");
    }
}

void Ui_DropdownMenuSDL::ProcessEvents()
{
    // 检查窗口是否正确初始化
    if (!m_SDL_Window) {
        m_Bool_Running = false;
        return;
    }

    try 
    {
        // 获取当前鼠标屏幕位置，根据限制矩形判断要不要消失
        int l_Int_GlobalMouseX, l_Int_GlobalMouseY;
        SDL_GetGlobalMouseState(&l_Int_GlobalMouseX, &l_Int_GlobalMouseY);
        if (m_Rect_Anchor)
        {
            if (!isPointInRect(l_Int_GlobalMouseX, l_Int_GlobalMouseY, *m_Rect_Anchor))
            {
                m_Bool_Running = false;
                return;
            }
        }

        // 处理SDL事件队列中的所有事件
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type) {
            case SDL_QUIT:
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]:窗口退出");
                m_Bool_Running = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]:鼠标按下");
                break;

            case SDL_MOUSEBUTTONUP:
                for (auto& btn : m_Btn_ControlButtons)//判断是否点击了按钮
                {
                    if (btn.IsPointInside(event.motion.x, event.motion.y))
                    {
                        btn.Click();
                    }
                }
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]:鼠标松开");
                break;

            case SDL_MOUSEMOTION:
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]:鼠标移动");
                SDL_RaiseWindow(m_SDL_Window);
                for (auto& btn : m_Btn_ControlButtons) 
                {
                    btn.UpdateHoverState(event.motion.x, event.motion.y);//更新按钮状态
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_LEAVE)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]:鼠标离开SDL窗口");
                    //m_Bool_Running = false;
                }
                break;
            }
        }

    }
    catch (const std::exception& e) {
        char l_Str_ErrorMsg[256] = { 0 };
        sprintf_s(l_Str_ErrorMsg, "事件处理中发生异常: %s", e.what());

        wchar_t l_WStr_ErrorMsg[256] = { 0 };
        size_t l_Size_ConvertedChars = 0;
        mbstowcs_s(&l_Size_ConvertedChars, l_WStr_ErrorMsg, _countof(l_WStr_ErrorMsg), l_Str_ErrorMsg, _TRUNCATE);

        DEBUG_CONSOLE_STR(ConsoleHandle, l_WStr_ErrorMsg);
        m_Bool_Running = false;
    }
}

void Ui_DropdownMenuSDL::Render()
{
    // 检查渲染器是否正确初始化
    if (!m_SDL_Renderer)
    {
        m_Bool_Running = false;
        return;
    }

    try 
    {
        // 获取当前窗口大小
        int windowWidth = 0, windowHeight = 0;
        SDL_GetWindowSize(m_SDL_Window, &windowWidth, &windowHeight);
        m_Int_PanelWidth = windowWidth;
        m_Int_PanelHeight = windowHeight;

        // 绘制面板背景 - 使用指定的面板背景颜色RGB(37,39,46)
        SDL_SetRenderDrawColor(m_SDL_Renderer, 37, 39, 46, 255);
        SDL_Rect panelRect = { 0, 0, m_Int_PanelWidth, m_Int_PanelHeight };
        SDL_RenderFillRect(m_SDL_Renderer, &panelRect);

        //绘制面板边框
        SDL_SetRenderDrawColor(m_SDL_Renderer, 73, 73, 73, 255);
        panelRect = { 0, 0, m_Int_PanelWidth - 2, m_Int_PanelHeight - 2 };
        SDL_RenderDrawRect(m_SDL_Renderer, &panelRect);

        // 绘制所有按钮 - 使用非透明色
        for (auto& button : m_Btn_ControlButtons) {
            button.Render(m_SDL_Renderer, m_TTF_Font);
        }

        // 呈现渲染结果
        SDL_RenderPresent(m_SDL_Renderer);
    }
    catch (const std::exception& e)
    {
        // 获取异常消息
        char errorMsg[256] = { 0 };
        sprintf_s(errorMsg, "渲染过程中发生异常: %s", e.what());

        // 转换为宽字符以便输出
        wchar_t wErrorMsg[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wErrorMsg, _countof(wErrorMsg), errorMsg, _TRUNCATE);

        DEBUG_CONSOLE_STR(ConsoleHandle, wErrorMsg);
    }
}

//========================================================================
// 回调函数设置接口
//========================================================================

void Ui_DropdownMenuSDL::SetOnCameraRecordCallback(std::function<void()> callback)
{
    m_Func_OnCameraRecord = callback;
}

void Ui_DropdownMenuSDL::SetOnMicroSystemAudioCallback(std::function<void()> callback)
{
    m_Func_OnMicroSystemAudioRecord = callback;
}

void Ui_DropdownMenuSDL::SetOnMouseRecordCallback(std::function<void()> callback)
{
    m_Func_OnMouseRecordRecord = callback;
}
