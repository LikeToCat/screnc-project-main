#include "stdafx.h"
#include "CDebug.h"
#include "Ui_AreaRecordingSDL.h"
#include "GlobalFunc.h"
#include "LarStringConversion.h"
#include "theApp.h"
#include "SDLResource.h"
// 调试控制台句柄
extern HANDLE ConsoleHandle;
bool Ui_AreaRecordingSDL::s_bool_IsRunning = false;

Ui_AreaRecordingSDL* Ui_AreaRecordingSDL::ins = nullptr;
Ui_AreaRecordingSDL::Ui_AreaRecordingSDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr),
    m_TTF_Font(nullptr),
    m_Bool_SDLInitialized(false),
    m_Bool_TTFInitialized(false),
    m_Func_OnStart(nullptr),
    m_Func_OnStop(nullptr),
    m_Func_OnCancel(nullptr),
    m_SDLTexture_PanelCenterText(nullptr),
    m_COLORREF_TransparentColor(RGB(0, 0, 0)), // 设置透明色为粉色
    m_Bool_IsDragging(false),
    m_Str_PanelCenterText(L"录区域"),
    m_CenterTextWidth(-1),
    m_CenterTextHeight(-1),
    m_IsAnyBtnHover(false),
    m_TransparentMenu(nullptr),
    m_MenuBtnMenu(nullptr),
    m_Alpha(255),
    m_Btn_CurSelect(nullptr),
    m_RedBorderSize(1),
    m_IsTimeCountActive(false),
    m_SDLTexture_RecordingText(nullptr),
    m_Bool_IsWindowPause(false),
    m_Scale(1.0f),
    m_Int_PanelHeight(35),
    m_bool_Interaction(true)
{
    m_Btn_PauseSDL = nullptr; 
    m_Btn_ResumeSDL = nullptr;
    m_WindowDrager = new COMCLASS::SDL::WindowDrag;
    m_Rect_RecordArea.SetRectEmpty();
    m_Scale = getUserDpi();
    m_Int_PanelHeight = 35 * m_Scale;
    m_wstring_curCenterText = L"";
    m_CurrentRecordTimeSec = 0;
    // 构造函数初始化类成员
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: Ui_AreaRecordingSDL构造");
}

Ui_AreaRecordingSDL* Ui_AreaRecordingSDL::GetInstance()
{
    if (!ins)
    {
        ins = new Ui_AreaRecordingSDL;
    }
    return ins;
}

void Ui_AreaRecordingSDL::ReleaseInstance()
{
    if (ins)
    {
        DB(ConsoleHandle, L" if (ins) in");
        delete ins;
        ins = nullptr;
    }
}

Ui_AreaRecordingSDL::~Ui_AreaRecordingSDL()
{
    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: Ui_AreaRecordingSDL析构");
    m_Bool_Running = false;
    Close();
}

CRect Ui_AreaRecordingSDL::getRecordRect()
{
    CRect recordRect;
    CalucateCenterRecordArea(&recordRect);
    return recordRect;
}

void Ui_AreaRecordingSDL::SetUiModeDuringRecord()
{
    auto btn = m_BtnManager.FindBtnByUid(AREAREOCRDSDL_BTN_RECORDING);
    auto thisBtn = m_BtnManager.FindBtnByUid(AREAREOCRDSDL_BTN_STARTREOCRD);
    if (btn)
        btn->ShowBtn();
    if (thisBtn)
        thisBtn->HideBtn();
    if (!m_IsTimeCountActive)
        m_IsTimeCountActive = true;
    m_Time_Start = std::chrono::steady_clock::now();
    m_StrStream.str(L"");
    m_StrStream.clear();

    //开始录制时复位“暂停计时”相关状态
    m_IsPaused = false;                                  
    m_PausedAccum = std::chrono::seconds::zero();        
    m_SuppressUpdates = false;                           
    m_Bool_IsWindowPause = false;

    // 显示顶部“暂停”按钮，隐藏“恢复”按钮
    if (m_Btn_PauseSDL)  m_Btn_PauseSDL->ShowBtn();    
    if (m_Btn_ResumeSDL) m_Btn_ResumeSDL->HideBtn();   
}

void Ui_AreaRecordingSDL::ResumeUiModeFromRecord()
{
    m_IsTimeCountActive = false;
    auto btn = m_BtnManager.FindBtnByUid(AREAREOCRDSDL_BTN_STARTREOCRD);
    auto thisBtn = m_BtnManager.FindBtnByUid(AREAREOCRDSDL_BTN_RECORDING);
    if (btn)
        btn->ShowBtn();
    if (thisBtn)
        thisBtn->HideBtn();

    // 结束/退出录制时，隐藏暂停与恢复按钮
    if (m_Btn_PauseSDL)  m_Btn_PauseSDL->HideBtn();  
    if (m_Btn_ResumeSDL) m_Btn_ResumeSDL->HideBtn(); 
}

void Ui_AreaRecordingSDL::HideSDLWindow()
{
    SDL_HideWindow(m_SDL_Window);
}

void Ui_AreaRecordingSDL::ShowSDLWindow()
{
    SDL_ShowWindow(m_SDL_Window);
}

void Ui_AreaRecordingSDL::SetDashedBorder(bool enable)
{
    m_Bool_DashedBorder = enable;
}

void Ui_AreaRecordingSDL::UpdateCenterPanelText(std::wstring CenterText)
{
    m_Str_PanelCenterText = CenterText;
    if (m_SDLTexture_PanelCenterText)
        SDL_DestroyTexture(m_SDLTexture_PanelCenterText);
    
    //更新文本纹理
    std::string utf8Str = GlobalFunc::AnsiToUtf8(LARSC::ws2s(CenterText));
    SDL_Surface* TextSurface = TTF_RenderUTF8_Blended(m_TTF_Font, utf8Str.c_str(), SDL_Color{ 255,255,255,255 });
    if (TextSurface)
    {
        m_SDLTexture_PanelCenterText = SDL_CreateTextureFromSurface(m_SDL_Renderer, TextSurface);
        m_CenterTextWidth = TextSurface->w;
        m_CenterTextHeight = TextSurface->h;
        if(!m_SDLTexture_PanelCenterText)
            DB(ConsoleHandle, L"创建中心文本纹理失败");
        SDL_FreeSurface(TextSurface);
    }
    else
    {
        DB(ConsoleHandle, L"创建中心文本纹理失败");
    }
}

void Ui_AreaRecordingSDL::SetWindowOpciaity(int alpha)
{
    m_Alpha = alpha;
}

void Ui_AreaRecordingSDL::SetRedBorderSize(int Size)
{
    m_RedBorderSize = Size;
}

void Ui_AreaRecordingSDL::SetInteractionEnable(bool enable)
{
    m_bool_Interaction = enable;
}

