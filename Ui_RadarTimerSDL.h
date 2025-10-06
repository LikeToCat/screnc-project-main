#pragma once

#include <SDL.h>
#include <SDL_ttf.h>       // 添加TTF支持
#include <SDL_syswm.h>     // 系统窗口信息头文件
#include <SDL2_gfxPrimitives.h>  // 添加SDL_gfx库头文件
#include <windows.h>       // Windows API
#include <string>
#include <memory>
#include <atltypes.h>      // For CRect
#include <atomic>
#include <thread>
#include <functional>
#include <cmath>           // 添加数学函数支持
#include <mutex>
#include <condition_variable>
#include <vector>
// 雷达倒计时SDL窗口类
class Ui_RadarTimerSDL
{
public:
    //外部执行回调
    using CountdownCallbackFunc = std::function<void()>;
public:
    static Ui_RadarTimerSDL* GetInstance();
    static void ReleaseInstance();
    static bool IsWindowRunning() { return s_IsRunning; }
    static bool IsInsExist() { return s_Instance == nullptr ? false : true; }

    // 初始化与控制方法
    bool Initialize(const CRect& DisplayArea, float Scale); // 初始化SDL窗口和渲染器，设置区域
    void Run();                               // 运行SDL窗口的主循环
    bool InitBotBanner();

    // 设置倒计时初始值
    void SetCountdown(int startCount) { m_Int_CountdownValue = startCount; }
    // 设置圆形边框颜色
    void SetCircleBorderColor(int r, int g, int b)
    {
        m_Color_Circle.r = r; 
        m_Color_Circle.g = g;
        m_Color_Circle.b = b;
    }
    // 设置矩形边框颜色
    void SetRectBorderColor(int r, int g, int b)
    {
        m_Color_Rect.r = r;
        m_Color_Rect.g = g;
        m_Color_Rect.b = b;
    }
    // 设置倒计时结束回调
    void SetCountDownCallBack(CountdownCallbackFunc callback) { m_Func_CountdownCallback = callback; }
    // 设置自动释放实例
    void SetAutoRelease(bool IsAutoRelease);
private:
    //SDL窗口线程
    void WindowThread();
    void AutoReleaseThread();

    // 核心功能方法
    bool InitializeSDL();     // 初始化SDL库
    bool CreateSDLWindow();   // 创建SDL窗口和渲染器
    void Render();            // 渲染窗口内容
    void Close();             // 关闭并清理SDL资源
    void DrawPerfectAntiAliasedCircle(int centerX, int centerY, int radius,
        SDL_Color fillColor, SDL_Color borderColor, int borderWidth);

    // 绘制方法
    void RenderCircle(int centerX, int centerY, int radius);  // 绘制圆形和边框
    void RenderCountdown(int centerX, int centerY);           // 绘制倒计时数字
    void RederBottomBanner();

    // 辅助绘画
    void UpdateCountdownTexture();// 倒计时纹理更新
    float EaseOutQuad(float t);   // 缓动函数

    //工具函数
    void SetTableLayout(int x, int y, int interval, int celHeight);

    // 调试打印
    void DebugSDLError(wchar_t error[256]);
private:
    // SDL核心对象
    SDL_Window* m_SDL_Window;      // SDL窗口句柄
    SDL_Renderer* m_SDL_Renderer;  // SDL渲染器句柄
    SDL_Texture* m_SDL_Transparent;// 透明纹理
    TTF_Font* m_TTF_Font;          // TTF字体对象
    TTF_Font* m_TTF_LargeFont;     // 大号字体对象（用于倒计时数字）
    SDL_Texture* m_Texture_CountdownText; // 倒计时文本纹理
    SDL_Rect m_Rect_CountdownText;        // 倒计时文本区域
    std::vector<Uint32> m_Pixbuffer;
    HWND m_Hwnd;

    // 字体和文本处理方法
    bool InitializeTTF();     // 初始化TTF库
    bool LoadFonts();         // 加载字体

    // SDL窗口线程
    std::thread m_Thread_WindowThread;
    std::thread m_Thread_AutoReleaseThread;
    bool m_IsAutoRelease = false;//窗口是否结束循环后自动销毁实例
    std::atomic<bool> m_Bool_ActiveRelease= false;
    std::condition_variable m_CV_ActiveRelease;
    std::mutex m_Mutex_ActiveRelease;

    // 界面属性
    CRect m_Rect_WindowArea;       // 区域的位置和大小
    bool m_Bool_Running;           // 窗口是否正在运行
    bool m_Bool_SDLInitialized;    // SDL是否已初始化
    bool m_Bool_TTFInitialized;    // TTF是否已初始化
    int m_Int_WindowHeight;        // 窗口的高度
    int m_Int_WindowWidth;         // 窗口的宽度
    float m_Scale;                 // 缩放比例

    // 倒计时动画属性
    Uint32 m_Uint32_LastTime;      // 上一帧的时间
    int m_Int_CountdownValue;      // 倒计时值
    int m_Int_LastCountdown;       // 上一次倒计时值
    int m_Int_LastCountdownValue;  // 上一次渲染的倒计时值（纹理缓存用）
    float m_Float_AnimationTime;   // 数字动画计时器
    bool m_Bool_ShowingAnimation;  // 是否正在显示数字变化动画
    SDL_Color m_Color_Circle;      // 圆形边框颜色
    SDL_Color m_Color_Rect;        // 矩形边框颜色

    //倒计时结束，外部回调
    CountdownCallbackFunc m_Func_CountdownCallback;

    //圆形相关
    SDL_Texture* m_CircleTexture = nullptr;
    int m_LastCircleRadius = -1;

    //底部banner相关
    int m_int_BannerRaduis;
    SDL_Rect m_Rect_Banner;
    struct BannerText
    {
        SDL_Texture* tex;
        SDL_Rect texR;
        BannerText() :
            tex(nullptr)
        {}
    };
    TTF_Font* m_Font_Brighter;
    TTF_Font* m_Font_Draker;
    BannerText m_txt_Header1;
    BannerText m_txt_Header2;
    BannerText m_txt_Header3;
    BannerText m_txt_StartOrStopRecording;
    BannerText m_txt_StartOrStopRecordingShortKey;
    BannerText m_txt_StartOrStopRecordingState;
private:
    Ui_RadarTimerSDL();
    ~Ui_RadarTimerSDL();
    static Ui_RadarTimerSDL* s_Instance;
    static bool s_IsRunning;
};