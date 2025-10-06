#include "stdafx.h"

#include "Ui_SDLButton.h"
#include "SDL_AnimaSystemCommon.h"
#include "Ui_RedPacketSDL.h"
#include "ComponentAPI.h"
#include "CDebug.h"
#include <filesystem>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern HANDLE ConsoleHandle;

Ui_RedPacketSDL::Ui_RedPacketSDL()
    : ret(0)
    , m_redPacketTexture(nullptr)
    , m_backgroundTexture(nullptr)
    , m_configSet(false)
    , m_animatedButton(nullptr)
    , m_pulsePhase(0.0f)
    , m_glowIntensity(0.0f)
    , m_isHovered(false)
    , m_mouseX(0)
    , m_mouseY(0)
    , m_mousePressed(false)
    , m_countdownActive(false)
    , m_countdownCompleted(false)
    , m_tex_countdownTexture(nullptr)
    , m_Font_countdownFont(nullptr)
    , m_remainingTime(std::chrono::hours(3)) // 默认3小时
    , m_initialCountdownDuration(std::chrono::hours(3))
    , m_Font_RmbSymbol(nullptr)
    , m_Font_Coupon(nullptr)
    , m_tex_RmbSymbol(nullptr)
    , m_tex_Coupon(nullptr)
    , m_tex_InfoRedPacket(nullptr)
    , m_tex_leftTime(nullptr)
    , m_Font_Info(nullptr)
    , m_Font_LestTime(nullptr)
{
    m_config = {};
    m_animationStartTime = std::chrono::steady_clock::now();
    m_advancedAnimator = std::make_unique<AdvancedButtonAnimator>();
    m_lastFrameTime = std::chrono::steady_clock::now();
}

Ui_RedPacketSDL::~Ui_RedPacketSDL()
{
    if (m_redPacketTexture)
    {
        SDL_DestroyTexture(m_redPacketTexture);
        m_redPacketTexture = nullptr;
    }
    if (m_backgroundTexture)
    {
        SDL_DestroyTexture(m_backgroundTexture);
        m_backgroundTexture = nullptr;
    }
    if (m_tex_countdownTexture)
    {
        SDL_DestroyTexture(m_tex_countdownTexture);
        m_tex_countdownTexture = nullptr;
    }
    if (m_Font_countdownFont)
    {
        TTF_CloseFont(m_Font_countdownFont);
        m_Font_countdownFont = nullptr;
    }
    if (m_tex_RmbSymbol)
    {
        SDL_DestroyTexture(m_tex_RmbSymbol);
        m_tex_RmbSymbol = nullptr;
    }
    if (m_tex_Coupon)
    {
        SDL_DestroyTexture(m_tex_Coupon);
        m_tex_Coupon = nullptr;
    }
    if (m_tex_leftTime)
    {
        SDL_DestroyTexture(m_tex_leftTime);
        m_tex_leftTime = nullptr;
    }
    if (m_tex_InfoRedPacket)
    {
        SDL_DestroyTexture(m_tex_InfoRedPacket);
        m_tex_InfoRedPacket = nullptr;
    }
    if (m_Font_RmbSymbol)
    {
        TTF_CloseFont(m_Font_RmbSymbol);
        m_Font_RmbSymbol = nullptr;
    }
    if (m_Font_Coupon)
    {
        TTF_CloseFont(m_Font_Coupon);
        m_Font_Coupon = nullptr;
    }
    if (m_Font_Info)
    {
        TTF_CloseFont(m_Font_Info);
        m_Font_Info = nullptr;
    }
    if (m_Font_LestTime)
    {
        TTF_CloseFont(m_Font_LestTime);
        m_Font_LestTime = nullptr;
    }
}

void Ui_RedPacketSDL::SetRedPacketConfig(const RedPacketConfig& config)
{
    m_config = config;
    m_configSet = true;

    // 设置倒计时初始时间
    if (config.initialCountdownTime.count() > 0)
    {
        m_initialCountdownDuration = config.initialCountdownTime;
        m_remainingTime = config.initialCountdownTime;
    }

    DEBUG_CONSOLE_FMT(
        ConsoleHandle,
        L"设置红包尺寸：%dx%d，背景父窗口句柄:Hwnd:%p",
        config.ReadPacket.Width(), config.ReadPacket.Height(),
        config.backgroundHWnd
    );
}