void Ui_AreaRecordingSDL::OnPauseRecordingUi()
{
    // 冻结计时显示（面板中部显示回到普通文案）、关闭动态虚线、抑制区域更新
    m_IsTimeCountActive = false;       
    SetDashedBorder(false);            
    m_SuppressUpdates = true;          

    // 记录暂停开始时间，后续恢复时累加
    if (!m_IsPaused)                                                         
    {
        m_IsPaused = true;                                                    
        m_PauseStart = std::chrono::steady_clock::now();                      
    }
    m_Bool_IsWindowPause = true;

    // 顶部按钮状态：隐藏“暂停”，显示“恢复”
    if (m_Btn_PauseSDL)  m_Btn_PauseSDL->HideBtn();   
    if (m_Btn_ResumeSDL) m_Btn_ResumeSDL->ShowBtn();  
}

void Ui_AreaRecordingSDL::OnResumeRecordingUi()
{
    // 恢复计时显示、恢复动态虚线、恢复区域更新
    m_IsTimeCountActive = true;            
    SetDashedBorder(true);                 
    m_SuppressUpdates = false;             

    // 累加暂停区间，使计时连续
    if (m_IsPaused)                                   
    {
        auto now = std::chrono::steady_clock::now();  
        m_PausedAccum += (now - m_PauseStart);        
        m_IsPaused = false;                           
    }

    m_Bool_IsWindowPause = false;

    //更新一次录制区域
    CRect* RecordRect = new CRect;
    CalucateCenterRecordArea(RecordRect);
    ::PostMessage(App.m_Dlg_Main, MSG_UIAREARECORD_UPDATE, 0, (LPARAM)RecordRect);

    //顶部按钮状态：显示“暂停”，隐藏“恢复”
    if (m_Btn_PauseSDL)  m_Btn_PauseSDL->ShowBtn();  
    if (m_Btn_ResumeSDL) m_Btn_ResumeSDL->HideBtn(); 
}

bool Ui_AreaRecordingSDL::InitializeTTF()
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

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: TTF初始化失败: %s", wTtfError);
        return false;
    }

    m_Bool_TTFInitialized = true;

    // 加载字体
    if (!LoadFonts()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 字体加载失败");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: TTF初始化成功");
    return true;
}

bool Ui_AreaRecordingSDL::LoadFonts()
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
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16 * m_Scale);
    if (!m_TTF_Font) {
        // 尝试加载备用字体
        std::wstring backupFontPath = std::wstring(windowsDir) + L"\\Fonts\\simhei.ttf"; // 备用黑体字体

        utf8Size = WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0) {
            utf8Path.resize(utf8Size);
            WideCharToMultiByte(CP_UTF8, 0, backupFontPath.c_str(), -1, &utf8Path[0], utf8Size, NULL, NULL);
        }

        m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 21);

        if (!m_TTF_Font) {
            const char* ttfError = TTF_GetError();
            wchar_t wTtfError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wTtfError, _countof(wTtfError), ttfError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: 字体加载失败: %s", wTtfError);
            return false;
        }
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 字体加载成功");
    return true;
}

void Ui_AreaRecordingSDL::OnBnClicked_StartReocrd()
{
    CRect* recordRect = new CRect;
    CalucateCenterRecordArea(recordRect);
    DBFMT(ConsoleHandle, L"开始录制区域,recordRect:left%d,top%d,right%d,bottom%d",recordRect->left, recordRect->top,
        recordRect->right, recordRect->bottom);
    DBFMT(ConsoleHandle, L"开始录制区域,m_Rect_RecordArea:left%d,top%d,right%d,bottom%d", m_Rect_RecordArea.left, m_Rect_RecordArea.top,m_Rect_RecordArea.right, m_Rect_RecordArea.bottom);

    ::PostMessage(App.m_Dlg_Main, MSG_UIAREARECORD_STARTRECORD,
        NULL, (LPARAM)recordRect);
}

void Ui_AreaRecordingSDL::CalucateCenterRecordArea(CRect* recordRect)
{
    recordRect->CopyRect(m_Rect_RecordArea);
    recordRect->top += (m_Int_PanelHeight + (m_RedBorderSize * 2));
    recordRect->left += m_RedBorderSize * 2;
    recordRect->right -= m_RedBorderSize;
    recordRect->bottom -= m_RedBorderSize;
}

float Ui_AreaRecordingSDL::getUserDpi()
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    if (screen == NULL) {
        AfxMessageBox(L"无法获取屏幕 DC。");
        return 1.0f;
    }
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);

    // 计算缩放因子（基准 DPI 为 96）
    double scaleX = static_cast<double>(dpiX) / 96.0;
    double scaleY = static_cast<double>(dpiY) / 96.0;
    m_Scale = scaleY;
    return scaleY;
}

void Ui_AreaRecordingSDL::RenderDashedRect(SDL_Renderer* renderer, const SDL_Rect& rect)
{
    int total = m_Int_DashLength + m_Int_GapLength;
    int offset = m_Int_DashOffset % total;

    for (int x = rect.x + offset; x < rect.x + rect.w; x += total) {
        int xEnd = min(x + m_Int_DashLength, rect.x + rect.w);
        SDL_RenderDrawLine(renderer, x, rect.y,
            xEnd, rect.y);
    }

    int by = rect.y + rect.h;
    for (int x = rect.x + offset; x < rect.x + rect.w; x += total) {
        int xEnd = min(x + m_Int_DashLength, rect.x + rect.w);
        SDL_RenderDrawLine(renderer, x, by,
            xEnd, by);
    }

    for (int y = rect.y + offset; y < rect.y + rect.h; y += total) {
        int yEnd = min(y + m_Int_DashLength, rect.y + rect.h);
        SDL_RenderDrawLine(renderer, rect.x, y,
            rect.x, yEnd);
    }

    int rx = rect.x + rect.w;
    for (int y = rect.y + offset; y < rect.y + rect.h; y += total) {
        int yEnd = min(y + m_Int_DashLength, rect.y + rect.h);
        SDL_RenderDrawLine(renderer, rx, y,
            rx, yEnd);
    }
}

bool Ui_AreaRecordingSDL::IsMouseOnPanel(int x, int y)
{
    POINT p{ x,y };
    return m_ClientRect_PanelArea.PtInRect(p);
}

