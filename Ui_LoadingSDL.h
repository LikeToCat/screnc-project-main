#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <SDL_syswm.h>
#include "WndShadow.h"
#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "Gdi32.lib") 
#endif

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
class Ui_LoadingSDL
{
private:
    //线程同步保护
    struct ThreadProtect
    {
        std::mutex m_initMutex;           // 初始化互斥锁
        std::condition_variable m_initCV; // 初始化条件变量
    };

    //状态机
    struct State
    {
        std::atomic<bool> m_running{ false };      // 窗口运行状态
        std::atomic<bool> m_initialized{ false };  // 初始化状态
    };

    //线程池
    struct ThreadPool
    {
        std::thread m_thread;             // 独立的SDL窗口线程
    };
public:
    static Ui_LoadingSDL* GetInstance();
    static void ReleaseInstance();

    // 窗口控制 
    bool Start(const char* imagePath, float dpiScale = 1.0f);
    void Hide();
    void RestoreFromHide();
    void Destroy();
    void SetWindowTopMost();
private:
    // SDL相关
    SDL_Window* m_window;                 // SDL窗口句柄
    SDL_Renderer* m_renderer;             // SDL渲染器
    SDL_Texture* m_logoTexture;           // 加载的图片纹理

    // 布局相关
    int m_windowWidth;                    // 窗口宽度
    int m_windowHeight;                   // 窗口高度
    int m_logoWidth;                      // Logo宽度
    int m_logoHeight;                     // Logo高度
    SDL_Rect m_logoRect;                  // Logo位置和尺寸
    SDL_Rect m_contentRect;               // 内容区域位置和尺寸
    float m_dpiScale;                     // DPI缩放因子（外部传入）
    std::string m_imagePath;              // 图片路径

    // 阴影相关
    int m_shadowSize;                     // 阴影宽度 
    int m_shadowAlpha;                    // 阴影透明度

    // 字体和文本相关
    TTF_Font* m_font;                     // 字体
    SDL_Texture* m_textTexture;           // 文本纹理
    SDL_Rect m_textRect;                  // 文本位置和尺寸

    // 线程相关
    ThreadProtect threadProtect;          // 接口线程保护
    State state;                          // 接口状态(状态机结构体)
    ThreadPool Thread;                    // 接口线程池
private:
    static Ui_LoadingSDL* s_instance;     // 接口实例
    static std::mutex s_instanceMutex;    // 接口锁
    Ui_LoadingSDL();
    ~Ui_LoadingSDL();
    Ui_LoadingSDL(const Ui_LoadingSDL&) = delete;
    Ui_LoadingSDL& operator=(const Ui_LoadingSDL&) = delete;


    // 功能函数
    void WindowThread(const char* imagePath);
    bool Initialize(const char* imagePath);
    void InitializeShadow();
    bool LoadLogoTexture(const char* imagePath);
    bool CreateTextTexture();
    void CalculateLayout();
    void RenderFrame();
    void DrawShadow();
    void RenderLoop();
    void CleanupResources();

    // 圆角相关
    int m_cornerRadius;                   // 圆角半径
    int m_windowRegionWidth;
    int m_windowRegionHeight;
    void DrawRoundedRect(const SDL_Rect& rect, int radius);
    void DrawCornerLines();
    void SetWindowRoundedShape();

    // 角落直角线相关
    int m_cornerLineWidth;                // 角落线宽度
    int m_cornerLineLength;               // 角落线长度
    SDL_Color m_cornerLineColor;          // 角落线颜色

    // 公司信息文本相关
    TTF_Font* m_chineseFont = nullptr;              // 中文字体
    SDL_Texture* m_companyNameTexture = nullptr;    // 公司名称纹理
    SDL_Texture* m_productInfoTexture = nullptr;    // 产品信息纹理
    SDL_Rect m_companyNameRect;           // 公司名称位置
    SDL_Rect m_productInfoRect;           // 产品信息位置
    int m_bottomCornerLineY = 0;          // 保存右下角线段Y坐标，用于对齐文本
    bool CreateChineseTextTextures();     // 中文文本渲染函数

    // 背景图片相关
    SDL_Texture* m_backgroundTexture = nullptr;     // 背景图片纹理
    SDL_Rect m_backgroundRect;            // 背景图片位置和尺寸
    bool LoadBackgroundTexture();

    CWndShadow m_Shadow;
};