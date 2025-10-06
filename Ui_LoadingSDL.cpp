#include "stdafx.h"
#include "Ui_LoadingSDL.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;
// 静态成员初始化
Ui_LoadingSDL* Ui_LoadingSDL::s_instance = nullptr;
std::mutex Ui_LoadingSDL::s_instanceMutex;

Ui_LoadingSDL* Ui_LoadingSDL::GetInstance()
{
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    if (s_instance == nullptr)
    {
        s_instance = new Ui_LoadingSDL();
    }
    return s_instance;
}

void Ui_LoadingSDL::ReleaseInstance()
{
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    if (s_instance != nullptr)
    {
        delete s_instance;
        s_instance = nullptr;
    }
}

Ui_LoadingSDL::Ui_LoadingSDL()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_logoTexture(nullptr)
    , m_font(nullptr)
    , m_textTexture(nullptr)
    , m_dpiScale(1.0f)
    , m_windowWidth(0)
    , m_windowHeight(0)
    , m_shadowSize(20)
    , m_shadowAlpha(255)
    , m_cornerRadius(20)
    , m_cornerLineWidth(5)
    , m_cornerLineLength(30)
    , m_cornerLineColor({ 85, 86, 90, 255 })
    , m_windowRegionWidth(0)
    , m_windowRegionHeight(0)
{
    // 构造函数已初始化成员变量
}

Ui_LoadingSDL::~Ui_LoadingSDL()
{
    Destroy();
}

bool Ui_LoadingSDL::Start(const char* imagePath, float dpiScale)
{
    // 避免重复启动
    if (state.m_running.load())
    {
        return true;
    }

    // 设置初始状态
    state.m_running = true;
    state.m_initialized = false;

    // 保存DPI缩放系数
    m_dpiScale = dpiScale;
    if (m_dpiScale > 1.5)
        m_dpiScale = 1.5;

    // 保存图片路径供初始化使用
    if(imagePath)
        m_imagePath = imagePath;
    DB(ConsoleHandle, L"保存图片路径成功");
    // 在新线程中创建和运行SDL窗口
    Thread.m_thread = std::thread(&Ui_LoadingSDL::WindowThread, this, imagePath);

    // 等待窗口初始化完成
    DB(ConsoleHandle, L"等待窗口初始化完成");
    std::unique_lock<std::mutex> lock(threadProtect.m_initMutex);
    threadProtect.m_initCV.wait(lock, [this] { return state.m_initialized.load(); });

    return state.m_running.load(); // 如果初始化失败，m_running会被设为false
}

void Ui_LoadingSDL::Hide()
{
    if (!m_window || !state.m_running.load())
        return;
    SDL_HideWindow(m_window);
}

void Ui_LoadingSDL::RestoreFromHide()
{
    if (!m_window || !state.m_running.load())
        return;
    SDL_ShowWindow(m_window);
    SDL_RaiseWindow(m_window);
}

void Ui_LoadingSDL::Destroy()
{
    // 设置停止标志
    if (state.m_running.exchange(false))
    {
        // 等待线程结束
        if (Thread.m_thread.joinable())
        {
            Thread.m_thread.join();
        }
    }
}

