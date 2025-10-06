#pragma once
#include <afxwin.h>
#include "resource.h"
#include "CLarToolsTips.h"
// AreaSelectDlg 对话框

class AreaSelectDlg : public CDialogEx
{
    DECLARE_DYNAMIC(AreaSelectDlg)

public:
    AreaSelectDlg(CWnd* pParent = nullptr);   // 标准构造函数
    virtual ~AreaSelectDlg();

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_AREASELECT_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
    void CaptureDesktop();
    void DoubleBufferPaint(CDC* cdc);
    void DrawGuidelines(CDC* pDC);
    void DrawTipBubble(CDC* pDC);
    double GetUserDPI();
public:
    inline const CRect GetSelectRect() { return m_SaveRect; }//获取选择的矩形
    inline void SetCrossLineWidth(int nWidth) { m_nCrossLineWidth = nWidth; }
    inline void SetCrossLineColor(COLORREF color) { m_CrossLineColor = color; }
    inline void SetDarkBkParam(int DrakParam) { m_iDarkParam = DrakParam; }
private:
    //窗口背景与区域选择功能相关
    CBitmap m_OriginalScreenBitmap;
    CBitmap m_DimmedScreenBitmap;
    int    m_nScreenWidth;
    int m_nScreenHeight;
    CRect m_SaveRect;
    bool isMouseHover;
    int m_iDarkParam = 3;
    double m_Scale;

    //十字红线相关
    CPoint m_MousePos;           // 当前鼠标位置
    int m_nCrossLineWidth;       // 十字线宽度
    COLORREF m_CrossLineColor;   // 十字线颜色

    //小气泡提示逻辑相关成员
    BOOL m_IsMouseMoving = FALSE;
    static const UINT MOUSE_TIEMR_ID = 1001;    //气泡提示定时器
    static const UINT MOUSE_STOP_DELAY = 500;   //气泡显示延迟
    struct TipsBuble
    {
        COLORREF bkColor;       //背景颜色
        COLORREF TextColor;     //字体颜色
        int TextSize;           //字体大小
        int x, y, width, height;//位置区域
        float argb;             //0 - 1，值越接近1，越不透明
    };
    TipsBuble m_TipsBuble;      //提示框气泡
};