#include "stdafx.h"
#include "Ui_SDLMeau.h"
#include "GlobalFunc.h"
#include "CDebug.h"
#include "ComponentAPI.h"
extern HANDLE ConsoleHandle;
Ui_MenuButton::Ui_MenuButton(
    const SDL_Rect& rect,
    const std::wstring& text,
    SDL_Color textColor,
    SDL_Color HoverTextColor,
    SDL_Color ClickTextColor,
    SDL_Color BorderCOlor,
    SDL_Color BkColor,
    SDL_Color BkHoverColor,
    SDL_Color BkClickColor,
    int BtnIndex,
    SDL_Renderer* render,
    TTF_Font* font,
    std::function<void(void*)> callback
)
    :
    m_Str_Text(text),
    m_Bool_Hovered(false),
    m_SDL_TextTexture(nullptr),
    m_textColor(textColor),
    m_HoverTextColor(HoverTextColor),
    m_ClickTextColor(ClickTextColor),
    m_BorderColor(BorderCOlor),
    m_BkColor(BkColor),
    m_BkHoverColor(BkHoverColor),
    m_BkClickColor(BkClickColor),
    m_Rect_Area(rect)
{
    // 构造函数初始化按钮属性
    m_Bool_Hovered = false;
    m_Bool_Clicked = false;
    m_IsTextBorder = false;
    m_IsSelected = false;
    m_IsHide = false;
    m_IsInterActive = true;
    m_renderer = render;
    m_ParentFont = font;
    m_thisFont = nullptr;
    m_Func_BtnCallback = callback;
    m_Rect_SelectImg = SDL_Rect();
    m_Index = BtnIndex;
    m_Rect_Text = SDL_Rect();
    if (!m_Str_Text.empty())
        CreateTextTexture();
}

Ui_MenuButton::~Ui_MenuButton()
{
    // 清理文本纹理资源
    if (m_SDL_TextTexture) {
        SDL_DestroyTexture(m_SDL_TextTexture);
        m_SDL_TextTexture = nullptr;
    }
}

void Ui_MenuButton::Render()
{
    // 安全检查
    if (!m_renderer) return;
    if (m_IsHide)return;

    // 绘制按钮背景
    if (m_Bool_Hovered)
    {// 悬停状态
        if (m_Bool_Clicked)
            SDL_SetRenderDrawColor(m_renderer, m_BkClickColor.r, m_BkClickColor.g, m_BkClickColor.b, m_BkClickColor.a);
        else
            SDL_SetRenderDrawColor(m_renderer, m_BkHoverColor.r, m_BkHoverColor.g, m_BkHoverColor.b, m_BkHoverColor.a);
    }
    else
    {// 正常状态
        SDL_SetRenderDrawColor(m_renderer, m_BkColor.r, m_BkColor.g, m_BkColor.b, m_BkColor.a);
    }
    SDL_RenderFillRect(m_renderer, &m_Rect_Area);

    // 绘制按钮边框 
    SDL_SetRenderDrawColor(m_renderer, m_BorderColor.r, m_BorderColor.g, m_BorderColor.b, m_BorderColor.a);
    SDL_RenderDrawRect(m_renderer, &m_Rect_Area);

    // 绘制按钮文本
    if (m_ParentFont && !m_Str_Text.empty())
    {
        // 如果文本纹理不存在，创建它
        if (!m_SDL_TextTexture)
        {
            DB(ConsoleHandle, L"渲染过程中，菜单项按钮文本纹理不存在，创建");
            CreateTextTexture();
        }
        // 如果文本纹理存在，渲染它
        if (m_SDL_TextTexture)
        {
            // 获取文本纹理大小
            int textWidth, textHeight;
            SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);
            if (SDL_RectEmpty(&m_Rect_Text))
            {//如果没有设置文本位置，则居中显示
                SDL_Rect destRect = CalculateCenterRect(textWidth, textHeight);
                SDL_RenderCopy(m_renderer, m_SDL_TextTexture, NULL, &destRect);
            }
            else
            {
                SDL_RenderCopy(m_renderer, m_SDL_TextTexture, NULL, &m_Rect_Text);
            }
        }
    }

    //绘制对勾
    if ((!SDL_RectEmpty(&m_Rect_SelectImg) && m_SDL_SelectTexture) && (m_IsSelected))
    {
        SDL_RenderCopy(m_renderer, m_SDL_SelectTexture, NULL, &m_Rect_SelectImg);
    }

    if (m_Func_ExtraRenderCallback)
    {//如果设置了额外绘制回调
        m_Func_ExtraRenderCallback();
    }
}