void Ui_LoadingSDL::SetWindowTopMost()
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(m_window, &wmInfo))
    {
        HWND hwnd = wmInfo.info.win.window;
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void Ui_LoadingSDL::WindowThread(const char* imagePath)
{
    // 在SDL线程中初始化窗口和资源
    bool success = Initialize(imagePath);
    DBFMT(ConsoleHandle, L"Ui_LoadingSDL初始化完成,结果success:%s,%s", success ? L"TRUE" : L"FALSE");
    // 通知主线程初始化已完成
    {
        std::lock_guard<std::mutex> lock(threadProtect.m_initMutex);
        state.m_initialized = true;
        // 如果初始化失败，设置running为false
        if (!success)
        {
            state.m_running = false;
        }
    }
    threadProtect.m_initCV.notify_one();
    DB(ConsoleHandle, L"通知挂起线程启动");
    // 如果初始化成功，进入渲染循环
    if (success && state.m_running.load())
    {
        DB(ConsoleHandle, L"进入渲染循环");
        RenderLoop();
    }

    // 清理资源（无论是否初始化成功）
    DB(ConsoleHandle, L"清理资源");
    CleanupResources();
}

bool Ui_LoadingSDL::Initialize(const char* imagePath)
{
    // 初始化SDL，只请求必要的视频子系统以提高性能
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"SDL_Init失败");
        return false;
    }
    DB(ConsoleHandle, L"初始化SDL成功");

    // 初始化SDL_image，只加载PNG格式以减少内存占用
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"IMG_Init失败");
        SDL_Quit();
        return false;
    }
    DB(ConsoleHandle, L"初始化SDL_image成功");

    // 初始化SDL_ttf
    if (TTF_Init() < 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"TTF_Init失败");
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    DB(ConsoleHandle, L"初始化SDL_ttf成功");

    // 获取屏幕尺寸用于居中显示窗口
    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"SDL_GetCurrentDisplayMode失败");
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    DB(ConsoleHandle, L"SDL_GetCurrentDisplayMode成功");

    // 使用固定基础窗口大小并根据DPI缩放
    const float BASE_WINDOW_WIDTH = 530;  // 基准宽度 (100% DPI)
    const float BASE_WINDOW_HEIGHT = 288; // 基准高度 (100% DPI)

    // 根据传入的DPI缩放系数计算实际窗口大小
    m_windowWidth = static_cast<int>(BASE_WINDOW_WIDTH * m_dpiScale);   // 四舍五入
    m_windowHeight = static_cast<int>(BASE_WINDOW_HEIGHT * m_dpiScale); // 四舍五入

    // 创建窗口，尺寸需要加上阴影区域
    m_window = SDL_CreateWindow(
        "Loading...",
        (dm.w - m_windowWidth) / 2,
        (dm.h - m_windowHeight) / 2,
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!m_window)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"SDL_CreateWindow失败");
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    DB(ConsoleHandle, L"SDL_CreateWindow成功");

    // 使用硬件加速渲染器以提高质量
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
    if (!m_renderer)
    {
        // 如果硬件渲染失败，尝试软件渲染
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        if (!m_renderer)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"SDL_CreateRenderer失败");
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            return false;
        }
    }
    DB(ConsoleHandle, L"SDL_CreateRenderer成功");

    // 设置渲染质量为最佳
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    // 创建渲染器后，立即加载背景图片
    if (!LoadBackgroundTexture())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"背景图片加载失败");
    }
    DB(ConsoleHandle, L"背景图片加载成功");

    // 加载图片并创建纹理
    if (imagePath)
    {
        if (!LoadLogoTexture(imagePath))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"LoadLogoTexture失败");
            SDL_DestroyRenderer(m_renderer);
            SDL_DestroyWindow(m_window);
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
            return false;
        }
    }
    DB(ConsoleHandle, L"LoadLogoTexture成功");

    // 创建"正在加载"文本纹理
    if (!CreateTextTexture())
    {
        // 即使无法创建文本，也继续加载窗口
        // 只是不显示文本，不要视为错误
        DB(ConsoleHandle, L"创建正在加载文本纹理失败");
    }

    //CreateChineseTextTextures(); // 创建中文文本纹理

    // 计算图片和文本布局
    //DrawCornerLines();
    CalculateLayout();

    // 预渲染第一帧
    RenderFrame();
    //SetWindowRoundedShape();  // 设置窗口形状为圆角
    SDL_ShowWindow(m_window); // 显示窗口

    DB(ConsoleHandle, L"开始添加SDL加载窗口的阴影效果");
    InitializeShadow();//添加窗口阴影效果
    return true;
}

void Ui_LoadingSDL::InitializeShadow()
{
    // 获取SDL窗口的Windows句柄
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(m_window, &wmInfo))
    {
        DB(ConsoleHandle, L"SDL_GetWindowWMInfo成功");
        HWND hwnd = wmInfo.info.win.window;
        if (hwnd)
        {
            //m_Shadow.SetHideSingleShadow(CWndShadow::ShadowExMode::noEx);
            DB(ConsoleHandle, L"初始化阴影成功,开始创建阴影");

            // 创建阴影（附加到SDL窗口句柄）
            m_Shadow.Create(hwnd);
            DB(ConsoleHandle, L"创建阴影成功");
        }
    }
}

void Ui_LoadingSDL::RenderFrame()
{
    // 清除整个窗口
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_renderer);

    // 使用相同的参数绘制边框
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255); // 纯黑色边框

    // 使用与窗口区域相同的圆角参数
    //DrawRoundedRect(borderRect, m_cornerRadius);

    // 2. 内容区域稍微内缩以创建边框效果
    SDL_SetRenderDrawColor(m_renderer, 26, 31, 37, 255); // 内容背景色

    SDL_Rect contentRect = {
        0,
        0,
        m_windowWidth, // 两边各减1像素
        m_windowHeight
    };

    // 渲染图片
    SDL_RenderCopy(m_renderer, m_backgroundTexture, nullptr, &contentRect);

    // 显示渲染内容
    SDL_RenderPresent(m_renderer);
}

