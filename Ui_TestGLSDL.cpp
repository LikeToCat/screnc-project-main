#include "stdafx.h"
#include "Ui_TestGLSDL.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;

bool Ui_TestGLSDL::s_isRunning = false;
Ui_TestGLSDL* Ui_TestGLSDL::s_ins = nullptr;
bool Ui_TestGLSDL::s_isInitlized = false;

Ui_TestGLSDL* Ui_TestGLSDL::GetInstance()
{
    if (!s_ins)
        s_ins = new Ui_TestGLSDL;
    return s_ins;
}

bool Ui_TestGLSDL::IsInsExist()
{
    if (s_ins)
        return true;
    else
        return false;
}

bool Ui_TestGLSDL::IsRunning()
{
    return s_isRunning;
}

bool Ui_TestGLSDL::IsInitlized()
{
    return s_isInitlized;
}

void Ui_TestGLSDL::ReleaseInstance()
{
    if (s_ins)
    {
        delete s_ins;
        s_ins = nullptr;
        s_isInitlized = false;
        s_isRunning = false;
    }
}

Ui_TestGLSDL::~Ui_TestGLSDL()
{

}

bool Ui_TestGLSDL::Initialize(const SDL_Rect& gArea)
{
    if (!InitSDL())
    {
        DBFMT(ConsoleHandle, L"SDL初始化失败！错误原因:%s", getSDLErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 初始化SDL成功");

    if (!InitSDL_Image())
    {
        DBFMT(ConsoleHandle, L"SDL_Image初始化失败！错误原因:%s", getSDLErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 初始化SDL_Image成功");

    if (!InitSDL_TTF())
    {
        DBFMT(ConsoleHandle, L"SDL_TTF初始化失败！错误原因:%s", getSDLErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 初始化SDL_TTF成功");

    if (!CreateSDLWindow(gArea))
    {
        DBFMT(ConsoleHandle, L"SDL窗口创建失败！错误原因:%s", getSDLErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 创建SDL窗口成功");

    s_isInitlized = true;
    return true;
}

void Ui_TestGLSDL::Run(const CRect& gArea)
{
    m_Rect_WindowRect.x = gArea.left;
    m_Rect_WindowRect.y = gArea.top;
    m_Rect_WindowRect.w = gArea.Width();
    m_Rect_WindowRect.h = gArea.Height();

    m_CtRect_WindowRect.w = gArea.Width();
    m_CtRect_WindowRect.h = gArea.Height();
    m_CtRect_WindowRect.x = 0;
    m_CtRect_WindowRect.y = 0;

    m_Thread_Window = std::thread(&Ui_TestGLSDL::WindowThread, this);
    m_Thread_Window.detach();
}

bool Ui_TestGLSDL::InitSDL()
{
    // 检查SDL是否已经初始化,如果没有则初始化视频子系统
    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L" SDL视频子系统已初始化");
        return true;
    }
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L" SDL初始化失败: %s", getSDLErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 当前SDL窗口对视频子系统进行初始化");
    return true;
}

bool Ui_TestGLSDL::InitSDL_Image()
{
    //初始化SDL_Image
    const int flags = IMG_INIT_PNG | IMG_INIT_JPG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags)
    {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L" SDL_Image 初始化失败: %s", getIMGErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" SDL_Image 库初始化成功");
    return true;
}

bool Ui_TestGLSDL::InitSDL_TTF()
{
    // 初始化SDL_ttf库
    if (TTF_WasInit() == 0)
    {
        if (TTF_Init() == -1)
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L" TTF初始化失败: %s", getTTFErrorMsg());
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
        DEBUG_CONSOLE_FMT(ConsoleHandle, L" 字体加载失败: %s", getTTFErrorMsg());
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L" 字体加载成功");
    return true;
}

bool Ui_TestGLSDL::CreateSDLWindow(const SDL_Rect& gArea)
{
    if (m_WindowOpenGL.CreateOpenGLSDLWindow(gArea.w, gArea.h))
    {
        DB(ConsoleHandle, L"创建SDL OpenGL窗口失败！");
        return false;
    }
    m_WindowOpenGL.setGLTexManager(&m_GLTexManager);
    DB(ConsoleHandle, L"OpenGL资源组件初始化成功");

    //顶点着色器
    const char* vertexShaderSource = \
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";
    //片段着色器
    const char* fragmentShaderSource = \
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}";
    unsigned int VAO;
    unsigned int VBO;
    if (m_VVShader.createShaderProgram(vertexShaderSource, fragmentShaderSource))
    {
        DB(ConsoleHandle, L"创建绑定着色器失败！");
        return false;
    }
    if (m_VVShader.createVBOVAO(&VAO, &VBO))
    {
        DB(ConsoleHandle, L"创建VBO，VAO失败!");
        return false;
    }
    m_WindowOpenGL.setVBOVAOShader(&m_VVShader);
    SetWindowDrawPaint();
    DB(ConsoleHandle, L"VBO/VAO Shader管线初始化成功");
    return true;
}

void Ui_TestGLSDL::ProcessEvents()
{
    SDL_Event e;
    // 轮询所有待处理事件
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            // 用户点击「关闭窗口」按钮
            // TODO: 在这里设置 s_isRunning = false 或者发送退出信号
            break;

        case SDL_WINDOWEVENT:
            // 窗口相关事件（最小化、恢复、移动、大小改变等）
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                // 窗口被请求关闭
                // TODO: 处理窗口关闭逻辑
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

void Ui_TestGLSDL::Render()
{
    m_WindowOpenGL.ClearFrame();        //清屏
    m_WindowOpenGL.fillFrame();         //填帧
    m_WindowOpenGL.SwapFrameToWindow(); //推帧
}

void Ui_TestGLSDL::WindowThread()
{
    if (!Initialize(m_Rect_WindowRect))
    {
        DB(ConsoleHandle, L"SDL窗口初始化失败！");
    }
    s_isRunning = true;
    DB(ConsoleHandle, L"开始进入SDL窗口循环");
    while (s_isRunning)
    {
        ProcessEvents();
        Render();
        SDL_Delay(16); // ~60 FPS
    }
    DB(ConsoleHandle, L"SDL窗口循环结束");
}

void Ui_TestGLSDL::SetWindowDrawPaint()
{
    m_WindowOpenGL.addOpenGLDrawCallBack("first opengl draw", []()
        {
            //TODO:在这里添加OpenGL绘画
        });
    DB(ConsoleHandle, L"绑定OpenGL绘画函数成功");
}

std::wstring Ui_TestGLSDL::getSDLErrorMsg()
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

std::wstring Ui_TestGLSDL::getTTFErrorMsg()
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

std::wstring Ui_TestGLSDL::getIMGErrorMsg()
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

float Ui_TestGLSDL::GetDPI()
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

Ui_TestGLSDL::Ui_TestGLSDL()
{
    m_Rect_WindowRect = SDL_Rect();
    m_CtRect_WindowRect = SDL_Rect();
    m_TTF_Font = nullptr;
    m_SDL_Renderer = nullptr;
    m_Scale = GetDPI();
}