bool Ui_AreaRecordingSDL::Initialize(const CRect& recordArea)
{
    // 保存录制区域 470 230
    m_Rect_RecordArea = recordArea;
    int minWidth = 470 * m_Scale;
    int minHeight = 250 * m_Scale;
    if (m_Rect_RecordArea.Width() < minWidth)
    {
        m_Rect_RecordArea.right = m_Rect_RecordArea.left + minWidth;
    }
    if (m_Rect_RecordArea.Height() < minHeight)
    {
        m_Rect_RecordArea.bottom = m_Rect_RecordArea.top + minHeight;
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: 初始化录制区域 左=%d 上=%d 宽=%d 高=%d",
        recordArea.left, recordArea.top, recordArea.Width(), recordArea.Height());

    // 验证录制区域尺寸是否有效
    if (recordArea.Width() <= 0 || recordArea.Height() <= 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 错误 - 录制区域尺寸无效");
        return false;
    }

    // 分步初始化，先初始化SDL
    if (!InitializeSDL())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL初始化失败");
        return false;
    }

    // 初始化TTF
    if (!InitializeTTF()) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: TTF初始化失败");
        return false;
    }

    // 然后创建SDL窗口
    if (!CreateSDLWindow()) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口创建失败");
        return false;
    }

    //初始化要使用的指针图标
    m_SDL_DefaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_SDL_HandCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    m_ClientRect_PanelArea.left = 0;
    m_ClientRect_PanelArea.top = 0;
    m_ClientRect_PanelArea.right = m_ClientRect_PanelArea.left + m_Rect_RecordArea.Width();
    m_ClientRect_PanelArea.bottom = m_ClientRect_PanelArea.top + m_Int_PanelHeight;

    // 创建面板区域界面按钮
    CreateButtons();
    // 创建菜单
    CreateMenu();
    // 创建面版区域中间文本纹理
    UpdateCenterPanelText(m_Str_PanelCenterText);

    m_Bool_Running = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口初始化成功");
    return true;
}

bool Ui_AreaRecordingSDL::InitializeSDL()
{
    // 检查SDL是否已经初始化
    if (SDL_WasInit(SDL_INIT_VIDEO)) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL视频子系统已初始化");
        m_Bool_SDLInitialized = true;
        return true;
    }

    // 只初始化视频子系统，避免不必要的初始化
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) 
    {
        // 获取SDL错误消息
        const char* sdlError = SDL_GetError();

        // 转换为宽字符以便输出
        wchar_t wSdlError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: SDL初始化失败: %s", wSdlError);
        return false;
    }

    // 设置SDL提示，提高在Windows上的兼容性
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1");
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "0");
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // 抗锯齿

    m_Bool_SDLInitialized = true;
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL初始化成功");

    // 创建鼠标指针
    m_SDL_DefaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_SDL_HandCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    if (!m_SDL_DefaultCursor || !m_SDL_HandCursor)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 警告 - 鼠标指针创建失败");
    }
    else
    {
        // 默认设置为箭头指针
        SDL_SetCursor(m_SDL_DefaultCursor);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 鼠标指针创建成功");
    }

    //获取用户DPI
    m_Scale = getUserDpi();
    return true;
}

bool Ui_AreaRecordingSDL::CreateSDLWindow()
{
    // 确保SDL已初始化
    if (!m_Bool_SDLInitialized) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 无法创建窗口 - SDL未初始化");
        return false;
    }

    // 清理之前可能存在的窗口和渲染器
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

    // 计算初始窗口位置和大小
    int left = max(0, m_Rect_RecordArea.left);
    int top = max(0, m_Rect_RecordArea.top);
    int width = max(100, min(GetSystemMetrics(SM_CXSCREEN) - left, m_Rect_RecordArea.Width()));
    int height = max(100, min(GetSystemMetrics(SM_CYSCREEN) - top, m_Rect_RecordArea.Height()));
    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 创建SDL透明窗口");

    // 创建窗口（使用计算的位置和大小）
    m_SDL_Window = SDL_CreateWindow(
        "Recording Area",  // 窗口标题
        left, top,         // 位置
        width, height,     // 大小
        windowFlags        // 标志
    );

    if (!m_SDL_Window) 
    {
        // 获取SDL错误消息
        const char* sdlError = SDL_GetError();
        wchar_t wSdlError[256] = { 0 };
        size_t convertedChars = 0;
        mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: 窗口创建失败: %s", wSdlError);
        return false;
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: 窗口创建成功，位置=%d,%d 大小=%d,%d",
        left, top, width, height);

    m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_SOFTWARE);

    if (!m_SDL_Renderer)
    {
        // 尝试创建硬件加速渲染器作为后备方案
        m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_ACCELERATED);

        if (!m_SDL_Renderer) 
        {
            const char* sdlError = SDL_GetError();
            wchar_t wSdlError[256] = { 0 };
            size_t convertedChars = 0;
            mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: 渲染器创建失败: %s", wSdlError);
            SDL_DestroyWindow(m_SDL_Window);
            m_SDL_Window = nullptr;
            return false;
        }
    }

    // 设置渲染器支持混合模式，用于半透明效果
    if (SDL_SetRenderDrawBlendMode(m_SDL_Renderer, SDL_BLENDMODE_BLEND) != 0) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 警告 - 无法设置混合模式");
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口和渲染器创建成功");

    if (!COMAPI::SDL::InitWindowOpacity(
        m_SDL_Renderer,m_SDL_Window,
        &m_SDLTexture_Transparent,width, height,
        &m_PixelBuffer,
        &m_Hwnd
    ))
    {
        DB(ConsoleHandle, L"窗口透明组件初始化失败!");
    }
    return true;
}

void Ui_AreaRecordingSDL::Close()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 正在清理SDL资源");

    // 清理字体资源
    if (m_TTF_Font) {
        DB(ConsoleHandle, L" m_TTF_Font delete");
        TTF_CloseFont(m_TTF_Font);
        m_TTF_Font = nullptr;
    }


    // 清理渲染器
    if (m_SDL_Renderer) {
        DB(ConsoleHandle, L" m_SDL_Renderer delete");
        SDL_DestroyRenderer(m_SDL_Renderer);
        m_SDL_Renderer = nullptr;
    }

    // 关闭TTF
    if (m_Bool_TTFInitialized) {
        DB(ConsoleHandle, L" TTF_Quit");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
    }

    // 清理窗口
    if (m_SDL_Window) {
        DB(ConsoleHandle, L" m_SDL_Window delete ");
        SDL_DestroyWindow(m_SDL_Window);
        m_SDL_Window = nullptr;
    }

    // 完全清理SDL资源以避免资源泄漏和重复初始化问题
    m_Bool_SDLInitialized = false;
    

    // 关闭窗口拖拽管理器
    if (m_WindowDrager)
    {
        DB(ConsoleHandle, L" delete m_WindowDrager; ");
        delete m_WindowDrager;
    }

    // 释放鼠标指针资源
    if (m_SDL_HandCursor)
    {
        DB(ConsoleHandle, L" SDL_FreeCursor(m_SDL_HandCursor); ");
        SDL_FreeCursor(m_SDL_HandCursor);
        m_SDL_HandCursor = nullptr;
    }
    if (m_SDL_DefaultCursor)
    {
        DB(ConsoleHandle, L" SDL_FreeCursor(m_SDL_DefaultCursor); ");
        SDL_FreeCursor(m_SDL_DefaultCursor);
        m_SDL_DefaultCursor = nullptr;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL资源清理完成");
}

