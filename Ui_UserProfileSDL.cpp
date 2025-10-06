#include "stdafx.h"
#include "CDebug.h"
#include "Ui_UserProfileSDL.h"
#include "theApp.h"
#include "GlobalFunc.h"
// 调试控制台句柄
extern HANDLE ConsoleHandle;
bool Ui_UserProfileSDL::s_Bool_IsRunnins = false;
Ui_UserProfileSDL* Ui_UserProfileSDL::Instance = nullptr;
//========================================================================
// Ui_SDLButton 实现 - 提供基本按钮功能
//========================================================================

Ui_UserProfileSDL::Ui_SDLButton::Ui_SDLButton(const SDL_Rect& rect, const std::wstring& text,
    std::function<void()> callback, bool isTextButton)
    : m_Rect_Area(rect), m_Str_Text(text), m_Func_Callback(callback),
    m_Bool_Hovered(false), m_Bool_Click(false), m_SDL_TextTexture(nullptr),
    m_Bool_IsTextButton(isTextButton)
{
    // 构造函数初始化按钮属性
}

Ui_UserProfileSDL::Ui_SDLButton::~Ui_SDLButton()
{
    // 清理文本纹理资源
    if (m_SDL_TextTexture)
    {
        SDL_DestroyTexture(m_SDL_TextTexture);
        m_SDL_TextTexture = nullptr;
    }
}

void Ui_UserProfileSDL::Ui_SDLButton::Render(SDL_Renderer* renderer, TTF_Font* font)const
{
    // 安全检查
    if (!renderer) return;

    // 使用实例成员变量而非静态变量来跟踪状态变化
    bool needUpdateTexture = false;

    // 如果不是纯文本按钮，绘制背景
    if (!m_Bool_IsTextButton) {
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
    }

    //绘画文本
    if (font && !m_Str_Text.empty())
    {
        // 根据按钮状态确定文本颜色
        SDL_Color textColor;
        if (m_Bool_IsTextButton) {
            // 纯文本按钮的颜色
            if (m_Bool_Click) {
                textColor = { 150, 220, 255, 255 }; // 点击时亮蓝色
            }
            else if (m_Bool_Hovered) {
                textColor = { 200, 200, 255, 255 }; // 悬停时偏蓝色
            }
            else {
                textColor = { 255, 255, 255, 255 }; // 默认白色
            }
        }
        else {
            // 普通按钮的文本是白色
            textColor = { 255, 255, 255, 255 };
        }

        // 每次都重新创建文本纹理，确保状态变化时能正确更新
        // 清理旧的纹理
        if (m_SDL_TextTexture) {
            SDL_DestroyTexture(m_SDL_TextTexture);
            m_SDL_TextTexture = nullptr;
        }

        // 转换为UTF-8编码，因为SDL_ttf使用UTF-8
        std::string utf8Text;
        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0)
        {
            utf8Text.resize(utf8Size);
            WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, &utf8Text[0], utf8Size, NULL, NULL);
        }

        // 创建文字表面
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, utf8Text.c_str(), textColor);

        if (textSurface)
        {
            // 创建文字纹理
            m_SDL_TextTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);
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
bool Ui_UserProfileSDL::Ui_SDLButton::IsPointInside(int x, int y) const
{
    // 检查坐标是否在按钮区域内
    return (x >= m_Rect_Area.x && x < m_Rect_Area.x + m_Rect_Area.w &&
        y >= m_Rect_Area.y && y < m_Rect_Area.y + m_Rect_Area.h);
}

bool Ui_UserProfileSDL::Ui_SDLButton::UpdateHoverState(int x, int y)
{
    // 仅更新悬停状态，不触发回调
    m_Bool_Hovered = IsPointInside(x, y);
    return m_Bool_Hovered;
}

void Ui_UserProfileSDL::Ui_SDLButton::UpdateClickState(bool isClick)
{
    m_Bool_Click = isClick;
}

void Ui_UserProfileSDL::Ui_SDLButton::Click()
{
    // 执行点击回调
    if (m_Func_Callback) {
        m_Func_Callback();
    }
}

