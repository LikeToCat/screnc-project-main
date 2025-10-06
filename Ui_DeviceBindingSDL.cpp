#include "stdafx.h"
#include "CDebug.h"
#include "Ui_DeviceBindingSDL.h"
#include "LarStringConversion.h"
#include "theApp.h"
#include "GlobalFunc.h"
#include "Ui_MainDlg.h"
#include "CMessage.h"

// 调试控制台句柄
extern HANDLE ConsoleHandle;
Ui_DeviceBindingSDL* Ui_DeviceBindingSDL::Instance = nullptr;
//========================================================================
// Ui_SDLButton 实现 - 提供录制控制按钮功能
//========================================================================

Ui_DeviceBindingSDL::Ui_SDLMenuButton::Ui_SDLMenuButton(const SDL_Rect& rect, const std::wstring& text,
    std::function<void()> callback)
    : m_Rect_Area(rect), m_Str_Text(text), m_Func_Callback(callback),
    m_Bool_Hovered(false), m_SDL_TextTexture(nullptr)
{
    // 构造函数初始化按钮属性
}

Ui_DeviceBindingSDL::Ui_SDLMenuButton::~Ui_SDLMenuButton()
{
    // 清理文本纹理资源
    if (m_SDL_TextTexture)
    {
        SDL_DestroyTexture(m_SDL_TextTexture);
        m_SDL_TextTexture = nullptr;
    }
}

void Ui_DeviceBindingSDL::Ui_SDLMenuButton::Render(SDL_Renderer* renderer, TTF_Font* font)
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

bool Ui_DeviceBindingSDL::Ui_SDLMenuButton::IsPointInside(int x, int y) const
{
    // 检查坐标是否在按钮区域内
    return (x >= m_Rect_Area.x && x < m_Rect_Area.x + m_Rect_Area.w &&
        y >= m_Rect_Area.y && y < m_Rect_Area.y + m_Rect_Area.h);
}

bool Ui_DeviceBindingSDL::Ui_SDLMenuButton::UpdateHoverState(int x, int y)
{
    // 仅更新悬停状态，不触发回调
    m_Bool_Hovered = IsPointInside(x, y);
    return m_Bool_Hovered;
}

void Ui_DeviceBindingSDL::Ui_SDLMenuButton::UpdateClickState(bool isClick)
{
    m_Bool_Click = isClick;
}

void Ui_DeviceBindingSDL::Ui_SDLMenuButton::Click()
{
    // 执行点击回调
    if (m_Func_Callback) {
        m_Func_Callback();
    }
}

Ui_DeviceBindingSDL::Ui_DeviceBindingSDL()
    : m_SDL_Window(nullptr),
    m_SDL_Renderer(nullptr),
    m_TTF_Font(nullptr),
    m_Bool_Running(false),
    m_Bool_SDLInitialized(false),
    m_Bool_TTFInitialized(false),
    m_WindowDrager(nullptr)
{
    // 构造函数初始化类成员
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: Ui_DeviceBindingSDL构造");

}

Ui_DeviceBindingSDL::~Ui_DeviceBindingSDL()
{
    // 析构函数调用Close清理资源
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: Ui_DeviceBindingSDL析构");
    Close();
}

void Ui_DeviceBindingSDL::RenderRoundedRect(int x, int y, int w, int h, int radius)
{
    // 如果半径太大，调整为合理值
    radius = min(radius, min(w / 2, h / 2));

    // 绘制四个角
    // 左上角
    for (int i = 0; i <= radius; i++) {
        for (int j = 0; j <= radius; j++) {
            float distance = std::sqrt((float)(i * i + j * j));
            if (distance <= radius) {
                SDL_RenderDrawPoint(m_SDL_Renderer, x + radius - i, y + radius - j);
            }
        }
    }

    // 右上角
    for (int i = 0; i <= radius; i++) {
        for (int j = 0; j <= radius; j++) {
            float distance = std::sqrt((float)(i * i + j * j));
            if (distance <= radius) {
                SDL_RenderDrawPoint(m_SDL_Renderer, x + w - radius + i - 1, y + radius - j);
            }
        }
    }

    // 左下角
    for (int i = 0; i <= radius; i++) {
        for (int j = 0; j <= radius; j++) {
            float distance = std::sqrt((float)(i * i + j * j));
            if (distance <= radius) {
                SDL_RenderDrawPoint(m_SDL_Renderer, x + radius - i, y + h - radius + j - 1);
            }
        }
    }

    // 右下角
    for (int i = 0; i <= radius; i++) {
        for (int j = 0; j <= radius; j++) {
            float distance = std::sqrt((float)(i * i + j * j));
            if (distance <= radius) {
                SDL_RenderDrawPoint(m_SDL_Renderer, x + w - radius + i - 1, y + h - radius + j - 1);
            }
        }
    }

    // 绘制四条边
    SDL_Rect rects[4] = {
        {x + radius, y, w - 2 * radius, h},             // 中间部分
        {x, y + radius, radius, h - 2 * radius},        // 左侧
        {x + w - radius, y + radius, radius, h - 2 * radius}, // 右侧
        {x + radius, y + radius, w - 2 * radius, h - 2 * radius} // 中心
    };

    for (int i = 0; i < 4; i++) {
        SDL_RenderFillRect(m_SDL_Renderer, &rects[i]);
    }
}

bool Ui_DeviceBindingSDL::InitializeTTF()
{
    if (m_Bool_TTFInitialized)
    { // 如果TTF已初始化，直接返回成功
        return true;
    }
    if (TTF_Init() == -1)   // 初始化SDL_ttf库
    {
        DebugSDLError(L"[SDL设备绑定窗口]: TTF初始化失败:");
        return false;
    }
    m_Bool_TTFInitialized = true;

    // 加载字体
    if (!LoadFonts())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 字体加载失败");
        TTF_Quit();
        m_Bool_TTFInitialized = false;
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: TTF初始化成功");
    return true;
}

bool Ui_DeviceBindingSDL::LoadFonts()
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

    // 加载普通字体，字号20
    m_TTF_Font = TTF_OpenFont(utf8Path.c_str(), 16 * m_Scale);
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

            DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 字体加载失败: %s", wTtfError);
            return false;
        }
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 字体加载成功");
    TTF_SetFontHinting(m_TTF_Font, TTF_HINTING_LIGHT);

    // 加载标题字体
    m_TTF_Title = TTF_OpenFont("C:/Windows/Fonts/msyhbd.ttc", 18 * m_Scale);
    if (!m_TTF_Title)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 字体加载失败");
        return false;
    }
    return true;
}

