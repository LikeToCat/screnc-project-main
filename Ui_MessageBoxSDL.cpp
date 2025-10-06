#include "stdafx.h"
#include "Ui_MessageBoxSDL.h"
#include "GlobalFunc.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;
const char* MSYH_FONT_PATH = "C:/Windows/Fonts/msyh.ttc";
const char* MSYH_FONT_PATH_BOLD = "C:/Windows/Fonts/msyhbd.ttc";
Ui_MessageBoxSDL::Ui_MessageBoxSDL(
    SDL_Renderer* renderer, 
    const std::string& title, 
    const std::string& message, 
    const std::string& type, 
    const char* iconPath,
    float Scale,
    int cornerRadius
)
    : m_renderer(renderer), 
      m_title(title), 
      m_message(message),
      m_MessageType(type), 
      m_IconPath(iconPath),
      m_isShown(false),
      m_result(-1), 
      m_dimTexture(nullptr),
      m_callback(nullptr),
      m_Scale(Scale),
      m_cornerRadius(cornerRadius)
{
    // 根据父窗口渲染器配置，设置对话框样式和位置
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    m_width = (420 * m_Scale) < (windowWidth - 40) ? (420 * m_Scale) : (windowWidth - 40);
    m_height = 223 * m_Scale; // 额外空间用于标题和按钮
    m_TestSize = 16;
    // 居中对话框
    m_x = (windowWidth - m_width) / 2;
    m_y = (windowHeight - m_height) / 2;
    AddButton("确定", 1); // 添加"确定"按钮
    AddButton("返回", 0); // 添加"返回"按钮
}

Ui_MessageBoxSDL::~Ui_MessageBoxSDL()
{
    Cleanup();
}

void Ui_MessageBoxSDL::AddButton(const std::string& text, int returnValue)
{
    Button btn;
    btn.text = text;
    btn.returnValue = returnValue;
    m_buttons.push_back(btn);
}

void Ui_MessageBoxSDL::Initialize()
{
    if (m_dimTexture) 
        return;
    //加载鼠标形状
    m_NormalCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_ClickCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    if (!m_NormalCursor || !m_ClickCursor)
        DEBUG_CONSOLE_STR(ConsoleHandle, L"对话框加载鼠标形状鼠标");

    //创建字体
    m_font = TTF_OpenFont(MSYH_FONT_PATH, m_TestSize * m_Scale);
    m_BoldFont = TTF_OpenFont(MSYH_FONT_PATH_BOLD, m_TestSize * m_Scale);
    if (!m_font)
        DEBUG_CONSOLE_STR(ConsoleHandle, L"字体创建失败");
    if(!m_BoldFont)
        DEBUG_CONSOLE_STR(ConsoleHandle, L"粗体字体创建失败");
    

    //加载弹框类型资源图标
    std::string iconPath = GlobalFunc::AnsiToUtf8(m_IconPath);
    SDL_Surface* iconSurface = IMG_Load(iconPath.c_str());
    m_IconWidth = iconSurface->w;
    m_IconHeight = iconSurface->h;
    if (!iconSurface)
        DEBUG_CONSOLE_STR(ConsoleHandle, L"SDL对话框图标资源加载失败");
    m_Icon = SDL_CreateTextureFromSurface(m_renderer, iconSurface);
    if(!m_Icon)
        DEBUG_CONSOLE_STR(ConsoleHandle, L"SDL对话框图标资源创建纹理失败");

    // 获取渲染器尺寸
    int width, height;
    SDL_GetRendererOutputSize(m_renderer, &width, &height);

    // 创建半透明背景纹理
    m_dimTexture = SDL_CreateTexture(m_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width, height);

    if (m_dimTexture) 
    {
        // 设置混合模式
        SDL_SetTextureBlendMode(m_dimTexture, SDL_BLENDMODE_BLEND);

        // 将渲染目标设置为纹理
        SDL_Texture* oldTarget = SDL_GetRenderTarget(m_renderer);
        SDL_SetRenderTarget(m_renderer, m_dimTexture);

        // 填充半透明黑色
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 180);
        SDL_RenderClear(m_renderer);
        SDL_SetRenderTarget(m_renderer, oldTarget);        // 恢复渲染目标
    }
}