std::chrono::milliseconds Ui_RedPacketSDL::GetRemainingTime() const
{
    if (!m_countdownActive) 
        return m_remainingTime;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_countdownStartTime);
    if (elapsed >= m_initialCountdownDuration) 
        return std::chrono::milliseconds(0);
    return m_initialCountdownDuration - elapsed;
}

void Ui_RedPacketSDL::SetCountdownCompleteCallback(std::function<void()> callback)
{
    m_countdownCompleteCallback = callback;
}

bool Ui_RedPacketSDL::LoadRedPacketImage()
{
    if (m_redPacketTexture)
    {
        SDL_DestroyTexture(m_redPacketTexture);
        m_redPacketTexture = nullptr;
    }

    // 加载图片纹理
    if (!COMAPI::SDL::ImageLoad(
        m_SDL_Renderer,
        m_config.redPacketImagePath,
        &m_redPacketTexture)
        )
    {
        DB(ConsoleHandle, L"加载红包图片失败");
        return false;
    }
    DEBUG_CONSOLE_STR(ConsoleHandle, L"红包图片加载成功");
    return true;
}

void Ui_RedPacketSDL::RenderSimpleGradient(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color1, SDL_Color color2)
{
    // 简单的垂直渐变实现
    for (int y = 0; y < rect.h; ++y)
    {
        float ratio = (float)y / (float)rect.h;
        Uint8 r = (Uint8)(color1.r * (1.0f - ratio) + color2.r * ratio);
        Uint8 g = (Uint8)(color1.g * (1.0f - ratio) + color2.g * ratio);
        Uint8 b = (Uint8)(color1.b * (1.0f - ratio) + color2.b * ratio);
        Uint8 a = (Uint8)(color1.a * (1.0f - ratio) + color2.a * ratio);

        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderDrawLine(renderer, rect.x, rect.y + y, rect.x + rect.w - 1, rect.y + y);
    }
}

void Ui_RedPacketSDL::RenderCountdownBackground(const SDL_Rect& textRect, int padding)
{
    // 计算圆角矩形区域，在文本周围添加内边距
    static SDL_Rect bgRect = 
    {
        textRect.x - padding,
        textRect.y - padding,
        textRect.w + 2 * padding,
        textRect.h + 2 * padding
    };

    // 圆角半径
    static int cornerRadius = 8 * m_Scale;

    // 使用指定的填充颜色
    Uint8 fillR = 246;
    Uint8 fillG = 246;
    Uint8 fillB = 250;
    Uint8 fillA = 255; 

    // 边框颜色
    Uint8 borderR = 237;
    Uint8 borderG = 237;
    Uint8 borderB = 237;
    Uint8 borderA = 255;

    // 先绘制填充的圆角矩形
    roundedBoxRGBA(m_SDL_Renderer,
        bgRect.x, bgRect.y,
        bgRect.x + bgRect.w, bgRect.y + bgRect.h,
        cornerRadius,
        fillR, fillG, fillB, fillA);

    // 再绘制边框
    roundedRectangleRGBA(m_SDL_Renderer,
        bgRect.x, bgRect.y,
        bgRect.x + bgRect.w, bgRect.y + bgRect.h,
        cornerRadius,
        borderR, borderG, borderB, borderA);

    // 添加一个更细的内边框以增加立体感
    roundedRectangleRGBA(m_SDL_Renderer,
        bgRect.x + 1, bgRect.y + 1,
        bgRect.x + bgRect.w - 1, bgRect.y + bgRect.h - 1,
        cornerRadius - 1,
        255, 255, 255, 120); 
}

