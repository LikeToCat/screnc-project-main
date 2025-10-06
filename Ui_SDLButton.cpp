#include "stdafx.h"
#include "Ui_SDLButton.h"
#include "GlobalFunc.h"
#include "ComponentAPI.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;

Ui_SDLButton::Ui_SDLButton(
    const SDL_Rect& rect,
    const std::wstring& text,
    SDL_Color textColor,
    SDL_Color HoverTextColor,
    SDL_Color ClickTextColor,
    SDL_Color BorderCOlor,
    SDL_Color BkColor,
    SDL_Color BkHoverColor,
    SDL_Color BkClickColor,
    CString ImagePath,
    SDL_Renderer* render,
    TTF_Font* font,
    std::function<void()> callback,
    int Uid,
    int cornerRadius
)
    : m_Rect_Area(rect),
    m_Str_Text(text),
    m_Func_Callback(callback),
    m_Bool_Hovered(false),
    m_SDL_TextTexture(nullptr),
    m_textColor(textColor),
    m_HoverTextColor(HoverTextColor),
    m_ClickTextColor(ClickTextColor),
    m_BorderColor(BorderCOlor),
    m_BkColor(BkColor),
    m_BkHoverColor(BkHoverColor),
    m_BkClickColor(BkClickColor),
    m_StretchParam(-1),
    m_IsNeedStretch(false),
    m_Aplha(255),
    m_Uid(Uid),
    m_CornerRadius(cornerRadius)
{
    // 构造函数初始化按钮属性
    m_Bool_Hovered = false;
    m_Bool_Clicked = false;
    m_IsTextBorder = false;
    m_IsInterActive = true;
    m_IsHide = false;
    m_IsGradient = false;
    m_renderer = render;
    m_ParentFont = font;
    m_Rect_Image = SDL_Rect();
    m_Rect_Text = SDL_Rect();
    m_thisFont = nullptr;
    m_SDL_ImageTexture = nullptr;
    m_wsBtnDesc = L"";
    m_gdColor1 = SDL_Color();
    m_gdColor2 = SDL_Color();                     
    m_hgdColor1 = SDL_Color();                    
    m_hgdColor2 = SDL_Color();                    
    if (ImagePath != L"")
    {
        ImagePath = GlobalFunc::GetExecutablePathFolder() + ImagePath;
        std::string ImagePathUtf8 = GlobalFunc::ConvertPathToUtf8(ImagePath);
        int imgFlags = IMG_INIT_PNG;
        if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL_image初始化失败");
        }
        else
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL设备绑定窗口]: SDL_image初始化成功");
            SDL_Surface* ImageSurface = IMG_Load(ImagePathUtf8.c_str());
            if (ImageSurface)
            {
                m_SDL_ImageTexture = SDL_CreateTextureFromSurface(m_renderer, ImageSurface);
                if (!m_SDL_ImageTexture)
                    DB(ConsoleHandle, L"按钮图片资源纹理加载失败!");
            }
            else
            {
                DB(ConsoleHandle, L"按钮图片资源加载失败!");
            }
        }
    }
    if (!m_Str_Text.empty())
        CreateTextTexture();
}

Ui_SDLButton::~Ui_SDLButton()
{
    // 清理文本纹理资源
    if (m_SDL_TextTexture) {
        SDL_DestroyTexture(m_SDL_TextTexture);
        m_SDL_TextTexture = nullptr;
    }
}