void Ui_MessageBoxSDL::Cleanup()
{
    if (m_dimTexture)
    {
        SDL_DestroyTexture(m_dimTexture);
        m_dimTexture = nullptr;
    }
    if (m_Icon)
    {
        SDL_DestroyTexture(m_Icon);
        m_Icon = nullptr;
    }
    if (m_font)
    {
        TTF_CloseFont(m_font);
    }
    if (m_BoldFont)
    {
        TTF_CloseFont(m_BoldFont);
    }
    if (m_ClickCursor)
    {
        SDL_FreeCursor(m_ClickCursor);
    }
    if (m_NormalCursor)
    {
        SDL_FreeCursor(m_NormalCursor);
    }
}

int Ui_MessageBoxSDL::DoModal() {
    if (m_isShown)
        return -1;

    m_isShown = true;
    m_result = -1;

    // 初始化资源
    Initialize();

    // 对话框阻塞循环
    bool running = true;
    SDL_Event e;

    while (running && m_result == -1) {
        // 处理事件
        while (SDL_PollEvent(&e)) {
            if (HandleEvent(e)) {
                if (m_result != -1 || e.type == SDL_QUIT) {
                    running = false;
                }
            }
        }

        // 渲染对话框
        Render();

        // 限制帧率
        SDL_Delay(16); // ~60fps
    }

    m_isShown = false;
    return m_result;
}

void Ui_MessageBoxSDL::Show(std::function<void(int)> callback)
{
    if (m_isShown)
        return;

    m_isShown = true;
    m_result = -1;
    m_callback = callback;
    Initialize();// 初始化资源
}

bool Ui_MessageBoxSDL::HandleEvent(const SDL_Event& e)
{
    if (!m_isShown)
        return false;

    switch (e.type) 
    {
    case SDL_QUIT:
        Close(-1); // 用户关闭窗口
        return true;

    case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_LEFT) 
        {
            // 检查是否点击按钮
            CheckButtonClick(e.button.x, e.button.y);
            if (m_result != -1)
            {
                Close(m_result);
                return true;
            }
        }
        return true; // 即使没有点击到按钮，也消耗这个事件

    case SDL_KEYDOWN:
        if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_RETURN) 
        { // Esc或Enter键关闭对话框
            Close(e.key.keysym.sym == SDLK_RETURN ? 0 : -1);
            return true;
        }
        return true; // 消耗所有键盘事件

    case SDL_MOUSEMOTION:
    {
        // 更新按钮悬停状态
        for (auto& btn : m_buttons)
        {
            btn.hovered = (
                e.motion.x >= btn.rect.x &&
                e.motion.x < btn.rect.x + btn.rect.w &&
                e.motion.y >= btn.rect.y &&
                e.motion.y < btn.rect.y + btn.rect.h
                );
        }
        bool IsHasBtnHoving = false;
        for (auto& btn : m_buttons)
        {
            if (btn.hovered == true)
            {
                IsHasBtnHoving = true;
                break;
            }
        }
        if (IsHasBtnHoving)
            SDL_SetCursor(m_ClickCursor);
        else
            SDL_SetCursor(m_NormalCursor);
    }
        return true;
    case SDL_WINDOWEVENT:
    {
        if (e.window.event == SDL_WINDOWEVENT_ENTER)
        {
            SDL_SetCursor(m_NormalCursor);
        }
        return true;
    }
    }
    return false; // 不消耗其他类型的事件
}

void Ui_MessageBoxSDL::Render()
{
    if (!m_isShown || !m_dimTexture)
        return;

    // 绘制半透明背景
    SDL_RenderCopy(m_renderer, m_dimTexture, NULL, NULL);
    DrawMessageBox();    // 绘制对话框
}

void Ui_MessageBoxSDL::Close(int result)
{
    if (!m_isShown)
        return;

    m_result = result;
    m_isShown = false;

    // 如果有回调，调用它
    if (m_callback) {
        m_callback(result);
    }
}

void Ui_MessageBoxSDL::CheckButtonClick(int mouseX, int mouseY)
{
    for (const auto& btn : m_buttons) {
        if (mouseX >= btn.rect.x && mouseX < btn.rect.x + btn.rect.w &&
            mouseY >= btn.rect.y && mouseY < btn.rect.y + btn.rect.h) {
            m_result = btn.returnValue;
            break;
        }
    }
}