void Ui_RedPacketSDL::CreateAnimatedButton()
{
    // 确保窗口已经初始化
    if (!m_SDL_Renderer)
    {
        DB(ConsoleHandle, L"渲染器未初始化，无法创建按钮");
        return;
    }

    // 计算按钮位置（在窗口下方居中）
    int buttonWidth = 200 * m_Scale;
    int buttonHeight = 40 * m_Scale;
    int buttonX = (m_CtRect_WindowRect.w - buttonWidth) / 2;
    int buttonY = m_CtRect_WindowRect.h - buttonHeight - 40 * m_Scale; // 距离底部40像素

    // 如果红包配置已设置，则将按钮放在红包下方
    if (m_configSet)
    {
        buttonY = m_config.ReadPacket.bottom - 230 * m_Scale;
        // 确保按钮不会超出窗口范围
        if (buttonY + buttonHeight > m_CtRect_WindowRect.h)
        {
            buttonY = m_CtRect_WindowRect.h - buttonHeight - 20 * m_Scale;
        }
    }

    SDL_Rect buttonRect = { buttonX, buttonY - 30 * m_Scale, buttonWidth, buttonHeight };

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"创建按钮位置: (%d, %d, %d, %d)",
        buttonRect.x, buttonRect.y, buttonRect.w, buttonRect.h);

    // 定义金色渐变色系 RGB(250,190,91)
    SDL_Color goldBase = { 250, 190, 91, 255 };
    SDL_Color goldHover = { 255, 215, 120, 255 };
    SDL_Color goldClick = { 230, 165, 70, 255 };
    SDL_Color goldBorder = { 200, 150, 60, 255 };

    // 文本颜色
    SDL_Color textNormal = { 60, 40, 20, 255 };    // 深棕色
    SDL_Color textHover = { 40, 25, 10, 255 };     // 更深的棕色
    SDL_Color textClick = { 80, 50, 25, 255 };     // 中等棕色

    // 创建按钮
    m_buttonManager.CreateBtn(
        buttonRect,
        L"领 取 红 包",
        textNormal,      // 正常文本颜色
        textHover,       // 悬停文本颜色  
        textClick,       // 点击文本颜色
        goldBorder,      // 边框颜色
        goldBase,        // 背景颜色
        goldHover,       // 悬停背景色
        goldClick,       // 点击背景色
        L"",             // 无图片
        m_SDL_Renderer,
        m_ReadPakcet_Font,
        [this]() { OnButtonClick(); },  // 点击回调
        1001,           // 按钮ID
        10 * m_Scale              // 圆角半径
    );

    // 获取创建的按钮指针
    m_animatedButton = m_buttonManager.FindBtnByUid(1001);
    if (m_animatedButton)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"按钮创建成功");

        // 设置渐变色
        SDL_Color grad1 = { 255, 220, 130, 255 };  // 亮金色
        SDL_Color grad2 = { 240, 180, 80, 255 };   // 深金色
        SDL_Color hGrad1 = { 255, 235, 150, 255 }; // 悬停时更亮
        SDL_Color hGrad2 = { 250, 195, 100, 255 }; // 悬停时深金色

        m_animatedButton->SetGdColor(grad1, grad2);
        m_animatedButton->SetHGdColor(hGrad1, hGrad2);
        m_animatedButton->SetTextFont(19 * m_Scale, true);                   // 设置字体
    }
    else
    {
        DB(ConsoleHandle, L"按钮创建失败");
    }
}

void Ui_RedPacketSDL::UpdateButtonAnimation()
{
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
        (currentTime - m_animationStartTime).count();

    // 更新脉冲相位
    m_pulsePhase = (elapsed * 0.003f);

    // 计算发光强度（基于脉冲和悬停状态）
    float basePulse = (sin(m_pulsePhase) + 1.0f) * 0.5f;  // 0.0 - 1.0
    float hoverBoost = m_isHovered ? 0.5f : 0.0f;
    m_glowIntensity = basePulse * 0.6f + hoverBoost;
}

void Ui_RedPacketSDL::OnButtonClick()
{
    // 按钮点击处理
    DEBUG_CONSOLE_STR(ConsoleHandle, L"红包按钮被点击！");
    m_remainingTime = GetRemainingTime();
    m_countdownActive = false;
    ret = 1;                // 表示用户选择领取红包
    s_isRunning = false;    // 关闭窗口
    
}