bool Ui_DeviceBindingSDL::InitText()
{
    //设置字体颜色
    m_TitleText_TextColor.r = 255;
    m_TitleText_TextColor.g = 255;
    m_TitleText_TextColor.b = 255;
    m_TitleText_TextColor.a = 255;

    //转换中文编码
    std::string utf8Text;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, m_TitleText_str, -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0)
    {
        utf8Text.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, m_TitleText_str, -1, &utf8Text[0], utf8Size, NULL, NULL);
    }

    //标题提示文本
    int m_Currnetbings = m_Vec_DeviceItems.size();
    int m_Maxbings = App.m_userInfo.maxBindings;
    CString MemberType = App.m_userInfo.level;
    if (MemberType == L"")
        MemberType = L"普通用户";
    CString TipsOfBingings;
    TipsOfBingings.Format(L"您是%s,可绑定%d台设备,目前已绑定%d台", MemberType, m_Maxbings, m_Currnetbings);
    std::string TipsOfBindsUtf8;
    utf8Size = WideCharToMultiByte(CP_UTF8, 0, TipsOfBingings, -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0)
    {
        TipsOfBindsUtf8.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, TipsOfBingings, -1, &TipsOfBindsUtf8[0], utf8Size, NULL, NULL);
    }

    //最下方提示文本
    std::string tipsInBottom = "解除绑定即清除该设备资料";
    std::string tipsInBottomUtf8 = GlobalFunc::AnsiToUtf8(tipsInBottom);
    SDL_Surface* SdlSurface_tipsInBottom = TTF_RenderUTF8_Blended(
        m_TTF_Font,
        tipsInBottomUtf8.c_str(),
        m_TitleText_TextColor
    );
    m_SDLTexture_TipsOfBindsInBottom = SDL_CreateTextureFromSurface(m_SDL_Renderer, SdlSurface_tipsInBottom);
    if (!m_SDLTexture_TipsOfBindsInBottom)
        return false;

    //初始化字体纹理
    m_TitleText_textSurface = TTF_RenderUTF8_Blended(m_TTF_Title, utf8Text.c_str(), m_TitleText_TextColor);
    if (!m_TitleText_textSurface)
        return false;
    m_TitleText_textTure = SDL_CreateTextureFromSurface(m_SDL_Renderer, m_TitleText_textSurface);
    if (!m_TitleText_textTure)
        return false;

    m_SDLSurface_TipsOfBinds = TTF_RenderUTF8_Blended(m_TTF_Font, TipsOfBindsUtf8.c_str(), m_TitleText_TextColor);
    if (!m_SDLSurface_TipsOfBinds)
        return false;
    m_SDLTexture_TipsOfBinds = SDL_CreateTextureFromSurface(m_SDL_Renderer, m_SDLSurface_TipsOfBinds);
    if (!m_SDLTexture_TipsOfBinds)
        return false;

    //设置字体大小和位置
    m_SDLRect_Rect.w = m_TitleText_textSurface->w;
    m_SDLRect_Rect.h = m_TitleText_textSurface->h;
    m_SDLRect_Rect.x = 15 * m_Scale;
    m_SDLRect_Rect.y = (m_Int_TitleBarHeight / 2 - m_SDLRect_Rect.h) / 2;

    m_SDLRect_TipsOfBinds.w = m_SDLSurface_TipsOfBinds->w;
    m_SDLRect_TipsOfBinds.h = m_SDLSurface_TipsOfBinds->h;
    m_SDLRect_TipsOfBinds.x = (m_Int_WindowWidth - m_SDLRect_TipsOfBinds.w) / 2;
    m_SDLRect_TipsOfBinds.y = m_Int_TitleBarHeight / 2 + (m_Int_TitleBarHeight / 2 - m_SDLRect_TipsOfBinds.h) / 2;

    m_SDLRect_TipsInBottom.w = SdlSurface_tipsInBottom->w;
    m_SDLRect_TipsInBottom.h = SdlSurface_tipsInBottom->h;
    m_SDLRect_TipsInBottom.x = (m_Int_WindowWidth - m_SDLRect_TipsInBottom.w) / 2 + m_Int_TipsSurWidth / 2;
    m_SDLRect_TipsInBottom.y = 0.911 * m_Int_WindowHeight;

    SDL_FreeSurface(m_TitleText_textSurface);
    SDL_FreeSurface(m_SDLSurface_TipsOfBinds);
    SDL_FreeSurface(SdlSurface_tipsInBottom);
}

bool Ui_DeviceBindingSDL::InitSDLImage()
{
    //logo文件路径转utf8编码 
    CString logoExePath = GlobalFunc::GetExecutablePathFolder();
    logoExePath += L"\\SDLAssets\\logo.png";
    std::string logoutf8Path;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, logoExePath, -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        logoutf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, logoExePath, -1, &logoutf8Path[0], utf8Size, NULL, NULL);
    }

    //电脑图片文件路径转utf8编码   
    CString deviceExePath = GlobalFunc::GetExecutablePathFolder();
    deviceExePath += L"\\SDLAssets\\deivce.png";
    std::string deviceutf8Path;
    utf8Size = WideCharToMultiByte(CP_UTF8, 0, deviceExePath, -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        deviceutf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, deviceExePath, -1, &deviceutf8Path[0], utf8Size, NULL, NULL);
    }

    //电脑图片文件路径转utf8编码   (悬停)
    CString deviceHovingExePath = GlobalFunc::GetExecutablePathFolder();
    deviceHovingExePath += L"\\SDLAssets\\hovingDevice.png";
    std::string HovingDeviceutf8Path;
    utf8Size = WideCharToMultiByte(CP_UTF8, 0, deviceHovingExePath, -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        HovingDeviceutf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, deviceHovingExePath, -1, &HovingDeviceutf8Path[0], utf8Size, NULL, NULL);
    }

    //对勾图片（选中）
    CString SelectExePath = GlobalFunc::GetExecutablePathFolder();
    SelectExePath += L"\\SDLAssets\\gou.png";
    std::string SelectimagePath = GlobalFunc::ConvertPathToUtf8(SelectExePath);

    //感叹号图片
    CString SurTipsPath = GlobalFunc::GetExecutablePathFolder() += L"\\SDLAssets\\tipsOfSur.png";
    std::string SurTipsPathPath = GlobalFunc::ConvertPathToUtf8(SurTipsPath);

    //创建感叹号图片纹理
    auto SurTipsSurface = IMG_Load(SurTipsPathPath.c_str());
    m_Int_TipsSurWidth = 18 * m_Scale;
    m_Int_TipsSurHeight = 18 * m_Scale;
    m_SDLTexture_TipsSur = SDL_CreateTextureFromSurface(m_SDL_Renderer, SurTipsSurface);
    if (!m_SDLTexture_TipsSur)
        return false;
    SDL_SetTextureScaleMode(m_SDLTexture_TipsSur, SDL_ScaleModeNearest);
    SDL_FreeSurface(SurTipsSurface);

    //创建对勾图片纹理
    auto surface = IMG_Load(SelectimagePath.c_str());
    m_SDLTexture_Selected = SDL_CreateTextureFromSurface(m_SDL_Renderer, surface);
    m_int_SelectedWidth = surface->w;
    m_int_SelectedHeight = surface->h;
    SDL_FreeSurface(surface);

    //创建logo图片纹理
    m_SDLSurface_LogoImage = IMG_Load(logoutf8Path.c_str());
    if (!m_SDLSurface_LogoImage)
        return false;
    m_SDLTexture_LogoImage = SDL_CreateTextureFromSurface(m_SDL_Renderer, m_SDLSurface_LogoImage);
    if (!m_SDLTexture_LogoImage)
        return false;

    //创建创建电脑图片纹理（悬停）
    SDL_Surface* SDLSurface_HovingDevice = IMG_Load(HovingDeviceutf8Path.c_str());
    m_SDLTexture_DeivceHovingImage = SDL_CreateTextureFromSurface(m_SDL_Renderer, SDLSurface_HovingDevice);
    if (!m_SDLTexture_DeivceHovingImage)
        return false;

    //创建电脑图片纹理
    m_SDLSurface_DeivceImage = IMG_Load(deviceutf8Path.c_str());
    if (!m_SDLSurface_DeivceImage)
        return false;
    m_SDLTexture_DeivceImage = SDL_CreateTextureFromSurface(m_SDL_Renderer, m_SDLSurface_DeivceImage);
    if (!m_SDLTexture_DeivceImage)
        return false;

    //设置logo位置
    m_SDLRect_LogoImage.w = m_SDLSurface_LogoImage->w * 0.4;
    m_SDLRect_LogoImage.h = m_SDLSurface_LogoImage->h * 0.4;
    m_SDLRect_LogoImage.x = 10 * m_Scale;
    m_SDLRect_LogoImage.y = (m_Int_TitleBarHeight / 2 - m_SDLRect_LogoImage.h) / 2;

    //设置电脑图片位置
    m_int_DeviceImageHeight = 36 * m_Scale;
    m_int_DeviceImageWidth = 36 * m_Scale;

    SDL_FreeSurface(m_SDLSurface_LogoImage);
    SDL_FreeSurface(m_SDLSurface_DeivceImage);
    return true;
}

void Ui_DeviceBindingSDL::RenderTitleText()
{
    //渲染标题文本
    SDL_RenderCopy(m_SDL_Renderer, m_TitleText_textTure, NULL, &m_SDLRect_Rect);
    SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_TipsOfBinds, NULL, &m_SDLRect_TipsOfBinds);
}

void Ui_DeviceBindingSDL::RenderTitleLogo()
{
    SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_LogoImage, NULL, &m_SDLRect_LogoImage);
}

void Ui_DeviceBindingSDL::RenderWindowShadow()
{
    SDL_SetRenderDrawBlendMode(m_SDL_Renderer, SDL_BLENDMODE_BLEND);
    Uint8 oldR, oldG, oldB, oldA;
    SDL_GetRenderDrawColor(m_SDL_Renderer, &oldR, &oldG, &oldB, &oldA);
    int shadowSize = 2;
    int alphaInterval = 10;
    int startAlpha = 105;

    //上阴影
    for (int i = 0; i <= shadowSize; i++)
    {
        int shadowY = shadowSize - i;
        SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, startAlpha - alphaInterval * i);
        SDL_RenderDrawLine(m_SDL_Renderer, 0, shadowY, m_Int_WindowWidth, shadowY);
    }

    //下阴影
    for (int i = 0; i <= shadowSize; i++)
    {
        int shadowY = m_Int_WindowHeight - shadowSize + i;
        SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, startAlpha - alphaInterval * i);
        SDL_RenderDrawLine(m_SDL_Renderer, 0, shadowY, m_Int_WindowWidth, shadowY);
    }

    //右阴影
    for (int i = 0; i <= shadowSize; i++)
    {
        int shadowX = m_Int_WindowWidth - shadowSize + i;
        SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, startAlpha - alphaInterval * i);
        SDL_RenderDrawLine(m_SDL_Renderer, shadowX, 0, shadowX, m_Int_WindowHeight);
    }

    //左阴影
    for (int i = 0; i <= shadowSize; i++)
    {
        int shadowX = shadowSize - i;
        SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, startAlpha - alphaInterval * i);
        SDL_RenderDrawLine(m_SDL_Renderer, shadowX, 0, shadowX, m_Int_WindowHeight);
    }

    SDL_SetRenderDrawColor(m_SDL_Renderer, oldR, oldG, oldB, oldA);//恢复颜色状态
    SDL_SetRenderDrawBlendMode(m_SDL_Renderer, SDL_BLENDMODE_NONE);
}