void Ui_MessageBoxSDL::DrawMessageBox()
{
    // 绘制对话框背景
    SDL_Rect boxRect = { m_x, m_y, m_width, m_height };
    SDL_Color boxColor = { 36, 37, 40, 255 };
    if (m_cornerRadius <= 0)
    {
        SDL_SetRenderDrawColor(m_renderer, boxColor.r, boxColor.g, boxColor.b, boxColor.a); //对话框深色背景
        SDL_RenderFillRect(m_renderer, &boxRect);
    }
    else
    {//如果为圆角模式
        DrawRoundedRectFilled(m_renderer, boxRect, m_cornerRadius, boxColor);
    }

    //绘制标题栏背景
    SDL_Rect titleRect = { m_x, m_y, m_width, 0.151 * m_height };
    SDL_Color titleBgColor = { 37, 39, 46, 255 };
    if (m_cornerRadius <= 0)
    {
        SDL_SetRenderDrawColor(m_renderer, titleBgColor.r, titleBgColor.g, titleBgColor.b, titleBgColor.a);//标题栏背景颜色
        SDL_RenderFillRect(m_renderer, &titleRect);
    }
    else
    {//如果为圆角模式
        SDL_Rect topTitleRect = 
        { // 标题栏需要特殊处理，保持上部为圆角，下部为直角
            m_x,
            m_y,
            m_width,
            m_cornerRadius
        };
        DrawRoundedRectFilled(m_renderer, topTitleRect, m_cornerRadius, titleBgColor);
        SDL_Rect bottomTitleRect = 
        {  // 标题栏下半部分为矩形
            m_x,
            m_y + m_cornerRadius,
            m_width,
            0.151 * m_height - m_cornerRadius
        };
        SDL_SetRenderDrawColor(m_renderer, titleBgColor.r, titleBgColor.g, titleBgColor.b, titleBgColor.a);
        SDL_RenderFillRect(m_renderer, &bottomTitleRect);
    }


    // 绘制窗口边框
    SDL_Color borderColor = { 73, 73, 73, 255 };
    if (m_cornerRadius <= 0)
    {
        int borderSize = 2;
        SDL_Rect BordRect = boxRect;
        SDL_SetRenderDrawColor(m_renderer, borderColor.r, borderColor.g, borderColor.a, 255); // 边框颜色
        for (size_t i = 0; i < borderSize; i++)
        {
            BordRect.x += i;
            BordRect.y += i;
            BordRect.w -= i * 2;
            BordRect.h -= i * 2;
            SDL_RenderDrawRect(m_renderer, &BordRect);
        }
    }
    else
    {//如果为圆角模式
        for (int i = 0; i < 2; i++) 
        {  // 圆角矩形边框
            SDL_Rect borderRect =
            {
                m_x + i,
                m_y + i,
                m_width - 2 * i,
                m_height - 2 * i
            };
            DrawRoundedRect(m_renderer, borderRect, m_cornerRadius - i, borderColor);
        }
    }

    // 绘制标题栏边框
    SDL_SetRenderDrawColor(m_renderer, 73, 73, 73, 255);//边框颜色
    int srclineY = m_y + 0.151 * m_height;
    int srclineX = m_x;
    for (size_t i = 0; i < 2; i++)
    {
        SDL_RenderDrawLine(m_renderer, srclineX, srclineY + i, srclineX + m_width - 1, srclineY + i);
    }
    SDL_SetRenderDrawColor(m_renderer, 60, 66, 79, 255);//设置回原颜色
    

    //绘制图标和提示类型文本
    int iconX = m_x + 0.077 * m_width;
    int iconY = m_y + 0.232 * m_height;
    if (m_Icon && m_font)
    {
        //绘制图标
        SDL_Rect iconRect = { iconX ,iconY ,m_IconWidth,m_IconHeight };
        SDL_RenderCopy(m_renderer, m_Icon, NULL, &iconRect);

        //绘制提示类型文本
        if (!m_MessageType.empty())
        {
            SDL_Color TypeTextColor = { 255, 255, 255, 255 }; // 深色文本
            SDL_Surface* TypeTextSurface = TTF_RenderUTF8_Blended(
                m_BoldFont,
                GlobalFunc::AnsiToUtf8(m_title).c_str(),
                TypeTextColor
            );
            int TypeTextX = iconX + m_IconWidth + 10 * m_Scale;
            int TypeTextY = iconY + (m_IconHeight - TypeTextSurface->h) / 2;
            SDL_Rect TypeTextRect = { TypeTextX,TypeTextY, TypeTextSurface->w,TypeTextSurface->h };
            if (TypeTextSurface)
            {
                SDL_Texture* TypeTextTexture = SDL_CreateTextureFromSurface(m_renderer, TypeTextSurface);
                SDL_RenderCopy(m_renderer, TypeTextTexture, NULL, &TypeTextRect);
                SDL_DestroyTexture(TypeTextTexture);
                SDL_FreeSurface(TypeTextSurface);
            }
        }
    }

    // 绘制标题文本
    if (m_font)
    {
        SDL_Color textColor = { 255,255,255,255 }; // 标题文本
        SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(m_font, GlobalFunc::AnsiToUtf8(m_title).c_str(), textColor);
        if (titleSurface)
        {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(m_renderer, titleSurface);
            SDL_Rect titleTextRect = 
            {
                m_x + 10 * m_Scale,
                m_y + 5 * m_Scale,
                titleSurface->w,
                titleSurface->h
            };

            SDL_RenderCopy(m_renderer, titleTexture, NULL, &titleTextRect);
            SDL_DestroyTexture(titleTexture);
            SDL_FreeSurface(titleSurface);
        }

        // 绘制消息文本
        SDL_Color textMessageColor = { 180,180,180,255 }; // 标题文本
        SDL_Surface* messageSurface = TTF_RenderUTF8_Blended_Wrapped(m_font, GlobalFunc::AnsiToUtf8(m_message).c_str(), textMessageColor, 0.761 * m_width);
        if (messageSurface) 
        {
            SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(m_renderer, messageSurface);
            SDL_Rect messageRect =
            {
                iconX + m_IconWidth + 10 * m_Scale,
                m_y + 0.407 * m_height,
                messageSurface->w,
                messageSurface->h
            };

            SDL_RenderCopy(m_renderer, messageTexture, NULL, &messageRect);
            SDL_DestroyTexture(messageTexture);
            SDL_FreeSurface(messageSurface);
        }
    }

    // 绘制按钮
    int buttonWidth = 86 * m_Scale;
    int buttonSpacing = 20 * m_Scale;
    int totalButtonsWidth = m_buttons.size() * buttonWidth + (m_buttons.size() - 1) * buttonSpacing;
    int buttonsStartX = m_x + (m_width - totalButtonsWidth) / 2;
    int buttonsY = m_y + 0.718 * m_height;
    int buttonHeight = 33 * m_Scale;

    for (size_t i = 0; i < m_buttons.size(); i++) 
    {
        Button& btn = m_buttons[i];

        btn.rect = 
        {
            buttonsStartX + static_cast<int>(i) * (buttonWidth + buttonSpacing),
            buttonsY,
            buttonWidth,
            buttonHeight
        };

        // 绘制按钮背景
        SDL_SetRenderDrawColor(m_renderer,
            btn.hovered ? 20 : 0,
            btn.hovered ? 159 : 139,
            btn.hovered ? 255 : 255,
            255);
        SDL_RenderFillRect(m_renderer, &btn.rect);
        SDL_RenderDrawRect(m_renderer, &btn.rect);   // 绘制按钮边框

        // 绘制按钮文本
        if (m_font) 
        {
            SDL_Color textColor = { 220, 220, 220, 255 };
            SDL_Surface* btnTextSurface = TTF_RenderUTF8_Blended(
                m_font, 
                GlobalFunc::AnsiToUtf8(btn.text).c_str(), 
                textColor
            );
            if (btnTextSurface) {
                SDL_Texture* btnTexture = SDL_CreateTextureFromSurface(m_renderer, btnTextSurface);
                SDL_Rect textRect =
                {
                    btn.rect.x + (buttonWidth - btnTextSurface->w) / 2,
                    btn.rect.y + (buttonHeight - btnTextSurface->h) / 2,
                    btnTextSurface->w,
                    btnTextSurface->h
                };
                SDL_RenderCopy(m_renderer, btnTexture, NULL, &textRect);
                SDL_DestroyTexture(btnTexture);
                SDL_FreeSurface(btnTextSurface);
            }
        }
    }
}

