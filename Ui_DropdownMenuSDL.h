#pragma once

#include <SDL.h>
#include <SDL_ttf.h>       // 添加TTF支持
#include <SDL_syswm.h>     // 系统窗口信息头文件
#include <windows.h>       // Windows API
#include <vector>
#include <functional>
#include <string>
#include <memory>
#include <atltypes.h>      // For CRect
#include <atomic>

// 按钮类，用于SDL界面中的交互元素
class Ui_SDLMenuButton {
public:
    // 构造与析构
    Ui_SDLMenuButton(const SDL_Rect& rect, const std::wstring& text,
        std::function<void()> callback); // 构造按钮，设置位置、文本和回调
    ~Ui_SDLMenuButton(); // 析构函数，清理资源

    // 渲染与交互方法
    void Render(SDL_Renderer* renderer, TTF_Font* font); // 渲染按钮和文本
    bool IsPointInside(int x, int y) const;   // 检查坐标是否位于按钮上
    void UpdateHoverState(int x, int y);      // 更新按钮的悬停状态
    void Click();                             // 触发按钮点击回调

private:
    // 按钮属性
    SDL_Rect m_Rect_Area;                     // 按钮的位置和大小
    std::wstring m_Str_Text;                  // 按钮上显示的文本(宽字符)
    std::function<void()> m_Func_Callback;    // 按钮点击时执行的回调函数
    bool m_Bool_Hovered;                      // 鼠标是否悬停在按钮上
    SDL_Texture* m_SDL_TextTexture;           // 按钮文本纹理
};

// 区域录制SDL叠加界面，显示录制区域和控制按钮=
class Ui_DropdownMenuSDL 
{
public:
    // 构造与析构
    static Ui_DropdownMenuSDL* GetInstance();
    static bool IsInsExist();
    void ReleaseInstance();
    ~Ui_DropdownMenuSDL();   // 析构函数，释放资源

    // 初始化与控制方法
    bool Initialize(const CRect& recordArea); // 初始化SDL窗口和渲染器，设置录制区域
    void Run();                               // 运行SDL窗口的主循环
    void Close();                             // 关闭并清理SDL资源
    void SetAnchorRect(CRect r);              // 设置限制区域

    // 回调函数设置方法
    void SetOnCameraRecordCallback(std::function<void()> callback);  // 设置摄像头录制回调
    void SetOnMicroSystemAudioCallback(std::function<void()> callback);   // 设置音频，麦克风录制回调
    void SetOnMouseRecordCallback(std::function<void()> callback); // 设置跟随鼠标录制

    // 核心功能方法
    bool InitializeSDL();     // 初始化SDL库
    bool CreateSDLWindow();   // 创建SDL窗口和渲染器
    void ProcessEvents();     // 处理SDL事件，如鼠标点击和键盘输入
    void Render();            // 渲染窗口内容，包括面板、按钮和录制区域边框
    void CreateButtons();     // 创建和配置界面上的控制按钮

    // 字体和文本处理方法
    bool InitializeTTF();     // 初始化TTF库
    bool LoadFonts();         // 加载字体

    void GetDpiScale();       // 获取用户DPI
private:
    // SDL核心对象
    SDL_Rect* m_Rect_Anchor;        // 鼠标超过这个区域就会关闭当前窗口
    SDL_Window* m_SDL_Window;      // SDL窗口句柄
    SDL_Renderer* m_SDL_Renderer;  // SDL渲染器句柄
    TTF_Font* m_TTF_Font;          // TTF字体对象

    // 界面属性
    CRect m_Rect_RecordArea;        // 录制区域的位置和大小
    bool m_Bool_Running;            // 窗口是否正在运行
    bool m_Bool_SDLInitialized;     // SDL是否已初始化
    bool m_Bool_TTFInitialized;     // TTF是否已初始化
    bool m_Bool_BtnHoverd;          // 是否悬停到按钮上
    int m_Int_PanelHeight = 180;    // 控制面板的高度
    int m_Int_PanelWidth = 216;     // 控制面板的宽度
    int m_WindowX;                  //窗口左上角x
    int m_WindowY;                  //窗口左上角y
    float m_Scale;                  //缩放系数

    // 界面控件
    std::vector<Ui_SDLMenuButton> m_Btn_ControlButtons; // 控制按钮集合

    // 回调函数
    std::function<void()> m_Func_OnCameraRecord;  // 摄像头录制按钮的回调函数
    std::function<void()> m_Func_OnMicroSystemAudioRecord;   // 麦克风音频录制,系统音频录制按钮回调函数
    std::function<void()> m_Func_OnMouseRecordRecord;   // 跟随鼠标录制按钮回调函数

private:
    Ui_DropdownMenuSDL();    // 构造函数，初始化成员变量
    static Ui_DropdownMenuSDL* s_ins;
};