void Ui_DeviceBindingSDL::RenderTitlePanel()
{
    //绘画背景
    SDL_SetRenderDrawColor(m_SDL_Renderer, 26, 27, 32, 255);
    SDL_Rect titleFirstPanelRect = { 0, 0, m_Int_WindowWidth, m_Int_TitleBarHeight / 2 };
    SDL_Rect titleSecondPanelRect = { 0, m_Int_TitleBarHeight / 2, m_Int_WindowWidth, m_Int_TitleBarHeight / 2 };
    SDL_RenderFillRect(m_SDL_Renderer, &titleFirstPanelRect);
    SDL_SetRenderDrawColor(m_SDL_Renderer, 26, 31, 37, 255);
    SDL_RenderFillRect(m_SDL_Renderer, &titleSecondPanelRect);
    SDL_SetRenderDrawColor(m_SDL_Renderer, 26, 27, 32, 255);

    //绘画与下方滚动界面的分界线
    int lineY = m_Int_TitleBarHeight - 1;
    int lineSize = 6;
    for (size_t i = 0; i < lineSize; i++)
    {
        SDL_RenderDrawLine(m_SDL_Renderer, 0, lineY - i, m_Int_WindowWidth, lineY - i);
    }

    // 绘制关闭按钮
    if (m_SDL_CloseButtonTexture)
    {
        // 如果鼠标悬停在按钮上，改变颜色
        if (m_Bool_CloseButtonHovered)
            SDL_SetTextureColorMod(m_SDL_CloseButtonTexture, 255, 100, 100); // 变红
        else
            SDL_SetTextureColorMod(m_SDL_CloseButtonTexture, 255, 255, 255); // 恢复正常颜色

        SDL_RenderCopy(m_SDL_Renderer, m_SDL_CloseButtonTexture, NULL, &m_Rect_CloseButton);
    }
}

void Ui_DeviceBindingSDL::RenderTipsInBottom()
{
    SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_TipsOfBindsInBottom, NULL, &m_SDLRect_TipsInBottom);
    SDL_Rect SurTipsRect =
    {
        m_SDLRect_TipsInBottom.x - m_Int_TipsSurWidth - 5 * m_Scale,
        m_SDLRect_TipsInBottom.y + (m_SDLRect_TipsInBottom.h - m_Int_TipsSurHeight) / 2,
        m_Int_TipsSurWidth,
        m_Int_TipsSurHeight
    };
    SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_TipsSur, NULL, &SurTipsRect);
}

void Ui_DeviceBindingSDL::RenderModalDialog()
{
    if (m_SDLWindow_SureToClose && m_SDLWindow_SureToClose->IsShowing())
    {// 如果有非模态对话框，渲染它
        m_SDLWindow_SureToClose->Render();
    }
    else if (m_SDLModelWindow_SureToUnBind && m_SDLModelWindow_SureToUnBind->IsShowing())
    {
        m_SDLModelWindow_SureToUnBind->Render();
    }
    else if (m_SDLModelWindow_UnableBind && m_SDLModelWindow_UnableBind->IsShowing())
    {
        m_SDLModelWindow_UnableBind->Render();
    }
}

void Ui_DeviceBindingSDL::ShowSureToCloseModalDialog(SDL_Renderer* renderer, TTF_Font* font)
{
    //创建非模态SDL对话框
    CString ExePath = GlobalFunc::GetExecutablePathFolder();
    ExePath += "\\SDLAssets\\MessageBoxQuestionType.png";
    if (m_SDLWindow_SureToClose)
        delete m_SDLWindow_SureToClose;
    m_SDLWindow_SureToClose = new Ui_MessageBoxSDL(
        renderer,
        "极速录屏大师",
        "设备绑定数量超限，某些功能将无法使用，是否继续?", "温馨提示",
        GlobalFunc::ConvertPathToUtf8(ExePath).c_str(),
        m_Scale,
        10 * m_Scale
    );

    // 设置回调函数
    m_SDLWindow_SureToClose->Show([this](int result)
        {
            if (result == 1)
            {//确定
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"非模态对话框关闭，结果: %d", result);
                this->m_Bool_Running = false;
            }
            else if (result == 0)
            {//返回
                m_SDLWindow_SureToClose->Close();
            }
        });
}

void Ui_DeviceBindingSDL::ShowSureToUnBindModalDialog(SDL_Renderer* renderer)
{
    CString ExePath = GlobalFunc::GetExecutablePathFolder();
    ExePath += "\\SDLAssets\\MessageBoxQuestionType.png";
    if (m_SDLModelWindow_SureToUnBind)
        delete m_SDLModelWindow_SureToUnBind;
    m_SDLModelWindow_SureToUnBind = new Ui_MessageBoxSDL(
        m_SDL_Renderer,
        "极速录屏大师",
        "确认解除绑定吗?",
        "温馨提示",
        GlobalFunc::ConvertPathToUtf8(ExePath).c_str(),
        m_Scale,
        10 * m_Scale
    );

    //设置回调
    m_SDLModelWindow_SureToUnBind->Show([this](int result)
        {
            if (result == 1)
            {//确定
                for (std::vector<Ui_DeviceItem>::iterator it = m_Vec_DeviceItems.begin();
                    it != m_Vec_DeviceItems.end();
                    it++)
                {
                    if ((*it).m_Bool_Checked)
                    {//删除选中的列表项
                        DEBUG_CONSOLE_FMT(ConsoleHandle, L"非模态对话框关闭，结果: %d,删除设备名:%s,删除设备编号:%s",
                            result, (*it).m_Str_Name.c_str(), LARSC::c2w((*it).m_Str_Id.c_str()));
                        m_Struct_DeleteDeviceItem = *it;
                        m_Vec_DeviceItems.erase(it);
                        App.m_userInfo.currentBindings--;
                        break;
                    }
                }
                m_Bool_Running = false;
            }
            else if (result == 0)
            {//返回
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"非模态对话框关闭，结果: %d", result);
                m_SDLModelWindow_SureToUnBind->Close();
            }
        });
}

void Ui_DeviceBindingSDL::ShowUnableBindModalDialog(SDL_Renderer* renderer)
{
    CString ExePath = GlobalFunc::GetExecutablePathFolder();
    ExePath += "\\SDLAssets\\MessageBoxQuestionType.png";
    if (m_SDLModelWindow_UnableBind)
        delete m_SDLModelWindow_UnableBind;
    m_SDLModelWindow_UnableBind = new Ui_MessageBoxSDL(
        m_SDL_Renderer,
        "极速录屏大师",
        "对不起，当前设备无法解绑",
        "温馨提示",
        GlobalFunc::ConvertPathToUtf8(ExePath).c_str(),
        m_Scale,
        10 * m_Scale
    );

    //设置回调
    m_SDLModelWindow_UnableBind->Show([this](int result)
        {
            if (result == 1)
            {//确定
                m_SDLModelWindow_UnableBind->Close();
            }
            else if (result == 0)
            {//返回
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"非模态对话框关闭，结果: %d", result);
                m_SDLModelWindow_UnableBind->Close();
            }
        });
}

Ui_DeviceBindingSDL* Ui_DeviceBindingSDL::GetInstance()
{
    if (!Instance)
    {
        Instance = new Ui_DeviceBindingSDL;
    }
    return Instance;
}

void Ui_DeviceBindingSDL::ReleaseInstance()
{
    if (Instance)
    {
        Instance->Close();
        delete Instance;
        Instance = nullptr;
    }
}