void Ui_AreaRecordingSDL::RenderToLayerWindow()
{
    // 渲染到 m_SDLTexture_Transparent
    SDL_SetRenderTarget(m_SDL_Renderer, m_SDLTexture_Transparent);
    SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_SDL_Renderer);
    RenderToTexture();
    COMAPI::SDL::UpdateArgbFrameToWindow(
        m_SDL_Window, m_SDL_Renderer, m_SDLTexture_Transparent,
        &m_PixelBuffer,
        &m_Hwnd,
        m_Alpha
    );
}

void Ui_AreaRecordingSDL::CreateButtons()
{
    // 清空按钮列表，防止重复创建
    m_BtnManager.GetBtns().clear();
    int buttonSpacing = 10 * m_Scale;
    int SeparteLineSize = 1;
    SDL_Color SeparteLineColor{ 73,73,73,255 };

    // 安全获取面板宽度
    int panelWidth = 0;
    int windowWidth = 0;
    if (m_SDL_Window)
    {
        SDL_GetWindowSize(m_SDL_Window, &windowWidth, nullptr);
        panelWidth = windowWidth;
    }
    else
    {
        panelWidth = m_Rect_RecordArea.Width();
    }

    //按钮属性
    SDL_Color textColor{ 255,255,255,255 };
    SDL_Color textHoverColor{ 255,255,255,255 };
    SDL_Color textClickColor{ 255,255,255,235 };
    SDL_Color BorderColor{ 37,39,46,255 };
    SDL_Color BkColor{ 37,39,46,255 };
    SDL_Color BkHoverColor{ 255,255,255,25 };
    SDL_Color BkClickColor{ 255,255,255,30 };

    //关闭按钮
    int CloseImageWidth = 22 * m_Scale;
    int CloseImageHeight = 22 * m_Scale;
    SDL_Rect Btn_CloseRect
    {
        windowWidth - CloseImageWidth - 10,
        (m_Int_PanelHeight - CloseImageHeight) / 2,
        CloseImageWidth,
        CloseImageHeight
    };
    Ui_SDLButton* Btn_Close = new Ui_SDLButton(
        Btn_CloseRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\Close.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"关闭按钮点击");
            m_Bool_Running = false;
        },
        AREAREOCRDSDL_BTN_CLOSE
    );
    int LineStartX = Btn_CloseRect.x - 10;
    int LineStartY = Btn_CloseRect.y;
    int LineEndX = Btn_CloseRect.x - 10;
    int LineEndY = Btn_CloseRect.y + Btn_CloseRect.h;
    Btn_Close->SetImageStretchParam(0.65f);
    Btn_Close->SetExtraRender([=]()
        {//绘画分界线
            SDL_Color OriColor;
            SDL_GetRenderDrawColor(m_SDL_Renderer, &OriColor.r, &OriColor.g, &OriColor.b, &OriColor.a);
            for (size_t i = 0; i < SeparteLineSize; i++)
            {//绘画指定厚度的线
                SDL_SetRenderDrawColor(m_SDL_Renderer,
                    SeparteLineColor.r, SeparteLineColor.g, SeparteLineColor.b, SeparteLineColor.a);
                SDL_RenderDrawLine(m_SDL_Renderer, LineStartX - i, LineStartY, LineEndX - i, LineEndY);
            }
            SDL_SetRenderDrawColor(m_SDL_Renderer, OriColor.r, OriColor.g, OriColor.b, OriColor.a);//颜色设置回去
        });
    Btn_Close->AdjustClientAreaByDiff(-3 * m_Scale, -3 * m_Scale, 6 * m_Scale, 6 * m_Scale);
    Btn_Close->SetBtnDesc(L"关闭");
    m_BtnManager.AddBtn(Btn_Close);

    //菜单按钮
    int MenuWidth = 22 * m_Scale;
    int MenuHeight = 22 * m_Scale;
    int MenuX = Btn_Close->GetBtnRect().x - MenuWidth - 15 * m_Scale;
    int MenuY = (m_Int_PanelHeight - MenuHeight) / 2;
    SDL_Rect MenuRect
    {
        MenuX,MenuY,
        MenuWidth,MenuHeight
    };
    Ui_SDLButton* MenuBtn = new Ui_SDLButton(
        MenuRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\meau.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"菜单点击");
            m_MenuBtnMenu->ShowMenu();
        },
        AREAREOCRDSDL_BTN_MENU
    );
    auto menuImageRect = Ui_SDLButton::ScaleRect(MenuBtn->GetImageRect(), 0.63f * m_Scale);
    MenuBtn->AdjustImageRect(menuImageRect);
    MenuBtn->SetBtnDesc(L"菜单");
    if (windowWidth < 235 * m_Scale)
        MenuBtn->HideBtn();
    m_BtnManager.AddBtn(MenuBtn);

    // 开始录制按钮 
    int buttonWidth;
    buttonWidth = 70 * m_Scale;
    int buttonHeight = 15 * m_Scale;
    int StartRecordX;
    StartRecordX = MenuRect.x - buttonWidth - 1 * m_Scale;
    SDL_Rect Btn_StartRecordRect{ StartRecordX , (m_Int_PanelHeight - buttonHeight) / 2,
                buttonWidth, buttonHeight };
    Ui_SDLButton* Btn_StartRecord = new Ui_SDLButton(
        Btn_StartRecordRect,
        windowWidth < 235 * m_Scale ? L"" : L"开始录制",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\record.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"开始录制点击");
            OnBnClicked_StartReocrd();
        },
        AREAREOCRDSDL_BTN_STARTREOCRD
    );
    SDL_Rect RecordImageRect = Btn_StartRecord->GetImageRect();
    RecordImageRect = Ui_SDLButton::ScaleRect(RecordImageRect, 0.46f * m_Scale);
    RecordImageRect.x = Btn_StartRecord->GetBtnRect().x - RecordImageRect.w - 5 * m_Scale;
    Btn_StartRecord->AdjustImageRect(RecordImageRect);

    SDL_Rect TextRect = Btn_StartRecord->GetTextRect();
    TextRect = Btn_StartRecord->CalculateCenterRect(TextRect);
    TextRect.x = Btn_StartRecord->GetBtnRect().x + 25 * m_Scale;
    TextRect.y -= 1 * m_Scale;
    Btn_StartRecord->AdjustTextRect(TextRect);
    Btn_StartRecord->AdjustClientAreaByDiff(0, -1.5 * m_Scale, 0, 6.5 * m_Scale);
    Btn_StartRecord->SetBtnDesc(L"开始录制");
    m_BtnManager.AddBtn(Btn_StartRecord);

    //不透明度图片按钮
    int TransparentBtnWidth = 31 * m_Scale;
    int TransparentBtnHeight = 32 * m_Scale;
    int TransparentBtnX = 10 * m_Scale;
    int TransparentBtnY = (m_Int_PanelHeight - TransparentBtnHeight) / 2;
    SDL_Rect TransparentBtnRect
    {
        TransparentBtnX,
        TransparentBtnY,
        TransparentBtnWidth,
        TransparentBtnHeight
    };
    Ui_SDLButton* TransparentBtn = new Ui_SDLButton(
        TransparentBtnRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\transparent.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"不透明度按钮点击");
            if (m_TransparentMenu)
            {
                int x, y;
                x = 0;
                y = TransparentBtnRect.y + TransparentBtnRect.h;
                m_TransparentMenu->MoveMenuToXY(x, y);
                m_TransparentMenu->ShowMenu();
            }
        },
        AREAREOCRDSDL_BTN_TRANSPARENT
    );
    TransparentBtn->SetBtnDesc(L"不透明度调整");
    TransparentBtn->SetImageStretchParam(0.76);
    m_BtnManager.AddBtn(TransparentBtn);

    //别针分辨率按钮115 28
    std::ostringstream stream;
    stream << m_Rect_RecordArea.Width() << "      " << m_Rect_RecordArea.Height();
    int NailWidth = 100 * m_Scale;
    int NailHeight = 20 * m_Scale;
    int NailX = TransparentBtnRect.x + TransparentBtnRect.w + 10 * m_Scale;
    int NailY = (m_Int_PanelHeight - NailHeight) / 2;
    SDL_Rect NailRect
    {
        NailX,NailY,
        NailWidth,NailHeight
    };
    int NailImageWidth = 18 * m_Scale;
    int NailImageHeight = 30 * m_Scale;
    int NailImageX = NailX + (NailWidth - NailImageWidth) / 2;
    int NailImageY = NailY + (NailHeight - NailImageHeight) / 2;
    SDL_Rect NailImageRect
    {
        NailImageX,NailImageY,
        NailImageWidth,NailImageHeight,
    };
    Ui_SDLButton* NailBtn = new Ui_SDLButton(
        NailRect,
        LARSC::s2ws(stream.str()),
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\nail.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"别针点击");
        },
        -1
    );
    NailBtn->AdjustImageRect(NailImageRect, false);
    NailBtn->SetNoInterActive(true);
    m_BtnManager.AddBtn(NailBtn);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL线程]: 创建了%d个按钮", (int)m_BtnManager.GetBtns().size());

    //不可见图片按钮
    int InvisiableBtnWidth = 30 * m_Scale;
    int InvisiableBtnHeight = 30 * m_Scale;
    int InvisiableBtnX = NailImageX + NailBtn->GetBtnRect().w - 35 * m_Scale;
    int InVisiableBtnY = (m_Int_PanelHeight - InvisiableBtnHeight) / 2;
    SDL_Rect InvisiableBtnRect
    {
        InvisiableBtnX,InVisiableBtnY,
        InvisiableBtnWidth,InvisiableBtnHeight
    };
    Ui_SDLButton* InvisiableBtn = new Ui_SDLButton(
        InvisiableBtnRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\Invisiable.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"不可见按钮点击");
            SetWindowOpciaity(0);
        },
        AREAREOCRDSDL_BTN_INVISIABLE
    );
    InvisiableBtn->SetBtnDesc(L"窗口不可见");
    InvisiableBtn->SetImageStretchParam(0.84f);
    m_BtnManager.AddBtn(InvisiableBtn);

    //录制中按钮
    int RecordingWidth = 28 * m_Scale;
    int RecordingHeight = 28 * m_Scale;
    int RecordingX = Btn_StartRecord->GetImageRect().x + Btn_StartRecord->GetBtnRect().w - RecordingWidth;
    int RecordingY = (m_Int_PanelHeight - RecordingHeight) / 2;
    SDL_Rect RecordingBtnRect
    {
        RecordingX,RecordingY,
        RecordingWidth,RecordingHeight
    };
    Ui_SDLButton* RecordingBtn = new Ui_SDLButton(
        RecordingBtnRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\Reocrding24.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"录制中按钮点击");
            m_Bool_Running = false;
        },
        AREAREOCRDSDL_BTN_RECORDING
    );
    RecordingBtn->HideBtn();
    SDL_Rect RecordingImageRect = Ui_SDLButton::ScaleRect(RecordingBtn->GetImageRect(), 0.58f * m_Scale);
    RecordingBtn->AdjustImageRect(RecordingImageRect);
    RecordingBtn->SetBtnDesc(L"停止录制");
    m_BtnManager.AddBtn(RecordingBtn);

    // 顶部“暂停/恢复”按钮（同位置、同大小）
    int TopCtrlWidth = 24 * m_Scale;                             
    int TopCtrlHeight = 24 * m_Scale;                            
    int spacing = 10 * m_Scale;                                  
    int TopCtrlX = RecordingBtnRect.x - TopCtrlWidth - spacing + 5 * m_Scale;  
    int TopCtrlY = RecordingY + 2 * m_Scale;                                  
    SDL_Rect PauseBtnRect{ TopCtrlX, TopCtrlY, TopCtrlWidth, TopCtrlHeight };     
    SDL_Rect ResumeBtnRect{ TopCtrlX, TopCtrlY, TopCtrlWidth, TopCtrlHeight };    

    //暂停按钮
    m_Btn_PauseSDL = new Ui_SDLButton(
        PauseBtnRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\topPause.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"顶部“暂停”按钮点击");  
            ::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_UIAREARECORD_PAUSERECORD, NULL, NULL);
        },
        -1 // 不强依赖Uid查找
    );
    m_Btn_PauseSDL->AdjustImageRect(Ui_SDLButton::ScaleRect(m_Btn_PauseSDL->GetImageRect(), 0.39f * m_Scale)); 
    m_Btn_PauseSDL->SetBtnDesc(L"暂停录制"); 
    m_Btn_PauseSDL->HideBtn();               
    m_BtnManager.AddBtn(m_Btn_PauseSDL);     

    //恢复录制按钮
    m_Btn_ResumeSDL = new Ui_SDLButton(
        ResumeBtnRect,
        L"",
        textColor,
        textHoverColor,
        textClickColor,
        BorderColor,
        BkColor,
        BkHoverColor,
        BkClickColor,
        L".\\SDLAssets\\topResumeRecord.png",
        m_SDL_Renderer,
        m_TTF_Font,
        [=]()
        {
            DB(ConsoleHandle, L"顶部“恢复录制”按钮点击");
            ::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_UIAREARECORD_RESUMERECORD, NULL, NULL);
        },
        -1 
    );
    m_Btn_ResumeSDL->AdjustImageRect(Ui_SDLButton::ScaleRect(m_Btn_ResumeSDL->GetImageRect(), 0.39f * m_Scale)); 
    m_Btn_ResumeSDL->SetBtnDesc(L"恢复录制"); 
    m_Btn_ResumeSDL->HideBtn();               
    m_BtnManager.AddBtn(m_Btn_ResumeSDL);     
}