Ui_UserProfileSDL::Ui_UserProfileSDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr),
    m_TTF_Font(nullptr),
    m_TTF_LargeFont(nullptr),
    m_TTF_ButtonFont(nullptr),  // 初始化按钮字体
    m_Texture_ContactService(nullptr),
    m_Texture_SignOut(nullptr),
    m_Texture_Icon(nullptr),
    m_Bool_Running(false),
    m_Bool_SDLInitialized(false),
    m_Bool_TTFInitialized(false),
    m_Bool_ImgInitialized(false),
    m_Float_Scale(1.0f),
    m_Bool_MouseEnteredWindow(false),
    m_Bool_HandCursorSet(false)
{
    // 构造函数初始化类成员
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: Ui_UserProfileSDL构造");
}

Ui_UserProfileSDL::~Ui_UserProfileSDL()
{
    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: Ui_UserProfileSDL析构");
    Close();
}

bool Ui_UserProfileSDL::InitializeSDLImage()
{
    if (m_Bool_ImgInitialized)
    {
        return true;
    }

    // 初始化SDL_image，设置支持PNG格式
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        const char* imgError = IMG_GetError();
        wchar_t wImgError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wImgError, _countof(wImgError), imgError, _TRUNCATE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: SDL_image初始化失败: %s", wImgError);
        return false;
    }

    m_Bool_ImgInitialized = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL_image初始化成功");
    return true;
}

void Ui_UserProfileSDL::SetCursorToCursor(bool isHandCursor)
{
    // 使用 SDL 的方式直接设置光标，而不是 Windows API
    SDL_Cursor* cursor = SDL_CreateSystemCursor(isHandCursor ? SDL_SYSTEM_CURSOR_HAND : SDL_SYSTEM_CURSOR_ARROW);
    if (cursor) {
        SDL_SetCursor(cursor);
        m_Bool_HandCursorSet = isHandCursor;
    }
}

bool Ui_UserProfileSDL::InitializeTTF()
{
    if (m_Bool_TTFInitialized)
    { // 如果TTF已初始化，直接返回成功
        return true;
    }
    if (TTF_Init() == -1)   // 初始化SDL_ttf库
    {
        DebugSDLError(L"[SDL用户资料窗口]: TTF初始化失败:");
        return false;
    }
    m_Bool_TTFInitialized = true;

    // 加载字体
    if (!LoadFonts())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 字体加载失败");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: TTF初始化成功");
    return true;
}

bool Ui_UserProfileSDL::LoadFonts()
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

    // 加载常规字体，字号16
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 10 * m_Float_Scale);
    // 加载大号字体，字号20
    m_TTF_LargeFont = TTF_OpenFont(utf8Path.c_str(), 13 * m_Float_Scale);
    // 加载按钮字体，字号18 - 比普通字体大，但比大号字体小
    m_TTF_ButtonFont = TTF_OpenFont(utf8Path.c_str(), 12 * m_Float_Scale);

    if (!m_TTF_Font || !m_TTF_LargeFont || !m_TTF_ButtonFont)
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
            m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16);
        if (!m_TTF_LargeFont)
            m_TTF_LargeFont = TTF_OpenFont(utf8Path.c_str(), 20);
        if (!m_TTF_ButtonFont)
            m_TTF_ButtonFont = TTF_OpenFont(utf8Path.c_str(), 18);

        if (!m_TTF_Font || !m_TTF_LargeFont || !m_TTF_ButtonFont)
        {
            const char* ttfError = TTF_GetError();
            wchar_t wTtfError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wTtfError, _countof(wTtfError), ttfError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 字体加载失败: %s", wTtfError);
            return false;
        }
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 字体加载成功");
    return true;
}