void Ui_RedPacketSDL::_ProcessEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            s_isRunning = false;
            ret = -1; // 用户关闭窗口
            break;

        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                s_isRunning = false;
                ret = -1; // 窗口被关闭
                break;
            }
            break;

        case SDL_MOUSEMOTION:
            m_mouseX = e.motion.x;
            m_mouseY = e.motion.y;

            // 更新按钮状态
            m_buttonManager.UpdateBtnsState(m_mouseX, m_mouseY, false);

            // 检查是否悬停在按钮上
            m_isHovered = m_buttonManager.CheckPosInAnyBtn(m_mouseX, m_mouseY);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (e.button.button == SDL_BUTTON_LEFT)
            {
                m_mousePressed = true;
                m_buttonManager.UpdateBtnsState(e.button.x, e.button.y, true);

                // 触发点击特效
                if (m_buttonManager.CheckPosInAnyBtn(e.button.x, e.button.y) && m_advancedAnimator)
                {
                    SDL_Rect btnRect = m_animatedButton->GetBtnRect();
                    m_advancedAnimator->TriggerClickEffect(btnRect);
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (e.button.button == SDL_BUTTON_LEFT && m_mousePressed)
            {
                m_mousePressed = false;

                // 检查是否点击了按钮
                if (m_buttonManager.CheckPosInAnyBtn(e.button.x, e.button.y))
                {
                    Ui_SDLButton* clickedBtn = m_buttonManager.GetInHoverOrClickedBtn();
                    if (clickedBtn)
                    {
                        clickedBtn->Click();
                    }
                }

                m_buttonManager.UpdateBtnsState(e.button.x, e.button.y, false);
            }
            break;

        case SDL_KEYDOWN:
            //if (e.key.keysym.sym == SDLK_ESCAPE)
            //{
            //    // ESC键退出
            //    ret = -1;
            //    s_isRunning = false;
            //}
            //else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_SPACE)
            //{
            //    // 回车键或空格键也可以触发按钮点击
            //    if (m_animatedButton)
            //    {
            //        m_animatedButton->Click();
            //    }
            //}
            break;
        }
    }
}