void Ui_SDLButton::Render()
{
    // 安全检查
    if (!m_renderer) return;
    if (m_IsHide)return;
    if (!m_IsGradient)
    {
        // 确定背景颜色
        SDL_Color backgroundColor;
        if (m_Bool_Hovered)
        {
            if (m_Bool_Clicked)
                backgroundColor = m_BkClickColor;
            else
                backgroundColor = m_BkHoverColor;
        }
        else
        {
            backgroundColor = m_BkColor;
        }

        // 根据是否有圆角来绘制不同形状的按钮
        if (m_CornerRadius <= 0)
        {
            // 正常矩形按钮
            SDL_SetRenderDrawColor(m_renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
            SDL_RenderFillRect(m_renderer, &m_Rect_Area);

            // 绘制边框
            SDL_SetRenderDrawColor(m_renderer, m_BorderColor.r, m_BorderColor.g, m_BorderColor.b, m_BorderColor.a);
            SDL_RenderDrawRect(m_renderer, &m_Rect_Area);
        }
        else
        {
            // 绘制圆角按钮背景
            roundedBoxRGBA(m_renderer,
                m_Rect_Area.x - 2, m_Rect_Area.y - 2, m_Rect_Area.x + m_Rect_Area.w + 2, m_Rect_Area.y + m_Rect_Area.h + 2,
                m_CornerRadius,
                backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a
            );

            //绘制圆角按钮边框
            for (size_t i = 1; i <= 2; i++)
            {
                int x1 = m_Rect_Area.x - i;
                int y1 = m_Rect_Area.y - i;
                int x2 = m_Rect_Area.x + m_Rect_Area.w + i;
                int y2 = m_Rect_Area.y + m_Rect_Area.h + i;
                roundedRectangleRGBA(
                    m_renderer,
                    x1, y1, x2, y2,
                    m_CornerRadius,
                    m_BorderColor.r, m_BorderColor.g, m_BorderColor.b, m_BorderColor.a
                );
            }
        }
    }
    else
    {
        SDL_Color gb1, gb2;
        // 回退到原有逻辑
        if (m_Bool_Clicked || m_Bool_Hovered)
        {
            gb1 = m_hgdColor1;
            gb2 = m_hgdColor2;
        }
        else
        {
            gb1 = m_gdColor1;
            gb2 = m_gdColor2;
        }
        // 渲染渐变
        COMAPI::SDL_RenderAPI::renderHorizontalGradient(m_renderer, m_Rect_Area, gb1, gb2, m_CornerRadius);
    }

    // 绘制按钮图片
    if (m_SDL_ImageTexture)
    {
        if (SDL_RectEmpty(&m_Rect_Image))
        {//按钮区域未进行设置，则判断是需要拉伸填充还是默认居中
            if (m_IsNeedStretch)
            {//填充
                SDL_Rect ImageRect = m_StretchParam != -1 ? Ui_SDLButton::ScaleRect(m_Rect_Area, m_StretchParam) : m_Rect_Area;
                SDL_RenderCopy(m_renderer, m_SDL_ImageTexture, NULL, &ImageRect);
            }
            else
            {//居中
                int imageWidth, imageHeight;
                SDL_Rect destRect;
                SDL_QueryTexture(m_SDL_ImageTexture, NULL, NULL, &imageWidth, &imageHeight);
                SDL_RenderCopy(m_renderer, m_SDL_ImageTexture, NULL, &CalculateCenterRect(imageWidth, imageHeight));
            }
        }
        else
        {
            SDL_RenderCopy(m_renderer, m_SDL_ImageTexture, NULL, &m_Rect_Image);
        }
    }

    // 绘制按钮文本
    if (m_ParentFont && !m_Str_Text.empty())
    {
        // 如果文本纹理不存在，创建它
        CreateTextTexture();
        // 如果文本纹理存在，渲染它
        if (m_SDL_TextTexture)
        {
            // 获取文本纹理大小
            int textWidth, textHeight;
            SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);
            if (SDL_RectEmpty(&m_Rect_Text))
            {//如果没有设置文本位置，则居中显示
                SDL_Rect destRect;
                SDL_RenderCopy(m_renderer, m_SDL_TextTexture, NULL, &CalculateCenterRect(textWidth, textHeight));
            }
            else
            {
                SDL_RenderCopy(m_renderer, m_SDL_TextTexture, NULL, &m_Rect_Text);
            }
        }
    }

    if (m_Func_ExtraRenderCallback)
    {//如果设置了额外绘制内容回调
        m_Func_ExtraRenderCallback();
    }

}

SDL_Rect Ui_SDLButton::CalculateCenterRect(int textWidth, int textHeight)
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

void Ui_SDLButton::CreateTextTexture()
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
        SDL_FreeSurface(textSurface);
    }
}

