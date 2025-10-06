#pragma once

//依赖
#include <afxwin.h>
#include <SDL.h>
#include <SDL_ttf.h>      
#include <SDL_image.h>
#include <SDL_syswm.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL2_gfxPrimitives_font.h>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

//其他依赖
#include "WndShadow.h"

// 区域录制SDL叠加界面，显示录制区域和控制按钮=
class LLarSDL
{
public:
    struct ShadowParam
    {
        COLORREF color;
        int Darkness;
        int size; 
        int sharpness;
    };
    LLarSDL();
    ~LLarSDL(); 

    void Run(
        const CRect& gArea,
        bool isActiveAccelerated,
        int frameRate,
        ShadowParam sp
    );                // 运行SDL窗口
    inline bool isRunning() { return s_isRunning; }
    inline HWND GetHwnd() const { return m_hWnd; }
protected:
    //窗口核心运行函数
    virtual void _ProcessEvents();          // 处理SDL事件，如鼠标点击和键盘输入
    virtual void _Render();                 // 渲染窗口内容，包括面板、按钮和录制区域边框
    virtual bool _Cus_Init();               // 自定义初始化
    virtual bool _RegSDLHitTestFunc();      // 注册SDL窗口的点击测试回调
private:
    //低级初始化接口
    bool _Initialize(const SDL_Rect& gArea);      // 初始化SDL窗口
    bool _InitSDL();
    bool _InitSDL_Image();
    bool _InitSDL_TTF();
    bool _CreateSDLWindow(const SDL_Rect& gArea);

    //线程函数
    void _WindowThread();      // 运行SDL窗口线程

    //debug
    std::wstring _getSDLErrorMsg();
    std::wstring _getTTFErrorMsg();
    std::wstring _getIMGErrorMsg();

    //DPI获取
    float _GetDPI();
protected:
    // SDL核心对象
    SDL_Window* m_SDL_Window;       // SDL窗口句柄
    SDL_Renderer* m_SDL_Renderer;   // SDL渲染器句柄
    TTF_Font* m_TTF_Font;           // TTF字体对象
    HWND m_hWnd;                    // 当前窗口的HWND句柄
    CWndShadow m_Shadow;            // 当前窗口的阴影框架
    ShadowParam m_sp;               // 窗口的阴影框架

    //线程相关
    std::thread m_Thread_Window;            //SDL窗口线程
    std::atomic<bool> s_isRunning;

    // 界面属性
    SDL_Rect m_Rect_WindowRect;     // 窗口区域的位置和大小（相对于屏幕）
    SDL_Rect m_CtRect_WindowRect;   // 窗口客户区域
    float m_Scale;                  // 缩放系数
    bool m_bIsAccelerated;          // 是否开启硬件加速
    int m_iframeRate;               // 帧率
};