SDL_Rect Ui_MenuButton::CalculateCenterRect(int textWidth, int textHeight)
{
    SDL_Rect destRect =
    {
        m_Rect_Area.x + (m_Rect_Area.w - textWidth) / 2,
        m_Rect_Area.y + (m_Rect_Area.h - textHeight) / 2,
        textWidth,
        textHeight
    };
    return destRect;
}

void Ui_MenuButton::CreateTextTexture()
{
    // 转换为UTF-8编码，因为SDL_ttf使用UTF-8
    if (m_SDL_TextTexture)
        SDL_DestroyTexture(m_SDL_TextTexture);
    std::string utf8Text;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        utf8Text.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, m_Str_Text.c_str(), -1, &utf8Text[0], utf8Size, NULL, NULL);
    }

    // 创建文字表面
    SDL_Surface* textSurface = nullptr;
    if (m_Bool_Clicked)
        textSurface = TTF_RenderUTF8_Blended(
            ((m_thisFont == nullptr) ? m_ParentFont : m_thisFont),
            utf8Text.c_str(),
            m_ClickTextColor
        );
    else if (m_Bool_Hovered)
        textSurface = TTF_RenderUTF8_Blended(
            ((m_thisFont == nullptr) ? m_ParentFont : m_thisFont),
            utf8Text.c_str(),
            m_HoverTextColor
        );
    else
        textSurface = TTF_RenderUTF8_Blended(
            ((m_thisFont == nullptr) ? m_ParentFont : m_thisFont),
            utf8Text.c_str(),
            m_textColor
        );
    if (textSurface)
    {// 创建文字纹理
        m_SDL_TextTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
        if(m_SDL_TextTexture)
            SDL_SetTextureBlendMode(m_SDL_TextTexture, SDL_BLENDMODE_BLEND);
        else
            DB(ConsoleHandle, L"菜单按钮开始文字纹理Texture创建失败!");
        SDL_FreeSurface(textSurface);
    }
    else
    {
        DB(ConsoleHandle, L"菜单按钮开始文字纹理表面创建失败!");
    }
}

int Ui_MenuButton::IsPointInside(int x, int y) const
{
    // 检查坐标是否在按钮区域内
    return (x >= m_Rect_Area.x && x < m_Rect_Area.x + m_Rect_Area.w &&
        y >= m_Rect_Area.y && y < m_Rect_Area.y + m_Rect_Area.h);
}

void Ui_MenuButton::IsPosOnBtn(int x, int y, bool* IsOnMouse)
{
    // 更新按钮状态
    if (m_IsInterActive)
        *IsOnMouse = IsPointInside(x, y);
    else
        *IsOnMouse = false;
}

void Ui_MenuButton::Click(void* param)
{
    if (m_IsHide)
        return;
    // 执行点击回调
    if (m_Func_BtnCallback) {
        m_Func_BtnCallback(param);
    }
}

void Ui_MenuButton::SetBtnText(std::wstring BtnText)
{
    m_Str_Text = BtnText;
    CreateTextTexture();
}

void Ui_MenuButton::AdjustTextRect(SDL_Rect TextRect)
{
    m_Rect_Text = TextRect;
}

void Ui_MenuButton::SetExtraRender(std::function<void()> extraCallback)
{
    m_Func_ExtraRenderCallback = extraCallback;
}

void Ui_MenuButton::SetTextFont(int size, bool border, std::string family)
{
    if (family.empty())
    {
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
        family = utf8Path;
    }
    m_thisFont = TTF_OpenFont(family.c_str(), size);
    if (!m_thisFont)
        DB(ConsoleHandle, L"错误!  按钮字体加载失败");
    else
        DB(ConsoleHandle, L"按钮字体加载成功");
}

void Ui_MenuButton::SetNoInterActive(bool NoInterActice)
{
    m_IsInterActive = !NoInterActice;
}

std::wstring Ui_MenuButton::GetBtnText()
{
    return m_Str_Text;
}

void Ui_MenuButton::GetTextSize(int* Width, int* Height)
{
    if (!m_Str_Text.empty())
    {
        SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, Width, Height);
    }
    else
    {
        *Width = -1;
        *Height = -1;
    }
}