void Ui_SDLButton::UpdateClientRect()
{
    // 初始化一个空的矩形作为基准
    SDL_Rect boundingRect = { 0, 0, 0, 0 };
    bool hasValidRect = false;

    // 获取有效的图像矩形（如果存在）
    SDL_Rect validImageRect;
    if (!SDL_RectEmpty(&m_Rect_Image) && m_SDL_ImageTexture)
    {
        validImageRect = m_Rect_Image;
        boundingRect = validImageRect;
        hasValidRect = true;
    }
    else if (m_SDL_ImageTexture)
    {
        // 如果图像矩形未设置但纹理存在，获取图像的实际大小并计算居中位置
        int imageWidth, imageHeight;
        SDL_QueryTexture(m_SDL_ImageTexture, NULL, NULL, &imageWidth, &imageHeight);
        validImageRect = CalculateCenterRect(imageWidth, imageHeight);
        boundingRect = validImageRect;
        hasValidRect = true;
    }

    // 获取有效的文本矩形（如果存在）
    SDL_Rect validTextRect;
    if (!SDL_RectEmpty(&m_Rect_Text) && m_SDL_TextTexture)
    {
        validTextRect = m_Rect_Text;

        if (hasValidRect)
        {
            // 如果已经有有效矩形，计算包含两者的最小矩形
            int left = min(boundingRect.x, validTextRect.x);
            int top = min(boundingRect.y, validTextRect.y);
            int right = max(boundingRect.x + boundingRect.w, validTextRect.x + validTextRect.w);
            int bottom = max(boundingRect.y + boundingRect.h, validTextRect.y + validTextRect.h);

            boundingRect.x = left;
            boundingRect.y = top;
            boundingRect.w = right - left;
            boundingRect.h = bottom - top;
        }
        else
        {
            boundingRect = validTextRect;
            hasValidRect = true;
        }
    }
    else if (m_SDL_TextTexture)
    {
        // 如果文本矩形未设置但纹理存在，获取文本的实际大小并计算居中位置
        int textWidth, textHeight;
        SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);
        validTextRect = CalculateCenterRect(textWidth, textHeight);

        if (hasValidRect)
        {
            // 如果已经有有效矩形，计算包含两者的最小矩形
            int left = min(boundingRect.x, validTextRect.x);
            int top = min(boundingRect.y, validTextRect.y);
            int right = max(boundingRect.x + boundingRect.w, validTextRect.x + validTextRect.w);
            int bottom = max(boundingRect.y + boundingRect.h, validTextRect.y + validTextRect.h);

            boundingRect.x = left;
            boundingRect.y = top;
            boundingRect.w = right - left;
            boundingRect.h = bottom - top;
        }
        else
        {
            boundingRect = validTextRect;
            hasValidRect = true;
        }
    }

    // 添加一些额外的边距
    if (hasValidRect)
    {
        const int padding = 1;
        boundingRect.x -= padding;
        boundingRect.y -= padding;
        boundingRect.w += padding * 2;
        boundingRect.h += padding * 2;

        // 更新按钮区域
        m_Rect_Area = boundingRect;
    }
    // 如果没有有效的矩形，保持m_Rect_Area不变
}

SDL_Rect Ui_SDLButton::ScaleRect(SDL_Rect original, float scale_factor)
{
    SDL_Rect scaled;

    // Scale from the center of the rectangle
    int center_x = original.x + original.w / 2;
    int center_y = original.y + original.h / 2;

    // Calculate new width and height
    scaled.w = (int)(original.w * scale_factor);
    scaled.h = (int)(original.h * scale_factor);

    // Calculate new x and y to keep the rectangle centered
    scaled.x = center_x - scaled.w / 2;
    scaled.y = center_y - scaled.h / 2;

    return scaled;
}

bool Ui_SDLButton::IsPointInside(int x, int y) const
{
    // 检查坐标是否在按钮区域内
    return (x >= m_Rect_Area.x && x < m_Rect_Area.x + m_Rect_Area.w &&
        y >= m_Rect_Area.y && y < m_Rect_Area.y + m_Rect_Area.h);
}

void Ui_SDLButton::IsPosOnBtn(int x, int y, bool* IsOnMouse)
{
    if (m_IsHide)
    {
        *IsOnMouse = false;
        return;
    }

    // 更新按钮状态
    if (m_IsInterActive)
        *IsOnMouse = IsPointInside(x, y);
    else
        *IsOnMouse = false;
}

void Ui_SDLButton::Click()
{
    if (m_IsHide)
        return;
    // 执行点击回调
    if (m_Func_Callback) {
        m_Func_Callback();
    }
}

void Ui_SDLButton::SetBtnText(std::wstring BtnText)
{
    m_Str_Text = BtnText;
    CreateTextTexture();
}

void Ui_SDLButton::AdjustTextRect(SDL_Rect TextRect, bool ClientRectAdapt)
{
    m_Rect_Text = TextRect;
    if(ClientRectAdapt)
        UpdateClientRect();
}

void Ui_SDLButton::AdjustTextRect(int offx, int offy)
{
    int textWidth, textHeight;
    SDL_QueryTexture(m_SDL_TextTexture, NULL, NULL, &textWidth, &textHeight);
    m_Rect_Text = CalculateCenterRect(textWidth, textHeight);
    m_Rect_Text.x += offx;
    m_Rect_Text.y += offy;
}

void Ui_SDLButton::AdjustImageRect(SDL_Rect ImageRect, bool ClientRectAdapt)
{
    m_Rect_Image = ImageRect;
    if(ClientRectAdapt)
        UpdateClientRect();
}