void Ui_MessageBoxSDL::DrawRoundedRect(SDL_Renderer* renderer, 
    const SDL_Rect& rect, int cornerRadius, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);  // 设置渲染颜色
    // 绘制四条直线，组成矩形的主体
    SDL_RenderDrawLine(renderer, rect.x + cornerRadius, rect.y,
        rect.x + rect.w - cornerRadius, rect.y);  // 上边线
    SDL_RenderDrawLine(renderer, rect.x + cornerRadius, rect.y + rect.h - 1,
        rect.x + rect.w - cornerRadius, rect.y + rect.h - 1);  // 下边线
    SDL_RenderDrawLine(renderer, rect.x, rect.y + cornerRadius,
        rect.x, rect.y + rect.h - cornerRadius);  // 左边线
    SDL_RenderDrawLine(renderer, rect.x + rect.w - 1, rect.y + cornerRadius,
        rect.x + rect.w - 1, rect.y + rect.h - cornerRadius);  // 右边线

    // 绘制四个角的圆弧
    int x = cornerRadius - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int error = tx - (cornerRadius << 1);
    while (x >= y) {
        // 左上角
        SDL_RenderDrawPoint(renderer, rect.x + cornerRadius - x, rect.y + cornerRadius - y);
        SDL_RenderDrawPoint(renderer, rect.x + cornerRadius - y, rect.y + cornerRadius - x);

        // 右上角
        SDL_RenderDrawPoint(renderer, rect.x + rect.w - cornerRadius + x, rect.y + cornerRadius - y);
        SDL_RenderDrawPoint(renderer, rect.x + rect.w - cornerRadius + y, rect.y + cornerRadius - x);

        // 右下角
        SDL_RenderDrawPoint(renderer, rect.x + rect.w - cornerRadius + x, rect.y + rect.h - cornerRadius + y);
        SDL_RenderDrawPoint(renderer, rect.x + rect.w - cornerRadius + y, rect.y + rect.h - cornerRadius + x);

        // 左下角
        SDL_RenderDrawPoint(renderer, rect.x + cornerRadius - x, rect.y + rect.h - cornerRadius + y);
        SDL_RenderDrawPoint(renderer, rect.x + cornerRadius - y, rect.y + rect.h - cornerRadius + x);

        if (error <= 0) 
        {
            y++;
            error += ty;
            ty += 2;
        }
        if (error > 0) 
        {
            x--;
            tx += 2;
            error += tx - (cornerRadius << 1);
        }
    }
}