// 修改资源加载函数，使用SDLAssets文件夹路径
bool Ui_UserProfileSDL::LoadResources()
{
    // 获取可执行程序路径
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // 找到最后一个反斜杠位置，截取目录部分
    std::string exeDir = exePath;
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }

    // 构建资源文件夹路径
    std::string assetsDir = exeDir + "\\SDLAssets\\";
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 资源目录: %S", assetsDir.c_str());

    // 加载联系客服图标
    std::string contactServicePath = assetsDir + "ContactService.png";
    m_Texture_ContactService = IMG_LoadTexture(m_SDL_Renderer, GlobalFunc::AnsiToUtf8(contactServicePath).c_str());
    if (!m_Texture_ContactService) {
        const char* imgError = IMG_GetError();
        wchar_t wImgError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wImgError, _countof(wImgError), imgError, _TRUNCATE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 加载ContactService.png失败: %s", wImgError);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 尝试加载路径: %S", contactServicePath.c_str());
        return false;
    }

    // 加载退出登录图标
    std::string signoutPath = assetsDir + "signout.png";
    m_Texture_SignOut = IMG_LoadTexture(m_SDL_Renderer, GlobalFunc::AnsiToUtf8(signoutPath).c_str());
    if (!m_Texture_SignOut) {
        const char* imgError = IMG_GetError();
        wchar_t wImgError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wImgError, _countof(wImgError), imgError, _TRUNCATE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 加载signout.png失败: %s", wImgError);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 尝试加载路径: %S", signoutPath.c_str());
        return false;
    }

    // 加载用户头像图标
    std::string iconPath = assetsDir + "Icon.png";
    m_Texture_Icon = IMG_LoadTexture(m_SDL_Renderer, GlobalFunc::AnsiToUtf8(iconPath).c_str());
    if (!m_Texture_Icon) {
        const char* imgError = IMG_GetError();
        wchar_t wImgError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wImgError, _countof(wImgError), imgError, _TRUNCATE);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 加载Icon.png失败: %s", wImgError);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 尝试加载路径: %S", iconPath.c_str());
        return false;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 所有图像资源加载成功");
    return true;
}
Ui_UserProfileSDL* Ui_UserProfileSDL::GetInstance()
{
    if (!Instance)
    {
        Instance = new Ui_UserProfileSDL;
    }
    return Instance;
}

void Ui_UserProfileSDL::ReleaseInstance()
{
    if (Instance)
    {
        Instance->Close();
        delete Instance;
        Instance = nullptr;
    }
}

bool Ui_UserProfileSDL::Initialize(const CRect& windowRect, float scale)
{
    // 初始化成员
    m_Float_Scale = scale;
    m_Rect_WindowArea = windowRect;
    m_Int_WindowWidth = windowRect.Width();
    m_Int_WindowHeight = windowRect.Height();

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 初始化窗口区域 左=%d 上=%d 宽=%d 高=%d",
        windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height());

    if (windowRect.Width() <= 0 || windowRect.Height() <= 0)
    { // 验证窗口区域尺寸是否有效
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 错误 - 窗口区域尺寸无效");
        return false;
    }

    if (!InitializeSDL()) // 分步初始化，先初始化SDL
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL初始化失败");
        return false;
    }
    if (!InitializeSDLImage()) // 初始化SDL_image
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL_image初始化失败");
        return false;
    }
    if (!InitializeTTF())  // 初始化TTF
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: TTF初始化失败");
        return false;
    }
    if (!CreateSDLWindow())// 然后创建SDL窗口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口创建失败");
        return false;
    }
    if (!LoadResources()) // 加载图像资源
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 资源加载失败");
        return false;
    }
    CreateButtons(); // 创建界面按钮
    m_Bool_Running = true;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口初始化成功");
    return true;
}

bool Ui_UserProfileSDL::InitializeSDL()
{
    if (SDL_WasInit(SDL_INIT_VIDEO)) // 检查SDL是否已经初始化
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL视频子系统已初始化");
        m_Bool_SDLInitialized = true;
        return true;
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    { // 只初始化视频子系统，避免不必要的初始化
        DebugSDLError(L"[SDL用户资料窗口]:SDL初始化失败:");
        return false;
    }
    m_Bool_SDLInitialized = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL初始化成功");
    return true;
}

void Ui_UserProfileSDL::DebugSDLError(wchar_t error[256])
{
    const char* sdlError = SDL_GetError();
    wchar_t wSdlError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s: %s", error, wSdlError);
}

