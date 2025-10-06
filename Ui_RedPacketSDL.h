#pragma once
#include "LLarSDL.h"
#include "Ui_SDLButton.h"
#include <string>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
class AdvancedButtonAnimator;
class Ui_RedPacketSDL :
    public LLarSDL
{
public:
    struct RedPacketConfig
    {
        CRect ReadPacket;
        HWND backgroundHWnd;                // 背景的父窗口的背景，捕获为纹理
        CString redPacketImagePath;         // 红包图片路径
        float dimParam;                     // 调暗降光系数
        std::chrono::milliseconds initialCountdownTime; // 初始倒计时时间
        UINT CouponNum;                     // 红包优惠数额
    };

    Ui_RedPacketSDL();
    ~Ui_RedPacketSDL();

    void SetRedPacketConfig(const RedPacketConfig& config);     //设置红包的基础配置
    std::chrono::milliseconds GetRemainingTime() const;         //获取剩余倒计时时间
    void SetCountdownCompleteCallback(std::function<void()> callback);  //设置倒计时完成回调
    inline int getRet() { return ret; }

protected:
    //虚函数重写，根据需要重写
    virtual void _ProcessEvents() override;      // 处理SDL事件（鼠标、图像、键盘）
    virtual void _Render() override;             // 渲染界面内容（背景、按钮、红包、文本、边框
    virtual bool _Cus_Init() override;           // 自定义初始化
    virtual bool _RegSDLHitTestFunc() override;  // 注册SDL窗口的点击测试回调

private:
    bool LoadRedPacketImage();          // 加载红包图片
    void CreateAnimatedButton();        // 创建带动画效果的按钮
    void UpdateButtonAnimation();       // 更新按钮动画
    void OnButtonClick();               // 按钮点击回调
    void RenderSimpleGradient(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color color1, SDL_Color color2);

    // 倒计时相关方法
    void UpdateCountdown();
    void RenderCountdown();
    void RenderCountdownBackground(const SDL_Rect& textRect, int padding);
    void FormatCountdownTime(std::chrono::milliseconds remaining, std::string& timeStr);

    // 价格显示相关方法
    void CreateCouponTextures();            // 创建优惠券纹理
    void CreatePacketLestTimeAndInfoTex();  // 红包描述,剩余时间纹理创建
    void RenderCouponAmount();              // 渲染优惠金额
private:
    //类内部的成员
    int ret;
    RedPacketConfig m_config;
    SDL_Texture* m_redPacketTexture;     // 红包图片纹理
    SDL_Texture* m_backgroundTexture;    // 背景纹理
    bool m_configSet;                    // 是否设置了配置
    TTF_Font* m_ReadPakcet_Font;

    // 按钮相关
    Ui_SDLButtonManager m_buttonManager; // 按钮管理器
    Ui_SDLButton* m_animatedButton;      // 动画按钮指针

    // 动画相关
    std::unique_ptr<AdvancedButtonAnimator> m_advancedAnimator; // 高级动画系统
    std::chrono::steady_clock::time_point m_lastFrameTime;      // 帧时间计算
    std::chrono::steady_clock::time_point m_animationStartTime;
    float m_pulsePhase;      // 脉冲动画相位
    float m_glowIntensity;   // 发光强度
    bool m_isHovered;        // 鼠标悬停状态

    // 鼠标状态
    int m_mouseX, m_mouseY;
    bool m_mousePressed;

    // 倒计时相关成员
    std::chrono::steady_clock::time_point m_countdownStartTime;    // 倒计时开始时间
    std::chrono::milliseconds m_initialCountdownDuration;         // 初始倒计时时长
    std::chrono::milliseconds m_remainingTime;                    // 剩余时间
    bool m_countdownActive;                                       // 倒计时是否激活
    bool m_countdownCompleted;                                    // 倒计时是否完成
    std::function<void()> m_countdownCompleteCallback;            // 倒计时完成回调

    // 倒计时文本纹理
    SDL_Texture* m_tex_countdownTexture;
    TTF_Font* m_Font_countdownFont;
    std::string m_lastTimeString; // 用于检测时间字符串变化

    //价格显示相关
    TTF_Font* m_Font_RmbSymbol;         //'￥'符号
    TTF_Font* m_Font_Info;              //红包描述
    TTF_Font* m_Font_LestTime;          //剩余时间
    TTF_Font* m_Font_Coupon;            //红包优惠 数额
    SDL_Texture* m_tex_RmbSymbol;       // '￥'符号纹理
    SDL_Texture* m_tex_InfoRedPacket;   // 红包描述纹理
    SDL_Texture* m_tex_leftTime;    // 剩余时间纹理
    SDL_Texture* m_tex_Coupon;      // 优惠数额纹理
    SDL_Rect m_rect_RmbSymbol;      // '￥'符号位置
    SDL_Rect m_rect_Coupon;         // 优惠数额位置
    SDL_Rect m_rect_InfoRedPacket;  //  红包描述位置
    SDL_Rect m_rect_leftTime;       //  剩余时间纹理位置
};