void Ui_MessageBoxSDL::DrawRoundedRectFilled(SDL_Renderer* renderer, 
    const SDL_Rect& rect, int cornerRadius, SDL_Color color)
{
    // 如果圆角半径为0，直接绘制填充矩形
    if (cornerRadius <= 0)
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        return;
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); // 设置渲染颜色

    // 填充矩形的中间部分
    SDL_Rect middleRect =
    {
        rect.x + cornerRadius,
        rect.y,
        rect.w - 2 * cornerRadius,
        rect.h
    };
    SDL_RenderFillRect(renderer, &middleRect);

    // 填充左右侧边
    SDL_Rect leftRect =
    {
        rect.x,
        rect.y + cornerRadius,
        cornerRadius,
        rect.h - 2 * cornerRadius
    };
    SDL_RenderFillRect(renderer, &leftRect);

    SDL_Rect rightRect =
    {
        rect.x + rect.w - cornerRadius,
        rect.y + cornerRadius,
        cornerRadius,
        rect.h - 2 * cornerRadius
    };
    SDL_RenderFillRect(renderer, &rightRect);

    // 绘制四个角上的圆角
    // 使用中点圆算法绘制填充的四分之一圆
    for (int cx = 0; cx < cornerRadius; cx++)
    {
        for (int cy = 0; cy < cornerRadius; cy++)
        {
            int dx = cornerRadius - cx;
            int dy = cornerRadius - cy;
            if (dx * dx + dy * dy <= cornerRadius * cornerRadius)
            {
                SDL_RenderDrawPoint(renderer, rect.x + cx, rect.y + cy);                          // 左上角
                SDL_RenderDrawPoint(renderer, rect.x + rect.w - cx - 1, rect.y + cy);             // 右上角
                SDL_RenderDrawPoint(renderer, rect.x + cx, rect.y + rect.h - cy - 1);             // 左下角
                SDL_RenderDrawPoint(renderer, rect.x + rect.w - cx - 1, rect.y + rect.h - cy - 1);// 右下角
            }
        }
    }
}