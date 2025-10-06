#include "stdafx.h"
#include "ComponentClass.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;

/// WindowDrag

bool COMCLASS::SDL::WindowDrag::MoveActive(_In_ SDL_Window* Window)
{
    //记录全局鼠标位置
    int gx, gy, left, top;
    SDL_GetGlobalMouseState(&gx, &gy);
    SDL_GetWindowPosition(Window, &left, &top);

    //计算偏移：鼠标点相对于窗口左上角
    m_DragOffsetX = gx - left;
    m_DragOffsetY = gy - top;

    // 开启拖拽
    m_Bool_IsDragging = true;
    SDL_RaiseWindow(Window);
    return false;
}

bool COMCLASS::SDL::WindowDrag::MoveTo(_In_ SDL_Window* Window, _Out_ CRect* MovedArea)
{
    if (m_Bool_IsDragging)
    {
        int gx, gy;
        SDL_GetGlobalMouseState(&gx, &gy);

        // 计算新窗口左上角坐标
        int newLeft = gx - m_DragOffsetX;
        int newTop = gy - m_DragOffsetY;

        // MovedArea(如果有则更新)
        if (MovedArea)
        {
            (*MovedArea).MoveToXY(newLeft, newTop);
        }
        SDL_SetWindowPosition(Window, newLeft, newTop);
        return true;
    }
    return false;
}

bool COMCLASS::SDL::WindowDrag::MoveInActive()
{
    m_DragOffsetX - 1;
    m_DragOffsetY - 1;
    m_Bool_IsDragging = false;
    return true;
}

COMCLASS::SDL::WindowDrag::WindowDrag()
{
    m_DragOffsetX - 1;
    m_DragOffsetY - 1;
    m_Bool_IsDragging = false;
}

COMCLASS::SDL::WindowDrag::~WindowDrag()
{

}

/// WindowManager

COMCLASS::SDL::WindowManager* COMCLASS::SDL::WindowManager::QueryComponent()
{
    return nullptr;
}

void COMCLASS::SDL::WindowManager::ReleaseComponent()
{
}

void COMCLASS::SDL::WindowManager::GlobalInit()
{
}

bool COMCLASS::SDL::WindowManager::AddWindow(void* windowIns, int uid)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::findWindow(int uid, void** windowIns)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::ShowWindow(int uid)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::HideWindow(int uid)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::DestoryWindow(int uid)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::GetWindowCount(int* count)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::GetWindowsMap(std::map<int, void*>** map)
{
    return false;
}

bool COMCLASS::SDL::WindowManager::findShowingWindow(int* uid, void** windowIns)
{
    return false;
}

/// WindowOpenGL

COMCLASS::SDL::WindowOpenGL::WindowOpenGL()
    :m_IsInitialized(false),
    m_glctx(nullptr),
    m_Win (nullptr),
    m_ptexMgr(nullptr),
    m_pShader(nullptr)
{
    m_sClearColor.r = 0.2;
    m_sClearColor.g = 0.2;
    m_sClearColor.b = 0.2;
    m_sClearColor.a = 1.0;
}

COMCLASS::SDL::WindowOpenGL::~WindowOpenGL()
{
    DB(ConsoleHandle, L"开始清理WindowOpenGL");
    if (m_Win)
        SDL_DestroyWindow(m_Win);
    if(m_glctx)
        SDL_GL_DeleteContext(m_glctx);
    m_Win = nullptr;
    m_IsInitialized = false;
    m_glctx = nullptr;
    m_ptexMgr = nullptr;
    DB(ConsoleHandle, L"清理WindowOpenGL资源完成");
}

bool COMCLASS::SDL::WindowOpenGL::CreateOpenGLSDLWindow(
    int windowwidth, int windowheight,
    const glCtxAbality& qtySet, const glCtxOption& dqtySet
)
{
    if (!initOpenGL(windowwidth, windowheight, qtySet, dqtySet))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"WindowOpenGL初始化SDLOpenGL失败！");
        return false;
    }
    DB(ConsoleHandle, L"初始化SDL OpenGL 完成");
    m_IsInitialized = true;

    //应用渲染选项
    setglCtxOption(dqtySet);
    DB(ConsoleHandle, L"应用SDL OpenGL渲染选项成功");
    return true;
}