bool Ui_UserProfileSDL::CreateSDLWindow()
{
    // 确保SDL已初始化
    if (!m_Bool_SDLInitialized)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 无法创建窗口 - SDL未初始化");
        return false;
    }

    // 计算初始窗口位置和大小
    int left = max(0, m_Rect_WindowArea.left);
    int top = max(0, m_Rect_WindowArea.top);
    int width = (int)(414 * m_Float_Scale);
    int height = (int)(119 * m_Float_Scale);

    // 确保窗口不超出屏幕
    width = max(100, min(GetSystemMetrics(SM_CXSCREEN) - left, width));
    height = max(100, min(GetSystemMetrics(SM_CYSCREEN) - top, height));

    // 使用 SDL_WINDOW_HIDDEN 而不是 SDL_WINDOW_SHOWN，这样窗口创建时不可见
    Uint32 windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 创建SDL窗口");
    m_SDL_Window = SDL_CreateWindow(
        "UserProfileWindow",    // 窗口标题
        left, top,             // 位置
        width, height,         // 大小
        windowFlags            // 标志
    );
    if (!m_SDL_Window)
    { // 获取SDL错误消息
        DebugSDLError(L"[SDL用户资料窗口]: 窗口创建失败:");
        return false;
    }
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 窗口创建成功，位置=%d,%d 大小=%d,%d",
        left, top, width, height);

    // 创建硬件渲染器 - 这种情况下工作更可靠
    m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_SOFTWARE);
    if (!m_SDL_Renderer) {
        // 尝试创建软件渲染器作为后备方案 
        m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_ACCELERATED);
        if (!m_SDL_Renderer)
        {
            DebugSDLError(L"[SDL用户资料窗口]: 渲染器创建失败:");
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
            return false;
        }
    }

    //设置为顶层窗口
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
    return true;
}

void Ui_UserProfileSDL::Close()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 正在清理SDL资源");

    // 释放图像资源
    if (m_Texture_ContactService) {
        SDL_DestroyTexture(m_Texture_ContactService);
        m_Texture_ContactService = nullptr;
    }
    if (m_Texture_SignOut) {
        SDL_DestroyTexture(m_Texture_SignOut);
        m_Texture_SignOut = nullptr;
    }
    if (m_Texture_Icon) {
        SDL_DestroyTexture(m_Texture_Icon);
        m_Texture_Icon = nullptr;
    }

    // 先清空按钮容器，防止后续访问无效的指针
    m_Vec_Buttons.clear();

    // 清理字体资源
    if (m_TTF_LargeFont) {
        TTF_CloseFont(m_TTF_LargeFont);
        m_TTF_LargeFont = nullptr;
    }
    if (m_TTF_Font) {
        TTF_CloseFont(m_TTF_Font);
        m_TTF_Font = nullptr;
    }
    if (m_TTF_ButtonFont) {
        TTF_CloseFont(m_TTF_ButtonFont);
        m_TTF_ButtonFont = nullptr;
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

    // 关闭SDL_image
    if (m_Bool_ImgInitialized) {
        IMG_Quit();
        m_Bool_ImgInitialized = false;
    }
    m_Bool_SDLInitialized = false;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL资源清理完成");
}

