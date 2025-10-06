#pragma once
#include <afxwin.h>
#include <memory>
#include <gdiplus.h>
#include <functional>
#pragma comment(lib,"gdiplus.lib")

class CLarPngBtn : public CButton
{
    DECLARE_DYNAMIC(CLarPngBtn)

public:
    CLarPngBtn();
    virtual ~CLarPngBtn();

    // 加载PNG图像 (需要在GDI关闭之前，手动调用ClearImages释放)
    BOOL LoadPNG(UINT resourceID);
    BOOL LoadPNG(const CString& filePath);
    BOOL LoadPNG(Gdiplus::Bitmap* bitmap);
    // 加载点击/悬停时显示的PNG图像
    BOOL LoadClickPNG(UINT resourceID);
    BOOL LoadClickPNG(const CString& filePath);
    BOOL LoadClickPNG(Gdiplus::Bitmap* bitmap);

    // 设置按钮的提示信息
    void SetToolTips(const CString& tipsText,bool IsDiplayImmediately);
    // 设置按钮提示信息的提示延迟时间
    void SetToolTipsDelayTime(int initialDelay = -1, int autoPopDelay = -1, int reshowDelay = -1);
    // 是否在鼠标悬停时使用 LoadClickPNG 加载的图片渲染按钮
    void SetUseHoverImage(BOOL bUse);
    // 在鼠标悬停时，对按钮背景应用一个高亮色
    void SetHoverEffectColor(BYTE alpha, BYTE red, BYTE green, BYTE blue);
    // 设置自定义背景颜色
    void SetBackgroundColor(COLORREF color);
    // 设置按钮点击效果颜色
    void SetClickEffectColor(BYTE alpha, BYTE red, BYTE green, BYTE blue);
    // 启用最高质量（抗锯齿、无模糊）
    void SetHighQualityPNG(BOOL bEnable);
    // 设置按钮的悬停回调
    void SetBtnHoverCallBack(std::function<void()> hoverCallback);
    // 设置鼠标离开按钮回调
    void SetBtnLeaveCallBack(std::function<void()> leaveCallback);
    // 设置按钮图像是否显示为圆形
    void SetCircularImage(BOOL bCircular, int radius = -1);
    // 设置圆形边框（当启用圆形显示时有效）
    void SetCircularBorder(BOOL bShowBorder, BYTE borderWidth = 1,
        BYTE alpha = 255, BYTE red = 0, BYTE green = 0, BYTE blue = 0);


    // 设置图片拉伸模式：旧接口（0=等比例居中，1=拉伸至整个按钮）
    void SetStretchMode(BOOL bStretch);
    void SetStretchMode(float fRatio);

    // 是否在鼠标悬停时显示手型光标
    void SetUseHandCursor(BOOL bUseHandCursor);

    void ClearImages();

    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    DECLARE_MESSAGE_MAP()

protected:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void PreSubclassWindow();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

private:
    std::unique_ptr<Gdiplus::Image> m_pImage;
    std::unique_ptr<Gdiplus::Image> m_pClickImage;
    bool  m_bUseCustomBgColor = false;
    COLORREF m_CustomBgColor = RGB(255, 255, 255);
    Gdiplus::Color m_ClickEffectColor = Gdiplus::Color(80, 0, 0, 0);

    BOOL  m_bStretchMode = TRUE;
    float m_fStretchRatio = 1.0f;

    BOOL m_bUseHandCursor = TRUE;
    BOOL m_bMouseTracking = FALSE;
    BOOL m_bUseHoverImage = FALSE;
    BOOL m_bMouseHover = FALSE;
    BOOL m_bUseHoverEffectColor = FALSE;
    bool m_bHighQualityPNG = false;
    Gdiplus::Color m_HoverEffectColor = Gdiplus::Color(80, 255, 255, 255);

    CString m_sToolTips;
    CToolTipCtrl m_ToolTip;
    BOOL m_bToolTipsEnabled;
    bool m_bToolTipsImmediate;
    std::function<void()> m_Func_HoverCallBack;
    std::function<void()> m_Func_LeaveCallback;

    //圆形按钮图片相关
    BOOL m_bCircularImage = FALSE;
    BOOL m_bShowCircularBorder = FALSE;
    BYTE m_nCircularBorderWidth = 1;
    Gdiplus::Color m_CircularBorderColor = Gdiplus::Color(255, 0, 0, 0);
    int m_nCircularRadius = -1;  // 圆形显示的半径，-1表示自动
};