bool COMCLASS::SDL::WindowOpenGL::ClearFrame()
{
    if (!JudgeCallLegal())
        return false;
    glClearColor(m_sClearColor.r, m_sClearColor.g, m_sClearColor.b, m_sClearColor.a);
    GLbitfield mask = GL_COLOR_BUFFER_BIT;
    if (m_dqtySet.depthTest) mask |= GL_DEPTH_BUFFER_BIT;
    if (m_dqtySet.stencilTest)mask |= GL_STENCIL_BUFFER_BIT;
    glClear(mask);
    return true;
}

bool COMCLASS::SDL::WindowOpenGL::SwapFrameToWindow()
{
    if (!JudgeCallLegal())
        return false;

    SDL_GL_SwapWindow(m_Win);
    return true;
}

void COMCLASS::SDL::WindowOpenGL::setglCtxOption(const glCtxOption& dqtySet)
{
    m_dqtySet = dqtySet;
    //设置是否开启MSAA
    if (dqtySet.msaaActive && m_qtySet.msaaSamples != 0)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }

    //设置是否启用sRGB帧缓冲
    if (m_qtySet.sRgbFramebuffer && dqtySet.srgbBuffer)
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else
    {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }

    //设置是否启用透明混合
    if (dqtySet.blend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(dqtySet.blendSrc, dqtySet.blendDst);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    //设置线段平滑
    if (dqtySet.lineSmooth)
        glEnable(GL_LINE_SMOOTH);
    else
        glDisable(GL_LINE_SMOOTH);

    //设置点平滑
    if (dqtySet.pointSmooth)
        glEnable(GL_POINT_SMOOTH);
    else
        glDisable(GL_POINT_SMOOTH);

    //设置Alpha-to-coverage是否开启 
    if (dqtySet.alphaToCoverage && m_qtySet.msaaSamples > 0)
        glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    else
        glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    //是否开启模板测试
    if (dqtySet.stencilTest)
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);

    //是否开启深度测试
    if (dqtySet.depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    //是否开启面剔除
    if (dqtySet.cullFace)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(dqtySet.cullMode);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

}

void COMCLASS::SDL::WindowOpenGL::setClearColor(float r, float g, float b, float a)
{
    m_sClearColor.r = r;
    m_sClearColor.g = g;
    m_sClearColor.b = b;
    m_sClearColor.a = a;
}

void COMCLASS::SDL::WindowOpenGL::setViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
}

void COMCLASS::SDL::WindowOpenGL::setGLTexManager(GLTexManager* texManager)
{
    m_ptexMgr = texManager;
}

void COMCLASS::SDL::WindowOpenGL::setVBOVAOShader(VBOVAOShader* Shader)
{
    m_pShader = Shader;
}

void COMCLASS::SDL::WindowOpenGL::addOpenGLDrawCallBack(
    std::string cbkey, 
    std::function<void()> drawCB
)
{
    m_fDrawCbs.emplace(cbkey, drawCB);
}

void COMCLASS::SDL::WindowOpenGL::removeOpenGLDrawCallBack(std::string cbkey)
{
    auto it = m_fDrawCbs.find(cbkey);
    if (it != m_fDrawCbs.end())
    {
        m_fDrawCbs.erase(it);
        DB(ConsoleHandle, L"成功删除一个渲染回调");
    }
    else
    {
        DB(ConsoleHandle, L"找不到要删除的渲染回调");
    }
}

void COMCLASS::SDL::WindowOpenGL::removeAllOpenGLDrawCallBack()
{
    m_fDrawCbs.clear();
}

void COMCLASS::SDL::WindowOpenGL::fillFrame()
{
    if (m_fDrawCbs.empty())
    {
        DB(ConsoleHandle, L"未设置任何渲染逻辑,fillFrame为空");
        return;
    }
    for (const auto& s : m_fDrawCbs)
    {
        s.second();
    }
}