void Ui_MenuButton::UpdateBtnState(bool IsHover, bool IsClicked)
{
    m_Bool_Hovered = IsHover;
    m_Bool_Clicked = IsClicked;
}

void Ui_MenuButton::GetBtnState(bool* IsHover, bool* IsClicked)
{
    if (IsClicked)
        *IsClicked = m_Bool_Clicked;
    if (IsHover)
        *IsHover = m_Bool_Hovered;
}

void Ui_MenuButton::SetTransparentParam(int Alpha)
{
    m_Alpha = Alpha;
    m_textColor.a = static_cast<Uint8>(m_Alpha);
    m_HoverTextColor.a = static_cast<Uint8>(m_Alpha);
    m_ClickTextColor.a = static_cast<Uint8>(m_Alpha);
    m_BorderColor.a = static_cast<Uint8>(m_Alpha);
    m_BkHoverColor.a = static_cast<Uint8>(m_Alpha);
    m_BkClickColor.a = static_cast<Uint8>(m_Alpha);
    m_BkColor.a = static_cast<Uint8>(m_Alpha);
}

SDL_Rect Ui_MenuButton::GetTextRect()
{
    if (SDL_RectEmpty(&m_Rect_Text) && m_SDL_TextTexture)
    {
        int textWidth, textHeight;
        SDL_Rect destRect;
        SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);
        return CalculateCenterRect(textWidth, textHeight);
    }
    else if (!SDL_RectEmpty(&m_Rect_Text) && m_SDL_TextTexture)
        return m_Rect_Text;
    else
        return SDL_Rect();
}

int Ui_MenuButton::GetBtnIndex()
{
    return m_Index;
}

void Ui_MenuButton::ShowBtn()
{
    m_IsHide = false;
}

void Ui_MenuButton::HideBtn()
{
    m_IsHide = true;
}

bool Ui_MenuButton::AddExtraRenderImage(CString Image, SDL_Rect ImageRect)
{
    if (!COMAPI::SDL::ImageLoad(m_renderer, Image, &m_SDL_SelectTexture))
    {
        DB(ConsoleHandle, L"选中图片纹理加载失败 ");
        return false;
    }
    m_Rect_SelectImg = ImageRect;
    return true;
}

void Ui_MenuButton::SetExtraImageShowState(bool IsSelect)
{
    m_IsSelected = IsSelect;
}

void Ui_MenuButton::MoveToXY(int x, int y)
{
    m_Rect_Area.x = x;
    m_Rect_Area.y = y;
    m_Rect_SelectImg.x = x;
    m_Rect_SelectImg.y = m_Rect_Area.y + (m_Rect_Area.h - m_Rect_SelectImg.h)/2;
    m_Rect_Text.x = x;
    m_Rect_Text.y = y;
}

Ui_SDLMeau::Ui_SDLMeau(
    SDL_Rect MenuRect,
    int ItemHeight,
    SDL_Color BkColor,
    SDL_Color BorderColor,
    SDL_Renderer* pParentRender,
    TTF_Font* pParentFont
)
{
    m_Int_MeauWidth = MenuRect.w;
    m_Int_MeauHeight = MenuRect.h;
    m_Rect_Menu = MenuRect;
    m_Int_ItemHeight = ItemHeight;
    m_Renderer = pParentRender;
    m_ParentFont = pParentFont;
    m_Color_BkColor = BkColor;
    m_Color_Border = BorderColor;
    m_thisFont = nullptr;
    m_IsHide = false;
    m_IsMouseEnteredMenu = false;
    m_LastHoverIndex = -1;
}

Ui_MenuButton* Ui_SDLMeau::AddMenuBtn(const SDL_Rect& rect, const std::wstring& text, SDL_Color textColor, SDL_Color HoverTextColor, SDL_Color ClickTextColor, SDL_Color BorderCOlor, SDL_Color BkColor, SDL_Color BkHoverColor, SDL_Color BkClickColor, int BtnIndex,std::function<void(void*)> callback)
{
    Ui_MenuButton* btn = new Ui_MenuButton(
        rect,
        text, textColor, HoverTextColor, ClickTextColor,
        BorderCOlor,
        BkColor, BkHoverColor, BkClickColor, BtnIndex,
        m_Renderer, m_thisFont == nullptr ? m_ParentFont : m_thisFont,
        callback
    );
    if(btn)
        m_Vec_MeauBtns.push_back(btn);
    return btn;
}