bool Ui_LoadingSDL::LoadLogoTexture(const char* imagePath)
{
    // 预加载图片以获取尺寸
    SDL_Surface* logoSurface = IMG_Load(imagePath);
    if (!logoSurface)
    {
        return false;
    }

    // 存储原始图像尺寸
    int origWidth = logoSurface->w;
    int origHeight = logoSurface->h;

    // 根据DPI缩放计算缩放后的图片尺寸 - 使用四舍五入以避免精度问题
    m_logoWidth = static_cast<int>(origWidth * (m_dpiScale - 0.5));
    m_logoHeight = static_cast<int>(origHeight * (m_dpiScale - 0.5));

    // 优化：直接创建最终尺寸的纹理，减少中间步骤
    SDL_Texture* texture = nullptr;

    // 如果不需要缩放，直接创建纹理以提高性能
    if (fabs(m_dpiScale - 1.0f) < 0.01f)
    {
        // 不需要缩放，直接创建纹理
        texture = SDL_CreateTextureFromSurface(m_renderer, logoSurface);
        SDL_FreeSurface(logoSurface);
    }
    else
    {
        // 需要缩放，使用更高效的算法
        SDL_Surface* scaledSurface = SDL_CreateRGBSurface(
            0, m_logoWidth, m_logoHeight,
            logoSurface->format->BitsPerPixel,
            logoSurface->format->Rmask,
            logoSurface->format->Gmask,
            logoSurface->format->Bmask,
            logoSurface->format->Amask
        );

        if (scaledSurface)
        {
            // 使用高性能的缩放算法
            SDL_BlitScaled(logoSurface, NULL, scaledSurface, NULL);
            texture = SDL_CreateTextureFromSurface(m_renderer, scaledSurface);
            SDL_FreeSurface(scaledSurface);
        }
        SDL_FreeSurface(logoSurface);
    }

    // 验证纹理是否成功创建
    if (!texture)
    {
        return false;
    }

    // 存储纹理
    m_logoTexture = texture;

    return true;
}

bool Ui_LoadingSDL::CreateTextTexture()
{
    // 尝试加载系统默认字体
    // 先尝试常见字体路径加载（根据操作系统）
    const char* fontPaths[] = {
        // Windows 字体 - 优先尝试更好看的字体
        "C:\\Windows\\Fonts\\segoeui.ttf",    // Segoe UI 更现代
        "C:\\Windows\\Fonts\\meiryo.ttc",     // 日文字体但英文也很好看
        "C:\\Windows\\Fonts\\calibri.ttf",    // Calibri
        "C:\\Windows\\Fonts\\arial.ttf",      // Arial 作为后备
        // 其他路径可以根据需要添加
    };

    const int numFonts = sizeof(fontPaths) / sizeof(fontPaths[0]);

    // 尝试加载字体，增大字体大小以提高清晰度
    for (int i = 0; i < numFonts; i++)
    {
        // 字体大小增大，提高清晰度
        int fontSize = static_cast<int>(20 * m_dpiScale);
        m_font = TTF_OpenFont(fontPaths[i], fontSize);
        if (m_font)
        {
            // 设置字体渲染模式为抗锯齿
            TTF_SetFontStyle(m_font, TTF_STYLE_NORMAL);
            TTF_SetFontOutline(m_font, 0);
            TTF_SetFontKerning(m_font, 1);  // 启用字距调整
            TTF_SetFontHinting(m_font, TTF_HINTING_LIGHT); // 使用轻度微调提高清晰度
            break;
        }
    }

    // 如果所有字体都加载失败，返回false
    if (!m_font)
    {
        return false;
    }

    // 使用浅灰色渲染"Loading"文本
    SDL_Color textColor = { 155, 155, 155, 255 }; // 浅灰色效果更好

    // 使用高质量渲染模式
    SDL_Surface* textSurface = TTF_RenderText_Blended(m_font, "Loading", textColor);
    if (!textSurface)
    {
        TTF_CloseFont(m_font);
        m_font = nullptr;
        return false;
    }

    // 创建文本纹理
    m_textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);

    // 保存文本尺寸用于后续布局
    m_textRect.w = textSurface->w;
    m_textRect.h = textSurface->h;

    // 释放表面
    SDL_FreeSurface(textSurface);

    return (m_textTexture != nullptr);
}