void Ui_AreaRecordingSDL::CreateMenu()
{
    //按钮互动 
    SDL_Color textColor{ 255,255,255,195 };
    SDL_Color textHoverColor{ 255,255,255,255 };
    SDL_Color textClickColor{ 255,255,255,235 };
    SDL_Color BorderColor{ 37,39,46,255 };
    SDL_Color BkColor{ 37,39,46,255 };
    SDL_Color BkHoverColor{ 255,255,255,25 };
    SDL_Color BkClickColor{ 255,255,255,30 };

    //菜单属性
    SDL_Color MenubkColor{ 36,37,40 ,255 };
    SDL_Color MenuBorderColor{ 73,73,73 ,255 };

    //创建不透明度选择菜单
    Ui_SDLButton* InvisiableBtn = nullptr;
    if (m_Rect_RecordArea.Width() < 560)
        InvisiableBtn = m_BtnManager.GetBtns().at(1);
    else
        InvisiableBtn = m_BtnManager.GetBtns().at(3);

    SDL_Rect transparentBtnRect = InvisiableBtn->GetBtnRect();
    int TransparentMenuItemHeight = 35 * m_Scale;
    int TransparentMenuWidth = 96 * m_Scale;
    int TransparentMenuHeight = TransparentMenuItemHeight * 5;//5个按钮

    int TransparentMenuX;
    int TransparentMenuY;
    if (m_Rect_RecordArea.Width() < 560)
    {
        TransparentMenuX = transparentBtnRect.x -
            (TransparentMenuWidth - transparentBtnRect.w) / 2 - TransparentMenuWidth - 2;
        TransparentMenuY = transparentBtnRect.y + transparentBtnRect.h + TransparentMenuItemHeight;
    }
    else
    {
        TransparentMenuX = transparentBtnRect.x;
        TransparentMenuY = transparentBtnRect.y + transparentBtnRect.h;
    }
    SDL_Rect TransparentMenuRect
    {
        TransparentMenuX,TransparentMenuY,
        TransparentMenuWidth,TransparentMenuHeight
    };
    m_Rect_TransparentRect = TransparentMenuRect;
    m_TransparentMenu = new Ui_SDLMeau(
        TransparentMenuRect,
        TransparentMenuItemHeight,
        MenubkColor,
        MenuBorderColor,
        m_SDL_Renderer,
        m_TTF_Font
    );

    //创建不透明菜单按钮
    std::ostringstream ostream1;
    int Percent = 100;
    SDL_Rect TransparentMenu
    {
        TransparentMenuX,
        TransparentMenuY,
        TransparentMenuWidth,
        TransparentMenuItemHeight,
    };
    for (size_t i = 0; i < 5; i++)
    {
        ostream1.str("");
        ostream1.clear();
        ostream1 << Percent - i * 20 << "%";
        Ui_MenuButton* btn = m_TransparentMenu->AddMenuBtn(
            TransparentMenu,
            LARSC::s2ws(ostream1.str()),
            textColor,
            textHoverColor,
            textClickColor,
            BorderColor,
            BkColor,
            BkHoverColor,
            BkClickColor,
            i,
            [=](void* param)
            {
                DB(ConsoleHandle, L"点击了不透明度百分比按钮");
                int* alpha = (int*)(param);
                SetWindowOpciaity(*alpha);
            }
        );
        if (btn)
        {
            DBFMT(ConsoleHandle, L"成功创建不透明菜单按钮，percent:%d", Percent - i * 20);
        }

        SDL_Rect SelctImgRect = TransparentMenu;
        SelctImgRect.w = 21 * m_Scale;
        SelctImgRect.h = 21 * m_Scale;
        SelctImgRect.x = TransparentMenu.x + TransparentMenu.w - SelctImgRect.w - 5;
        SelctImgRect.y = TransparentMenu.y + (TransparentMenu.h - SelctImgRect.h) / 2;
        btn->AddExtraRenderImage(L".\\SDLAssets\\gou1.png", SelctImgRect);
        if (i == 0)
            btn->SetExtraImageShowState(true);
        TransparentMenu.y += TransparentMenuItemHeight;
    }
    m_TransparentMenu->HideMenu();

    //创建菜单按钮菜单
    auto MenuBtn = m_BtnManager.GetBtns().at(1);
    SDL_Rect MenuBtnRect = MenuBtn->GetBtnRect();
    int MenuBtnMenuItemHeight = 35 * m_Scale;
    int MenuBtnMenuWidth = 96 * m_Scale;
    int MenuBtnMenuHeight = MenuBtnMenuItemHeight * 2;//2个按钮
    int MenuBtnMenuX = MenuBtnRect.x + (MenuBtnRect.w - MenuBtnMenuWidth)/2;
    int MenuBtnMenuY = MenuBtnRect.y + MenuBtnRect.h;
    SDL_Rect MenuBtnMenuRect
    {
        MenuBtnMenuX,MenuBtnMenuY,
        MenuBtnMenuWidth,MenuBtnMenuHeight
    };
    m_Rect_MenuBtnMenuRect = MenuBtnMenuRect;
    m_MenuBtnMenu = new Ui_SDLMeau(
        MenuBtnMenuRect,
        MenuBtnMenuItemHeight,
        MenubkColor,
        MenuBorderColor,
        m_SDL_Renderer,
        m_TTF_Font
    );

    //创建菜单按钮菜单的按钮
    SDL_Rect MenuBtnMenuBtnRect
    {
        MenuBtnMenuX,
        MenuBtnMenuY,
        MenuBtnMenuWidth,
        MenuBtnMenuItemHeight,
    };
    std::vector<std::wstring> texts
    {
        L"窗口不可见",
        L"透明度"
    };
    for (size_t i = 0; i < 2; i++)
    {
        Ui_MenuButton* btn = m_MenuBtnMenu->AddMenuBtn(
            MenuBtnMenuBtnRect,
            texts.at(i),
            textColor,
            textHoverColor,
            textClickColor,
            BorderColor,
            BkColor,
            BkHoverColor,
            BkClickColor,
            i,
            [=](void* param)
            {
                int index = *(int*)param;
                if (index == 0)
                    SetWindowOpciaity(0);
            }
        );
        SDL_Rect TriangleRect = MenuBtnMenuBtnRect;
        TriangleRect.w = 32 * m_Scale;
        TriangleRect.h = 32 * m_Scale;
        TriangleRect.x = MenuBtnMenuBtnRect.x;
        TriangleRect.y = MenuBtnMenuBtnRect.y + (MenuBtnMenuBtnRect.h - TriangleRect.h) / 2 + 2;
        if (i == 1)
        {
            btn->AddExtraRenderImage(L".\\SDLAssets\\ic_retangle_left.png", TriangleRect);
            btn->SetExtraImageShowState(true);
        }
        MenuBtnMenuBtnRect.y += TransparentMenuItemHeight;
    }
    m_MenuBtnMenu->HideMenu();
}

