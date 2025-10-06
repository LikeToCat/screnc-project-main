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
#include <chrono>
#include "CameraCapture.h"
// 按钮类，用于SDL界面中的交互元素
// 区域录制SDL叠加界面，显示录制区域和控制按钮=
class Ui_CameraPreviewSDL
{
    enum class AdjustPointType// 调整点类型枚举
    {
        None,
        Top,
        Bottom,
        Left,
        Right
    };
    //按钮控件类
    class Ui_SDLMenuButton
    {
    public:
        // 构造与析构
        Ui_SDLMenuButton(const SDL_Rect& rect, const std::wstring& text,
            std::function<void()> callback); // 构造按钮，设置位置、文本和回调
        ~Ui_SDLMenuButton(); // 析构函数，清理资源

        // 渲染与交互方法
        void Render(SDL_Renderer* renderer, TTF_Font* font); // 渲染按钮和文本
        bool IsPointInside(int x, int y) const;   // 检查坐标是否位于按钮上
        bool UpdateHoverState(int x, int y);      // 更新按钮的悬停状态
        void Click();                             // 触发按钮点击回调
        void UpdateClickState(bool isClick);
    private:
        // 按钮属性
        SDL_Rect m_Rect_Area;                     // 按钮的位置和大小
        std::wstring m_Str_Text;                  // 按钮上显示的文本(宽字符)
        std::function<void()> m_Func_Callback;    // 按钮点击时执行的回调函数
        bool m_Bool_Hovered;                      // 鼠标是否悬停在按钮上
        bool m_Bool_Click = false;                // 按钮是否被点击状态
        SDL_Texture* m_SDL_TextTexture;           // 按钮文本纹理
    };
public:
    static Ui_CameraPreviewSDL* GetInstance();
    static void ReleaseInstance();
    static bool IsInsExist();
    static bool IsRunning(); 

    // 初始化与控制方法
    bool SetWindowParam(const CRect& DisplayArea, CameraOptions cameraOption, float Scale); //设置SDL窗口参数,设置显示区域
    void RunWindowThread();        // SDL窗口线程
    // 回调函数设置方法
    void SetOnCancelBindingCallback(std::function<void()> callback);        // 设置取消绑定按钮
    void SetRaiseWindow();
private:
    // 初始化窗口
    bool Init();
    bool InitializeSDL();                               // 初始化SDL库
    bool InitCameraCapture(CameraOptions cameraOption); // 初始化摄像头捕获器
    bool InitSwsCtx(AVPixelFormat pixfmt);              // 初始化FFmpeg转换器
    bool InitFrameTexture(SDL_Renderer* render);        // 初始化渲染帧纹理数据
    bool InitializeTTF();                               // 初始化TTF库

    //创建窗口
    bool CreateSDLWindow();         // 创建SDL窗口和渲染器
    void CreateButtons();           // 创建和配置界面上的控制按钮
    void CreateSystemCursor();      // 创建系统光标
    bool CreateWindowRoundShape();  // 设置窗口为圆形窗口
    bool CreateFonts();             // 创建加载字体

    //窗口运行核心主函数
    void ProcessEvents();       // 处理SDL事件
    void Render();              // 渲染窗口内容
    void WindowThreadFunc();    // 运行SDL窗口的主循环
    void Close();               // 关闭并清理SDL资源
private://辅助函数
    //调整窗口大小相关
    void SetAdaptAdjustPointRect();                                    // 更新调整点位置
    bool IsPointInAdjustPoint(int x, int y, AdjustPointType& outType); // 判断点击是否在调整点上
    void ResizeWindow(int mouseX, int mouseY);                         // 调整窗口大小
    
    //光标设置相关
    void SetCursorByPos(int mouseX, int mouseY);

    //具体渲染内容相关
    void RenderCameraFrame();   // 渲染摄像头帧
    void RenderAdjustPoint();   // 渲染四个调整点

    //调试打印
    void DebugSDLError(wchar_t error[256]);
private:
    // SDL核心对象
    SDL_Window* m_SDL_Window = nullptr;      // SDL窗口句柄
    SDL_Renderer* m_SDL_Renderer = nullptr;  // SDL渲染器句柄
    TTF_Font* m_TTF_Font = nullptr;          // TTF字体对象
    std::thread m_Thread_WindowThread;       // SDL窗口线程
   
    //摄像头帧渲染相关
    CameraCapture* m_Interface_CameraCapture = nullptr;// 捕获器接口
    CameraOptions m_Struct_CameraOpt;                  // 捕获器需要的参数
    AVFrame* m_AVFrame_Camera = nullptr;               // 摄像头原始帧缓冲区
    AVPixelFormat m_AVPixfmt_OriFmt = AV_PIX_FMT_NONE; // 原始帧格式
    SwsContext* m_SwsCtx_Format = nullptr;             // 格式转换器相关
    SDL_Texture* m_Texture_Camera = nullptr;           // 摄像头纹理数据
    uint8_t* m_uint8_RenderFrameBuffer = nullptr;      // 渲染帧的缓冲区 
    int m_int_RenderFrameLinesize = 0;                 // 渲染帧的每行字节数
    int m_int_OriFrameWidth = 0;                       // 上一帧宽度
    int m_int_OriFrameHeight = 0;                      // 上一帧高度

    //调整点相关
    SDL_Rect m_SDLRect_topPoint;       //上方调整点
    SDL_Rect m_SDLRect_bottomPoint;    //下方调整点
    SDL_Rect m_SDLRect_leftPoint;      //左方调整点
    SDL_Rect m_SDLRect_rightPoint;     //右方调整点

    //鼠标光标
    SDL_Cursor* m_SDLCursor_tb = nullptr;
    SDL_Cursor* m_SDLCursor_lr = nullptr;
    SDL_Cursor* m_SDLCursor_default = nullptr;

    //窗口大小调整相关
    AdjustPointType m_Enum_DraggingPoint = AdjustPointType::None; // 当前正在拖拽的点
    SDL_Point m_Point_DragStartMousePos;                          // 拖拽开始时的鼠标位置
    SDL_Rect m_Rect_DragStartWindowRect;                          // 拖拽开始时窗口的位置和大小
    bool m_Bool_UpdateWindowXY = false;
    int m_Int_dragStartX;
    int m_Int_dragStartY;
    int m_Int_dragDiffX;
    int m_Int_dragDiffY;

    // 界面属性
    CRect m_Rect_WindowArea;       // 窗口区域的位置和大小
    bool m_Bool_Running;           // 窗口是否正在运行
    bool m_Bool_SDLInitialized;    // SDL是否已初始化
    bool m_Bool_TTFInitialized;    // TTF是否已初始化
    bool m_Bool_BtnHoverd;         // 是否悬停到按钮上
    int m_Int_WindowHeight;        // 窗口的高度
    int m_Int_WindowWidth;         // 窗口的宽度
    float m_Scale = 1;             // 缩放系数
    int m_Int_FrameRate = 24;      // 摄像头帧数
    std::atomic<bool> m_Bool_IsMouseInWindow;   // 鼠标是否在窗口上

    // 界面控件
    std::vector<Ui_SDLMenuButton> m_Btn_ControlButtons; // 控制按钮集合

    // 回调函数
    std::function<void()> m_Func_OnCancelBindingRecord;   // 取消绑定按钮按下
private:
    Ui_CameraPreviewSDL();
    ~Ui_CameraPreviewSDL();
    static Ui_CameraPreviewSDL* Instance;
    static bool s_IsRunning;
};