void Ui_LoadingSDL::CalculateLayout()
{
    // 1. 首先调整主要内容(logo和loading文本)向上移动
    int contentAreaHeight = m_windowHeight;
    int contentShift = 30; // 向上移动的像素数

    // 计算图片的水平居中位置
    m_logoRect.x = m_shadowSize + (m_windowWidth - m_logoWidth) / 2;

    // 向上偏移图片位置，多偏移contentShift像素
    m_logoRect.y = m_shadowSize + (contentAreaHeight - m_logoHeight) / 2 -
        (contentAreaHeight / 15);
    m_logoRect.w = m_logoWidth;
    m_logoRect.h = m_logoHeight;

    // 如果有Loading文本，也向上移动
    if (m_textTexture)
    {
        m_textRect.x = m_shadowSize + (m_windowWidth - m_textRect.w) / 2;
        m_textRect.y = m_logoRect.y + m_logoRect.h + static_cast<int>(15 * m_dpiScale);
    }
}

void Ui_LoadingSDL::DrawShadow()
{
    // 不再使用多层矩形阴影，而是使用与窗口形状相同的圆角阴影
    int layers = 3; // 减少阴影层数提高性能

    for (int i = 0; i < layers; i++) {
        // 计算阴影透明度，从外到内递减
        float ratio = (float)(layers - i) / layers;
        int alpha = (int)(m_shadowAlpha * ratio * ratio);

        // 设置阴影颜色
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, alpha);

        // 计算阴影矩形，逐渐向内收缩
        int offset = i * m_shadowSize / layers;
        SDL_Rect shadowRect = {
            offset,
            offset,
            m_windowWidth + m_shadowSize * 2 - offset * 2,
            m_windowHeight + m_shadowSize * 2 - offset * 2
        };

        // 使用圆角矩形绘制阴影层
        DrawRoundedRect(shadowRect, m_cornerRadius);
    }
}

void Ui_LoadingSDL::RenderLoop()
{
    // 帧率控制初始化 - 使用更精确的计时方法
    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double deltaTime = 0;

    // 使用SDL的高精度计时器来获取性能频率
    const double PERF_FREQ = static_cast<double>(SDL_GetPerformanceFrequency());
    const double TARGET_FRAME_TIME = 1.0 / 60.0; // 60 FPS

    while (state.m_running.load())
    {
        // 更新计时器
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = (NOW - LAST) / PERF_FREQ; // 精确的帧时间计算

        // 处理事件 - 最小化处理以提高性能
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                state.m_running = false;
                break;
            }
        }

        // 渲染当前帧
        RenderFrame();

        // 更精确的帧率控制
        double frameTime = deltaTime;
        if (frameTime < TARGET_FRAME_TIME)
        {
            Uint32 delayMS = static_cast<Uint32>((TARGET_FRAME_TIME - frameTime) * 1000);
            if (delayMS > 0)
            {
                SDL_Delay(delayMS);
            }
        }
    }
}