void Ui_AreaRecordingSDL::Run()
{
    // 检查窗口和渲染器是否正确初始化
    if (!m_SDL_Window || !m_SDL_Renderer) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 无法运行 - 窗口或渲染器未初始化");
        return;
    }
    // 运行SDL窗口的主循环，直到窗口关闭
    s_bool_IsRunning = true;
    m_Rect_RecordArea.bottom -= m_Int_PanelHeight;//调整录制区域为正确的录制区域
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口主循环开始运行");
  
    while (m_Bool_Running)
    {
        ProcessEvents();
        RenderToLayerWindow();
        SDL_Delay(16); // ~60 FPS
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口主循环结束");
    s_bool_IsRunning = false;
    ::PostMessage(App.m_Dlg_Main, MSG_UIAREARECORD_CLOSEWINDOW, NULL, NULL);
}

void Ui_AreaRecordingSDL::ProcessEvents()
{
    // 检查窗口是否正确初始化
    if (!m_SDL_Window) {
        m_Bool_Running = false;
        return;
    }

    // 处理SDL事件队列中的所有事件
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        switch (event.type) 
        {
        case SDL_QUIT:
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 收到窗口关闭事件");
            m_Bool_Running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) 
            {
                if (m_bool_Interaction)
                {
                    bool IsAnyBtnClick = false;
                    SDL_RaiseWindow(m_SDL_Window);//设置窗口焦点
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 鼠标左键点窗口");
                    //首先判断是否点击按钮，如果点击则执行按钮回调
                    m_BtnManager.UpdateBtnsState(event.button.x, event.button.y, true);
                    Ui_SDLButton* btn = m_BtnManager.GetInHoverOrClickedBtn();
                    if (btn)
                    {
                        btn->Click();
                        IsAnyBtnClick = true;
                    }

                    //判断是否点击不透明度菜单按钮，点击则执行回调
                    if (m_TransparentMenu)
                        m_TransparentMenu->UpdateMenuBtnState(event.motion.x, event.motion.y, true);
                    if (m_MenuBtnMenu)
                        m_MenuBtnMenu->UpdateMenuBtnState(event.motion.x, event.motion.y, true);
                    if (m_TransparentMenu)
                    {
                        Ui_MenuButton* TransparentMenuBtn = m_TransparentMenu->GetHoverBtn();
                        if (TransparentMenuBtn)
                        {
                            IsAnyBtnClick = true;
                            int percent;
                            std::wstring btnText = TransparentMenuBtn->GetBtnText();
                            std::wstringstream wstream(btnText);
                            wstream >> percent;
                            float percentF = static_cast<float>(percent) / 100.0f;
                            int alpha = 255 * percentF;
                            m_Btn_CurSelect = TransparentMenuBtn;
                            m_TransparentMenu->ResetAllMenuBtnState();
                            TransparentMenuBtn->SetExtraImageShowState(true);
                            TransparentMenuBtn->Click(&alpha);
                        }
                    }
                    if (m_MenuBtnMenu)
                    {
                        Ui_MenuButton* MenuBtnMenu = m_MenuBtnMenu->GetHoverBtn();
                        if (MenuBtnMenu)
                        {
                            IsAnyBtnClick = true;
                            int index = MenuBtnMenu->GetBtnIndex();
                            MenuBtnMenu->Click(&index);
                        }
                    }
                    if (!IsAnyBtnClick)
                    {
                        m_WindowDrager->MoveActive(m_SDL_Window);
                        if (!m_TransparentMenu->IsMenuHiding())
                            m_TransparentMenu->HideMenu();
                        if (!m_MenuBtnMenu->IsMenuHiding())
                            m_MenuBtnMenu->HideMenu();
                    }
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 开始拖动窗口");
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) 
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 鼠标左键松窗口");
                m_WindowDrager->MoveInActive();
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 停止拖动窗口");
            }
            break;

        case SDL_MOUSEMOTION:
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 鼠标移动");
            SDL_RaiseWindow(m_SDL_Window);
            m_IsAnyBtnHover = m_BtnManager.CheckPosInAnyBtn(event.motion.x, event.motion.y);
            m_BtnManager.UpdateBtnsState(event.motion.x, event.motion.y, false);
            bool transparetMenuHover = false;
            bool MenuBtnMenuHover = false;
            if (m_IsAnyBtnHover)//如果悬停到某一个按钮，则设置中间文本为按钮提示内容
            {
                auto btn = m_BtnManager.GetInHoverOrClickedBtn();
                auto btntext = btn->GetBtnDesc();
                if (btntext != L"")
                {
                    COMAPI::SDL::TextLoad(
                        m_SDL_Renderer, m_TTF_Font,
                        btntext, SDL_Color{ 255,255,255,255 },
                        &m_SDLTexture_PanelCenterText, &m_CenterTextWidth, &m_CenterTextHeight
                    );
                    m_Str_PanelCenterText = btntext;
                }
            }
            else
            {
                if (m_Str_PanelCenterText != L"录区域")
                {
                    COMAPI::SDL::TextLoad(
                        m_SDL_Renderer, m_TTF_Font,
                        L"录区域", SDL_Color{ 255,255,255,255 },
                        &m_SDLTexture_PanelCenterText, &m_CenterTextWidth, &m_CenterTextHeight
                    );
                    m_Str_PanelCenterText = L"录区域";
                }
            }

            if (m_TransparentMenu)
            {
                m_TransparentMenu->UpdateMenuBtnState(event.motion.x, event.motion.y, false);
                transparetMenuHover = (m_TransparentMenu->GetHoverBtn() != nullptr) ? true : false;
            }
            if (m_MenuBtnMenu)
            {
                m_MenuBtnMenu->UpdateMenuBtnState(event.motion.x, event.motion.y, false);
                MenuBtnMenuHover = (m_MenuBtnMenu->GetHoverBtn() != nullptr) ? true : false;
            }
            m_IsAnyMenuBtnHover =
                transparetMenuHover || MenuBtnMenuHover ?
                true : false;

            if (!m_IsAnyMenuBtnHover)
            {//如果鼠标进入过菜单，并且当前鼠标不在菜单中，则隐藏菜单
                if (m_MenuBtnMenu && m_MenuBtnMenu->IsMouseEnteredMenu())
                {
                    m_MenuBtnMenu->HideMenu();
                    m_TransparentMenu->HideMenu();
                }
                if (m_TransparentMenu && m_TransparentMenu->IsMouseEnteredMenu())
                {
                    DB(ConsoleHandle, L"if (m_TransparentMenu && m_TransparentMenu->IsMouseEnteredMenu())");
                    m_TransparentMenu->HideMenu();
                }
            }
            else
            {
                if (m_MenuBtnMenu)
                {
                    int index = m_MenuBtnMenu->GetLastHoverIndex();
                    auto btn = m_MenuBtnMenu->GetHoverBtn();
                    if (btn && (btn->GetBtnIndex() == 1) && m_TransparentMenu)
                    {//如果菜单按钮菜单有悬停按钮，并且悬停的按钮是"透明度"按钮,则显示不透明子菜单
                        int x = btn->GetTextRect().x - m_Rect_MenuBtnMenuRect.w - btn->GetTextRect().w / 2 - 1 * m_Scale;
                        int y = btn->GetTextRect().y;
                        m_TransparentMenu->MoveMenuToXY(x, y);
                        m_TransparentMenu->ShowMenu();
                    }
                    else if (btn && index != -1 && index != 1)
                    {
                        DB(ConsoleHandle, L"else if (index != -1 && index != 1)");
                        m_TransparentMenu->HideMenu();
                    }
                }
            }
            if (m_WindowDrager->MoveTo(m_SDL_Window, &m_Rect_RecordArea))
            { //发送消息通知主线程更新录制接口的参数
                if (!m_SuppressUpdates)                      
                {
                    CRect* RecordRect = new CRect;
                    CalucateCenterRecordArea(RecordRect);
                    ::PostMessage(App.m_Dlg_Main, MSG_UIAREARECORD_UPDATE, 0, (LPARAM)RecordRect);
                }
            }
        }
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) 
            {//恢复为不透明，设置到前台
            case SDL_WINDOWEVENT_SHOWN:
            case SDL_WINDOWEVENT_RESTORED:
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                SetWindowOpciaity(255);
                break;
            }
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 按下ESC键，关闭窗口");
                m_Bool_Running = false;
            }
            break;
        }
    } 

    if (m_IsAnyBtnHover || m_IsAnyMenuBtnHover)
    {
        SDL_SetCursor(m_SDL_HandCursor);
    }
    else
    {
        SDL_SetCursor(m_SDL_DefaultCursor);
    }
}

