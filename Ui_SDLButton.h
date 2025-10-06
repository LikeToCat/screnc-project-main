#pragma once
#include <SDL.h>
#include <SDL_ttf.h>      
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h>
#include <string>
#include <functional>
#include <vector>
// SDL按钮控件类
class Ui_SDLButton
{
public:
    // 构造与析构
    Ui_SDLButton(
        const SDL_Rect& rect,
        const std::wstring& text,
        SDL_Color textColor,
        SDL_Color HoverTextColor,
        SDL_Color ClickTextColor,
        SDL_Color BorderCOlor,
        SDL_Color BkColor,
        SDL_Color BkHoverColor,
        SDL_Color BkClickColor,
        CString ImagePath,
        SDL_Renderer* render,
        TTF_Font* font,
        std::function<void()> callback,
        int Uid,
        int cornerRadius = 0
    );
    ~Ui_SDLButton();
    static SDL_Rect ScaleRect(SDL_Rect original, float scale_factor);

    // 渲染与交互方法
    void Render(); // 渲染按钮和文本
    void IsPosOnBtn(int x, int y, bool* IsOnMouse);
    void Click();
    void SetBtnText(std::wstring BtnText);
    void AdjustTextRect(SDL_Rect TextRect, bool ClientRectAdapt = true);
    void AdjustTextRect(int offx, int offy);
    void AdjustImageRect(SDL_Rect ImageRect, bool ClientRectAdapt = true);
    void SetImageStretchParam(float StretchParam);
    void SetImageNeedStretch(bool Enable);
    void SetExtraRender(std::function<void()> extraCallback);     //在默认渲染的基础上，增加各种进一步的各种效果的渲染回调
    void SetTextFont(int size, bool border, std::string family = std::string());
    void SetNoInterActive(bool NoInterActice);
    void SetTransparentParam(int Aplha);
    void SetGdColor(SDL_Color gd1, SDL_Color gd2);
    void SetHGdColor(SDL_Color hgd1, SDL_Color hgd2);
    SDL_Rect CalculateCenterRect(SDL_Rect srcRect);
    std::wstring GetBtnText();
    const SDL_Rect& GetBtnRect();
    void GetImageSize(int* Width, int* Height);
    void GetTextSize(int* Width, int* Height);
    void UpdateBtnState(bool IsHover, bool IsClicked);
    void AdjustClientAreaByDiff(int diffx, int diffy, int extendx = 0, int extendy = 0);
    void GetBtnState(bool* IsHover, bool* IsClicked);
    void MoveBtnToXY(int x, int y);
    void MoveBtn(int left, int top, int width, int height);
    SDL_Rect GetImageRect();
    SDL_Rect GetTextRect();
    int GetUid();
    void HideBtn();
    void ShowBtn();
    void SetBtnDesc(std::wstring btndesc);
    std::wstring GetBtnDesc();
private:
    bool IsPointInside(int x, int y) const;   // 检查坐标是否位于按钮上
    void CreateTextTexture();
    void UpdateClientRect();
    SDL_Rect CalculateCenterRect(int textWidth, int textHeight);
private:
    // 按钮区域属性
    SDL_Rect m_Rect_Area;                     // 按钮的位置和大小
    SDL_Rect m_Rect_Image;                    // 按钮显示的图片资源位置与大小
    SDL_Rect m_Rect_Text;                     // 按钮显示文字位置与大小

    // 按钮显示属性
    std::wstring m_Str_Text;                  // 按钮上显示的文本(宽字符)
    SDL_Color m_textColor;                    // 文本颜色
    SDL_Color m_HoverTextColor;               // 文本悬停颜色
    SDL_Color m_ClickTextColor;               // 按钮点击颜色
    SDL_Color m_BorderColor;                  // 按钮边框颜色
    SDL_Color m_BkHoverColor;                 // 按钮悬停颜色
    SDL_Color m_BkClickColor;                 // 按钮点击颜色
    SDL_Color m_BkColor;                      // 按钮默认背景色
    SDL_Color m_gdColor1;                     // 按钮渐变色1
    SDL_Color m_gdColor2;                     // 按钮渐变色2
    SDL_Color m_hgdColor1;                    // 悬停时按钮渐变色1
    SDL_Color m_hgdColor2;                    // 悬停时按钮渐变色2
    SDL_Texture* m_SDL_TextTexture;           // 按钮文本纹理
    SDL_Texture* m_SDL_ImageTexture;          // 按钮图片
    float m_StretchParam;                     // 按钮图片缩放比例
    bool m_IsNeedStretch;                     // 按钮图片是否需要拉伸填充
    bool m_IsTextBorder;                      // 按钮文本是否加粗
    bool m_IsInterActive;                     // 按钮是否有互动效果
    bool m_IsHide;                            // 按钮是否隐藏
    bool m_IsGradient;                        // 按钮是否开启渐变色背景模式
    int m_Aplha;                              // 按钮不透明度
    int m_CornerRadius;                       // 按钮圆角

    //按钮状态
    bool m_Bool_Hovered;                      // 鼠标是否悬停在按钮上
    bool m_Bool_Clicked;                      // 鼠标是否点击在按钮上

    //父窗口绘画对象
    SDL_Renderer* m_renderer = nullptr;
    TTF_Font* m_ParentFont = nullptr;       //父对象字体
    TTF_Font* m_thisFont = nullptr;         //接口设置的本对象的字体

    //按钮回调
    std::function<void()> m_Func_Callback;           // 按钮点击时执行的回调函数
    std::function<void()> m_Func_ExtraRenderCallback;// 按钮额外绘制

    //资源标识
    int m_Uid;

    //按钮提示内容
    std::wstring m_wsBtnDesc;
};

class Ui_SDLButtonManager
{
public:
    void CreateBtn(const SDL_Rect& rect,
        const std::wstring& text,
        SDL_Color textColor,
        SDL_Color HoverTextColor,
        SDL_Color ClickTextColor,
        SDL_Color BorderCOlor,
        SDL_Color BkColor,
        SDL_Color BkHoverColor,
        SDL_Color BkClickColor,
        CString ImagePath,
        SDL_Renderer* render,
        TTF_Font* font,
        std::function<void()> callback,
        int Uid,
        int cornerRadius = 0
    );
    ~Ui_SDLButtonManager();
    void AddBtn(Ui_SDLButton* btn);
    std::vector<Ui_SDLButton*>& GetBtns();
    bool CheckPosInAnyBtn(int x, int y);
    void UpdateBtnsState(int x, int y, bool updateClick);
    Ui_SDLButton* GetInHoverOrClickedBtn();
    Ui_SDLButton* FindBtnByBtnText(std::wstring BtnText);
    Ui_SDLButton* FindBtnByUid(int uid);
    void RenderBtns();
    void SetTransparentParam(int alpha);
private:
    std::vector<Ui_SDLButton*> m_Vec_Btns;
};