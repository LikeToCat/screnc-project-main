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
#include <cmath>           // 添加数学函数支持
#include <SDL_image.h>
#include "Ui_MessageBoxSDL.h"
#include "ComponentClass.h"

// 设备项类，表示列表中的每个设备
class Ui_DeviceItem
{
public:
    std::wstring m_Str_Name;       // 设备名称
    std::string m_Str_Id;          // 设备ID
    bool m_Bool_IsCurrnet;         // 是否为当前设备
    bool m_Bool_Checked;           // 是否选中


    Ui_DeviceItem(const std::wstring& name = L"", const std::string& id = "", bool IsCurrnet = false, bool connected = true)
        : m_Str_Name(name), m_Str_Id(id), m_Bool_IsCurrnet(IsCurrnet), m_Bool_Checked(false) {}
};
// 区域录制SDL叠加界面，显示录制区域和控制按钮
class Ui_DeviceBindingSDL
{
    //按钮控件类
    class Ui_SDLMenuButton
    {
    public:
        // 构造与析构
        Ui_SDLMenuButton(const SDL_Rect& rect, const std::wstring& text,
            std::function<void()> callback); // 构造按钮，设置位置、文本和回调
        ~Ui_SDLMenuButton();                 // 析构函数，清理资源

        // 渲染与交互方法
        void Render(SDL_Renderer* renderer, TTF_Font* font); // 渲染按钮和文本
        bool IsPointInside(int x, int y) const;   // 检查坐标是否位于按钮上
        bool UpdateHoverState(int x, int y);      // 更新按钮的悬停状态
        void Click();                             // 触发按钮点击回调
        void UpdateClickState(bool isClick);
    public:
        std::wstring m_Str_Text;                  // 按钮上显示的文本(宽字符)
        SDL_Texture* m_SDL_TextTexture;           // 按钮文本纹理
    private:
        // 按钮属性
        SDL_Rect m_Rect_Area;                     // 按钮的位置和大小
        std::function<void()> m_Func_Callback;    // 按钮点击时执行的回调函数
        bool m_Bool_Hovered;                      // 鼠标是否悬停在按钮上
        bool m_Bool_Click = false;                // 按钮是否被点击状态
    };
public:
    static Ui_DeviceBindingSDL* GetInstance();
    static void ReleaseInstance();

    // 初始化与控制方法
    bool Initialize(const CRect& DisplayArea, float Scale); // 初始化SDL窗口和渲染器，设置录制区域
    void Run();                               // 运行SDL窗口的主循环
    void Close();                             // 关闭并清理SDL资源

    // 回调函数设置方法
    void SetOnCancelBindingCallback(std::function<void()> callback); // 设置取消绑定按钮

    // 设备列表管理方法
    void AddDevice(const std::wstring& name, std::string& id, bool IsCurrent);
    void ClearDevices();
    int GetDeviceCount() const { return static_cast<int>(m_Vec_DeviceItems.size()); }
    void SetDeviceChecked(int index, bool checked);
    bool IsDeviceChecked(int index) const;
    std::wstring GetDeviceName(int index) const;
    std::string GetDeviceId(int index) const;

    // 核心功能方法
    bool InitializeSDL();     // 初始化SDL库
    bool CreateSDLWindow();   // 创建SDL窗口和渲染器
    void ProcessEvents();     // 处理SDL事件，如鼠标点击和键盘输入
    void Render();            // 渲染窗口内容，包括面板、按钮和录制区域边框
    void CreateButtons();     // 创建和配置界面上的控制按钮

    // 字体和文本处理方法
    bool InitializeTTF();     // 初始化TTF库
    bool LoadFonts();         // 加载字体
    bool InitText();
    bool InitSDLImage();      // 初始化图片加载

    //渲染相关
    void RenderTitleText();
    void RenderTitleLogo();
    void RenderWindowShadow();
    void RenderTitlePanel();
    void RenderTipsInBottom();
    void RenderModalDialog();

    //弹框
    void ShowSureToCloseModalDialog(SDL_Renderer* renderer, TTF_Font* font);
    void ShowSureToUnBindModalDialog(SDL_Renderer* renderer);
    void ShowUnableBindModalDialog(SDL_Renderer* renderer);

    //调试打印
    void DebugSDLError(wchar_t error[256]);
private:
    // 设备列表方法
    void RenderDeviceList();              // 渲染设备列表
    void UpdateScrollPhysics();           // 更新滚动物理模型
    int GetDeviceAtPosition(int x, int y); // 获取指定位置的设备索引
    void CreateCheckboxTextures();        // 创建复选框纹理
    void RenderRoundedRect(int x, int y, int w, int h, int radius);   // 绘制圆角矩形
    SDL_Rect GetUnbindButtonRect(int deviceIndex);  // 获取解绑按钮的矩形区域
    bool IsPointInUnbindButton(int x, int y);// 检查点是否在解绑按钮内
    void UpdateBottomButtonText();// 根据选中的设备数量更新底部按钮文本
    bool LoadCloseButtonTexture();
    void UpdateCursorShape(bool shouldBeHand);

    //功能函数
    void DrawFilledCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius);