void Ui_LoadingSDL::CleanupResources()
{
    // 清理文本资源
    if (m_textTexture)
    {
        SDL_DestroyTexture(m_textTexture);
        m_textTexture = nullptr;
    }
    DB(ConsoleHandle, L"清理文本资源完毕");

    if (m_font)
    {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
    DB(ConsoleHandle, L"清理字体资源完毕");

    // 清理图像资源
    if (m_logoTexture)
    {
        SDL_DestroyTexture(m_logoTexture);
        m_logoTexture = nullptr;
    }
    DB(ConsoleHandle, L"清理图像资源完毕");

    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    DB(ConsoleHandle, L"清理渲染器资源完毕");

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    DB(ConsoleHandle, L"清理窗口资源完毕");

    // 清理中文文本资源
    if (m_companyNameTexture)
    {
        SDL_DestroyTexture(m_companyNameTexture);
        m_companyNameTexture = nullptr;
    }
    DB(ConsoleHandle, L"清理中文文本资源完毕");

    if (m_productInfoTexture)
    {
        SDL_DestroyTexture(m_productInfoTexture);
        m_productInfoTexture = nullptr;
    }
    DB(ConsoleHandle, L"清理m_productInfoTexture资源完毕");

    if (m_chineseFont)
    {
        TTF_CloseFont(m_chineseFont);
        m_chineseFont = nullptr;
    }
    DB(ConsoleHandle, L"清理m_chineseFont资源完毕");

    // 按照相反的初始化顺序进行清理
    DB(ConsoleHandle, L"TTF_Quit调用");
    TTF_Quit();
    DB(ConsoleHandle, L"IMG_Quit调用");
    IMG_Quit();
    DB(ConsoleHandle, L"SDL_Quit调用");
    SDL_Quit();

    DB(ConsoleHandle, L"清理完毕");
}

void Ui_LoadingSDL::DrawRoundedRect(const SDL_Rect& rect, int radius)
{
    // 使用SDL2的内置绘制函数(如果有)或完全匹配Windows API的圆角计算
    // 这个函数实现需要精确匹配CreateRoundRectRgn的行为

    // 确保圆角不超过矩形的一半
    radius = min(radius, min(rect.w / 2, rect.h / 2));

    // 绘制主体矩形
    SDL_Rect centerRect = {
        rect.x + radius,
        rect.y,
        rect.w - 2 * radius,
        rect.h
    };
    SDL_RenderFillRect(m_renderer, &centerRect);

    // 绘制上下矩形
    SDL_Rect topRect = {
        rect.x + radius,
        rect.y,
        rect.w - 2 * radius,
        radius
    };
    SDL_RenderFillRect(m_renderer, &topRect);

    SDL_Rect bottomRect = {
        rect.x + radius,
        rect.y + rect.h - radius,
        rect.w - 2 * radius,
        radius
    };
    SDL_RenderFillRect(m_renderer, &bottomRect);

    // 绘制左右矩形
    SDL_Rect leftRect = {
        rect.x,
        rect.y + radius,
        radius,
        rect.h - 2 * radius
    };
    SDL_RenderFillRect(m_renderer, &leftRect);

    SDL_Rect rightRect = {
        rect.x + rect.w - radius,
        rect.y + radius,
        radius,
        rect.h - 2 * radius
    };
    SDL_RenderFillRect(m_renderer, &rightRect);

    // 绘制四个圆角，使用扇形填充而不是线条
    // 使用更多点以获得更平滑的圆角
    int numPoints = radius * 4;

    // 绘制填充扇形
    for (int corner = 0; corner < 4; corner++) {
        int centerX, centerY;

        // 确定圆心位置
        switch (corner) {
        case 0: // 左上
            centerX = rect.x + radius;
            centerY = rect.y + radius;
            break;
        case 1: // 右上
            centerX = rect.x + rect.w - radius;
            centerY = rect.y + radius;
            break;
        case 2: // 右下
            centerX = rect.x + rect.w - radius;
            centerY = rect.y + rect.h - radius;
            break;
        case 3: // 左下
            centerX = rect.x + radius;
            centerY = rect.y + rect.h - radius;
            break;
        }

        // 绘制填充四分之一圆
        for (int i = 0; i < radius; i++) {
            for (int j = 0; j < radius; j++) {
                // 检查点(i,j)是否在圆内
                if (i * i + j * j <= radius * radius) {
                    int x, y;

                    // 根据当前角落确定实际坐标
                    switch (corner) {
                    case 0: // 左上
                        x = centerX - i;
                        y = centerY - j;
                        break;
                    case 1: // 右上
                        x = centerX + i;
                        y = centerY - j;
                        break;
                    case 2: // 右下
                        x = centerX + i;
                        y = centerY + j;
                        break;
                    case 3: // 左下
                        x = centerX - i;
                        y = centerY + j;
                        break;
                    }

                    // 画点
                    SDL_RenderDrawPoint(m_renderer, x, y);
                }
            }
        }
    }
}

void Ui_LoadingSDL::SetWindowRoundedShape()
{
#ifdef _WIN32
    // 获取窗口句柄
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(m_window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    if (hwnd) 
    {
        // 直接使用m_shadowSize计算窗口尺寸
        int width = m_windowWidth + m_shadowSize * 2;
        int height = m_windowHeight + m_shadowSize * 2;

        // 保存这些值以便调试和其他用途
        m_windowRegionWidth = width;
        m_windowRegionHeight = height;

        // 创建圆角区域
        HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1,
            m_cornerRadius * 2, m_cornerRadius * 2);

        // 设置窗口形状
        SetWindowRgn(hwnd, region, TRUE);
    }
#endif
}

bool Ui_LoadingSDL::CreateChineseTextTextures()
{
    // 尝试加载支持中文的字体 - 先尝试微软雅黑
    const char* fontPaths[] = {
        "C:\\Windows\\Fonts\\msyh.ttc",      // 微软雅黑 (首选)
        "C:\\Windows\\Fonts\\simhei.ttf",    // 黑体
        "C:\\Windows\\Fonts\\FZSTK.TTF",     // 方正书体
        "C:\\Windows\\Fonts\\simsun.ttc",    // 宋体 (后备)
    };

    const int numFonts = sizeof(fontPaths) / sizeof(fontPaths[0]);

    // 增大字体尺寸，确保清晰可见
    int fontSize = static_cast<int>(14 * m_dpiScale);
    bool fontLoaded = false;

    // 尝试加载中文字体
    for (int i = 0; i < numFonts; i++)
    {
        m_chineseFont = TTF_OpenFont(fontPaths[i], fontSize);
        if (m_chineseFont)
        {
            fontLoaded = true;
            break;
        }
    }

    // 如果所有字体都加载失败，尝试使用英文字体
    if (!fontLoaded)
    {
        // 复用已加载的英文字体
        m_chineseFont = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", fontSize);
        if (!m_chineseFont)
            return false;
    }

    // 文本颜色为亮灰色
    SDL_Color textColor = { 180, 180, 180, 255 };

    // 确保代码文件以UTF-8编码保存
    const char* companyName = "\xe6\x88\x90\xe9\x83\xbd\xe6\x85\xa7\xe4\xb9\x90\xe6\x99\xae\xe6\x96\xaf\xe7\xa7\x91\xe6\x8a\x80\xe6\x9c\x89\xe9\x99\x90\xe5\x85\xac\xe5\x8f\xb8";
    const char* productInfo = "\xe5\xbd\x95\xe8\xaf\xbe\xe7\xa8\x8b \xe5\xbd\x95\xe4\xbc\x9a\xe8\xae\xae \xe5\xbd\x95\xe6\xb8\xb8\xe6\x88\x8f \xe5\xbd\x95\xe5\xba\x94\xe7\x94\xa8";

    // 创建公司名称纹理 - 使用UTF-8编码的硬编码字符串
    SDL_Surface* companySurface = TTF_RenderUTF8_Blended(m_chineseFont, companyName, textColor);
    if (!companySurface)
    {
        return false;
    }
    m_companyNameTexture = SDL_CreateTextureFromSurface(m_renderer, companySurface);
    m_companyNameRect.w = companySurface->w;
    m_companyNameRect.h = companySurface->h;
    SDL_FreeSurface(companySurface);

    // 创建产品信息纹理
    SDL_Surface* productSurface = TTF_RenderUTF8_Blended(m_chineseFont, productInfo, textColor);
    if (!productSurface)
    {
        return false;
    }
    m_productInfoTexture = SDL_CreateTextureFromSurface(m_renderer, productSurface);
    m_productInfoRect.w = productSurface->w;
    m_productInfoRect.h = productSurface->h;
    SDL_FreeSurface(productSurface);

    return (m_companyNameTexture != nullptr && m_productInfoTexture != nullptr);
}

bool Ui_LoadingSDL::LoadBackgroundTexture()
{
    // 获取可执行文件路径
    char executablePath[MAX_PATH];
    GetModuleFileNameA(NULL, executablePath, MAX_PATH);

    // 提取可执行文件所在目录
    std::string exePath(executablePath);
    size_t lastSlash = exePath.find_last_of("\\");
    std::string exeDir = exePath.substr(0, lastSlash + 1);

    // 构建背景图片的完整路径
    std::string backgroundPath = exeDir + "SDLAssets\\loadingbk.png";

    // 加载背景图片
    SDL_Surface* backgroundSurface = IMG_Load(backgroundPath.c_str());
    if (!backgroundSurface)
    {
        // 如果找不到图片，尝试使用相对路径
        backgroundPath = "SDLAssets\\loadingbk.png";
        backgroundSurface = IMG_Load(backgroundPath.c_str());

        if (!backgroundSurface)
        {
            return false; // 无法加载背景图片
        }
    }

    // 创建背景纹理
    m_backgroundTexture = SDL_CreateTextureFromSurface(m_renderer, backgroundSurface);
    m_backgroundRect.x = 0; 
    m_backgroundRect.y = 0;
    m_backgroundRect.w = m_windowWidth;
    m_backgroundRect.h = m_windowHeight;

    // 释放表面
    SDL_FreeSurface(backgroundSurface);

    return (m_backgroundTexture != nullptr);
}