bool Ui_DeviceBindingSDL::Initialize(const CRect& DisplayArea, float Scale)
{
    // 初始化成员 670 430 windowsize
    m_Scale = Scale;
    m_Rect_RecordArea = DisplayArea;
    m_Int_WindowWidth = DisplayArea.Width();
    m_Int_WindowHeight = DisplayArea.Height();
    m_Int_TitleBarHeight = 0.229 * m_Int_WindowHeight;
    m_Int_TitleBarY = m_Int_TitleBarHeight;
    m_Int_DeviceItemHeight = (int)(66 * m_Scale);
    m_Int_CheckboxSize = (int)(20 * m_Scale);
    m_Int_ItemPadding = (int)(10 * m_Scale);
    m_Int_ItemsMaxScroll = 5;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 初始化录制区域 左=%d 上=%d 宽=%d 高=%d",
        DisplayArea.left, DisplayArea.top, DisplayArea.Width(), DisplayArea.Height());

    if (DisplayArea.Width() <= 0 || DisplayArea.Height() <= 0)// 验证录制区域尺寸是否有效
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 错误 - 录制区域尺寸无效");
        return false;
    }

    if (!InitializeSDL()) // 分步初始化，先初始化SDL
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL初始化失败");
        return false;
    }
    if (!InitializeTTF())  // 初始化TTF
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: TTF初始化失败");
        return false;
    }
    if (!CreateSDLWindow())// 然后创建SDL窗口
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL窗口创建失败");
        return false;
    }
    if (!InitSDLImage())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL图片资源初始化失败");
        return false;
    }
    if (!InitText())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL标题初始化失败");
        return false;
    }

    m_WindowDrager = new COMCLASS::SDL::WindowDrag;

    // 初始化鼠标光标
    m_SDL_DefaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_SDL_HandCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    LoadCloseButtonTexture();// 加载关闭按钮
    //CreateButtons(); // 创建界面按钮
    m_Bool_Running = true;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL窗口初始化成功");
    return true;
}

bool Ui_DeviceBindingSDL::InitializeSDL()
{
    if (SDL_WasInit(SDL_INIT_VIDEO)) // 检查SDL是否已经初始化
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL视频子系统已初始化");
        m_Bool_SDLInitialized = true;
    }
    else if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    { // 只初始化视频子系统，避免不必要的初始化
        DebugSDLError(L"[SDL设备绑定窗口]:SDL初始化失败:");
        return false;
    }
    else
    {
        m_Bool_SDLInitialized = true;
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL初始化成功");
    }

    // 初始化SDL_image
    int imgFlags = IMG_INIT_PNG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL_image初始化失败");
    }
    else
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL_image初始化成功");
    }

    return true;
}

void Ui_DeviceBindingSDL::DebugSDLError(wchar_t error[256])
{
    const char* sdlError = SDL_GetError();
    wchar_t wSdlError[256] = { 0 };
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wSdlError, _countof(wSdlError), sdlError, _TRUNCATE);
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s: %s", error, wSdlError);
}