bool COMCLASS::SDL::WindowOpenGL::drawTexture(
    const std::string& key, 
    float x, float y, 
    float w, float h
)
{
    if (!JudgeCallLegal())
    {
        DB(ConsoleHandle, L"调用判断非法，调用drawTexture失败!");
        return false;
    }
    if (!m_ptexMgr)
    {
        DB(ConsoleHandle, L"未设置资源组件，调用drawTexture失败!");
        return false;
    }
    GLuint tex = m_ptexMgr->get(key);
    if (tex == 0)
    {
        DB(ConsoleHandle, L"未找到tex资源，调用drawTexture失败!");
        return false;
    }

    //开启2D绘图
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    //绘画纹理
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x, y);
    glTexCoord2f(0, 1); glVertex2f(x, y + h);
    glTexCoord2f(1, 1); glVertex2f(x + w, y + h);
    glTexCoord2f(1, 0); glVertex2f(x + w, y);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    return true;
}

bool COMCLASS::SDL::WindowOpenGL::initOpenGL(
    int windowwidth, int windowheight, 
    const glCtxAbality& qtySet, const glCtxOption& dqtySet
)
{
    //检查是否初始化SDL视频子系统，否则退出
    if (!(SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO)) 
    {
        DB(ConsoleHandle, L"WindowOpenGL: SDL Video 子系统尚未初始化，请先调用 SDL_Init(SDL_INIT_VIDEO)！");
        return false;
    }
    
    //设置选项结构体
    m_qtySet = qtySet;
    m_dqtySet = dqtySet;

    //设置版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, qtySet.glMajor);//设置OpenGL版本
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, qtySet.glMinor);//设置OpenGL次版本

    //设置是否开启MSAA
    if (qtySet.msaaSamples > 0)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);              //设置是否开启MSAA
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, qtySet.msaaSamples);//设置MSAA值
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);//不开启MSAA
    }
    
    //指定深度和模板的位数
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, qtySet.depthBits);      //像素深度位数
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, qtySet.stencilBits);  //模板位数

    //设置是否开启sRGB帧缓冲(做色彩空间->线性空间的自动计算)
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, qtySet.sRgbFramebuffer ? 1 : 0);

    //设置是否开启双缓冲
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, qtySet.doubleBuffer ? 1 : 0);

    //创建带OpenGL的SDL窗口
    m_Win = SDL_CreateWindow(
        "SDL OpenGL Window",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowwidth, windowheight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (m_Win)
    {
        DB(ConsoleHandle, L"创建SDL OpenGL窗口成功");
        m_glctx = SDL_GL_CreateContext(m_Win);
        if (!m_glctx)
        {
            DB(ConsoleHandle, "SDL_GL_CreateContext 失败");
            m_Win = nullptr;
            m_IsInitialized = false;
            return false;
        }
        DB(ConsoleHandle, L"创建SDL OpenGL渲染上下文成功");

        //初始化GL状态
        int w, h;
        SDL_GetWindowSize(m_Win, &w, &h);
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, h, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        DB(ConsoleHandle, L"初始化GL状态成功!");
        return true;
    }
    return false;
}

bool COMCLASS::SDL::WindowOpenGL::JudgeCallLegal()
{
    if (!m_IsInitialized)
    {
        DB(ConsoleHandle, L"非法调用WindowOpenGL，未进行初始化");
        return false;
    }
    if (!m_Win)
    {
        DB(ConsoleHandle, L"非法调用WindowOpenGL,窗口无效");
        return false;
    }
    if (!m_glctx)
    {
        DB(ConsoleHandle, L"非法调用WindowOpenGL,OpenGL上下文无效");
        return false;
    }
    return true;
}

/// GLTexManager

COMCLASS::SDL::GLTexManager::GLTexManager()
{
}

COMCLASS::SDL::GLTexManager::~GLTexManager()
{
    DB(ConsoleHandle, L"开始清理SDL OpenGL资源管理器组件");
    for (auto& kv : m_map)
        if (kv.second) glDeleteTextures(1, &kv.second);
    DB(ConsoleHandle, L"开始清理SDL OpenGL资源管理器组件完成");
}