void Ui_SDLButton::SetImageStretchParam(float StretchParam)
{
    m_IsNeedStretch = true;
    m_StretchParam = StretchParam;
}

void Ui_SDLButton::SetImageNeedStretch(bool Enable)
{
    m_IsNeedStretch = Enable;
}

void Ui_SDLButton::SetExtraRender(std::function<void()> extraCallback)
{
    m_Func_ExtraRenderCallback = extraCallback;
}

void Ui_SDLButton::SetTextFont(int size, bool border, std::string family)
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

void Ui_SDLButton::SetNoInterActive(bool NoInterActice)
{
    m_IsInterActive = !NoInterActice;
}

void Ui_SDLButton::SetTransparentParam(int Aplha)
{
    m_Aplha = Aplha;
    m_textColor.a = static_cast<Uint8>(m_Aplha);
    m_HoverTextColor.a = static_cast<Uint8>(m_Aplha);
    m_ClickTextColor.a = static_cast<Uint8>(m_Aplha);
    m_BorderColor.a = static_cast<Uint8>(m_Aplha);
    m_BkHoverColor.a = static_cast<Uint8>(m_Aplha);
    m_BkClickColor.a = static_cast<Uint8>(m_Aplha);
    m_BkColor.a = static_cast<Uint8>(m_Aplha);
}

void Ui_SDLButton::SetGdColor(SDL_Color gd1, SDL_Color gd2)
{
    m_IsGradient = true;
    m_gdColor1 = gd1;
    m_gdColor2 = gd2;
    m_hgdColor1 = gd1;
    m_hgdColor2 = gd2;
}

void Ui_SDLButton::SetHGdColor(SDL_Color hgd1, SDL_Color hgd2)
{
    m_IsGradient = true;
    m_hgdColor1 = hgd1;
    m_hgdColor2 = hgd2;
}

SDL_Rect Ui_SDLButton::CalculateCenterRect(SDL_Rect srcRect)
{
    SDL_Rect destRect;

    // 计算按钮区域的中心点
    int centerX = m_Rect_Area.x + m_Rect_Area.w / 2;
    int centerY = m_Rect_Area.y + m_Rect_Area.h / 2;

    // 计算源矩形在目标区域中居中时的左上角坐标
    destRect.x = centerX - srcRect.w / 2;
    destRect.y = centerY - srcRect.h / 2;

    // 保持源矩形的宽高不变
    destRect.w = srcRect.w;
    destRect.h = srcRect.h;

    return destRect;
}

std::wstring Ui_SDLButton::GetBtnText()
{
    return m_Str_Text;
}

const SDL_Rect& Ui_SDLButton::GetBtnRect()
{
    return m_Rect_Area;
}

void Ui_SDLButton::GetImageSize(int* Width, int* Height)
{
    if (m_SDL_ImageTexture)
    {
        SDL_QueryTexture(m_SDL_ImageTexture, NULL, NULL, Width, Height);
    }
    else
    {
        *Width = -1;
        *Height = -1;
    }
}

void Ui_SDLButton::GetTextSize(int* Width, int* Height)
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

void Ui_SDLButton::UpdateBtnState(bool IsHover, bool IsClicked)
{
    m_Bool_Hovered = IsHover;
    m_Bool_Clicked = IsClicked;
}

void Ui_SDLButton::AdjustClientAreaByDiff(int diffx, int diffy,int extendx,int extendy)
{
    m_Rect_Area.x += diffx;
    m_Rect_Area.y += diffy;
    m_Rect_Area.w += extendx;
    m_Rect_Area.h += extendy;
}

void Ui_SDLButton::GetBtnState(bool* IsHover, bool* IsClicked)
{
    if(IsClicked)
        *IsClicked = m_Bool_Clicked;
    if(IsHover)
        *IsHover = m_Bool_Hovered;
}

void Ui_SDLButton::MoveBtnToXY(int x, int y)
{
    m_Rect_Area.x = x;
    m_Rect_Area.y = y;
}

void Ui_SDLButton::MoveBtn(int left, int top, int width, int height)
{
    m_Rect_Area.x = left;
    m_Rect_Area.y = top;
    m_Rect_Area.w = width;
    m_Rect_Area.h = height;
}

SDL_Rect Ui_SDLButton::GetImageRect()
{
    if (SDL_RectEmpty(&m_Rect_Image) && m_SDL_ImageTexture)
    {
        int imageWidth, imageHeight;
        SDL_Rect destRect;
        SDL_QueryTexture(m_SDL_ImageTexture, NULL, NULL, &imageWidth, &imageHeight);
        return CalculateCenterRect(imageWidth, imageHeight);
    }
    else if (!SDL_RectEmpty(&m_Rect_Image) && m_SDL_ImageTexture)
        return m_Rect_Image;
    else
        return SDL_Rect();
}

