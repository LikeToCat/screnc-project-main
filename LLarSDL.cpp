#include "stdafx.h"
#include "LLarSDL.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;

static SDL_HitTestResult SDLCALL HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data);

LLarSDL::~LLarSDL()
{
    //通知窗口线程退出
    s_isRunning = false;
    if (m_Thread_Window.joinable())
    {
        m_Thread_Window.join();
    }

    //清理 SDL 资源
    if (m_TTF_Font)
    {
        TTF_CloseFont(m_TTF_Font);
        m_TTF_Font = nullptr;
    }
    if (m_SDL_Renderer)
    {
        SDL_DestroyRenderer(m_SDL_Renderer);
        m_SDL_Renderer = nullptr;
    }
    if (m_SDL_Window)
    {
        SDL_DestroyWindow(m_SDL_Window);
        m_SDL_Window = nullptr;
    }
}

bool LLarSDL::_Initialize(const SDL_Rect& gArea)
{
	if (!_InitSDL())
	{
        DBFMT(ConsoleHandle, L"SDL初始化失败！错误原因:%ls", _getSDLErrorMsg().c_str());
        return false;
	}
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 初始化SDL成功");

	if (!_InitSDL_Image())
	{
        DBFMT(ConsoleHandle, L"SDL_Image初始化失败！错误原因:%ls", _getIMGErrorMsg().c_str());
        return false;
	}
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 初始化SDL_Image成功");
	
	if (!_InitSDL_TTF())
	{
        DBFMT(ConsoleHandle, L"SDL_TTF初始化失败！错误原因:%ls", _getSDLErrorMsg().c_str());
        return false;
	}
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 初始化SDL_TTF成功");

	if (!_CreateSDLWindow(gArea))
	{
        DBFMT(ConsoleHandle, L"SDL窗口创建失败！错误原因:%ls", _getSDLErrorMsg().c_str());
        return false;
	}
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 创建SDL窗口成功");

    if (!_Cus_Init())
    {
        DB(ConsoleHandle, L"自定义初始化失败！");
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 自定义初始化成功");
	return true;
}

void LLarSDL::Run(
    const CRect& gArea,
    bool isActiveAccelerated,
    int iframeRate,
    ShadowParam sp
)
{
    if (s_isRunning)
    {
        DB(ConsoleHandle, L"SDL窗口已在运行，Run调用非法");
        return;
    }
    s_isRunning = true;

    m_Rect_WindowRect.x = gArea.left;
    m_Rect_WindowRect.y = gArea.top;
    m_Rect_WindowRect.w = gArea.Width();
    m_Rect_WindowRect.h = gArea.Height();

    m_CtRect_WindowRect.w = gArea.Width();
    m_CtRect_WindowRect.h = gArea.Height();
    m_CtRect_WindowRect.x = 0;
    m_CtRect_WindowRect.y = 0;
    m_bIsAccelerated = isActiveAccelerated;
    m_iframeRate = iframeRate;
    m_sp = sp;

    m_Thread_Window = std::thread(&LLarSDL::_WindowThread, this);
}

bool LLarSDL::_InitSDL()
{
    // 检查SDL是否已经初始化,如果没有则初始化视频子系统
    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L" SDL视频子系统已初始化");
        return true;
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L" SDL初始化失败: %s", _getSDLErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 当前SDL窗口对视频子系统进行初始化");
    return true;
}

bool LLarSDL::_InitSDL_Image()
{
    //初始化SDL_Image
    const int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags) 
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L" SDL_Image 初始化失败: %s", _getIMGErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" SDL_Image 库初始化成功");
    return true;
}