bool Ui_DeviceBindingSDL::CreateSDLWindow()
{
    // 确保SDL已初始化
    if (!m_Bool_SDLInitialized)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 无法创建窗口 - SDL未初始化");
        return false;
    }

    // 计算初始窗口位置和大小
    int left = max(0, m_Rect_RecordArea.left);
    int top = max(0, m_Rect_RecordArea.top);
    int width = max(100, min(GetSystemMetrics(SM_CXSCREEN) - left, m_Rect_RecordArea.Width()));
    int height = max(100, min(GetSystemMetrics(SM_CYSCREEN) - top, m_Rect_RecordArea.Height()));
    Uint32 windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS |
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 创建SDL窗口");
    m_SDL_Window = SDL_CreateWindow(
        "DeviceWindow",    // 窗口标题
        left, top,         // 位置
        width, height,     // 大小
        windowFlags        // 标志
    );
    if (!m_SDL_Window)
    { // 获取SDL错误消息
        DebugSDLError(L"[SDL设备绑定窗口]: 窗口创建失败:");
        return false;
    }
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 窗口创建成功，位置=%d,%d 大小=%d,%d",
        left, top, width, height);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    // 创建硬件渲染器
    m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!m_SDL_Renderer)
    {
        // 尝试创建软件渲染器作为后备方案
        m_SDL_Renderer = SDL_CreateRenderer(m_SDL_Window, -1, SDL_RENDERER_SOFTWARE);
        if (!m_SDL_Renderer)
        {
            DebugSDLError(L"[SDL设备绑定窗口]: 渲染器创建失败:");
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

void Ui_DeviceBindingSDL::CreateButtons()
{
    //获取窗口宽高，按钮高度   
    int WindowWidth, WindowHeight, BtnHeight, BtnWidth, WindowSpacing;
    SDL_GetWindowSize(m_SDL_Window, &WindowWidth, &WindowHeight);

    // 底部居中按钮
    float CancaleBindingWidth = 161 * m_Scale;
    float CancaleBindingHeight = 40 * m_Scale;
    float CancaleBindingY = 0.803 * WindowHeight;
    float CancaleBindingX = (WindowWidth - CancaleBindingWidth) / 2;
    m_Btn_ControlButtons.emplace_back(
        SDL_Rect{ (int)CancaleBindingX, (int)CancaleBindingY,
                (int)CancaleBindingWidth, (int)CancaleBindingHeight },
        L"解除绑定",
        [this]()
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 点击了解除绑定按钮");
            if (m_Func_OnCancelBindingRecord)
            {
                m_Func_OnCancelBindingRecord();
            }
        }
    );
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 创建了%d个按钮", (int)m_Btn_ControlButtons.size());
}

void Ui_DeviceBindingSDL::UpdateBottomButtonText()
{
    if (m_Btn_ControlButtons.empty()) {
        return; // 安全检查
    }

    // 计算选中的设备数量
    int selectedCount = 0;
    for (const auto& device : m_Vec_DeviceItems) {
        if (device.m_Bool_Checked) {
            selectedCount++;
        }
    }

    // 根据选中数量更新按钮文字
    if (selectedCount > 1) {
        // 释放旧的纹理
        if (m_Btn_ControlButtons[0].m_SDL_TextTexture) {
            SDL_DestroyTexture(m_Btn_ControlButtons[0].m_SDL_TextTexture);
            m_Btn_ControlButtons[0].m_SDL_TextTexture = nullptr;
        }
        m_Btn_ControlButtons[0].m_Str_Text = L"批量解除绑定";
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 更新按钮文字为「批量解除绑定」(选中了%d个设备)", selectedCount);
    }
    else {
        // 释放旧的纹理
        if (m_Btn_ControlButtons[0].m_SDL_TextTexture) {
            SDL_DestroyTexture(m_Btn_ControlButtons[0].m_SDL_TextTexture);
            m_Btn_ControlButtons[0].m_SDL_TextTexture = nullptr;
        }
        m_Btn_ControlButtons[0].m_Str_Text = L"解除绑定";
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 更新按钮文字为「解除绑定」");
    }
}

bool Ui_DeviceBindingSDL::LoadCloseButtonTexture()
{
    // 获取程序路径
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath(path);
    std::string::size_type pos = exePath.find_last_of("\\/");
    std::string dirPath = exePath.substr(0, pos);

    // 关闭按钮图像路径 (假设在程序目录中有close_btn.png文件)
    std::string closeButtonPath = dirPath + "\\SDLAssets\\Close.png";
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"加载的图片资源路径:%s", LARSC::s2ws(closeButtonPath).c_str());

    // 如果文件不存在，则创建一个简单的关闭按钮图像
    SDL_Surface* closeBtnSurface = nullptr;

    // 尝试加载图像文件
    FILE* file = fopen(closeButtonPath.c_str(), "rb");
    if (file) {
        fclose(file);
        // 使用SDL_image加载图像
        closeBtnSurface = IMG_Load(closeButtonPath.c_str());
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 找到关闭按钮图像文件");
    }
    else {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 未找到关闭按钮图像文件，将创建默认按钮");
    }

    // 如果无法加载图像文件，创建一个默认的关闭按钮
    if (!closeBtnSurface) {
        int btnSize = (int)(16 * m_Scale);  // 16像素 * 缩放系数

        closeBtnSurface = SDL_CreateRGBSurface(0, btnSize, btnSize, 32,
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

        if (closeBtnSurface) {
            // 设置透明背景
            SDL_SetSurfaceBlendMode(closeBtnSurface, SDL_BLENDMODE_BLEND);
            SDL_FillRect(closeBtnSurface, NULL, SDL_MapRGBA(closeBtnSurface->format, 0, 0, 0, 0));

            // 绘制X形状
            SDL_Rect lines[4] = {
                {0, 0, btnSize - 1, 2},                    // 左上到右上
                {0, 0, 2, btnSize - 1},                    // 左上到左下
                {0, btnSize - 2, btnSize - 1, 2},          // 左下到右下
                {btnSize - 2, 0, 2, btnSize - 1}           // 右上到右下
            };

            for (int i = 0; i < 4; i++) {
                SDL_FillRect(closeBtnSurface, &lines[i], SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255));
            }

            // 绘制X中的斜线
            for (int i = 0; i < btnSize; i++) {
                int j = i;
                if (j >= 0 && j < btnSize) {
                    Uint32* pixels = (Uint32*)closeBtnSurface->pixels;
                    // 画第一条对角线
                    pixels[j * closeBtnSurface->pitch / 4 + j] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                    // 画第二条对角线
                    pixels[j * closeBtnSurface->pitch / 4 + (btnSize - 1 - j)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);

                    // 加粗对角线
                    if (j > 0 && j < btnSize - 1) {
                        pixels[(j - 1) * closeBtnSurface->pitch / 4 + j] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                        pixels[(j + 1) * closeBtnSurface->pitch / 4 + j] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                        pixels[j * closeBtnSurface->pitch / 4 + (j - 1)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                        pixels[j * closeBtnSurface->pitch / 4 + (j + 1)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);

                        // 第二条对角线加粗
                        pixels[(j - 1) * closeBtnSurface->pitch / 4 + (btnSize - 1 - j)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                        pixels[(j + 1) * closeBtnSurface->pitch / 4 + (btnSize - 1 - j)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                        pixels[j * closeBtnSurface->pitch / 4 + (btnSize - 1 - j - 1)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                        pixels[j * closeBtnSurface->pitch / 4 + (btnSize - 1 - j + 1)] = SDL_MapRGBA(closeBtnSurface->format, 220, 220, 220, 255);
                    }
                }
            }
        }
    }

    if (closeBtnSurface) {
        // 创建纹理
        m_SDL_CloseButtonTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, closeBtnSurface);
        SDL_SetTextureBlendMode(m_SDL_CloseButtonTexture, SDL_BLENDMODE_BLEND);

        // 设置关闭按钮的位置
        int btnWidth, btnHeight;
        btnWidth = 20 * m_Scale;
        btnHeight = 20 * m_Scale;
        int padding = (int)(15 * m_Scale);  // 边距
        m_Rect_CloseButton = {
            m_Int_WindowWidth - btnWidth - padding,  // X坐标
            (m_Int_TitleBarHeight / 2 - btnHeight) / 2,  // Y坐标
            btnWidth,                                // 宽度
            btnHeight                                // 高度
        };

        // 释放表面
        SDL_FreeSurface(closeBtnSurface);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 关闭按钮创建成功");
        return true;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 关闭按钮创建失败");
    return false;
}

void Ui_DeviceBindingSDL::UpdateCursorShape(bool shouldBeHand)
{
    // 只有当状态改变时才更新光标
    if (m_Bool_CursorIsHand != shouldBeHand) {
        if (shouldBeHand && m_SDL_HandCursor) {
            SDL_SetCursor(m_SDL_HandCursor);
            m_Bool_CursorIsHand = true;
        }
        else if (m_SDL_DefaultCursor) {
            SDL_SetCursor(m_SDL_DefaultCursor);
            m_Bool_CursorIsHand = false;
        }
    }
}

void Ui_DeviceBindingSDL::DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius)
{
    for (int y = -radius; y <= radius; ++y)
    { // 使用中点圆算法绘制填充圆
        for (int x = -radius; x <= radius; ++x)
        {
            if (x * x + y * y <= radius * radius)
            {  // 检查点(x,y)是否在圆内
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}

void Ui_DeviceBindingSDL::SetOnCancelBindingCallback(std::function<void()> callback)
{
    m_Func_OnCancelBindingRecord = callback;
}

void Ui_DeviceBindingSDL::AddDevice(const std::wstring& name, std::string& id, bool IsCurrent)
{
    if (id == "")
        id = std::string("None");
    Ui_DeviceItem deviceItem;
    deviceItem.m_Str_Name = name;
    deviceItem.m_Str_Id = id;
    deviceItem.m_Bool_Checked = false;
    deviceItem.m_Bool_IsCurrnet = IsCurrent;
    m_Vec_DeviceItems.push_back(deviceItem);
}

void Ui_DeviceBindingSDL::ClearDevices()
{
    m_Vec_DeviceItems.clear();
    m_Float_CurrentScroll = 0.0f;
    m_Float_TargetScroll = 0.0f;
    m_Float_ScrollVelocity = 0.0f;
}

void Ui_DeviceBindingSDL::SetDeviceChecked(int index, bool checked)
{
    if (index >= 0 && index < m_Vec_DeviceItems.size()) {
        m_Vec_DeviceItems[index].m_Bool_Checked = checked;
        UpdateBottomButtonText();
    }
}

bool Ui_DeviceBindingSDL::IsDeviceChecked(int index) const
{
    if (index >= 0 && index < m_Vec_DeviceItems.size()) {
        return m_Vec_DeviceItems[index].m_Bool_Checked;
    }
    return false;
}

std::wstring Ui_DeviceBindingSDL::GetDeviceName(int index) const
{
    if (index >= 0 && index < m_Vec_DeviceItems.size()) {
        return m_Vec_DeviceItems[index].m_Str_Name;
    }
    return L"";
}

std::string Ui_DeviceBindingSDL::GetDeviceId(int index) const
{
    if (index >= 0 && index < m_Vec_DeviceItems.size()) {
        return m_Vec_DeviceItems[index].m_Str_Id;
    }
    return "";
}

void Ui_DeviceBindingSDL::CreateCheckboxTextures()
{
    if (m_SDL_CheckboxTexture && m_SDL_CheckboxCheckedTexture) {
        return; // 已创建
    }

    // 未选中复选框
    SDL_Surface* surface = SDL_CreateRGBSurface(0, m_Int_CheckboxSize, m_Int_CheckboxSize, 32,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (surface) {
        // 设置背景色
        SDL_Rect rect = { 0, 0, m_Int_CheckboxSize, m_Int_CheckboxSize };
        SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, 40, 40, 40, 255));

        // 绘制边框
        SDL_Rect border[4] = {
            {0, 0, m_Int_CheckboxSize, 1},
            {0, 0, 1, m_Int_CheckboxSize},
            {0, m_Int_CheckboxSize - 1, m_Int_CheckboxSize, 1},
            {m_Int_CheckboxSize - 1, 0, 1, m_Int_CheckboxSize}
        };

        for (int i = 0; i < 4; i++) {
            SDL_FillRect(surface, &border[i], SDL_MapRGBA(surface->format, 180, 180, 180, 255));
        }

        // 创建纹理
        m_SDL_CheckboxTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, surface);
        SDL_FreeSurface(surface);
    }

    // 选中复选框 - 修改这部分代码
    surface = SDL_CreateRGBSurface(0, m_Int_CheckboxSize, m_Int_CheckboxSize, 32,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (surface) {
        // 设置背景色 - 使用绿色填充
        SDL_Rect rect = { 0, 0, m_Int_CheckboxSize, m_Int_CheckboxSize };
        SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, 1, 192, 131, 255));

        // 绘制边框
        SDL_Rect border[4] = {
            {0, 0, m_Int_CheckboxSize, 1},
            {0, 0, 1, m_Int_CheckboxSize},
            {0, m_Int_CheckboxSize - 1, m_Int_CheckboxSize, 1},
            {m_Int_CheckboxSize - 1, 0, 1, m_Int_CheckboxSize}
        };

        for (int i = 0; i < 4; i++) {
            SDL_FillRect(surface, &border[i], SDL_MapRGBA(surface->format, 220, 220, 220, 255));
        }

        // 创建纹理
        m_SDL_CheckboxCheckedTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, surface);
        SDL_FreeSurface(surface);
    }
}

int Ui_DeviceBindingSDL::GetDeviceAtPosition(int x, int y)
{
    // 检查是否在设备列表区域内
    if (y < m_Int_TitleBarHeight || y >= m_Int_WindowHeight) {
        return -1;
    }

    // 计算相对于列表开始的Y坐标
    int relY = y - m_Int_TitleBarHeight + (int)m_Float_CurrentScroll;

    // 计算设备索引
    int index = relY / m_Int_DeviceItemHeight;

    if (index >= 0 && index < m_Vec_DeviceItems.size()) {
        return index;
    }

    return -1;
}

void Ui_DeviceBindingSDL::UpdateScrollPhysics()
{
    // 计算帧时间差
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - m_Uint32_LastFrameTime) / 1000.0f;
    m_Uint32_LastFrameTime = currentTime;

    // 防止时间跳变
    if (deltaTime > 0.1f) {
        deltaTime = 0.016f; // 约60FPS
    }

    // 计算最大滚动范围
    int contentHeight = m_Vec_DeviceItems.size() * m_Int_DeviceItemHeight;
    int viewportHeight = m_Int_WindowHeight - m_Int_TitleBarHeight;
    int maxScroll = max(0, contentHeight - viewportHeight);

    // 应用滚动速度
    m_Float_TargetScroll += m_Float_ScrollVelocity * deltaTime * 60.0f;

    // 限制滚动范围
    if (m_Float_TargetScroll < 0) {
        // 顶部边界弹性
        m_Float_TargetScroll *= 0.5f;
        m_Float_ScrollVelocity *= -0.2f;
    }
    else if (m_Float_TargetScroll > maxScroll && maxScroll > 0) {
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
    if (fabs(m_Float_ScrollVelocity) < 0.5f && fabs(diff) < 0.5f) {
        m_Float_ScrollVelocity = 0.0f;
        m_Float_CurrentScroll = round(m_Float_CurrentScroll); // 对齐到整数像素
    }
}

void Ui_DeviceBindingSDL::RenderDeviceList()
{
    // 确保复选框纹理已创建
    if (!m_SDL_CheckboxTexture || !m_SDL_CheckboxCheckedTexture) {
        CreateCheckboxTextures();
    }

    // 设置裁剪区域（只显示滚动区域内容）
    SDL_Rect clipRect =
    {
        0, m_Int_TitleBarHeight,
        m_Int_WindowWidth, m_Int_WindowHeight - m_Int_TitleBarHeight
    };
    SDL_RenderSetClipRect(m_SDL_Renderer, &clipRect);

    // 计算可见项范围
    int firstVisible = (int)(m_Float_CurrentScroll / m_Int_DeviceItemHeight);
    firstVisible = max(0, firstVisible);

    // 计算可见项数量（多渲染一个确保平滑过渡）
    int visibleCount = (clipRect.h / m_Int_DeviceItemHeight) + 2;
    int lastVisible = min(firstVisible + visibleCount, (int)m_Vec_DeviceItems.size());

    // 亚像素偏移
    float fractionalOffset = fmodf(m_Float_CurrentScroll, (float)m_Int_DeviceItemHeight);

    // 渲染可见设备项
    for (int i = firstVisible; i < lastVisible; i++)
    {
        // 计算精确Y位置（亚像素定位）
        float exactY = m_Int_TitleBarHeight + (i * m_Int_DeviceItemHeight) - m_Float_CurrentScroll;

        // 转换为整数像素位置
        SDL_Rect itemRect =
        {
            0,
            (int)exactY,
            m_Int_WindowWidth,
            m_Int_DeviceItemHeight
        };
        //绘画列表项背景颜色
        SDL_SetRenderDrawColor(m_SDL_Renderer, 26, 31, 37, 255);
        SDL_RenderFillRect(m_SDL_Renderer, &itemRect);

        //准备电脑图片区域和圆形遮罩区域
        int deviceImageX = 25 * m_Scale;
        int deviceImageY = (int)exactY + (m_Int_DeviceItemHeight - m_int_DeviceImageHeight) / 2;
        SDL_Rect deviceRect =
        {
            deviceImageX,
            deviceImageY,
            m_int_DeviceImageWidth,
            m_int_DeviceImageHeight
        };//图片显示区域

        //是否高亮背景色
        if (m_Int_HoveredDeviceIndex == i || m_Int_SelectIndex == i)
        { // 设置悬停高亮背景色
            Uint8 r, g, b, a;
            SDL_GetRenderDrawColor(m_SDL_Renderer, &r, &g, &b, &a);
            SDL_SetRenderDrawColor(m_SDL_Renderer, 64, 65, 70, 255);
            SDL_RenderFillRect(m_SDL_Renderer, &itemRect);
            SDL_SetRenderDrawColor(m_SDL_Renderer, r, g, b, a);
        }
        //图片圆形背景区域
        int deviceImgCenterCircleX = deviceImageX + m_int_DeviceImageWidth / 2;
        int deviceImgCenterCircleY = deviceImageY + m_int_DeviceImageHeight / 2;
        int CircleRadius = m_Int_DeviceItemHeight/2;

        //渲染遮罩和电脑图标
        SDL_Color FillCircleColor{ 32,32,32,255 };
        SDL_Color OriRenderColor;
        SDL_GetRenderDrawColor(m_SDL_Renderer, &OriRenderColor.r, &OriRenderColor.g, &OriRenderColor.b, &OriRenderColor.a);
        SDL_SetRenderDrawColor(m_SDL_Renderer, FillCircleColor.r, FillCircleColor.g, FillCircleColor.b, FillCircleColor.a);
        DrawFilledCircle(m_SDL_Renderer, deviceImgCenterCircleX, deviceImgCenterCircleY, CircleRadius);//先绘画圆形遮罩
        SDL_SetRenderDrawColor(m_SDL_Renderer, OriRenderColor.r, OriRenderColor.g, OriRenderColor.b, OriRenderColor.a);
        if (m_Int_SelectIndex == i || m_Int_HoveredDeviceIndex == i)
        {
            SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_DeivceHovingImage, NULL, &deviceRect);
        }
        else
        {
            SDL_RenderCopy(m_SDL_Renderer, m_SDLTexture_DeivceImage, NULL, &deviceRect);
        }

        // 渲染设备名称
        if (m_TTF_Font && !m_Vec_DeviceItems[i].m_Str_Name.empty())
        {
            // 设备名转换为UTF-8
            std::string utf8DeviceName;
            std::wstring deviceName = m_Vec_DeviceItems[i].m_Str_Name;
            int utf8Size = WideCharToMultiByte(CP_UTF8, 0, deviceName.c_str(),
                -1, NULL, 0, NULL, NULL);
            if (utf8Size > 0)
            {
                utf8DeviceName.resize(utf8Size);
                WideCharToMultiByte(CP_UTF8, 0, deviceName.c_str(),
                    -1, &utf8DeviceName[0], utf8Size, NULL, NULL);
            }

            // 渲染(设备名)
            SDL_Color TextColor = { 220, 220, 220, 255 }; // 白色文本
            SDL_Surface* deviceTextSurface = TTF_RenderUTF8_Blended(m_TTF_Font, utf8DeviceName.c_str(), TextColor);
            int devicetextWidth = deviceTextSurface->w;
            int devicetextHeight = deviceTextSurface->h;
            int deviceTextX = deviceImageX + m_int_DeviceImageWidth + 25 * m_Scale;
            int deviceTextY = (int)exactY + (m_Int_DeviceItemHeight - devicetextHeight) / 2 - devicetextHeight / 2;
            if (deviceTextSurface)
            {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, deviceTextSurface);
                // 计算文本位置
                SDL_QueryTexture(textTexture, NULL, NULL, &devicetextWidth, &devicetextHeight);
                SDL_Rect textRect = {
                    deviceTextX,
                    deviceTextY,
                    devicetextWidth,
                    devicetextHeight
                };
                SDL_RenderCopy(m_SDL_Renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
                SDL_FreeSurface(deviceTextSurface);
            }

            // 渲染(设备编号)
            std::string deviceNum = "设备编号:" + m_Vec_DeviceItems[i].m_Str_Id;
            std::string utf8DeviceNum = GlobalFunc::AnsiToUtf8(deviceNum);
            SDL_Surface* DeviceNumFace = TTF_RenderUTF8_Blended(m_TTF_Font, utf8DeviceNum.c_str(), TextColor);
            int deviceNumWidth = DeviceNumFace->w;
            int deivceNumHeight = DeviceNumFace->h;
            int deviceNumX = deviceTextX;
            int deviceNumY = deviceTextY + devicetextHeight + 5 * m_Scale;
            if (DeviceNumFace)
            {
                SDL_Texture* DeviceNumTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, DeviceNumFace);
                int textWidth, textHeight;
                SDL_QueryTexture(DeviceNumTexture, NULL, NULL, &textWidth, &textHeight);
                SDL_Rect textRect = {
                       deviceNumX,
                       deviceNumY,
                       deviceNumWidth,
                       deivceNumHeight
                };
                SDL_RenderCopy(m_SDL_Renderer, DeviceNumTexture, NULL, &textRect);
                SDL_FreeSurface(DeviceNumFace);
                SDL_DestroyTexture(DeviceNumTexture);
            }

            //渲染是否是当前设备
            if (m_Vec_DeviceItems[i].m_Bool_IsCurrnet)
            {
                std::string CurrentDeviceStr = "当前设备";
                std::string utf8CurrentDevice = GlobalFunc::AnsiToUtf8(CurrentDeviceStr);
                SDL_Surface* CurrentDeviceFace = TTF_RenderUTF8_Blended(m_TTF_Font, utf8CurrentDevice.c_str(), TextColor);
                int CurrentDeviceWidth = CurrentDeviceFace->w;
                int CurrentDeviceHeight = CurrentDeviceFace->h;
                int CurrentDeviceX = deviceTextX + devicetextWidth + 10 * m_Scale;
                int CurrentDeviceY = deviceTextY;
                if (CurrentDeviceFace)
                {
                    SDL_Texture* CurrentDeviceTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, CurrentDeviceFace);
                    int textWidth, textHeight;
                    SDL_Rect textRect = {
                           CurrentDeviceX,
                           CurrentDeviceY,
                           CurrentDeviceWidth,
                           CurrentDeviceHeight
                    };
                    SDL_RenderCopy(m_SDL_Renderer, CurrentDeviceTexture, NULL, &textRect);
                    SDL_FreeSurface(CurrentDeviceFace);
                    SDL_DestroyTexture(CurrentDeviceTexture);
                }
            }
        }

        // 渲染圆角矩形按钮（解除绑定）
        SDL_Rect btnRect = GetUnbindButtonRect(i);
        SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, 255); // 设置按钮背景颜色（黑色）
        if (m_Bool_UnbindBtnHovered && m_Int_HoveredDeviceIndex == i)
            SDL_SetRenderDrawColor(m_SDL_Renderer, 20, 20, 20, 255); // 如果鼠标悬停在按钮上，稍微改变颜色
        RenderRoundedRect(btnRect.x, btnRect.y, btnRect.w, btnRect.h, (int)(5 * m_Scale));//设置圆角
        std::string unbindText = "\xe8\xa7\xa3\xe9\x99\xa4\xe7\xbb\x91\xe5\xae\x9a";
        SDL_Color textColor = { 244, 42, 72, 255 }; // 红色
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(m_TTF_Font, unbindText.c_str(), textColor);
        if (textSurface)
        {
            // 计算文本位置
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, textSurface);
            int textWidth, textHeight;
            SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
            SDL_Rect textRect =
            {
                btnRect.x + (btnRect.w - textWidth) / 2,
                btnRect.y + (btnRect.h - textHeight) / 2,
                textWidth,
                textHeight
            };
            SDL_RenderCopy(m_SDL_Renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
    }
    // 绘制滚动条（如果需要滚动）
    int contentHeight = m_Vec_DeviceItems.size() * m_Int_DeviceItemHeight;
    int viewportHeight = m_Int_WindowHeight - m_Int_TitleBarHeight;
    if (contentHeight > viewportHeight)
    {
        // 计算滚动条尺寸和位置
        float scrollRatio = (float)viewportHeight / contentHeight;
        float scrollHandleHeight = viewportHeight * scrollRatio;
        scrollHandleHeight = max(30.0f, scrollHandleHeight); // 最小高度

        float scrollPosition = m_Float_CurrentScroll / (contentHeight - viewportHeight);
        float scrollHandleY = m_Int_TitleBarHeight + scrollPosition * (viewportHeight - scrollHandleHeight);

        // 背景
        SDL_Rect scrollbarBg = {
            m_Int_WindowWidth - 8,
            m_Int_TitleBarHeight,
            8,
            viewportHeight
        };

        // 滑块
        SDL_Rect scrollHandle = {
            m_Int_WindowWidth - 8,
            (int)scrollHandleY,
            8,
            (int)scrollHandleHeight
        };

        // 绘制背景
        SDL_SetRenderDrawColor(m_SDL_Renderer, 30, 30, 30, 200);
        SDL_RenderFillRect(m_SDL_Renderer, &scrollbarBg);

        // 绘制滑块
        SDL_SetRenderDrawColor(m_SDL_Renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(m_SDL_Renderer, &scrollHandle);
    }

    // 重置裁剪区域
    SDL_RenderSetClipRect(m_SDL_Renderer, NULL);
}

