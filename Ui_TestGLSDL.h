#pragma once

//SDL包含
#include <afxwin.h>
#include <SDL.h>
#include <SDL_ttf.h>      
#include <SDL_image.h>
#include <SDL_syswm.h>
#include <string>
#include <thread>

//组件类
#include "ComponentClass.h"

// 区域录制SDL叠加界面，显示录制区域和控制按钮=
class Ui_TestGLSDL
{
public:
    // 构造与析构
    static Ui_TestGLSDL* GetInstance();
    static bool IsInsExist();
    static bool IsRunning();
    static bool IsInitlized();
    void ReleaseInstance();
    ~Ui_TestGLSDL();

    void Run(const CRect& gArea);             // 运行SDL窗口
private:
    //低级初始化接口
    bool Initialize(const SDL_Rect& gArea);      // 初始化SDL窗口
    bool InitSDL();
    bool InitSDL_Image();
    bool InitSDL_TTF();
    bool CreateSDLWindow(const SDL_Rect& gArea);

    //窗口核心运行函数
    void ProcessEvents();     // 处理SDL事件，如鼠标点击和键盘输入
    void Render();            // 渲染窗口内容，包括面板、按钮和录制区域边框

    //线程函数
    void WindowThread();      // 运行SDL窗口线程

    //绘画函数
    void SetWindowDrawPaint();

    //debug
    std::wstring getSDLErrorMsg();
    std::wstring getTTFErrorMsg();
    std::wstring getIMGErrorMsg();

    //DPI获取
    float GetDPI();
private:
    // SDL核心对象
    SDL_Renderer* m_SDL_Renderer;   // SDL渲染器句柄
    TTF_Font* m_TTF_Font;           // TTF字体对象

    //OpenGL组件
    COMCLASS::SDL::WindowOpenGL m_WindowOpenGL; //SDL OpenGL渲染器主组件
    COMCLASS::SDL::GLTexManager m_GLTexManager; //SDL OpenGL资源管理器组件
    COMCLASS::SDL::VBOVAOShader m_VVShader;     //VAO/VBO Shader现代OpenGL渲染管线组件

    //线程相关
    std::thread m_Thread_Window;    // SDL窗口线程

    // 基本界面属性
    SDL_Rect m_Rect_WindowRect;     // 窗口区域的位置和大小（相对于屏幕）
    SDL_Rect m_CtRect_WindowRect;   // 窗口客户区域
    float m_Scale;                  // 缩放系数
private:
    Ui_TestGLSDL();    // 构造函数，初始化成员变量
    static Ui_TestGLSDL* s_ins;
    static bool s_isRunning;
    static bool s_isInitlized;
};