bool COMCLASS::SDL::GLTexManager::load(const std::string& key, const std::string& path)
{
    DB(ConsoleHandle, L"开始加载SDL OpenGL纹理到SDL OpenGL资源管理器");
    // 如果已有相同 key，先删除旧纹理
    unload(key);

    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) return false;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLenum fmt = (surf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, surf->w, surf->h, 0, fmt, GL_UNSIGNED_BYTE, surf->pixels);

    //参数设置
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(surf);
    m_map[key] = tex;
    DB(ConsoleHandle, L"加载SDL OpenGL纹理到SDL OpenGL资源管理器完成");
    return true;
}

bool COMCLASS::SDL::GLTexManager::unload(const std::string& key)
{
    DB(ConsoleHandle, L"开始从SDL OpenGL资源管理器中卸载指定SDL OpenGL纹理");
    auto it = m_map.find(key);
    if (it != m_map.end() && it->second)
    {
        glDeleteTextures(1, &it->second);
        m_map.erase(it);
    }
    DB(ConsoleHandle, L"从SDL OpenGL资源管理器中卸载指定SDL OpenGL纹理完成");
    return true;
}

bool COMCLASS::SDL::GLTexManager::unloadAll()
{
    for (const auto& s : m_map)
    {
        glDeleteTextures(1, &s.second);
    }
    return true;
}

GLuint COMCLASS::SDL::GLTexManager::get(const std::string& key) const
{
    DB(ConsoleHandle, L"从SDL OpenGL资源管理器中获取指定SDL OpenGL纹理texid");
    auto it = m_map.find(key);
    return (it != m_map.end() ? it->second : 0u);
}

COMCLASS::SDL::VBOVAOShader::VBOVAOShader()
{
}

COMCLASS::SDL::VBOVAOShader::~VBOVAOShader()
{
}

bool COMCLASS::SDL::VBOVAOShader::createShaderProgram(std::string sVSSouce, std::string m_sFSSouce)
{
    m_sVertexShaderSource = sVSSouce;
    m_sFragmentShaderSource = m_sFSSouce;
    int success;//是否编译成功

    //创建并编译顶点着色器
    const char* vertexShaderSouce = m_sVertexShaderSource.c_str();
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSouce, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        debugGLErrorMsg(L"编译顶点着色器失败!", vertexShader);
        return false;
    }

    //创建并编译片段着色器
    const char* fragmentShaderSouce = m_sFragmentShaderSource.c_str();
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSouce, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        debugGLErrorMsg(L"编译片段着色器失败!", fragmentShader);
        return false;
    }

    //将顶点着色器和片段着色器绑定给gl
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        debugGLErrorMsg(L"链接着色器失败!", shaderProgram);
        return false;
    }

    //删除已经使用的着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}

bool COMCLASS::SDL::VBOVAOShader::createVBOVAO(unsigned int* vbo, unsigned int* vao)
{
    //顶点数据
    float vertices[] = {
        -0.5f,   0.0f, 0.0f,
         0.5f,   0.0f, 0.0f,
         0.0f, 0.866f, 0.0f,
    };

    //创建VAO和VBO（顶点缓冲对象和顶点数组对象）
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);

    //绑定vao和vbo
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);

    //拷贝顶点数据到顶点缓冲对象中
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    //告诉GPU如何识别顶点数据
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //启用指定顶点属性
    glEnableVertexAttribArray(0);

    //解绑vao和vbo从而保存顶点属性调用
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return true;
}

void COMCLASS::SDL::VBOVAOShader::debugGLErrorMsg(wchar_t info[128], int shaderId)
{
    char  charLog[512] = { 0 };
    glGetShaderInfoLog(shaderId, sizeof(charLog), nullptr, charLog);
    wchar_t wLog[512] = { 0 };
    MultiByteToWideChar(
        CP_ACP,           
        0,
        charLog,
        -1,
        wLog,
        _countof(wLog)
    );
    DBFMT(ConsoleHandle, L"%ls: %ls", info, wLog);
}

COMCLASS::SDL::WindowScroller::WindowScroller(SDL_Rect scrollArea, int itemSize, int itemHeight)
{
    m_Uint32_LastFrameTime = 0;
    m_Float_CurrentScroll = 0.0f;
    m_Float_TargetScroll = 0.0f;
    m_Float_ScrollVelocity = 0.0f;
    m_Float_ScrollDamping = 0.94f;
    m_Float_ScrollSensitivity = 8.0f;
    m_ScrollArea = scrollArea;
    m_itemSize = itemSize;
    m_itemHeight = itemHeight;
}