void Ui_RedPacketSDL::_Render()
{
    // 计算帧时间
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
    m_lastFrameTime = currentTime;

    // 清空渲染器
    SDL_SetRenderDrawColor(m_SDL_Renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_SDL_Renderer);

    // 渲染背景（捕获的父窗口的背景）
    if (m_backgroundTexture)
    {
        SDL_RenderCopy(m_SDL_Renderer, m_backgroundTexture, NULL, &m_CtRect_WindowRect);
    }
    else
    {
        // 如果没有背景纹理，使用默认背景色
        SDL_SetRenderDrawColor(m_SDL_Renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(m_SDL_Renderer, &m_CtRect_WindowRect);
    }

    // 渲染红包图片（如果配置已设置）
    if (m_configSet)
    {
        static SDL_Rect redPacketRect
        {
            m_config.ReadPacket.left,
            m_config.ReadPacket.top,
            m_config.ReadPacket.Width(),
            m_config.ReadPacket.Height()
        };

        if (m_redPacketTexture)
        {
            SDL_RenderCopy(m_SDL_Renderer, m_redPacketTexture, NULL, &redPacketRect);
        }
        else
        {
            // 如果没有红包纹理，绘制一个红色矩形作为占位符
            SDL_SetRenderDrawColor(m_SDL_Renderer, 255, 100, 100, 255);
            SDL_RenderFillRect(m_SDL_Renderer, &redPacketRect);
        }
    }

    SDL_RenderCopy(m_SDL_Renderer, m_tex_InfoRedPacket, NULL, &m_rect_InfoRedPacket);
    {
        static int padding = 1 * m_Scale;
        static int w = 130 * m_Scale, h = m_rect_leftTime.h;
        static SDL_Rect CountDownBkRect
        {
            m_rect_leftTime.x,
            m_rect_leftTime.y,
            w,
            h,
        };
        RenderCountdownBackground(CountDownBkRect, padding);
    }
    {
        static int shadowSize = 1 * m_Scale;
        SDL_Rect shadowRect = m_rect_leftTime;
        shadowRect.x += shadowSize;
        shadowRect.y += shadowSize;
        SDL_SetTextureAlphaMod(m_tex_leftTime, 128);
        SDL_SetTextureColorMod(m_tex_leftTime, 0, 0, 0);
        SDL_RenderCopy(m_SDL_Renderer, m_tex_leftTime, NULL, &shadowRect);
        SDL_SetTextureAlphaMod(m_tex_leftTime, 255);
        SDL_SetTextureColorMod(m_tex_leftTime, 255, 255, 255);
        SDL_RenderCopy(m_SDL_Renderer, m_tex_leftTime, NULL, &m_rect_leftTime);
    }
    RenderCouponAmount();   // 渲染优惠金额
    UpdateCountdown();      //更新倒计时
    RenderCountdown();      //渲染倒计时

    // 更新并渲染高级动画系统
    if (m_animatedButton && m_advancedAnimator)
    {
        SDL_Rect btnRect = m_animatedButton->GetBtnRect();
        bool isHovered = m_isHovered;
        bool isClicked = m_mousePressed && m_buttonManager.CheckPosInAnyBtn(m_mouseX, m_mouseY);
        m_advancedAnimator->Update(deltaTime, btnRect, isHovered, isClicked);

        // 在按钮渲染之前绘制背景效果
        m_advancedAnimator->Render(m_SDL_Renderer, btnRect);
    }

    // 渲染按钮
    m_buttonManager.RenderBtns();

    // 最后显示
    SDL_RenderPresent(m_SDL_Renderer);
}

bool Ui_RedPacketSDL::_Cus_Init()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"开始自定义初始化");

    //创建窗口字体
    if (!COMAPI::SDL::TTFFontLoad(&m_ReadPakcet_Font, L"msyh.ttc", 22 * m_Scale))
    {
        DB(ConsoleHandle, L"警告：创建窗口字体失败 ");
    }

    //创建倒计时字体
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_countdownFont, L"msyh.ttc", 14 * m_Scale))
    {
        DB(ConsoleHandle, L"警告：创建倒计时字体字体失败 ");
    }

    //创建红包描述
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_Info, L"msyh.ttc", 16 * m_Scale))
    {
        DB(ConsoleHandle, L"警告：创建倒计时字体字体失败 ");
    }

    //创建剩余时间字体
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_LestTime, L"msyh.ttc", 13 * m_Scale))
    {
        DB(ConsoleHandle, L"警告：创建剩余时间字体失败 ");
    }

    //创建优惠数额字体
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_Coupon, L"msyh.ttc", 35 * m_Scale))
    {
        DB(ConsoleHandle, L"警告：创建优惠数额字体失败 ");
    }

    //创建‘￥’符号字体
    if (!COMAPI::SDL::TTFFontLoad(&m_Font_RmbSymbol, L"msyh.ttc", 16 * m_Scale))
    {
        DB(ConsoleHandle, L"警告：创建‘￥’符号字体字体失败 ");
    }

    // 加载红包图片（如果配置已设置）
    if (m_configSet && !m_config.redPacketImagePath.IsEmpty())
    {
        if (!LoadRedPacketImage())
        {
            DB(ConsoleHandle, L"警告：红包图片加载失败");
            // 不返回false，继续执行
        }
    }

    // 从HWND句柄背景位图（如果配置已设置）
    if (m_configSet && m_config.backgroundHWnd)
    {
        if (!COMAPI::SDL::CreateBackgroundFromWindow(
            m_SDL_Renderer,
            &m_backgroundTexture,
            m_config.backgroundHWnd,
            m_config.dimParam
        ))
        {
            DB(ConsoleHandle, L"警告：创建窗口背景纹理失败");
            // 不返回false，继续执行
        }
    }

    // 设置置顶窗口
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // 初始化高级动画系统
    m_advancedAnimator->Initialize(m_SDL_Renderer);

    CreateAnimatedButton(); // 创建动画按钮
    CreateCouponTextures(); // 创建优惠券纹理
    CreatePacketLestTimeAndInfoTex();   //创建红包描述和剩余时间纹理 

    // 启动倒计时
    m_countdownStartTime = std::chrono::steady_clock::now();
    m_countdownActive = true;
    m_countdownCompleted = false;

    DEBUG_CONSOLE_STR(ConsoleHandle, L"自定义初始化完成");
    return true;
}