void Ui_AreaRecordingSDL::RenderToTexture()
{
    // 检查渲染器是否正确初始化
    if (!m_SDL_Renderer) 
    {
        m_Bool_Running = false;
        return;
    }
    if (m_Bool_DashedBorder) 
    {
        int total = m_Int_DashLength + m_Int_GapLength;
        m_Int_DashOffset = (m_Int_DashOffset + 1) % total;
    }

    // 获取当前窗口大小
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(m_SDL_Window, &windowWidth, &windowHeight);

    // 绘制按钮面板背景
    SDL_SetRenderDrawColor(m_SDL_Renderer, 37, 39, 46, m_Alpha);
    SDL_Rect panelRect = { 0, 0, windowWidth, m_Int_PanelHeight };
    SDL_RenderFillRect(m_SDL_Renderer, &panelRect);

    // 绘制录制区域周围的红色矩形边框
    SDL_SetRenderDrawColor(m_SDL_Renderer, 255, 0, 0, m_Alpha);
    SDL_Rect outlineRect
        = {
        m_RedBorderSize, m_Int_PanelHeight,
        windowWidth - m_RedBorderSize * 2, windowHeight - m_Int_PanelHeight - m_RedBorderSize * 2
    };
    if (m_Bool_DashedBorder)
    {
        RenderDashedRect(m_SDL_Renderer, outlineRect);
    }
    else
    {
        SDL_RenderDrawRect(m_SDL_Renderer, &outlineRect);
    }

    // 绘制所有按钮
    m_BtnManager.RenderBtns();

    //绘制不透明度选择菜单
    if (m_TransparentMenu)
        m_TransparentMenu->Render();
    if (m_MenuBtnMenu)
        m_MenuBtnMenu->Render();
    

    //按钮面板中间的文本渲染
    SDL_SetRenderDrawColor(m_SDL_Renderer, 255, 255, 255, m_Alpha);
    if (m_IsTimeCountActive)
    {
        std::wstring timeCountStr = GetCurTimeCountStr();
        int TextWidth, TextHeight;
        if (!COMAPI::SDL::TextLoad(
            m_SDL_Renderer, m_TTF_Font,
            timeCountStr,
            SDL_Color{ 255,255,255,255 }, &m_SDLTexture_RecordingText, &TextWidth, &TextHeight)
            )
        {
            DB(ConsoleHandle, L"录制中计数文本纹理加载失败");
        }
        else
        {
            SDL_Rect CenterTextRect = CalculatePanelCenterRect(TextWidth, TextHeight, 50, 1 * m_Scale);
            SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_RecordingText, NULL, &CenterTextRect);
        }
    }
    else
    {
        SDL_Rect CenterTextRect = CalculatePanelCenterRect(m_CenterTextWidth, m_CenterTextHeight, 0, 0);
        SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_PanelCenterText, NULL, &CenterTextRect);
    }
    

    // 绘制按钮面板边框
    int BorderSize = 2;
    SDL_SetRenderDrawColor(m_SDL_Renderer, 73, 73, 73, m_Alpha);
    for (size_t i = 0; i < BorderSize; i++)
    {
        SDL_Rect BorderRect = { i , i, windowWidth - i * 2, m_Int_PanelHeight - i * 2 };
        SDL_RenderDrawRect(m_SDL_Renderer, &BorderRect);
    }
}