COMCLASS::SDL::WindowScroller::~WindowScroller()
{
}

void COMCLASS::SDL::WindowScroller::SetScrollSensitivity(float sensitivity)
{
    m_Float_ScrollSensitivity = sensitivity;
}

void COMCLASS::SDL::WindowScroller::EnterRender(SDL_Renderer* renderer)
{
    SDL_RenderSetClipRect(renderer, &m_ScrollArea);
}

void COMCLASS::SDL::WindowScroller::ExitRender(SDL_Renderer* renderer)
{
    // 绘制滚动条（如果需要滚动）
    int contentHeight = m_itemSize * m_itemHeight;
    int viewportHeight = m_ScrollArea.h;
    if (contentHeight > viewportHeight)
    {
        // 计算滚动条尺寸和位置
        float scrollRatio = (float)viewportHeight / contentHeight;
        float scrollHandleHeight = viewportHeight * scrollRatio;
        scrollHandleHeight = max(30.0f, scrollHandleHeight); // 最小高度

        float scrollPosition = m_Float_CurrentScroll / (contentHeight - viewportHeight);
        float scrollHandleY = m_ScrollArea.y + scrollPosition * (viewportHeight - scrollHandleHeight);

        // 背景
        SDL_Rect scrollbarBg =
        {
            m_ScrollArea.x + m_ScrollArea.w - 8,
            m_ScrollArea.y,
            8,
            viewportHeight
        };

        // 滑块
        SDL_Rect scrollHandle =
        {
            m_ScrollArea.x + m_ScrollArea.w - 8,
            (int)scrollHandleY,
            8,
            (int)scrollHandleHeight
        };

        // 绘制滑块和滑动区域背景
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 200);
        SDL_RenderFillRect(renderer, &scrollbarBg);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &scrollHandle);
    }
    SDL_RenderSetClipRect(renderer, NULL);
}

void COMCLASS::SDL::WindowScroller::UpdateScrollPhysics()
{
    // 计算帧时间差
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - m_Uint32_LastFrameTime) / 1000.0f;
    m_Uint32_LastFrameTime = currentTime;

    // 防止时间跳变
    if (deltaTime > 0.1f)
        deltaTime = 0.016f; // 约60FPS

    // 计算最大滚动范围
    int contentHeight = m_itemSize * m_itemHeight;
    int viewportHeight = m_ScrollArea.h;
    int maxScroll = max(0, contentHeight - viewportHeight);

    // 应用滚动速度
    m_Float_TargetScroll += m_Float_ScrollVelocity * deltaTime * 60.0f;

    // 限制滚动范围
    if (m_Float_TargetScroll < 0)
    {
        // 顶部边界弹性
        m_Float_TargetScroll *= 0.5f;
        m_Float_ScrollVelocity *= -0.2f;
    }
    else if (m_Float_TargetScroll > maxScroll && maxScroll > 0)
    {
        // 底部边界弹性
        float overScroll = m_Float_TargetScroll - maxScroll;
        m_Float_TargetScroll = maxScroll + overScroll * 0.5f;
        m_Float_ScrollVelocity *= -0.2f;
    }

    // 平滑过渡到目标位置
    float diff = m_Float_TargetScroll - m_Float_CurrentScroll;
    float smoothFactor = min(1.0f, deltaTime * 12.0f);
    m_Float_CurrentScroll += diff * smoothFactor;

    // 应用阻尼
    m_Float_ScrollVelocity *= pow(m_Float_ScrollDamping, deltaTime * 60.0f);

    // 如果速度很小，停止滚动
    if (fabs(m_Float_ScrollVelocity) < 0.5f && fabs(diff) < 0.5f)
    {
        m_Float_ScrollVelocity = 0.0f;
        m_Float_CurrentScroll = round(m_Float_CurrentScroll); // 对齐到整数像素
    }
}

void COMCLASS::SDL::WindowScroller::UpdateMouseWheelEvent(SDL_Event* e)
{
    m_Float_ScrollVelocity -= e->wheel.y * m_Float_ScrollSensitivity;
}


