#if !defined(AFX_LARPROGRESSCTRL_H__INCLUDED_)
#define AFX_LARPROGRESSCTRL_H__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afxwin.h>
#include <afxcontrolbars.h>     // MFC support for ribbons and control bars
#include <afxcmn.h>             // MFC support for Windows common controls
#include <afxdtctl.h>           // MFC support for IE 4 common controls
#include <afxext.h>             // MFC extensions
#include "resource.h"

// -------------------------
// Internal helper class: CProgressMemDC
// -------------------------
// A helper class for drawing offscreen to reduce flicker.
class CProgressMemDC : public CDC
{
public:
    // Constructor: constructs an offscreen device context from a given DC.
    CProgressMemDC(CDC* pDC) : CDC()
    {
        ASSERT(pDC != NULL);
        m_pDC = pDC;
        m_pOldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();

        if (m_bMemDC)
        {
            pDC->GetClipBox(&m_rect);
            CreateCompatibleDC(pDC);
            m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
            m_pOldBitmap = SelectObject(&m_bitmap);
            SetWindowOrg(m_rect.left, m_rect.top);
        }
        else
        {
            m_bPrinting = pDC->m_bPrinting;
            m_hDC = pDC->m_hDC;
            m_hAttribDC = pDC->m_hAttribDC;
        }
    }

    // Destructor: copies the offscreen DC content to the original DC.
    ~CProgressMemDC()
    {
        if (m_bMemDC)
        {
            m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
                this, m_rect.left, m_rect.top, SRCCOPY);
            SelectObject(m_pOldBitmap);
        }
        else
        {
            m_hDC = m_hAttribDC = NULL;
        }
    }

    CProgressMemDC* operator->() { return this; }
    operator CProgressMemDC* () { return this; }

private:
    CBitmap  m_bitmap;
    CBitmap* m_pOldBitmap;
    CDC* m_pDC;
    CRect    m_rect;
    BOOL     m_bMemDC;
};

// ----------------------------
// Internal control class: CGradientProgressCtrl
// ----------------------------
// Inherits from CProgressCtrl to create a gradient effect and display percentage text.
class CGradientProgressCtrl : public CProgressCtrl
{
public:
    CGradientProgressCtrl();

    void SetRange(int nLower, int nUpper);
    int SetPos(int nPos);
    int SetStep(int nStep);
    int StepIt(void);

    // Color setting interfaces
    void SetTextColor(COLORREF color) { m_clrText = color; }
    void SetBkColor(COLORREF color) { m_clrBkGround = color; }
    void SetStartColor(COLORREF color) { m_clrStart = color; }
    void SetEndColor(COLORREF color) { m_clrEnd = color; }

    // Enable or disable percent display
    void ShowPercent(BOOL bShowPercent = TRUE) { m_bShowPercent = bShowPercent; }

    // Get interfaces
    COLORREF GetTextColor(void) { return m_clrText; }
    COLORREF GetBkColor(void) { return m_clrBkGround; }
    COLORREF GetStartColor(void) { return m_clrStart; }
    COLORREF GetEndColor(void) { return m_clrEnd; }

protected:
    // Overridden painting function
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    DECLARE_MESSAGE_MAP()

    // Helper function to draw the gradient
    void DrawGradient(CPaintDC* pDC, const RECT& rectClient, const int& nMaxWidth);

    int m_nLower, m_nUpper, m_nStep, m_nCurrentPosition;
    COLORREF m_clrStart, m_clrEnd, m_clrBkGround, m_clrText;
    BOOL m_bShowPercent;
    double m_scale;
};

// ------------------------------
// External interface class: CLarProgressCtrl
// ------------------------------
// This dialog class is used exclusively to display the custom progress bar control.
// It is implemented as a child dialog, and when resized or moved, the progress bar
// control resizes accordingly.
class CLarProgressCtrl : public CDialog
{
public:
    // Constructor; pParent can be specified as the parent window pointer.
    CLarProgressCtrl();

    // External interfaces to control the embedded progress bar.
    void LarSetProgressCtrlPos(int pos);
    void LarSetProgressCtrlRange(int lower, int upper);
    void LarSetProgressCtrlPercentText(bool enable);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    // Overridden to catch resize events.
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()

public:
    // Embedded custom progress control.
    CGradientProgressCtrl m_ProgressCtrl;
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
    double m_scale;
};

#endif // !defined(AFX_LARPROGRESSCTRL_H__INCLUDED_)