bool LLarSDL::_InitSDL_TTF()
{
    // 初始化SDL_ttf库
    if (TTF_WasInit() == 0)
    {
        if (TTF_Init() == -1)
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L" TTF初始化失败: %s", _getTTFErrorMsg());
            return false;
        }
        DEBUG_CONSOLE_STR(ConsoleHandle, L" SDL_ttf库还未初始化，当前窗口初始化SDL_ttf库成功");
    }

    // 获取系统字体路径
    wchar_t windowsDir[MAX_PATH];
    GetWindowsDirectoryW(windowsDir, MAX_PATH);
    std::wstring fontPath = std::wstring(windowsDir) + L"\\Fonts\\msyh.ttc"; // 使用微软雅黑字体

    // 转换为UTF-8编码
    std::string utf8Path;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) 
    {
        utf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, fontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 转换字体路径为UTF-8编码成功");

    // 加载字体，字号16
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16 * m_Scale);
    if (!m_TTF_Font)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L" 字体加载失败: %s", _getTTFErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 字体加载成功");

    if (m_TTF_Font)
    {
        TTF_SetFontStyle(m_TTF_Font, TTF_STYLE_NORMAL);
        TTF_SetFontOutline(m_TTF_Font, 0); // 设置为0表示无轮廓
        TTF_SetFontKerning(m_TTF_Font, 1); // 启用字距调整
        TTF_SetFontHinting(m_TTF_Font, TTF_HINTING_LIGHT); // 使用轻度微调
    }
    return true;
}

bool LLarSDL::_CreateSDLWindow(const SDL_Rect& gArea)
{
    // 根据传入的 MFC CRect 计算窗口位置和大小
    const int x = gArea.x;
    const int y = gArea.y;
    const int w = gArea.w;
    const int h = gArea.h;

    // 创建 SDL 窗口（无边框、显示）
    m_SDL_Window = SDL_CreateWindow(
        "SDL Window",    // 窗口标题
        x, y,             // 窗口左上角位置
        w, h,             // 宽度和高度
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN
    );
    if (!m_SDL_Window)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle,
            L" SDL_CreateWindow 失败: %s",
            _getSDLErrorMsg().c_str());
        return false;
    }

    // 创建渲染器
    m_SDL_Renderer = SDL_CreateRenderer(
        m_SDL_Window,
        -1,                     // 让 SDL 自动选择驱动
        m_bIsAccelerated ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE  
    );
    if (!m_SDL_Renderer) 
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle,
            L" SDL_CreateRenderer 失败: %s",
            _getSDLErrorMsg().c_str());
        SDL_DestroyWindow(m_SDL_Window);
        m_SDL_Window = nullptr;
        return false;
    }

    //设置渲染模式
    SDL_SetRenderDrawBlendMode(m_SDL_Renderer, SDL_BLENDMODE_BLEND);//启用alpha通道
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    //获取窗口的HWND句柄
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(m_SDL_Window, &wmInfo))
        m_hWnd = wmInfo.info.win.window;
    m_Shadow.Create(m_hWnd);//初始化窗口阴影框架
    m_Shadow.SetColor(m_sp.color);
    m_Shadow.SetDarkness(m_sp.Darkness);
    m_Shadow.SetSize(m_sp.size);
    m_Shadow.SetSharpness(m_sp.sharpness);     

    //注册点击测试回调
    _RegSDLHitTestFunc();
    return true;
}

void LLarSDL::_ProcessEvents()
{
    SDL_Event e;
    // 轮询所有待处理事件
    while (SDL_PollEvent(&e))
    {
        switch (e.type) 
        {
        case SDL_QUIT:
            // 用户点击「关闭窗口」按钮
            // TODO:若要关闭窗口，请发送消息让其他线程调用ReleaseInstance
            break;

        case SDL_WINDOWEVENT:
            // 窗口相关事件（最小化、恢复、移动、大小改变等）
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                // 窗口被请求关闭
                // TODO:若要关闭窗口，请发送消息让其他线程调用ReleaseInstance
                break;
            case SDL_WINDOWEVENT_RESIZED:
                // 窗口大小改变
                // e.window.data1 = 新宽度； e.window.data2 = 新高度
                // TODO: 更新渲染目标、重置视口等
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                // 窗口获得焦点
                // TODO: 处理焦点获得
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                // 窗口失去焦点
                // TODO: 处理焦点丢失
                break;
            default:
                // 其他窗口事件
                break;
            }
            break;

        case SDL_KEYDOWN:
            // 键盘按下
            // e.key.keysym.sym 包含键值
            // TODO: 处理按键按下
            break;

        case SDL_KEYUP:
            // 键盘释放
            // TODO: 处理按键释放
            break;

        case SDL_MOUSEMOTION:
            // 鼠标移动
            // e.motion.x, e.motion.y：鼠标位置
            // e.motion.xrel, e.motion.yrel：相对移动
            // TODO: 处理鼠标移动
            break;

        case SDL_MOUSEBUTTONDOWN:
            // 鼠标按键按下
            // e.button.button：哪一个按钮（左、中、右）
            // e.button.x, e.button.y：点击位置
            // TODO: 处理鼠标按下
            break;

        case SDL_MOUSEBUTTONUP:
            // 鼠标按键释放
            // TODO: 处理鼠标释放
            break;

        case SDL_MOUSEWHEEL:
            // 鼠标滚轮
            // e.wheel.x, e.wheel.y：水平/垂直滚动
            // TODO: 处理滚轮滚动
            break;

        case SDL_TEXTINPUT:
            // 文本输入事件（用于输入法/Unicode 字符）
            // e.text.text：输入的文本
            // TODO: 处理文本输入
            break;

        default:
            // 其他事件（手柄、触摸、拖拽等）
            break;
        }
    }
}