SDL_Rect Ui_AreaRecordingSDL::CalculatePanelLeftRect(int Width, int Height, int diffx, int diffy)
{
    SDL_Rect LeftTextRect
    {
        10 + diffx,
        (m_Int_PanelHeight - Height) / 2 - 2 + diffy,
        Width,
        Height
    };
    return LeftTextRect;
}

std::wstring Ui_AreaRecordingSDL::GetCurTimeCountStr()
{
    auto now = std::chrono::steady_clock::now();                                 
    auto elapsed = now - m_Time_Start - m_PausedAccum;                           
    if (elapsed < std::chrono::steady_clock::duration::zero())                   
        elapsed = std::chrono::steady_clock::duration::zero();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    m_CurrentRecordTimeSec = seconds;

    int hours = static_cast<int>(seconds / 3600);
    int minutes = static_cast<int>((seconds % 3600) / 60);
    int secs = static_cast<int>(seconds % 60);

    m_StrStream.str(L"");
    m_StrStream.clear();

    m_StrStream << L"录制中"
        << L":" << std::setfill(L'0') << std::setw(2) << hours
        << L":" << std::setfill(L'0') << std::setw(2) << minutes
        << L":" << std::setfill(L'0') << std::setw(2) << secs;

    return m_StrStream.str();
}

SDL_Rect Ui_AreaRecordingSDL::CalculatePanelCenterRect(int Width, int Height, int diffx, int diffy)
{
    SDL_Rect CenterTextRect
    {
        (m_Rect_RecordArea.Width() - Width) / 2 + diffx,
        (m_Int_PanelHeight - Height) / 2 - 2 + diffy,
        Width,
        Height
    };
    return CenterTextRect;
}