void Ui_UserProfileSDL::RenderCircularImage(SDL_Texture* texture, int x, int y, int radius)
{
    if (!texture || radius <= 0) return;

    // 获取纹理原始尺寸
    int texWidth, texHeight;
    SDL_QueryTexture(texture, NULL, NULL, &texWidth, &texHeight);

    // 使用更简单的方法 - 渲染完整纹理，然后使用遮罩

    // 1. 在内存中创建圆形遮罩表面
    SDL_Surface* maskSurface = SDL_CreateRGBSurface(0, radius * 2, radius * 2, 32,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!maskSurface) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 创建遮罩表面失败");
        return;
    }

    // 2. 在遮罩表面上绘制圆形
    // 先将整个表面填充为透明
    SDL_FillRect(maskSurface, NULL, 0x00000000);

    // 绘制实心圆
    Uint32* pixels = (Uint32*)maskSurface->pixels;
    int centerX = radius;
    int centerY = radius;

    for (int cy = 0; cy < radius * 2; cy++) {
        for (int cx = 0; cx < radius * 2; cx++) {
            int dx = cx - centerX;
            int dy = cy - centerY;
            // 如果点在圆内，设置为白色不透明
            if (dx * dx + dy * dy <= radius * radius) {
                pixels[cy * maskSurface->pitch / 4 + cx] = 0xFFFFFFFF;
            }
        }
    }

    // 3. 将原始图像渲染到屏幕上指定位置的临时纹理上
    SDL_Texture* tempTexture = SDL_CreateTexture(
        m_SDL_Renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        radius * 2,
        radius * 2
    );

    if (!tempTexture) {
        SDL_FreeSurface(maskSurface);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 创建临时纹理失败");
        return;
    }

    // 设置混合模式
    SDL_SetTextureBlendMode(tempTexture, SDL_BLENDMODE_BLEND);

    // 保存当前渲染目标
    SDL_Texture* previousTarget = SDL_GetRenderTarget(m_SDL_Renderer);
    SDL_SetRenderTarget(m_SDL_Renderer, tempTexture);

    // 清除为透明
    SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_SDL_Renderer);

    // 计算源矩形，确保图像居中并填满圆形区域
    float scale = max((float)radius * 2 / texWidth, (float)radius * 2 / texHeight);
    int srcW = (int)(radius * 2 / scale);
    int srcH = (int)(radius * 2 / scale);
    int srcX = (texWidth - srcW) / 2;
    int srcY = (texHeight - srcH) / 2;

    SDL_Rect srcRect = { srcX, srcY, srcW, srcH };
    SDL_Rect dstRect = { 0, 0, radius * 2, radius * 2 };

    // 渲染原始图像到临时纹理
    SDL_RenderCopy(m_SDL_Renderer, texture, &srcRect, &dstRect);

    // 4. 创建遮罩纹理
    SDL_Texture* maskTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, maskSurface);
    SDL_FreeSurface(maskSurface); // 释放表面

    if (!maskTexture) {
        SDL_DestroyTexture(tempTexture);
        SDL_SetRenderTarget(m_SDL_Renderer, previousTarget);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 创建遮罩纹理失败");
        return;
    }

    // 5. 设置遮罩纹理的混合模式为相乘
    SDL_SetTextureBlendMode(maskTexture, SDL_BLENDMODE_MOD);

    // 6. 将遮罩应用到图像上
    SDL_RenderCopy(m_SDL_Renderer, maskTexture, NULL, NULL);

    // 恢复原始渲染目标
    SDL_SetRenderTarget(m_SDL_Renderer, previousTarget);

    // 7. 渲染最终结果到屏幕
    dstRect.x = x - radius;
    dstRect.y = y - radius;
    SDL_RenderCopy(m_SDL_Renderer, tempTexture, NULL, &dstRect);

    // 清理资源
    SDL_DestroyTexture(maskTexture);
    SDL_DestroyTexture(tempTexture);
}