void Ui_RedPacketSDL::CreateCouponTextures()
{
    if (!m_configSet || !m_SDL_Renderer)
        return;

    // 创建'￥'符号纹理
    if (m_Font_RmbSymbol)
    {
        SDL_Color couponStrColor{ 255, 0, 0, 255 };
        int RmbSymbolW, RmbSymbolH;
        if (!COMAPI::SDL::TextLoad(
            m_SDL_Renderer,
            m_Font_RmbSymbol,
            L"￥",
            couponStrColor,
            &m_tex_RmbSymbol,
            &RmbSymbolW,
            &RmbSymbolH
        ))
        {
            DB(ConsoleHandle, L"创建优惠数额纹理失败");
        }
        m_rect_RmbSymbol.w = RmbSymbolW;
        m_rect_RmbSymbol.h = RmbSymbolH;
    }

    // 创建优惠数额纹理
    if (m_Font_Coupon && m_config.CouponNum > 0)
    {
        auto couponStr = std::to_wstring(m_config.CouponNum);
        SDL_Color couponStrColor{ 255, 0, 0, 255 };
        int couponStrW, couponStrH;
        if (!COMAPI::SDL::TextLoad(
            m_SDL_Renderer,
            m_Font_Coupon,
            couponStr,
            couponStrColor,
            &m_tex_Coupon,
            &couponStrW,
            &couponStrH
        ))
        {
            DB(ConsoleHandle, L"创建优惠数额纹理失败");
        }
        m_rect_Coupon.w = couponStrW;
        m_rect_Coupon.h = couponStrH;
    }

    // 计算位置（红包中心偏上方，符号在数额左边）
    if (m_tex_RmbSymbol && m_tex_Coupon)
    {
        // 获取红包矩形
        static SDL_Rect redPacketRect =
        {
            m_config.ReadPacket.left,
            m_config.ReadPacket.top,
            m_config.ReadPacket.Width(),
            m_config.ReadPacket.Height()
        };

        // 计算总宽度（符号 + 间距 + 数额）
        static int symbolSpacing = 3 * m_Scale; // 符号与数额间距
        int totalWidth = m_rect_RmbSymbol.w + symbolSpacing + m_rect_Coupon.w;
        int maxHeight = max(m_rect_RmbSymbol.h, m_rect_Coupon.h);

        // 计算中心位置，并向上偏移到红包中心偏上方
        static int centerX = redPacketRect.x + redPacketRect.w / 2;
        static int centerY = redPacketRect.y + redPacketRect.h / 2 - 35 * m_Scale; // 向上偏移35像素

        // 设置￥符号位置（在左边）
        m_rect_RmbSymbol.x = centerX - totalWidth / 2 - 10 * m_Scale;
        m_rect_RmbSymbol.y = centerY - maxHeight / 2 + (maxHeight - m_rect_RmbSymbol.h) / 2 + 5 * m_Scale; // 垂直居中对齐

        // 设置数额位置（在符号右边）
        m_rect_Coupon.x = m_rect_RmbSymbol.x + m_rect_RmbSymbol.w + symbolSpacing;
        m_rect_Coupon.y = m_rect_RmbSymbol.y + (m_rect_RmbSymbol.h - m_rect_Coupon.h) / 2 
            - m_rect_RmbSymbol.h / 4 + 5 * m_Scale;

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"优惠券位置计算完成: 符号(%d,%d) 数额(%d,%d)",
            m_rect_RmbSymbol.x, m_rect_RmbSymbol.y, m_rect_Coupon.x, m_rect_Coupon.y);
    }
}

