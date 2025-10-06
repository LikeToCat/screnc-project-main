#pragma once
#include "afxwin.h"

class CColorfulEdit : public CEdit
{
    DECLARE_DYNAMIC(CColorfulEdit)

public:
    CColorfulEdit();
    virtual ~CColorfulEdit();

    void SetTextColor(COLORREF color);
    void SetBkColor(COLORREF color);
    void SetFontSize(int nSize);
    void SetPlaceholderText(LPCTSTR lpszText);
    void SetPlaceholderColor(COLORREF color);

protected:
    virtual void PreSubclassWindow();
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg void OnPaint();
    afx_msg void OnNcPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKillFocus(CWnd* pNewWnd);

private:
    void UpdateCtrl();

    COLORREF m_clrText;
    COLORREF m_clrBack;
    COLORREF m_clrBorder;
    int      m_nBorderWidth;
    CBrush   m_brBackground;
    bool     m_bHasFocus;

    CFont    m_font;
    int      m_nFontSize;
    bool     m_bVertCenter;

    CString  m_strPlaceholder;
    COLORREF m_clrPlaceholder;
};