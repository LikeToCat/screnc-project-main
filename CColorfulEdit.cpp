#include "stdafx.h"  
#include "CColorfulEdit.h"

IMPLEMENT_DYNAMIC(CColorfulEdit, CEdit)

BEGIN_MESSAGE_MAP(CColorfulEdit, CEdit)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_PAINT()
    ON_WM_NCPAINT()
    ON_WM_ERASEBKGND()
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// 构造函数，初始化默认颜色
CColorfulEdit::CColorfulEdit()
    : m_clrText(RGB(0, 0, 0))          // 默认黑色文本
    , m_clrBack(RGB(255, 255, 255))    // 默认白色背景
    , m_clrBorder(RGB(169, 169, 169))  // 默认灰色边框
    , m_nBorderWidth(1)                // 默认边框宽度
    , m_bHasFocus(false)
    , m_nFontSize(16)                  // 默认字体大小
    , m_bVertCenter(false)             // 默认不垂直居中
{
}

CColorfulEdit::~CColorfulEdit()
{
    if (m_brBackground.GetSafeHandle())
        m_brBackground.DeleteObject();
    if (m_font.GetSafeHandle())
        m_font.DeleteObject();
}

// 设置文本颜色
void CColorfulEdit::SetTextColor(COLORREF color)
{
    m_clrText = color;
    UpdateCtrl();
}

// 设置背景颜色
void CColorfulEdit::SetBkColor(COLORREF color)
{
    m_clrBack = color;

    // 重新创建背景画刷
    if (m_brBackground.GetSafeHandle())
        m_brBackground.DeleteObject();
    m_brBackground.CreateSolidBrush(m_clrBack);

    UpdateCtrl();
}

// 更新控件
void CColorfulEdit::UpdateCtrl()
{
    // 强制完全repaint
    if (GetSafeHwnd())
    {
        RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
    }
}

void CColorfulEdit::SetFontSize(int nSize)
{
    m_nFontSize = nSize;

    // 创建新字体
    m_font.DeleteObject();
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = -m_nFontSize; // 负值表示字体高度
    lf.lfWeight = FW_NORMAL;    // 正常粗细
    lf.lfCharSet = DEFAULT_CHARSET;
    _tcscpy_s(lf.lfFaceName, _T("微软雅黑")); // 字体名称，可根据需要更改
    m_font.CreateFontIndirect(&lf);

    // 设置字体
    if (GetSafeHwnd())
    {
        SetFont(&m_font);
        UpdateCtrl();
    }
}

void CColorfulEdit::SetPlaceholderText(LPCTSTR lpszText)
{
    m_strPlaceholder = lpszText;
    UpdateCtrl();
}

void CColorfulEdit::SetPlaceholderColor(COLORREF color)
{
    m_clrPlaceholder = color;
    UpdateCtrl();
}

// 自定义文本和背景颜色
HBRUSH CColorfulEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
    pDC->SetTextColor(m_clrText);
    pDC->SetBkColor(m_clrBack);

    // 使用自定义字体
    if (m_font.GetSafeHandle())
        pDC->SelectObject(&m_font);

    if (m_brBackground.GetSafeHandle() == NULL)
        m_brBackground.CreateSolidBrush(m_clrBack);

    return m_brBackground;
}

// 处理WM_PAINT消息
void CColorfulEdit::OnPaint()
{
    Default();  // 让默认处理绘制编辑框内容
    CString strText;
    GetWindowText(strText);
    CDC* pDC = GetDC();
    if (strText.IsEmpty() && !m_bHasFocus && !m_strPlaceholder.IsEmpty())
    {
        if (pDC)
        {
            CFont* pOld = pDC->SelectObject(&m_font);
            pDC->SetTextColor(m_clrPlaceholder);
            pDC->SetBkMode(TRANSPARENT);
            CRect rc;
            SendMessage(EM_GETRECT, 0, (LPARAM)&rc);

            pDC->DrawText(m_strPlaceholder, &rc, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX);
            pDC->SelectObject(pOld);
            ReleaseDC(pDC);
        }
    }
}


// 非客户区repaint
void CColorfulEdit::OnNcPaint()
{
    // 基类处理
    Default();
}

// 背景擦除
BOOL CColorfulEdit::OnEraseBkgnd(CDC* pDC)
{
    return TRUE; // 背景已处理
}

void CColorfulEdit::PreSubclassWindow()
{
    CEdit::PreSubclassWindow();

    // 去掉系统默认边框
    ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);
    ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);

    // 初始化字体
    SetFontSize(m_nFontSize);
}

void CColorfulEdit::OnSetFocus(CWnd* pOldWnd)
{
    CEdit::OnSetFocus(pOldWnd);
    m_bHasFocus = true;
    UpdateCtrl();
}

void CColorfulEdit::OnKillFocus(CWnd* pNewWnd)
{
    CEdit::OnKillFocus(pNewWnd);
    m_bHasFocus = false;
    UpdateCtrl();
}