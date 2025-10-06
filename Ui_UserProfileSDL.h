#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>  // 添加SDL_image库
#include <SDL_syswm.h>
#include <windows.h>
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <atltypes.h>
#include <atomic>

// 用户资料界面SDL窗口类
class Ui_UserProfileSDL
{
    //按钮控件类
    class Ui_SDLButton
    {
    public:
        // 构造与析构
        Ui_SDLButton(const SDL_Rect& rect, const std::wstring& text,
            std::function<void()> callback, bool isTextButton = false);
        ~Ui_SDLButton();

        // 渲染与交互方法
        void Render(SDL_Renderer* renderer, TTF_Font* font)const;
        bool IsPointInside(int x, int y) const;
        bool UpdateHoverState(int x, int y);
        void Click();
        void UpdateClickState(bool isClick);
        bool IsHovered() const { return m_Bool_Hovered; }

        // 获取按钮区域
        SDL_Rect GetRect() const { return m_Rect_Area; }

    private:
        // 按钮属性
        SDL_Rect m_Rect_Area;
        std::wstring m_Str_Text;
        std::function<void()> m_Func_Callback;
        bool m_Bool_Hovered;
        bool m_Bool_Click;
        mutable SDL_Texture* m_SDL_TextTexture;
        bool m_Bool_IsTextButton;
    };

public:
    static Ui_UserProfileSDL* GetInstance();
    static void ReleaseInstance();
    static bool IsInsExist() { return Instance == nullptr ? false : true; }
    static bool IsInsRunning() { return s_Bool_IsRunnins; }

    // 初始化与控制方法
    bool Initialize(const CRect& windowRect, float scale);
    void Run();
    void Close();

    // 回调函数设置方法
    void SetOnCloseCallback(std::function<void()> callback);
    void SetOnLogoutCallback(std::function<void()> callback);
    void SetOnContactSupportCallback(std::function<void()> callback);

private:
    // 核心功能方法
    bool InitializeSDL();
    bool InitializeSDLImage();
    bool CreateSDLWindow();
    void ProcessEvents();
    void Render();
    void CreateButtons();
    bool LoadResources();
    void RenderCircularImage(SDL_Texture* texture, int x, int y, int radius);
    void RenderUserInfo();

    // 字体和文本处理方法
    bool InitializeTTF();
    bool LoadFonts();

    // 窗口交互方法
    void SetCursorToCursor(bool isHandCursor);

    // 调试打印
    void DebugSDLError(wchar_t error[256]);

private:
    // SDL核心对象
    SDL_Window* m_SDL_Window;
    SDL_Renderer* m_SDL_Renderer;
    TTF_Font* m_TTF_Font;
    TTF_Font* m_TTF_LargeFont;  // 添加大字体用于用户名
    TTF_Font* m_TTF_ButtonFont;  // 按钮专用字体

    // 图像资源
    SDL_Texture* m_Texture_ContactService;
    SDL_Texture* m_Texture_SignOut;
    SDL_Texture* m_Texture_Icon;
    bool m_Bool_ImgInitialized;

    // 界面属性
    CRect m_Rect_WindowArea;
    bool m_Bool_Running;
    bool m_Bool_SDLInitialized;
    bool m_Bool_TTFInitialized;
    int m_Int_WindowWidth;
    int m_Int_WindowHeight;
    float m_Float_Scale;

    // 鼠标状态追踪
    bool m_Bool_MouseEnteredWindow;
    bool m_Bool_HandCursorSet;

    // 界面控件
    std::vector<Ui_SDLButton> m_Vec_Buttons;

    // 回调函数
    std::function<void()> m_Func_OnClose;
    std::function<void()> m_Func_OnLogout;
    std::function<void()> m_Func_OnContactSupport;

private:
    Ui_UserProfileSDL();
    ~Ui_UserProfileSDL();
    static Ui_UserProfileSDL* Instance;
    static bool s_Bool_IsRunnins;
};