void Ui_DeviceBindingSDL::Render()
{
    // 清除背景
    SDL_SetRenderDrawColor(m_SDL_Renderer, 26, 31, 37, 255);
    SDL_RenderClear(m_SDL_Renderer);

    //渲染
    RenderTitlePanel();   //渲染标题背景
    RenderTitleText();    //渲染标题文本
    RenderDeviceList();   //渲染设备列表
    //RenderTitleLogo();  //渲染标题logo
    RenderWindowShadow(); //渲染窗口阴影
    RenderTipsInBottom(); //渲染下方tips
    RenderModalDialog();  //根据状态渲染模态对话框

    SDL_RenderPresent(m_SDL_Renderer); // 呈现渲染结果
}

void Ui_DeviceBindingSDL::ProcessEvents()
{
    // 检查窗口是否正确初始化
    if (!m_SDL_Window) {
        m_Bool_Running = false;
        return;
    }

    // 更新滚动物理模型
    UpdateScrollPhysics();

    // 处理SDL事件队列中的所有事件
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (m_SDLWindow_SureToClose && m_SDLWindow_SureToClose->HandleEvent(event))
        { // 对话框已处理事件，不再传递给其他控件
            continue;
        }
        else if (m_SDLModelWindow_SureToUnBind && m_SDLModelWindow_SureToUnBind->HandleEvent(event))
        {// 对话框已处理事件，不再传递给其他控件
            continue;
        }
        else if (m_SDLModelWindow_UnableBind && m_SDLModelWindow_UnableBind->HandleEvent(event))
        {// 对话框已处理事件，不再传递给其他控件
            continue;
        }
        switch (event.type)
        {
        case SDL_QUIT:
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]:窗口退出");
            m_Bool_Running = false;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                bool isClickBtn = false;
                // 检查是否点击按钮
                for (auto& btn : m_Btn_ControlButtons)
                {
                    if (btn.IsPointInside(event.button.x, event.button.y))
                    {
                        btn.UpdateClickState(true);
                        isClickBtn = true;
                    }
                }

                if (!m_Bool_ButtonHoved && !isClickBtn)
                {
                    // 检查是否点击了列表项
                    int deviceIndex = GetDeviceAtPosition(event.button.x, event.button.y);
                    if (deviceIndex != -1)
                    {// 重置所有的选中状态
                        for (auto& item : m_Vec_DeviceItems)
                        {
                            item.m_Bool_Checked = false;
                        }
                        m_Vec_DeviceItems[deviceIndex].m_Bool_Checked = !m_Vec_DeviceItems[deviceIndex].m_Bool_Checked;
                        m_Int_SelectIndex = deviceIndex;
                    }
                }

                SDL_Rect titleFirstPanelRect = { 0, 0, m_Int_WindowWidth, m_Int_TitleBarHeight / 2 };
                SDL_Rect titleSecondPanelRect = { 0, m_Int_TitleBarHeight / 2, m_Int_WindowWidth, m_Int_TitleBarHeight / 2 };
                SDL_Point p{ event.motion.x,event.motion.y };
                if (SDL_PointInRect(&p, &titleFirstPanelRect) || SDL_PointInRect(&p, &titleSecondPanelRect) && !isClickBtn)
                {
                    m_WindowDrager->MoveActive(m_SDL_Window);
                }
            }
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]:鼠标按下");
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                if (m_Bool_CloseButtonHovered)
                {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 点击了关闭按钮");
                    if (App.m_userInfo.maxBindings < m_Vec_DeviceItems.size())
                    {
                        ShowSureToCloseModalDialog(m_SDL_Renderer, m_TTF_Font);
                        m_Bool_CloseButtonHovered = false;
                    }
                    else
                    {
                        m_Bool_Running = false;
                    }
                    break;
                }
                for (auto& btn : m_Btn_ControlButtons)
                {
                    if (btn.IsPointInside(event.button.x, event.button.y))
                    {
                        btn.UpdateClickState(false);
                        btn.Click();
                    }
                }
                // 检查是否点击了解绑按钮
                if (IsPointInUnbindButton(event.button.x, event.button.y))
                {
                    if (m_Vec_DeviceItems[m_Int_HoveredDeviceIndex].m_Bool_IsCurrnet)
                    {
                        ShowUnableBindModalDialog(m_SDL_Renderer);
                    }
                    else
                    {
                        DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 解绑设备: %s",
                            m_Vec_DeviceItems[m_Int_HoveredDeviceIndex].m_Str_Name.c_str());
                        ShowSureToUnBindModalDialog(m_SDL_Renderer);
                    }
                }
            }
            m_WindowDrager->MoveInActive();
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]:鼠标松开");
            break;

        case SDL_MOUSEMOTION:
        {
            // 检查是否有任何按钮被悬停
            bool anyButtonHovered = false;

            // 检查关闭按钮 - 确保按钮存在
            if (m_SDL_CloseButtonTexture)
            {
                m_Bool_CloseButtonHovered = (
                    event.motion.x >= m_Rect_CloseButton.x &&
                    event.motion.x < m_Rect_CloseButton.x + m_Rect_CloseButton.w &&
                    event.motion.y >= m_Rect_CloseButton.y &&
                    event.motion.y < m_Rect_CloseButton.y + m_Rect_CloseButton.h
                    );
                if (m_Bool_CloseButtonHovered)
                    anyButtonHovered = true;

            }

            // 检查设备列表悬停状态
            m_Int_HoveredDeviceIndex = GetDeviceAtPosition(event.motion.x, event.motion.y);

            // 检查解绑按钮
            m_Bool_UnbindBtnHovered = IsPointInUnbindButton(event.motion.x, event.motion.y);
            if (m_Bool_UnbindBtnHovered) {
                anyButtonHovered = true;
            }

            // 更新鼠标形状 - 添加调试输出
            if (anyButtonHovered != m_Bool_CursorIsHand)
            {
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]: 更新鼠标光标: %s",
                    anyButtonHovered ? L"手型" : L"默认");
            }
            UpdateCursorShape(anyButtonHovered);
            m_WindowDrager->MoveTo(m_SDL_Window);
        }
        break;

        case SDL_MOUSEWHEEL:
            if (m_Bool_EnableScroll)
            { // 处理鼠标滚轮事件，实现滚动
                m_Float_ScrollVelocity -= event.wheel.y * m_Float_ScrollSensitivity;
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"[SDL设备绑定窗口]:鼠标滚轮滚动: %d", event.wheel.y);
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_LEAVE)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]:鼠标离开SDL窗口");
            }
            else if (event.window.event == SDL_WINDOWEVENT_ENTER)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]:鼠标进入SDL窗口");
                SDL_RaiseWindow(m_SDL_Window);
                SDL_SetWindowInputFocus(m_SDL_Window);
            }
            break;

        case SDL_KEYDOWN:
            if (m_Bool_EnableScroll)
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]:按键按下");
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 按下ESC键，关闭窗口");
                    m_Bool_Running = false;
                }
                else if (event.key.keysym.sym == SDLK_UP)
                {// 上箭头键
                    m_Float_ScrollVelocity = -15.0f;
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                { // 下箭头键
                    m_Float_ScrollVelocity = 15.0f;
                }
                else if (event.key.keysym.sym == SDLK_PAGEUP)
                { // 向上翻页
                    m_Float_ScrollVelocity = -60.0f;
                }
                else if (event.key.keysym.sym == SDLK_PAGEDOWN)
                { // 向下翻页
                    m_Float_ScrollVelocity = 60.0f;
                }
                else if (event.key.keysym.sym == SDLK_HOME)
                { // 回到顶部
                    m_Float_TargetScroll = 0;
                }
                else if (event.key.keysym.sym == SDLK_END)
                {  // O
                    int contentHeight = m_Vec_DeviceItems.size() * m_Int_DeviceItemHeight;
                    int viewportHeight = m_Int_WindowHeight - m_Int_TitleBarHeight;
                    m_Float_TargetScroll = max(0, contentHeight - viewportHeight);
                }
            }
            break;
        }
    }
}

