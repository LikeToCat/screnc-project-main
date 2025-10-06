#include "CLarProgressCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////
// CGradientProgressCtrl Implementation
//////////////////////////////
BEGIN_MESSAGE_MAP(CGradientProgressCtrl, CProgressCtrl)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CGradientProgressCtrl::CGradientProgressCtrl()
{
    m_nLower = 0;
    m_nUpper = 100;
    m_nCurrentPosition = 0;
    m_nStep = 10;

    // Initialize color settings
    m_clrStart = RGB(219, 253, 255);
    m_clrEnd = RGB(0, 0, 205);
    m_clrBkGround = ::GetSysColor(COLOR_3DFACE);
    m_clrText = RGB(95, 60, 119);

    m_bShowPercent = FALSE;
}

void CGradientProgressCtrl::OnPaint()
{
    CPaintDC dc(this);

    if (m_nCurrentPosition <= m_nLower || m_nCurrentPosition >= m_nUpper)
    {
        CRect rect;
        GetClientRect(rect);
        CBrush brush;
        brush.CreateSolidBrush(::GetSysColor(COLOR_3DFACE));
        dc.FillRect(&rect, &brush);
        VERIFY(brush.DeleteObject());
        return;
    }

    CRect rectClient;
    GetClientRect(&rectClient);
    float maxWidth = (float)m_nCurrentPosition / (float)m_nUpper * rectClient.right;

    DrawGradient(&dc, rectClient, (int)maxWidth);

    if (m_bShowPercent)
    {
        CString percent;
        percent.Format(_T("%.2f%%"), 100.0f * (float)m_nCurrentPosition / (float)m_nUpper);
        dc.SetTextColor(m_clrText);
        dc.SetBkMode(TRANSPARENT);
        dc.DrawText(percent, rectClient, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
    }
}

BOOL CGradientProgressCtrl::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CGradientProgressCtrl::DrawGradient(CPaintDC* pDC, const RECT& rectClient, const int& nMaxWidth)
{
    RECT rectFill;
    float fStep;
    CBrush brush;

    CProgressMemDC memDC(pDC);

    int r = GetRValue(m_clrEnd) - GetRValue(m_clrStart);
    int g = GetGValue(m_clrEnd) - GetGValue(m_clrStart);
    int b = GetBValue(m_clrEnd) - GetBValue(m_clrStart);

    int nSteps = max(abs(r), max(abs(g), abs(b)));
    fStep = (float)rectClient.right / (float)nSteps;

    float rStep = r / (float)nSteps;
    float gStep = g / (float)nSteps;
    float bStep = b / (float)nSteps;

    r = GetRValue(m_clrStart);
    g = GetGValue(m_clrStart);
    b = GetBValue(m_clrStart);

    for (int i = 0; i < nSteps; i++)
    {
        ::SetRect(&rectFill,
            (int)(i * fStep),
            0,
            (int)((i + 1) * fStep),
            rectClient.bottom + 1);

        VERIFY(brush.CreateSolidBrush(RGB(r + (int)(rStep * i),
            g + (int)(gStep * i),
            b + (int)(bStep * i))));
        memDC.FillRect(&rectFill, &brush);
        VERIFY(brush.DeleteObject());

        if (rectFill.right > nMaxWidth)
        {
            ::SetRect(&rectFill, rectFill.right, 0, rectClient.right, rectClient.bottom);
            VERIFY(brush.CreateSolidBrush(m_clrBkGround));
            memDC.FillRect(&rectFill, &brush);
            VERIFY(brush.DeleteObject());
            return;
        }
    }
}

int CGradientProgressCtrl::SetPos(int nPos)
{
    m_nCurrentPosition = nPos;
    return CProgressCtrl::SetPos(nPos);
}

void CGradientProgressCtrl::SetRange(int nLower, int nUpper)
{
    m_nLower = nLower;
    m_nUpper = nUpper;
    m_nCurrentPosition = nLower;
    CProgressCtrl::SetRange(nLower, nUpper);
}

int CGradientProgressCtrl::SetStep(int nStep)
{
    m_nStep = nStep;
    return CProgressCtrl::SetStep(nStep);
}

int CGradientProgressCtrl::StepIt(void)
{
    m_nCurrentPosition += m_nStep;
    return CProgressCtrl::StepIt();
}


//////////////////////////////
// CLarProgressCtrl Implementation (Embedded Child Dialog)
//////////////////////////////
BEGIN_MESSAGE_MAP(CLarProgressCtrl, CDialog)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CLarProgressCtrl::CLarProgressCtrl()
    : CDialog(IDD_DIALOG_PROGRESS) // Ensure your dialog template ID is correct!
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    if (screen == NULL) {
        AfxMessageBox(L"无法获取屏幕 DC。");
        return;
    }
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);

    // 计算缩放因子（基准 DPI 为 96）
    m_scale = static_cast<double>(dpiX) / 96.0;
}

void CLarProgressCtrl::LarSetProgressCtrlPos(int pos)
{
    m_ProgressCtrl.SetPos(pos);
}

void CLarProgressCtrl::LarSetProgressCtrlRange(int lower, int upper)
{
    m_ProgressCtrl.SetRange(lower, upper);
}

void CLarProgressCtrl::LarSetProgressCtrlPercentText(bool enable)
{
    m_ProgressCtrl.ShowPercent(enable);
}

void CLarProgressCtrl::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    // Bind the control with ID PROGRESSDLG_PROGRESS to the embedded progress control.
    DDX_Control(pDX, PROGRESSDLG_PROGRESS, m_ProgressCtrl);
}

BOOL CLarProgressCtrl::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Remove the title bar and border for a child dialog look.
    ModifyStyle(WS_CAPTION | WS_BORDER, 0);

    // Initialize progress control properties.
    m_ProgressCtrl.SetRange(0, 100);
    m_ProgressCtrl.SetPos(0);
    m_ProgressCtrl.SetBkColor(RGB(250, 250, 250));
    CRect WindowRect;
    GetWindowRect(WindowRect);
    m_ProgressCtrl.ShowPercent(TRUE);
    //SetWindowPos(NULL, 0, 0, cx * m_scale, 12* m_scale, SWP_HIDEWINDOW);
    //m_ProgressCtrl.SetWindowPos(NULL, 0, 0, cx * m_scale, 12* m_scale, SWP_SHOWWINDOW);
    //m_ProgressCtrl.MoveWindow(0, 0, WindowRect.Width() + 285, 12);
    return TRUE;
}

// Override OnSize to ensure the progress control always fills the client area.
void CLarProgressCtrl::OnSize(UINT nType, int cx, int cy)
{
    //CDialog::OnSize(nType, cx, cy);
    
    if (m_ProgressCtrl.GetSafeHwnd())
    {
        CRect WindowRect;
        GetWindowRect(WindowRect);
        m_ProgressCtrl.SetWindowPos(NULL, 0, 0, WindowRect.Width(), WindowRect.Height(), SWP_SHOWWINDOW);
    }
}

BOOL CLarProgressCtrl::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(rect, RGB(22, 255, 255)); // 使用一个不透明的背景色
	return TRUE;
}
