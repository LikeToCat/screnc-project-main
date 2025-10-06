#pragma once

#include <SDL.h>
#include <SDL_ttf.h>      
#include <SDL_syswm.h>    
#include <SDL_image.h>
#include <windows.h>       // Windows API
#include <vector>
#include <functional>
#include <string>
#include <sstream>
#include <memory>
#include <atltypes.h>      // For CRect
#include <atomic>
#include <thread>
#include <mutex>
#include "Ui_SDLButton.h"
#include "Ui_SDLMeau.h"
#include "ComponentClass.h"
#include "ComponentAPI.h"
// 区域录制SDL叠加界面，显示录制区域和控制按钮=
class Ui_AreaRecordingSDL
{
public:
    static Ui_AreaRecordingSDL* GetInstance();
    static bool IsInstanceExist() { return ins == nullptr ? false : true; }
    static bool IsInsRuning() { return s_bool_IsRunning; }
    void ReleaseInstance();
    ~Ui_AreaRecordingSDL();   // 析构函数，释放资源
    inline bool GetRunningState() { return m_Bool_Running; }
    inline void SetRunningState(bool state) { m_Bool_Running = state; }
    inline void PauseWindow() { m_Bool_IsWindowPause = true; }
    inline void ResumeWindowFromPause() { m_Bool_IsWindowPause = false; }
    inline int getRecordTime() { return m_CurrentRecordTimeSec; }
    CRect getRecordRect();
    void SetUiModeDuringRecord();
    void ResumeUiModeFromRecord();
    void HideSDLWindow();
    void ShowSDLWindow();
    void SetDashedBorder(bool enable);
    void UpdateCenterPanelText(std::wstring CenterText);
    void SetWindowOpciaity(int alpha);
    void SetRedBorderSize(int Size);
    void SetInteractionEnable(bool enable);
    void OnPauseRecordingUi();         // 关计时、关动态虚线、禁止区域更新
    void OnResumeRecordingUi();        // 开计时、开动态虚线、恢复区域更新

    // 初始化与控制方法
    bool Initialize(const CRect& recordArea); // 初始化SDL窗口和渲染器，设置录制区域
    void Run();                               // 运行SDL窗口的主循环
    void Close();                             // 关闭并清理SDL资源
    inline int GetPanelHeight() { return m_Int_PanelHeight; }// 获取按钮工具栏高度
private:
    // 核心功能方法
    bool InitializeSDL();     // 初始化SDL库
    bool CreateSDLWindow();   // 创建SDL窗口和渲染器
    void ProcessEvents();     // 处理SDL事件，如鼠标点击和键盘输入
    void RenderToTexture();   // 渲染窗口内容，包括面板、按钮和录制区域边框
    std::wstring  GetCurTimeCountStr();
    void RenderToLayerWindow();//渲染完整窗口内容到分层窗口
    void CreateButtons();     // 创建和配置界面上的控制按钮
    void CreateMenu();        // 创建菜单

    // 字体和文本处理方法
    bool InitializeTTF();     // 初始化TTF库
    bool LoadFonts();         // 加载字体

    //按钮响应
    void OnBnClicked_StartReocrd();
private://辅助函数
    float getUserDpi();
    void RenderDashedRect(SDL_Renderer* renderer, const SDL_Rect& rect); //绘画录制中的虚线效果
    bool IsMouseOnPanel(int x,int y);
    void CalucateCenterRecordArea(CRect* recordRect);
    SDL_Rect CalculatePanelCenterRect(int Width, int Height, int diffx = 0, int diffy = 0);
    SDL_Rect CalculatePanelLeftRect(int Width, int Height, int diffx = 0, int diffy = 0);
private:
    HWND m_Hwnd;    //窗口句柄
    std::vector<Uint32> m_PixelBuffer;//不透明度相关
    float m_Scale;

    // SDL核心对象
    SDL_Window* m_SDL_Window = nullptr;       // SDL窗口句柄
    SDL_Renderer* m_SDL_Renderer = nullptr;   // SDL渲染器句柄
    TTF_Font* m_TTF_Font = nullptr;           // TTF字体对象
    SDL_Texture* m_SDLTexture_PanelCenterText = nullptr;// 按钮面板中间的录制区域绘制
    SDL_Texture* m_SDLTexture_Transparent = nullptr;    // 透明纹理
    SDL_Texture* m_SDLTexture_RecordingText = nullptr;  // 录制中的计数纹理

    //SDL组件
    COMCLASS::SDL::WindowDrag* m_WindowDrager;//窗口拖拽管理器
    
