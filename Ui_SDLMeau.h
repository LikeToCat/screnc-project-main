#pragma once
#pragma once
#include <SDL.h>
#include <SDL_ttf.h>      
#include <SDL_image.h>
#include <string>
#include <functional>
#include <vector>
// SDL按钮控件类
class Ui_MenuButton
{
public:
    // 构造与析构
    Ui_MenuButton(
        const SDL_Rect& rect,
        const std::wstring& text,
        SDL_Color textColor,
        SDL_Color HoverTextColor,
        SDL_Color ClickTextColor,
        SDL_Color BorderCOlor,
        SDL_Color BkColor,
        SDL_Color BkHoverColor,
        SDL_Color BkClickColor,
        int BtnIndex,
        SDL_Renderer* render,
        TTF_Font* font,
        std::function<void(void*)> callback
    );
    ~Ui_MenuButton();

    // 渲染与交互方法
    void Render(); // 渲染按钮和文本
    void IsPosOnBtn(int x, int y, bool* IsOnMouse);
    void Click(void* param = nullptr);
    void SetBtnText(std::wstring BtnText);
    void AdjustTextRect(SDL_Rect TextRect);
    void SetExtraRender(std::function<void()> extraCallback);
    void SetTextFont(int size, bool border, std::string family = std::string());
    void SetNoInterActive(bool NoInterActice);
    std::wstring GetBtnText();
    void GetTextSize(int* Width, int* Height);
    void UpdateBtnState(bool IsHover, bool IsClicked);
    void GetBtnState(bool* IsHover, bool* IsClicked);
    void SetTransparentParam(int Alpha);
    SDL_Rect GetTextRect();
    int GetBtnIndex();
    void ShowBtn();
    void HideBtn();
    bool AddExtraRenderImage(CString Image, SDL_Rect ImageRect);
    void SetExtraImageShowState(bool IsSelect);
    void MoveToXY(int x, int y);
private:
    int IsPointInside(int x, int y) const;   // 检查坐标是否位于按钮上
    void CreateTextTexture();
    SDL_Rect CalculateCenterRect(int textWidth, int textHeight);
private:
    // 按钮区域属性
    int m_BtnHeight;
    int m_BtnText;
    SDL_Rect m_Rect_Text;
    SDL_Rect m_Rect_Area;
    SDL_Rect m_Rect_SelectImg;

    // 按钮显示属性
    std::wstring m_Str_Text;                  // 按钮上显示的文本(宽字符)
    SDL_Color m_textColor;                    // 文本颜色
    SDL_Color m_HoverTextColor;               // 文本悬停颜色
    SDL_Color m_ClickTextColor;               // 菜单按钮点击颜色
    SDL_Color m_BorderColor;                  // 菜单按钮边框颜色
    SDL_Color m_BkHoverColor;                 // 菜单按钮悬停颜色
    SDL_Color m_BkClickColor;                 // 菜单按钮点击颜色
    SDL_Color m_BkColor;                      // 菜单按钮默认背景色
    SDL_Texture* m_SDL_TextTexture;           // 菜单按钮文本纹理
    SDL_Texture* m_SDL_SelectTexture;         // 菜单按钮选中绘画图片纹理
    bool m_IsTextBorder;                      // 菜单按钮文本是否加粗
    bool m_IsInterActive;                     // 菜单按钮是否有互动效果
    bool m_IsHide;                            // 菜单按钮是否显示
    bool m_IsSelected;                        // 菜单项是否被选中
    int m_Alpha;                              // 透明度

    //按钮状态
    bool m_Bool_Hovered;                      // 鼠标是否悬停在按钮上
    bool m_Bool_Clicked;                      // 鼠标是否点击在按钮上

    //父窗口绘画对象
    SDL_Renderer* m_renderer = nullptr;     //父对象渲染器
    TTF_Font* m_ParentFont = nullptr;       //父对象字体
    TTF_Font* m_thisFont = nullptr;         //接口设置的本对象的字体

    //按钮回调
    int m_Index;                                       //当前按钮菜单中的下标
    std::function<void(void*)> m_Func_BtnCallback;  //当前按钮执行的按钮回调
    std::function<void()> m_Func_ExtraRenderCallback;  //菜单按钮额外绘制
};

class Ui_SDLMeau
{
public:
    Ui_SDLMeau(
        SDL_Rect MenuRect,
        int ItemHeight,
        SDL_Color BkColor,
        SDL_Color BorderColor,
        SDL_Renderer* pParentRender,
        TTF_Font* pParentFont
    );
    Ui_MenuButton* AddMenuBtn(const SDL_Rect& rect,
        const std::wstring& text,
        SDL_Color textColor,
        SDL_Color HoverTextColor,
        SDL_Color ClickTextColor,
        SDL_Color BorderCOlor,
        SDL_Color BkColor,
        SDL_Color BkHoverColor,
        SDL_Color BkClickColor,
        int BtnIndex,
        std::function<void(void*)> callback
    );
    Ui_MenuButton* GetBtnByIndex(int index);
    void UpdateMenuBtnState(int x, int y, bool IsClickOn);
    Ui_MenuButton* GetHoverBtn();
    void Render();
    void MoveMenuToXY(int x, int y);
    void ShowMenu();
    void HideMenu();
    bool IsMouseEnteredMenu()const;
    void SetTransparentParam(int Alpha);
    void ResetAllMenuBtnState();
    inline bool IsMenuHiding() { return m_IsHide; }
    int GetLastHoverIndex();
private:
    //菜单属性
    int m_Int_MeauWidth;    // 菜单宽度
    int m_Int_MeauHeight;   // 菜单高度
    int m_Int_ItemHeight;   // 每一个菜单项的高度
    SDL_Rect m_Rect_Menu;   // 菜单区域
    SDL_Color m_Color_Border;   // 菜单边框颜色 
    SDL_Color m_Color_BkColor;  // 菜单背景颜色    
    std::vector<Ui_MenuButton*> m_Vec_MeauBtns; // 菜单项所有按钮
    bool m_IsHide;            // 是否隐藏菜单
    bool m_IsMouseEnteredMenu;// 鼠标是否进入过菜单
    int m_Alpha;
    int m_LastHoverIndex;

    //菜单渲染器(继承自父对象)
    SDL_Renderer* m_Renderer = nullptr;
    TTF_Font* m_ParentFont = nullptr;
    TTF_Font* m_thisFont = nullptr;
};