void LLarSDL::_Render()
{
    //填充窗口背景色
    SDL_SetRenderDrawColor(m_SDL_Renderer, 155, 155, 155, 255);
    SDL_RenderFillRect(m_SDL_Renderer, &m_CtRect_WindowRect);

    // TODO: 处理窗口内容渲染
    //....

    //更新当前帧
    SDL_RenderPresent(m_SDL_Renderer);
}

bool LLarSDL::_Cus_Init()
{
    return true;
}

bool LLarSDL::_RegSDLHitTestFunc()
{
    SDL_SetWindowHitTest(m_SDL_Window, HitTestCallback, NULL);
    return true;
}

void LLarSDL::_WindowThread()
{
    if (!_Initialize(m_Rect_WindowRect))
    {
        DB(ConsoleHandle, L"SDL窗口初始化失败！");
        return;
    }
    s_isRunning = true;
    //SDL_SetWindowInputFocus(m_SDL_Window);
    DB(ConsoleHandle, L"开始进入SDL窗口循环");
    while (s_isRunning)
    {
        _ProcessEvents();
        if (!s_isRunning.load())
            break;
        _Render();
        SDL_Delay(1000 / m_iframeRate); // ~60 FPS
    }
    DB(ConsoleHandle, L"SDL窗口循环结束");
}

std::wstring LLarSDL::_getSDLErrorMsg()
{
    const char* sdlError = SDL_GetError();
    if (!sdlError || *sdlError == '\0') 
    {
        return L""; 
    }
    wchar_t wSdlError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars,
        wSdlError,
        _countof(wSdlError),
        sdlError,
        _TRUNCATE);
    return std::wstring(wSdlError);
}

std::wstring LLarSDL::_getTTFErrorMsg()
{
    const char* ttfError = TTF_GetError();
    if (!ttfError || *ttfError == '\0')
        return L"";
    wchar_t wTtfError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars,
        wTtfError,
        _countof(wTtfError),
        ttfError,
        _TRUNCATE);
    return std::wstring(wTtfError);
}

std::wstring LLarSDL::_getIMGErrorMsg()
{
    const char* imgError = IMG_GetError();
    if (!imgError || *imgError == '\0') 
    {
        return L"";
    }
    wchar_t wImgError[256] = { 0 };
    size_t converted = 0;
    mbstowcs_s(&converted,
        wImgError,
        _countof(wImgError),
        imgError,
        _TRUNCATE);
    return std::wstring(wImgError);
}

float LLarSDL::_GetDPI()
{
    HDC screen = ::GetDC(NULL);
    if (screen == NULL) 
    {
        DB(ConsoleHandle, L"无法获取用户DPI!");
        return 1.0f;
    }
    int dpi = GetDeviceCaps(screen, LOGPIXELSX);
    ::ReleaseDC(NULL, screen);
    return static_cast<double>(dpi) / 96.0;
}

LLarSDL::LLarSDL()
{
    m_Rect_WindowRect = SDL_Rect();
    m_CtRect_WindowRect = SDL_Rect();
    m_TTF_Font = nullptr;
    m_SDL_Renderer = nullptr;
    m_SDL_Window = nullptr;
    m_Scale = _GetDPI();
    s_isRunning.store(false);
}

SDL_HitTestResult SDLCALL HitTestCallback(SDL_Window* win, const SDL_Point* area, void* data)
{
    const int TITLEBAR_HEIGHT = 40;
    if (area->y <= TITLEBAR_HEIGHT && area->x)
    {
        return SDL_HITTEST_DRAGGABLE;
    }
    return SDL_HITTEST_NORMAL;
}