void Ui_RedPacketSDL::CreatePacketLestTimeAndInfoTex()
{
    if (!m_configSet || !m_SDL_Renderer)
        return;

    // 创建info符号纹理
    if (m_Font_Info)
    {
        SDL_Color InfoAndLest{ 255, 0, 0, 255 };
        int w, h;
        if (!COMAPI::SDL::TextLoad(
            m_SDL_Renderer,
            m_Font_Info,
            L"限时优惠券",
            InfoAndLest,
            &m_tex_InfoRedPacket,
            &w,
            &h
        ))
        {
            DB(ConsoleHandle, L"限时优惠券纹理闯进啊失败");
        }

        SDL_Color LestColor{ 255,0,0 ,255 };
        int w1, h1;
        if (!COMAPI::SDL::TextLoad(
            m_SDL_Renderer,
            m_Font_LestTime,
            L"剩余时间",
            LestColor,
            &m_tex_leftTime,
            &w1,
            &h1
        ))
        {
            DB(ConsoleHandle, L"限时优惠券纹理闯进啊失败");
        }

        m_rect_leftTime.w = w1;
        m_rect_leftTime.h = h1;
        m_rect_leftTime.x = (m_Rect_WindowRect.w - m_rect_leftTime.w) / 2 - 40 * m_Scale;
        m_rect_leftTime.y = 280 * m_Scale;

        m_rect_InfoRedPacket.w = w;
        m_rect_InfoRedPacket.h = h;
        m_rect_InfoRedPacket.x = (m_Rect_WindowRect.w - m_rect_InfoRedPacket.w) / 2;
        m_rect_InfoRedPacket.y = 215 * m_Scale;
    }

    // 创建优惠数额纹理
    if (m_Font_Coupon && m_config.CouponNum > 0)
    {
        auto couponStr = std::to_wstring(m_config.CouponNum);
        SDL_Color couponStrColor{ 255, 0, 0, 255 };
        int couponStrW, couponStrH;
        if (!COMAPI::SDL::TextLoad(
            m_SDL_Renderer,
            m_Font_Coupon,
            couponStr,
            couponStrColor,
            &m_tex_Coupon,
            &couponStrW,
            &couponStrH
        ))
        {
            DB(ConsoleHandle, L"创建优惠数额纹理失败");
        }
        m_rect_Coupon.w = couponStrW;
        m_rect_Coupon.h = couponStrH;
    }
}

void Ui_RedPacketSDL::RenderCouponAmount()
{
    if (!m_SDL_Renderer || !m_tex_RmbSymbol || !m_tex_Coupon)
        return;

    // 渲染￥符号（带阴影效果）
    SDL_Rect symbolShadowRect = m_rect_RmbSymbol;
    symbolShadowRect.x += 1;
    symbolShadowRect.y += 1;

    // 渲染符号阴影
    SDL_SetTextureAlphaMod(m_tex_RmbSymbol, 100);
    SDL_SetTextureColorMod(m_tex_RmbSymbol, 0, 0, 0);
    SDL_RenderCopy(m_SDL_Renderer, m_tex_RmbSymbol, nullptr, &symbolShadowRect);

    // 渲染符号主体
    SDL_SetTextureAlphaMod(m_tex_RmbSymbol, 255);
    SDL_SetTextureColorMod(m_tex_RmbSymbol, 255, 255, 255);
    SDL_RenderCopy(m_SDL_Renderer, m_tex_RmbSymbol, nullptr, &m_rect_RmbSymbol);

    // 渲染优惠数额（带阴影效果）
    SDL_Rect couponShadowRect = m_rect_Coupon;
    couponShadowRect.x += 1;
    couponShadowRect.y += 1;

    // 渲染数额阴影
    SDL_SetTextureAlphaMod(m_tex_Coupon, 100);
    SDL_SetTextureColorMod(m_tex_Coupon, 0, 0, 0);
    SDL_RenderCopy(m_SDL_Renderer, m_tex_Coupon, nullptr, &couponShadowRect);

    // 渲染数额主体
    SDL_SetTextureAlphaMod(m_tex_Coupon, 255);
    SDL_SetTextureColorMod(m_tex_Coupon, 255, 255, 255);
    SDL_RenderCopy(m_SDL_Renderer, m_tex_Coupon, nullptr, &m_rect_Coupon);
}

void Ui_RedPacketSDL::UpdateCountdown()
{
    if (!m_countdownActive || m_countdownCompleted) 
        return;
    m_remainingTime = GetRemainingTime();
    if (m_remainingTime <= std::chrono::milliseconds(0))
    {
        m_remainingTime = std::chrono::milliseconds(0);
        m_countdownCompleted = true;
        m_countdownActive = false;

        // 触发回调
        if (m_countdownCompleteCallback) 
            m_countdownCompleteCallback();
    }
}