void Ui_UserProfileSDL::RenderUserInfo()
{
    // 获取用户昵称
    CString nickname = App.m_userInfo.nickname;
    // 获取过期时间
    CString expiresAt = App.m_userInfo.expiresAt;
    if (expiresAt == L"")
    {
        expiresAt = L"永久VIP";
    }

    // 将CString转换为宽字符串
    std::wstring wNickname = (LPCTSTR)nickname;
    std::wstring wExpiresAt = (LPCTSTR)expiresAt;

    // 将宽字符串转换为UTF-8编码
    std::string utf8Nickname;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wNickname.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        utf8Nickname.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, wNickname.c_str(), -1, &utf8Nickname[0], utf8Size, NULL, NULL);
    }

    std::string utf8ExpiresAt;
    utf8Size = WideCharToMultiByte(CP_UTF8, 0, wExpiresAt.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        utf8ExpiresAt.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, wExpiresAt.c_str(), -1, &utf8ExpiresAt[0], utf8Size, NULL, NULL);
    }

    // 左上角用户昵称
    SDL_Color textColor = { 255, 255, 255, 255 }; // 白色
    SDL_Surface* smallNameSurface = TTF_RenderUTF8_Blended(m_TTF_Font, utf8Nickname.c_str(), textColor);
    if (smallNameSurface) {
        SDL_Texture* smallNameTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, smallNameSurface);
        if (smallNameTexture) {
            int textWidth = smallNameSurface->w;
            int textHeight = smallNameSurface->h;

            // 在左上角显示
            SDL_Rect smallNameRect = { 15, 15, textWidth, textHeight };
            SDL_RenderCopy(m_SDL_Renderer, smallNameTexture, NULL, &smallNameRect);

            SDL_DestroyTexture(smallNameTexture);
        }
        SDL_FreeSurface(smallNameSurface);
    }

    // 中部大号用户昵称
    SDL_Surface* largeNameSurface = TTF_RenderUTF8_Blended(m_TTF_LargeFont, utf8Nickname.c_str(), textColor);
    if (largeNameSurface)
    {
        SDL_Texture* largeNameTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, largeNameSurface);
        if (largeNameTexture)
        {
            int textWidth = largeNameSurface->w;
            int textHeight = largeNameSurface->h;

            // 计算图标位置
            int iconRadius = (int)(27 * m_Float_Scale);
            int iconX = 50 * m_Float_Scale;
            int dividerY = (int)(40 * m_Float_Scale);
            int iconY = dividerY + iconRadius + 20;

            // 在图标右侧显示大号昵称
            SDL_Rect largeNameRect =
            {
                iconX + iconRadius + 20,
                iconY - textHeight / 2, // 确保文本垂直居中于图标
                textWidth,
                textHeight
            };
            SDL_RenderCopy(m_SDL_Renderer, largeNameTexture, NULL, &largeNameRect);

            SDL_DestroyTexture(largeNameTexture);
        }
        SDL_FreeSurface(largeNameSurface);
    }

    // 显示"会员到期时间："前缀和过期时间
    SDL_Color expiresColor = { 180, 180, 180, 255 }; // 灰色

    // 先渲染前缀文本 - 使用宽字符串转UTF-8的方式处理中文
    std::wstring wPrefixText = L"会员到期时间：";
    std::string utf8PrefixText;
    utf8Size = WideCharToMultiByte(CP_UTF8, 0, wPrefixText.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0)
    {
        utf8PrefixText.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, wPrefixText.c_str(), -1, &utf8PrefixText[0], utf8Size, NULL, NULL);
    }

    SDL_Surface* prefixSurface = TTF_RenderUTF8_Blended(m_TTF_Font, utf8PrefixText.c_str(), expiresColor);

    // 然后渲染实际的过期时间
    SDL_Surface* expiresSurface = TTF_RenderUTF8_Blended(m_TTF_Font, utf8ExpiresAt.c_str(), expiresColor);

    if (prefixSurface && expiresSurface) {
        SDL_Texture* prefixTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, prefixSurface);
        SDL_Texture* expiresTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, expiresSurface);

        if (prefixTexture && expiresTexture) {
            int prefixWidth = prefixSurface->w;
            int prefixHeight = prefixSurface->h;
            int expiresWidth = expiresSurface->w;
            int expiresHeight = expiresSurface->h;

            // 计算图标和位置
            int iconRadius = (int)(27 * m_Float_Scale);
            int iconX = 50;
            int dividerY = (int)(40 * m_Float_Scale);
            int iconY = dividerY + iconRadius + 20;

            // 设置右边距
            const int rightMargin = 15;

            // 计算过期时间的位置，使其右侧接近窗口右边缘
            // 从右向左计算：窗口宽度 - 右边距 - 过期时间宽度 = 过期时间的起始X坐标
            int expiresX = m_Int_WindowWidth - rightMargin - expiresWidth;

            // 前缀位于过期时间的左侧
            int prefixX = expiresX - prefixWidth;

            // 绘制前缀
            SDL_Rect prefixRect = {
                prefixX,
                iconY - prefixHeight / 2, // 与昵称垂直对齐
                prefixWidth,
                prefixHeight
            };
            SDL_RenderCopy(m_SDL_Renderer, prefixTexture, NULL, &prefixRect);

            // 绘制过期时间值
            SDL_Rect expiresRect = {
                expiresX, // 从计算得到的位置开始
                iconY - expiresHeight / 2, // 与昵称垂直对齐
                expiresWidth,
                expiresHeight
            };
            SDL_RenderCopy(m_SDL_Renderer, expiresTexture, NULL, &expiresRect);

            SDL_DestroyTexture(prefixTexture);
            SDL_DestroyTexture(expiresTexture);
        }

        SDL_FreeSurface(prefixSurface);
        SDL_FreeSurface(expiresSurface);
    }
}