void Ui_DeviceBindingSDL::Run()
{
    // 检查窗口和渲染器是否正确初始化
    if (!m_SDL_Window || !m_SDL_Renderer)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 无法运行 - 窗口或渲染器未初始化");
        return;
    }

    RenderWindowShadow();              // 绘画窗口阴影
    SDL_RenderPresent(m_SDL_Renderer); // 呈现渲染结果
    SDL_ShowWindow(m_SDL_Window);
    SDL_RaiseWindow(m_SDL_Window);
    SDL_SetWindowInputFocus(m_SDL_Window);

    m_Bool_Running = true;
    m_Uint32_LastFrameTime = SDL_GetTicks(); // 初始化时间戳
    m_Bool_EnableScroll = m_Int_ItemsMaxScroll < m_Vec_DeviceItems.size() ? true : false;

    // 运行SDL窗口的主循环，直到窗口关闭
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL窗口主循环开始运行");

    // 使用高帧率以确保滚动流畅
    Uint32 frameStart, frameTime;
    const int FPS = 120;
    const int FRAME_DELAY = 1000 / FPS;
    SDL_Event dummyEvent;
    SDL_PollEvent(&dummyEvent);
    while (m_Bool_Running)
    {
        frameStart = SDL_GetTicks();

        ProcessEvents();
        Render();

        // 精确帧率控制
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL窗口主循环结束");
}