    // 界面属性
    CRect m_Rect_RecordArea;       // 录制区域的位置和大小
    CRect m_ClientRect_PanelArea;  // 按钮面板区域
    bool m_Bool_Running;           // 窗口是否正在运行
    bool m_Bool_SDLInitialized;    // SDL是否已初始化
    bool m_Bool_TTFInitialized;    // TTF是否已初始化
    int m_Int_PanelHeight = 50;    // 控制面板的高度
    int m_Int_RedLineWidth = 2;
    COLORREF m_COLORREF_TransparentColor; // 透明色值
    int m_Int_DragStartX;           //刚开始拖动时鼠标的X坐标（相对于窗口）
    int m_Int_DragStartY;           //刚开始拖动时鼠标的Y坐标（相对于窗口）
    int m_Int_WindowStartX = 0;     // 窗口拖动开始时的X坐标
    int m_Int_WindowStartY = 0;     // 窗口拖动开始时的Y坐标
    std::wstring m_Str_PanelCenterText;
    int m_CenterTextWidth;
    int m_CenterTextHeight;
    bool m_IsAnyBtnHover = false;    // 是否有悬停到任何按钮上
    bool m_IsAnyMenuBtnHover = false;// 是否有悬停到任何菜单按钮上
    int m_Alpha = 255;               // 不透明度
    Ui_MenuButton* m_Btn_CurSelect;  // 当前选中的设备
    SDL_Rect m_Rect_MenuBtnMenuRect; // 菜单按钮菜单
    SDL_Rect m_Rect_TransparentRect; // 透明度菜单
    int m_RedBorderSize = 1;         // 红色边框厚度
    bool m_IsTimeCountActive = false;    // 是否开启录制时间技术
    std::chrono::steady_clock::time_point m_Time_Start;//录制开始的时间
    std::wstringstream m_StrStream;      // 计时器格式化流
    bool m_Bool_IsWindowPause = false;   // 窗口线程是否暂停
    std::wstring m_wstring_curCenterText;// 当前面板中间显示的文本
    bool m_bool_Interaction = false;

    //虚线移动
    bool  m_Bool_DashedBorder = false;  // 当 true 时使用虚线
    int   m_Int_DashOffset = 0;      // 动画偏移（每帧累加）
    int   m_Int_DashLength = 5;      // 虚线段长度（像素）
    int   m_Int_GapLength = 5;       // 间隔长度（像素）

    // 界面控件
    Ui_SDLButtonManager m_BtnManager;       // 控制按钮集合
    Ui_SDLMeau* m_TransparentMenu;          // 不透明度选择菜单
    Ui_SDLMeau* m_MenuBtnMenu;              // 菜单按钮菜单
    Ui_SDLButton* m_Btn_PauseSDL = nullptr; // 暂停按钮
    Ui_SDLButton* m_Btn_ResumeSDL = nullptr;// 恢复按钮

    // 回调函数
    std::function<void(const CRect&)> m_Func_OnStart;  // 开始录制的回调函数
    std::function<void()> m_Func_OnStop;   // 停止录制的回调函数
    std::function<void()> m_Func_OnCancel; // 取消录制的回调函数
    std::function<void(const CRect&)> m_Func_OnUpdateRect; // 录制区域实时更新回调函数

    // 鼠标指针相关
    SDL_Cursor* m_SDL_HandCursor = nullptr;      // 手型鼠标指针
    SDL_Cursor* m_SDL_DefaultCursor = nullptr;   // 默认鼠标指针
    bool m_Bool_IsHandCursorShown = false;       // 是否显示手型鼠标指针

    //拖拽计算
    bool   m_Bool_IsDragging;    // 是否正在拖拽
    int    m_DragOffsetX;        // 鼠标按下时，鼠标点相对于窗口左上角的 X 偏移
    int    m_DragOffsetY;        // 鼠标按下时，鼠标点相对于窗口左上角的 Y 偏移

    // 录制面板计时的“暂停累加”相关
    bool m_IsPaused = false;                                                    
    std::chrono::steady_clock::time_point m_PauseStart;                         
    std::chrono::steady_clock::duration    m_PausedAccum = std::chrono::seconds::zero(); 
    bool m_SuppressUpdates = false;
    int m_CurrentRecordTimeSec;


private:
    Ui_AreaRecordingSDL();    // 构造函数，初始化成员变量
    static Ui_AreaRecordingSDL* ins;
    static bool s_bool_IsRunning;
};