void Ui_UserProfileSDL::CreateButtons()
{
    // 计算合适的按钮位置
    const int fontSize = (int)(16 * m_Float_Scale);
    const int buttonMargin = (int)(10 * m_Float_Scale);
    const int buttonHeight = (int)(20 * m_Float_Scale);

    // 移动按钮位置，为图标留出空间
    const int iconWidth = (int)(18 * m_Float_Scale);
    const int buttonWidth = (int)(80 * m_Float_Scale);
    const int spaceBetweenIconAndText = (int)(10 * m_Float_Scale);

    // 创建"联系客服"按钮 - 位于右上角
    m_Vec_Buttons.emplace_back(
        SDL_Rect{
            m_Int_WindowWidth - buttonWidth - buttonMargin,
            buttonMargin,
            buttonWidth,
            buttonHeight
        },
        L"联系客服",
        [this]()
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 点击了联系客服按钮");
            if (m_Func_OnContactSupport)
            {
                m_Func_OnContactSupport();
            }
        },
        true  // 设置为纯文本按钮
            );

    // 创建"退出登录"按钮 - 位于"联系客服"按钮左侧
    m_Vec_Buttons.emplace_back(
        SDL_Rect{
            m_Int_WindowWidth - 2 * buttonWidth - iconWidth - spaceBetweenIconAndText - 2 * buttonMargin,
            buttonMargin,
            buttonWidth,
            buttonHeight
        },
        L"退出登录",
        [this]()
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 点击了退出登录按钮");

            // 仅触发外部回调，不在此处实现登出逻辑
            if (m_Func_OnLogout)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 调用退出登录回调");
                m_Func_OnLogout();
            }

            // 关闭窗口
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 窗口即将关闭");
            m_Bool_Running = false;

            // 执行关闭回调
            if (m_Func_OnClose)
            {
                m_Func_OnClose();
            }
        },
        true  // 设置为纯文本按钮
            );

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL用户资料窗口]: 创建了%d个按钮", (int)m_Vec_Buttons.size());
}

void Ui_UserProfileSDL::Run()
{
    // 检查窗口和渲染器是否正确初始化
    if (!m_SDL_Window || !m_SDL_Renderer)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 无法运行 - 窗口或渲染器未初始化");
        return;
    }
    m_Bool_Running = true;
    s_Bool_IsRunnins = true;
    // 先执行一次渲染，确保内容已准备好
    Render();

    // 渲染完成后再显示窗口
    SDL_ShowWindow(m_SDL_Window);

    // 运行SDL窗口的主循环，直到窗口关闭
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口主循环开始运行");

    while (m_Bool_Running)
    {
        ProcessEvents();
        Render();
        SDL_Delay(10); // ~100 FPS
    }
    s_Bool_IsRunnins = false;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口主循环结束");
}

void Ui_UserProfileSDL::ProcessEvents()
{
    // 检查窗口是否正确初始化
    if (!m_SDL_Window) {
        m_Bool_Running = false;
        return;
    }

    // 获取当前鼠标位置
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // 处理每个按钮的悬停状态 - 在事件循环前先重置所有按钮
    bool anyButtonHovered = false;
    for (auto& btn : m_Vec_Buttons) {
        // 检查每个按钮是否在鼠标下
        bool isHovered = btn.IsPointInside(mouseX, mouseY);
        btn.UpdateHoverState(mouseX, mouseY);
        if (isHovered) {
            anyButtonHovered = true;
        }
    }

    // 更新鼠标指针
    if (anyButtonHovered && !m_Bool_HandCursorSet) {
        SetCursorToCursor(true);
    }
    else if (!anyButtonHovered && m_Bool_HandCursorSet) {
        SetCursorToCursor(false);
    }

    // 处理SDL事件队列中的所有事件
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 窗口退出");
            m_Bool_Running = false;
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_ENTER) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 鼠标进入窗口");
                m_Bool_MouseEnteredWindow = true;
            }
            else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 鼠标离开窗口");
                // 鼠标离开窗口时，确保所有按钮都不处于悬停状态
                for (auto& btn : m_Vec_Buttons) {
                    btn.UpdateHoverState(-1, -1); // 传递窗口外的坐标，确保悬停状态被重置
                }

                // 只有当鼠标曾经进入过窗口，离开时才关闭窗口
                if (m_Bool_MouseEnteredWindow) {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 鼠标离开窗口，关闭窗口");
                    m_Bool_Running = false;
                    if (m_Func_OnClose) {
                        m_Func_OnClose();
                    }
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            for (auto& btn : m_Vec_Buttons) {
                if (btn.IsPointInside(event.motion.x, event.motion.y)) {
                    btn.UpdateClickState(true);
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            for (auto& btn : m_Vec_Buttons) {
                if (btn.IsPointInside(event.motion.x, event.motion.y)) {
                    btn.UpdateClickState(false);
                    btn.Click();
                }
                else {
                    // 确保鼠标松开时也重置点击状态
                    btn.UpdateClickState(false);
                }
            }
            break;

        case SDL_MOUSEMOTION:
            // 更新每个按钮的悬停状态
            anyButtonHovered = false;
            for (auto& btn : m_Vec_Buttons) {
                if (btn.UpdateHoverState(event.motion.x, event.motion.y)) {
                    anyButtonHovered = true;
                }
            }

            // 根据按钮悬停状态更新鼠标样式
            if (anyButtonHovered && !m_Bool_HandCursorSet) {
                SetCursorToCursor(true);
            }
            else if (!anyButtonHovered && m_Bool_HandCursorSet) {
                SetCursorToCursor(false);
            }
            break;

        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 按下ESC键，关闭窗口");
                m_Bool_Running = false;
                if (m_Func_OnClose) {
                    m_Func_OnClose();
                }
            }
            break;
        }
    }
}