SDL_Rect Ui_SDLButton::GetTextRect()
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

int Ui_SDLButton::GetUid()
{
    return m_Uid;
}

void Ui_SDLButton::HideBtn()
{
    m_IsHide = true;
}

void Ui_SDLButton::ShowBtn()
{
    m_IsHide = false;
}

void Ui_SDLButton::SetBtnDesc(std::wstring btndesc)
{
    m_wsBtnDesc = btndesc;
}

std::wstring Ui_SDLButton::GetBtnDesc()
{
    return m_wsBtnDesc;
}

void Ui_SDLButtonManager::CreateBtn(
    const SDL_Rect& rect,
    const std::wstring& text,
    SDL_Color textColor,
    SDL_Color HoverTextColor,
    SDL_Color ClickTextColor,
    SDL_Color BorderCOlor,
    SDL_Color BkColor,
    SDL_Color BkHoverColor,
    SDL_Color BkClickColor,
    CString ImagePath,
    SDL_Renderer* render,
    TTF_Font* font, 
    std::function<void()> callback,
    int Uid,
    int cornerRadius
)
{
    m_Vec_Btns.push_back(new Ui_SDLButton(
        rect,
        text, textColor, HoverTextColor, ClickTextColor,
        BorderCOlor,
        BkColor, BkHoverColor, BkClickColor,
        ImagePath,
        render, font,
        callback,
        Uid,
        cornerRadius
    ));
}

Ui_SDLButtonManager::~Ui_SDLButtonManager()
{
    for (auto btn : m_Vec_Btns)
    {
        if (btn != nullptr) 
        {
            delete btn;   
            btn = nullptr; 
        }
    }
    m_Vec_Btns.clear();
}

void Ui_SDLButtonManager::AddBtn(Ui_SDLButton* btn)
{
    m_Vec_Btns.push_back(btn);
}

std::vector<Ui_SDLButton*>& Ui_SDLButtonManager::GetBtns()
{
    return m_Vec_Btns;
}

bool Ui_SDLButtonManager::CheckPosInAnyBtn(int x, int y)
{
    bool IsMouseOnBtn = false;
    for (const auto& btn : m_Vec_Btns)
    {
        btn->IsPosOnBtn(x, y, &IsMouseOnBtn);
        if (IsMouseOnBtn)
            break;
    }
    return IsMouseOnBtn;
}

void Ui_SDLButtonManager::UpdateBtnsState(int x, int y, bool updateClick)
{
    for (std::vector<Ui_SDLButton*>::iterator it = m_Vec_Btns.begin(); it < m_Vec_Btns.end(); ++it)
    {
        bool IsMouseOnBtn = false;
        (*it)->IsPosOnBtn(x, y, &IsMouseOnBtn);
        if (IsMouseOnBtn)
            (*it)->UpdateBtnState(true, updateClick);
        else
            (*it)->UpdateBtnState(false, false);
    }
}

Ui_SDLButton* Ui_SDLButtonManager::GetInHoverOrClickedBtn()
{
    bool IsHover = false;
    bool IsClicked = false;
    for (std::vector<Ui_SDLButton*>::iterator it = m_Vec_Btns.begin(); it < m_Vec_Btns.end(); ++it)
    {
        (*it)->GetBtnState(&IsHover, &IsClicked);
        if(IsHover || IsClicked)
            return (*it);
    }
    return nullptr;
}

Ui_SDLButton* Ui_SDLButtonManager::FindBtnByBtnText(std::wstring BtnText)
{
    for (std::vector<Ui_SDLButton*>::iterator it = m_Vec_Btns.begin(); it < m_Vec_Btns.end(); ++it)
    {
        if ((*it)->GetBtnText() == BtnText)
            return (*it);
    }
    return nullptr;
}

Ui_SDLButton* Ui_SDLButtonManager::FindBtnByUid(int uid)
{
    for (const auto& btn : m_Vec_Btns)
    {
        if (btn->GetUid() == uid)
            return btn;
    }
    return nullptr;
}

void Ui_SDLButtonManager::RenderBtns()
{
    for (std::vector<Ui_SDLButton*>::iterator it = m_Vec_Btns.begin(); it < m_Vec_Btns.end(); ++it)
    {
        (*it)->Render();
    }
}

void Ui_SDLButtonManager::SetTransparentParam(int alpha)
{
    for (const auto& btn : m_Vec_Btns)
    {
        btn->SetTransparentParam(alpha);
    }
}