public:
    //小组件
    COMCLASS::SDL::WindowDrag* m_WindowDrager = nullptr;

    // SDL核心对象
    SDL_Window* m_SDL_Window;      // SDL窗口句柄
    SDL_Renderer* m_SDL_Renderer;  // SDL渲染器句柄
    TTF_Font* m_TTF_Font;          // TTF字体对象
    TTF_Font* m_TTF_Title;         // 标题字段

    //SDL弹框提示
    Ui_MessageBoxSDL* m_SDLWindow_SureToClose = nullptr;
    Ui_MessageBoxSDL* m_SDLModelWindow_SureToUnBind = nullptr;
    Ui_MessageBoxSDL* m_SDLModelWindow_UnableBind = nullptr;

    //滚动相关
    int m_Int_ItemsMaxScroll;       //多少列表项时开启滚动
    bool m_Bool_EnableScroll;       //启动滚动

    //图片渲染相关
    SDL_Surface* m_SDLSurface_LogoImage = nullptr;// 标题logo图
    SDL_Surface* m_SDLSurface_DeivceImage = nullptr;//电脑图片表面
    SDL_Texture* m_SDLTexture_LogoImage = nullptr;// logo纹理
    SDL_Texture* m_SDLTexture_DeivceImage = nullptr;// 电脑绑定图片纹理
    SDL_Texture* m_SDLTexture_DeivceHovingImage = nullptr;// 电脑绑定图片纹理(悬停效果)
    SDL_Texture* m_SDLTexture_Selected = nullptr;
    SDL_Texture* m_SDLTexture_TipsOfBinds = nullptr;//标题栏提示文本纹理
    SDL_Surface* m_SDLSurface_TipsOfBinds = nullptr;
    SDL_Texture* m_SDLTexture_TipsOfBindsInBottom = nullptr;
    SDL_Texture* m_SDLTexture_TipsSur = nullptr;
    SDL_Rect m_SDLRect_LogoImage;   //logo位置大小
    int m_int_DeviceImageWidth; //电脑高
    int m_int_DeviceImageHeight;//电脑宽
    int m_int_SelectedWidth;    //选中图标宽
    int m_int_SelectedHeight;   //选中图标高
    int m_Int_TipsSurWidth; //感叹号图标高
    int m_Int_TipsSurHeight; //感叹号图标宽

    //标题渲染相关
    SDL_Color m_TitleText_TextColor;
    SDL_Surface* m_TitleText_textSurface = nullptr;
    SDL_Texture* m_TitleText_textTure = nullptr;
    SDL_Rect m_SDLRect_Rect;
    SDL_Rect m_SDLRect_TipsOfBinds;
    SDL_Rect m_SDLRect_TipsInBottom;
    const wchar_t* m_TitleText_str = L"解除绑定";

    // 界面属性
    CRect m_Rect_RecordArea;       // 录制区域的位置和大小
    bool m_Bool_Running;           // 窗口是否正在运行
    bool m_Bool_SDLInitialized;    // SDL是否已初始化
    bool m_Bool_TTFInitialized;    // TTF是否已初始化
    bool m_Bool_BtnHoverd;         // 是否悬停到按钮上
    int m_Int_WindowHeight;        // 窗口的高度
    int m_Int_WindowWidth;         // 窗口的宽度
    int m_Int_TitleBarHeight;      // 标题栏高度
    int m_Int_TitleBarY;           // 标题栏y坐标
    bool m_Bool_ButtonHoved = false;       // 鼠标在按钮上
    float m_Scale = 1;

    // 设备列表相关
    std::vector<Ui_DeviceItem> m_Vec_DeviceItems;  // 设备列表
    Ui_DeviceItem m_Struct_DeleteDeviceItem;  // 被删除的设备列表
    float m_Float_CurrentScroll = 0.0f;            // 当前滚动位置
    float m_Float_TargetScroll = 0.0f;             // 目标滚动位置
    float m_Float_ScrollVelocity = 0.0f;           // 滚动速度
    float m_Float_ScrollDamping = 0.94f;           // 滚动阻尼系数
    float m_Float_ScrollSensitivity = 8.0f;        // 滚轮灵敏度
    Uint32 m_Uint32_LastFrameTime = 0;             // 上一帧时间戳
    int m_Int_DeviceItemHeight = 41;               // 设备项高度
    int m_Int_CheckboxSize = 20;                   // 复选框大小
    int m_Int_ItemPadding = 10;                    // 项内边距
    int m_Int_SelectIndex;

    // 复选框纹理
    SDL_Texture* m_SDL_CheckboxTexture = nullptr;          // 未选中纹理
    SDL_Texture* m_SDL_CheckboxCheckedTexture = nullptr;   // 选中纹理

    // 设备列表悬停和解绑按钮相关
    int m_Int_HoveredDeviceIndex = -1;      // 当前鼠标悬停的设备索引
    bool m_Bool_UnbindBtnHovered = false;   // 解绑按钮是否被悬停

    // 界面控件
    std::vector<Ui_SDLMenuButton> m_Btn_ControlButtons; // 控制按钮集合

    // 关闭按钮相关
    SDL_Texture* m_SDL_CloseButtonTexture = nullptr;
    SDL_Rect m_Rect_CloseButton;
    bool m_Bool_CloseButtonHovered = false;

    // 鼠标光标相关
    SDL_Cursor* m_SDL_DefaultCursor = nullptr;
    SDL_Cursor* m_SDL_HandCursor = nullptr;
    bool m_Bool_CursorIsHand = false;

    // 回调函数
    std::function<void()> m_Func_OnCancelBindingRecord;   // 取消绑定按钮按下
    void ResetAllButtonStates();
private:
    Ui_DeviceBindingSDL();
    ~Ui_DeviceBindingSDL();
    static Ui_DeviceBindingSDL* Instance;
};