void Ui_UserProfileSDL::Render()
{
    // 清除窗口背景，设置为RGB(36,37,40)的颜色
    SDL_SetRenderDrawColor(m_SDL_Renderer, 36, 37, 40, 255);
    SDL_RenderClear(m_SDL_Renderer);

    // 绘制2像素宽的RGB(73,73,73)边框
    SDL_SetRenderDrawColor(m_SDL_Renderer, 73, 73, 73, 255);

    // 绘制外边框
    SDL_Rect outerBorderRect = { 0, 0, m_Int_WindowWidth, m_Int_WindowHeight };
    SDL_RenderDrawRect(m_SDL_Renderer, &outerBorderRect);

    // 绘制内边框(缩小1像素)
    SDL_Rect innerBorderRect = { 1, 1, m_Int_WindowWidth - 2, m_Int_WindowHeight - 2 };
    SDL_RenderDrawRect(m_SDL_Renderer, &innerBorderRect);

    // 绘制顶部和底部间的分割线
    int dividerY = 40 * m_Float_Scale;
    SDL_SetRenderDrawColor(m_SDL_Renderer, 180, 180, 180, 100); // 白灰色
    SDL_RenderDrawLine(m_SDL_Renderer, 0, dividerY, m_Int_WindowWidth, dividerY);

    // 渲染用户信息
    RenderUserInfo();

    // 绘制图标和按钮
    for (size_t i = 0; i < m_Vec_Buttons.size(); i++)
    {
        const auto& btn = m_Vec_Buttons[i];
        // 先渲染按钮
        btn.Render(m_SDL_Renderer, m_TTF_ButtonFont);

        // 根据按钮类型渲染对应图标
        SDL_Rect btnRect = btn.GetRect();
        if (i == 0) { // 联系客服按钮
            int imgSize = (int)(18 * m_Float_Scale);
            SDL_Rect imgRect = {
                btnRect.x - imgSize - 5,
                btnRect.y + (btnRect.h - imgSize) / 2,
                imgSize, imgSize
            };
            SDL_RenderCopy(m_SDL_Renderer, m_Texture_ContactService, NULL, &imgRect);
        }
        else if (i == 1) { // 退出登录按钮
            int imgSize = (int)(18 * m_Float_Scale);
            SDL_Rect imgRect = {
                btnRect.x - imgSize - 5,
                btnRect.y + (btnRect.h - imgSize) / 2,
                imgSize, imgSize
            };
            SDL_RenderCopy(m_SDL_Renderer, m_Texture_SignOut, NULL, &imgRect);
        }
    }

    // 绘制圆形头像 - 调整位置，避开分界线
    int iconRadius = (int)(27 * m_Float_Scale);
    int iconX = 40 * m_Float_Scale;
    int dividerY1 = (int)(40 * m_Float_Scale);
    int iconY = dividerY1 + iconRadius + 20; // 分界线下方20像素 + 半径
    RenderCircularImage(m_Texture_Icon, iconX, iconY, iconRadius);

    // 刷新显示
    SDL_RenderPresent(m_SDL_Renderer);
}

void Ui_UserProfileSDL::SetOnCloseCallback(std::function<void()> callback)
{
    m_Func_OnClose = callback;
}

void Ui_UserProfileSDL::SetOnLogoutCallback(std::function<void()> callback)
{
    m_Func_OnLogout = callback;
}

void Ui_UserProfileSDL::SetOnContactSupportCallback(std::function<void()> callback)
{
    m_Func_OnContactSupport = callback;
}