void Ui_RedPacketSDL::FormatCountdownTime(std::chrono::milliseconds remaining, std::string& timeStr)
{
    auto totalMs = remaining.count();

    auto hours = totalMs / (1000 * 60 * 60);
    totalMs %= (1000 * 60 * 60);

    auto minutes = totalMs / (1000 * 60);
    totalMs %= (1000 * 60);

    auto seconds = totalMs / 1000;
    auto milliseconds = totalMs % 1000;

    char buffer[32];
    if (hours > 0)
    {
        // 显示小时:分钟:秒.毫秒
        sprintf_s(buffer, "%02lld:%02lld:%02lld.%01lld",
            hours, minutes, seconds, milliseconds / 100);
    }
    else if (minutes > 0)
    {
        // 显示分钟:秒.毫秒
        sprintf_s(buffer, "%02lld:%02lld.%01lld",
            minutes, seconds, milliseconds / 100);
    }
    else 
    {
        // 显示秒.毫秒
        sprintf_s(buffer, "%02lld.%02lld",
            seconds, milliseconds / 10);
    }
    timeStr = buffer;
}

void Ui_RedPacketSDL::RenderCountdown()
{
    if (!m_Font_countdownFont || !m_SDL_Renderer) 
        return;
    std::string timeStr;
    FormatCountdownTime(m_remainingTime, timeStr);

    // 只有当时间字符串发生变化时才重新创建纹理
    if (timeStr != m_lastTimeString || !m_tex_countdownTexture)
    {
        if (m_tex_countdownTexture) 
        {
            SDL_DestroyTexture(m_tex_countdownTexture);
            m_tex_countdownTexture = nullptr;
        }
        static SDL_Color textColor{ 253,75,75,255 };
        SDL_Surface* textSurface = TTF_RenderText_Blended(m_Font_countdownFont, timeStr.c_str(), textColor);
        if (textSurface)
        {
            m_tex_countdownTexture = SDL_CreateTextureFromSurface(m_SDL_Renderer, textSurface);
            SDL_SetTextureBlendMode(m_tex_countdownTexture, SDL_BLENDMODE_BLEND);
            SDL_FreeSurface(textSurface);
        }
        m_lastTimeString = timeStr;
    }
    if (m_tex_countdownTexture)
    {
        int textWidth, textHeight;
        SDL_QueryTexture(m_tex_countdownTexture, nullptr, nullptr, &textWidth, &textHeight);

        // 计算红包中心位置
        static SDL_Rect redPacketRect =
        {
            m_config.ReadPacket.left,
            m_config.ReadPacket.top,
            m_config.ReadPacket.Width(),
            m_config.ReadPacket.Height()
        };
        static int centerX = redPacketRect.x + redPacketRect.w / 2 - 10 * m_Scale;   //稍微向左偏移
        static int centerY = redPacketRect.y + redPacketRect.h / 2;
        static SDL_Rect destRect =
        {
            centerX - textWidth / 2 + 30 * m_Scale,
            centerY - textHeight / 2 + 5 * m_Scale, // 稍微向下偏移
            textWidth,
            textHeight
        };

        // 添加文字阴影效果
        SDL_Rect shadowRect = destRect;
        static int ShadowSize = 1 * m_Scale;
        shadowRect.x += ShadowSize;
        shadowRect.y += ShadowSize;

        // 创建阴影纹理
        SDL_SetTextureAlphaMod(m_tex_countdownTexture, 128);
        SDL_SetTextureColorMod(m_tex_countdownTexture, 0, 0, 0);
        SDL_RenderCopy(m_SDL_Renderer, m_tex_countdownTexture, nullptr, &shadowRect);

        // 渲染主文字
        SDL_SetTextureAlphaMod(m_tex_countdownTexture, 255);
        SDL_SetTextureColorMod(m_tex_countdownTexture, 255, 255, 255);
        SDL_RenderCopy(m_SDL_Renderer, m_tex_countdownTexture, nullptr, &destRect);
    }
}

bool Ui_RedPacketSDL::_RegSDLHitTestFunc()
{
    // 红包窗口通常不需要拖拽功能，所以可以根据需要关闭
    return true;
}