void Ui_DeviceBindingSDL::Close()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: 正在清理SDL资源");

    // 清理设备列表资源
    ClearDevices();

    // 清理光标资源
    if (m_SDL_HandCursor) {
        SDL_FreeCursor(m_SDL_HandCursor);
        m_SDL_HandCursor = nullptr;
    }

    if (m_SDL_DefaultCursor) {
        SDL_FreeCursor(m_SDL_DefaultCursor);
        m_SDL_DefaultCursor = nullptr;
    }

    // 清理关闭按钮纹理
    if (m_SDL_CloseButtonTexture) {
        SDL_DestroyTexture(m_SDL_CloseButtonTexture);
        m_SDL_CloseButtonTexture = nullptr;
    }

    // 清理复选框纹理
    if (m_SDL_CheckboxTexture) {
        SDL_DestroyTexture(m_SDL_CheckboxTexture);
        m_SDL_CheckboxTexture = nullptr;
    }

    if (m_SDL_CheckboxCheckedTexture) {
        SDL_DestroyTexture(m_SDL_CheckboxCheckedTexture);
        m_SDL_CheckboxCheckedTexture = nullptr;
    }

    // 先清空按钮容器，防止后续访问无效的指针
    m_Btn_ControlButtons.clear();

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

    //清理窗口拖拽器
    if (m_WindowDrager)
    {
        delete m_WindowDrager;
    }

    if (m_Bool_SDLInitialized)
    {// 退出SDL视频子系统
        IMG_Quit();        // 退出SDL_image
        // 退出SDL视频子系统
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        // 检查是否还有其他子系统在运行
        if (SDL_WasInit(0) == 0)
        {// 如果没有，完全退出SDL
            SDL_Quit();
        }
        m_Bool_SDLInitialized = false;
    }

    //清理字体
    if (m_TitleText_textSurface)
    {
        SDL_FreeSurface(m_TitleText_textSurface);
        m_TitleText_textSurface = nullptr;
    }
    if (m_TitleText_textTure)
    {
        SDL_DestroyTexture(m_TitleText_textTure);
        m_TitleText_textTure = nullptr;
    }
    if (m_SDLTexture_TipsOfBindsInBottom)
    {
        SDL_DestroyTexture(m_SDLTexture_TipsOfBindsInBottom);
        m_SDLTexture_TipsOfBindsInBottom = nullptr;
    }

    if (m_SDLModelWindow_SureToUnBind)
    {
        delete m_SDLModelWindow_SureToUnBind;
    }
    if (m_SDLModelWindow_UnableBind)
    {
        delete m_SDLModelWindow_SureToUnBind;
    }
    if (m_SDLWindow_SureToClose)
    {
        delete m_SDLWindow_SureToClose;
    }

    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL资源清理完成");
}

SDL_Rect Ui_DeviceBindingSDL::GetUnbindButtonRect(int deviceIndex)
{
    float exactY = m_Int_TitleBarHeight + (deviceIndex * m_Int_DeviceItemHeight) - m_Float_CurrentScroll;

    int btnWidth = (int)(100 * m_Scale);   // 按钮宽度
    int btnHeight = (int)(28 * m_Scale);   // 按钮高度
    int rightPadding = (int)(15 * m_Scale); // 右侧边距

    SDL_Rect rect = {
        m_Int_WindowWidth - btnWidth - rightPadding,
        (int)exactY + (m_Int_DeviceItemHeight - btnHeight) / 2,
        btnWidth,
        btnHeight
    };

    return rect;
}

bool Ui_DeviceBindingSDL::IsPointInUnbindButton(int x, int y)
{
    if (m_Int_HoveredDeviceIndex < 0 || m_Int_HoveredDeviceIndex >= m_Vec_DeviceItems.size()) {
        return false;
    }

    SDL_Rect btnRect = GetUnbindButtonRect(m_Int_HoveredDeviceIndex);
    return (x >= btnRect.x && x < btnRect.x + btnRect.w &&
        y >= btnRect.y && y < btnRect.y + btnRect.h);
}