Ui_MenuButton* Ui_SDLMeau::GetBtnByIndex(int index)
{
    for (const auto& btn : m_Vec_MeauBtns)
    {
        if (index == btn->GetBtnIndex())
            return btn;
    }
    return nullptr;
}

void Ui_SDLMeau::UpdateMenuBtnState(int x, int y, bool IsClickOn)
{
    if (m_IsHide)
        return;
    bool mouseOnMenu = false;
    for (const auto& btn : m_Vec_MeauBtns)
    {
        bool IsMouseOnBtn = false;
        btn->IsPosOnBtn(x, y, &IsMouseOnBtn);
        if (IsMouseOnBtn)
        {
            btn->UpdateBtnState(true, IsClickOn);
            m_LastHoverIndex = btn->GetBtnIndex();
        }
        else
            btn->UpdateBtnState(false, false);

        if (IsMouseOnBtn && !m_IsMouseEnteredMenu)
            m_IsMouseEnteredMenu = true;
    }
}

Ui_MenuButton* Ui_SDLMeau::GetHoverBtn()
{
    bool isHovered = false;
    bool isClicked = false;
    for (const auto& btn : m_Vec_MeauBtns)
    {
        btn->GetBtnState(&isHovered, &isClicked);
        if (isHovered || isClicked)
            return btn;
    }
    return nullptr;
}

void Ui_SDLMeau::Render()
{
    if (m_IsHide)
        return;
    SDL_Color OriColor;
    SDL_GetRenderDrawColor(m_Renderer, &OriColor.r, &OriColor.g, &OriColor.b, &OriColor.a);
    SDL_SetRenderDrawColor(m_Renderer, m_Color_BkColor.r, m_Color_BkColor.g, m_Color_BkColor.b, m_Color_BkColor.a);
    int BorderSize = 2;
    SDL_RenderFillRect(m_Renderer, &m_Rect_Menu);
    for (const auto& btn : m_Vec_MeauBtns)
    {
        btn->Render();
    }
    SDL_SetRenderDrawColor(m_Renderer, m_Color_Border.r, m_Color_Border.g, m_Color_Border.b, m_Color_Border.a);
    for (size_t i = 0; i < BorderSize; i++)
    {
        SDL_Rect BorderRect
        {
            m_Rect_Menu.x + i,
            m_Rect_Menu.y + i,
            m_Rect_Menu.w - i * 2,
            m_Rect_Menu.h - i * 2
        };
        SDL_RenderDrawRect(m_Renderer, &BorderRect);
    }
    SDL_SetRenderDrawColor(m_Renderer, OriColor.r, OriColor.g, OriColor.b, OriColor.a);
}

void Ui_SDLMeau::MoveMenuToXY(int x, int y)
{
    m_Rect_Menu.x = x;
    m_Rect_Menu.y = y;
    int Size = m_Vec_MeauBtns.size();
    for (size_t i = 0; i < Size; i++)
    {
        m_Vec_MeauBtns.at(i)->MoveToXY(x, y + i * m_Int_ItemHeight);
    }
}

void Ui_SDLMeau::ShowMenu()
{
    for (const auto& btns : m_Vec_MeauBtns)
    {
        btns->ShowBtn();
    }
    m_IsHide = false;
}

void Ui_SDLMeau::HideMenu()
{
    for (const auto& btns : m_Vec_MeauBtns)
    {
        btns->HideBtn();
    }
    m_IsHide = true;
    m_IsMouseEnteredMenu = false;
}

bool Ui_SDLMeau::IsMouseEnteredMenu()const
{
    return m_IsMouseEnteredMenu;
}

void Ui_SDLMeau::SetTransparentParam(int Alpha)
{
    m_Alpha = Alpha;
    for (const auto& btn : m_Vec_MeauBtns)
    {
        btn->SetTransparentParam(Alpha);
    }
}

void Ui_SDLMeau::ResetAllMenuBtnState()
{
    for (const auto& btn : m_Vec_MeauBtns)
    {
        btn->SetExtraImageShowState(false);
    }
}

int Ui_SDLMeau::GetLastHoverIndex()
{
    return m_LastHoverIndex;
}
