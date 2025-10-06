#include "stdafx.h"
#include "CLarToolsTips.h"
using namespace Gdiplus;
// 窗口类名定义
const LPCTSTR CLarToolsTips::m_lpszClassName = L"CLarToolsTipsClass";

IMPLEMENT_DYNAMIC(CLarToolsTips, CWnd)

BEGIN_MESSAGE_MAP(CLarToolsTips, CWnd)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CLarToolsTips::CLarToolsTips()
{
    m_Scale = GetUserDPI();
    m_Color_border = Color(103, 103, 103);
    m_Color_BkColor = Color(36, 37, 40);
    m_Color_textColor = Color(255, 255, 255);
    m_int_borderSize = 4;
    m_bool_IsAlawysDisplay = true;
    m_TimePoint_Display = std::chrono::system_clock::time_point(std::chrono::milliseconds(3000));
    m_Str_Tips = L"提示";
    m_Uint_Timer = 1000;
    m_CRect_Window.SetRect(0, 0, 100, 100);
    m_WindowWidth = 100;
    m_WindowHeight = 100;
    m_fontSize = 14 * m_Scale;
    m_IsDisplayShadow = false;
}

CLarToolsTips::~CLarToolsTips()
{
}

void CLarToolsTips::LarSetBorderColor(Gdiplus::Color borderColor)
{
    m_Color_border = borderColor;
    Invalidate();
}

void CLarToolsTips::LarSetBorderSize(int size)
{
    m_int_borderSize = size;
    Invalidate();
}

void CLarToolsTips::LarSetTextColor(Gdiplus::Color textColor)
{
    m_Color_textColor = textColor;
    Invalidate();
}

void CLarToolsTips::LarSetBkColor(Gdiplus::Color bkColor)
{
    m_Color_BkColor = bkColor;
}

BOOL CLarToolsTips::RegisterWindowClass()
{
    WNDCLASS wndClass;
    HINSTANCE hInstance = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInstance, m_lpszClassName, &wndClass)))
    {
        wndClass.style = CS_SAVEBITS;
        wndClass.lpfnWndProc = ::DefWindowProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = NULL;
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = NULL;
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = m_lpszClassName;

        if (!AfxRegisterClass(&wndClass))
            return FALSE;
    }

    return TRUE;
}

float CLarToolsTips::GetUserDPI()
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);

    // 计算缩放因子（基准 DPI 为 96）
    double scaleX = static_cast<double>(dpiX) / 96.0;
    double scaleY = static_cast<double>(dpiY) / 96.0;
    return scaleY;
}

void CLarToolsTips::LarSetTextSize(int fontSize)
{
    m_fontSize = fontSize * m_Scale;
    Invalidate();
}

void CLarToolsTips::LarSetShadowEnable(bool IsEnableShadow)
{
    m_IsDisplayShadow = IsEnableShadow;
    Invalidate();
}

void CLarToolsTips::LarSetTipText(const CString& text)
{
    m_Str_Tips = text;
    Invalidate();
}

void CLarToolsTips::LarSetToolTipsDisplayTime(int millSeconds)
{
    m_bool_IsAlawysDisplay = false;
    m_TimePoint_Display = std::chrono::steady_clock::time_point(std::chrono::milliseconds(millSeconds));
}

BOOL CLarToolsTips::LarCreate(CRect gWindowRect, CWnd* pParentWnd)
{
    if (!RegisterWindowClass())
        return FALSE;

    bool IsCreateSuccess = CWnd::CreateEx(
        WS_EX_TOPMOST,
        m_lpszClassName,
        L"",
        WS_POPUP,
        gWindowRect.left, gWindowRect.top, gWindowRect.Width(), gWindowRect.Height(),
        pParentWnd ? pParentWnd->GetSafeHwnd() : NULL,
        NULL);

    if (!IsCreateSuccess)
        return false;

    m_CRect_Window = gWindowRect;
    m_WindowWidth = gWindowRect.Width();
    m_WindowHeight = gWindowRect.Height();
    m_Shadow.Create(m_hWnd);
    ShowWindow(SW_HIDE);

    InitToolWindow(false);
    return true;
}

int CLarToolsTips::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CLarToolsTips::ShowToolTipsWindow(int x, int y, int diffwidth, int diffheight)
{
    //默认字体为微软雅黑字体，根据当前m_Str_Tips字符串内容，以及当前的m_Scale缩放系数，以及当前字体大小，测算出字体所占的长度和宽度，
    //然后根据测算出的字体的长度和宽度，赋值m_CRect_Window，m_WindowWidth，m_WindowHeight这三个成员，同时调用MoveWindow更新窗口区域
    //如果diffwidth或者diffheight不为0，则根据根据这两个值来调整窗口的宽高

    // 创建临时的Graphics对象用于测量文本
    Bitmap tempBitmap(1, 1);
    Graphics tempGraphics(&tempBitmap);
    tempGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // 创建字体对象
    FontFamily fontFamily(L"微软雅黑");
    Gdiplus::Font font(&fontFamily, m_fontSize, FontStyleRegular, UnitPixel);

    // 设置测量文本的字符串格式
    StringFormat stringFormat;
    if (m_Str_Tips.Find(L'\n') != -1 || m_Str_Tips.GetLength() > 30)
    {
        stringFormat.SetFormatFlags(0); // 允许自动换行
    }
    else 
    {
        stringFormat.SetFormatFlags(StringFormatFlagsNoWrap); // 不自动换行
    }

    // 测量文本所需的区域大小
    RectF boundingBox;
    int maxWidth = 400; // 最大宽度约束
    RectF layoutRect(0, 0, (float)maxWidth, 0); // 高度设为0以获取所需的高度
    tempGraphics.MeasureString(
        m_Str_Tips, -1, &font, layoutRect, &stringFormat, &boundingBox);

    // 计算窗口所需的宽度和高度
    int textWidth = (int)ceil(boundingBox.Width);
    int textHeight = (int)ceil(boundingBox.Height);
    int windowWidth = textWidth + m_int_borderSize * 2 + 10 * m_Scale + diffwidth;
    int windowHeight = textHeight + m_int_borderSize * 2 + 2 * m_Scale + diffheight;

    // 设置窗口位置和大小自适应
    m_WindowWidth = windowWidth;
    m_WindowHeight = windowHeight;
    m_CRect_Window.SetRect(x, y, x + windowWidth, y + windowHeight);
    MoveWindow(m_CRect_Window);
    if (!m_bool_IsAlawysDisplay)
    {
        UINT millSeconds = static_cast<UINT>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                m_TimePoint_Display.time_since_epoch()).count());
        SetTimer(m_Uint_Timer, millSeconds, NULL);
    }
    ShowWindow(SW_SHOW);
    Invalidate();
}

void CLarToolsTips::ShowToolTipsWindow(CRect gWindowRect)
{
    m_CRect_Window = gWindowRect;
    m_WindowWidth = gWindowRect.Width();
    m_WindowHeight = gWindowRect.Height();
    MoveWindow(gWindowRect);
    if (!m_bool_IsAlawysDisplay)
    {
        UINT millSeconds = static_cast<UINT>(std::chrono::duration_cast<std::chrono::milliseconds>(m_TimePoint_Display.time_since_epoch()).count());
        SetTimer(m_Uint_Timer, millSeconds, NULL);
    }
    ShowWindow(SW_SHOW);
    Invalidate();
}

BOOL CLarToolsTips::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}


void CLarToolsTips::OnPaint()
{
    CPaintDC dc(this);
    if(m_IsDisplayShadow)
        m_Shadow.Show(m_hWnd);
    //预缓冲Gdiplus对象
    using namespace Gdiplus;
    Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
    Graphics memGraphics(&memBitmap);

    //设置文本绘画高质量 
    memGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    

    //绘画窗口背景
    SolidBrush bkBrush(m_Color_BkColor);
    memGraphics.FillRectangle(&bkBrush, 0, 0, m_WindowWidth, m_WindowHeight);

    //绘画文本内容
    DrawMyText(&memGraphics);

    //绘画窗口边框
    SolidBrush boarderBrush(m_Color_border);
    Pen boarderPen(&boarderBrush, m_int_borderSize);
    memGraphics.DrawRectangle(&boarderPen, 0, 0, m_WindowWidth, m_WindowHeight);

    //一次性绘画到窗口上
    Graphics graphice(dc.GetSafeHdc());
    graphice.DrawImage(&memBitmap, 0, 0, m_WindowWidth, m_WindowHeight);
}

void CLarToolsTips::DrawMyText(Gdiplus::Graphics* memGraphics)
{
    // 创建字体对象
    FontFamily fontFamily(L"微软雅黑");
    Gdiplus::Font font(&fontFamily, m_fontSize, FontStyleRegular, UnitPixel);

    // 创建文本画刷
    SolidBrush textBrush(m_Color_textColor);

    // 计算文本绘制区域（考虑边框大小）
    int borderOffset = m_int_borderSize + 5 * m_Scale; // 边框大小加额外留白
    RectF textRect(
        (float)borderOffset,
        (float)borderOffset,
        (float)(m_WindowWidth - 2 * borderOffset),
        (float)(m_WindowHeight - 2 * borderOffset)
    );

    // 创建字符串格式，支持换行和垂直居中
    StringFormat stringFormat;
    stringFormat.SetAlignment(StringAlignmentCenter);             // 水平左对齐
    stringFormat.SetLineAlignment(StringAlignmentCenter);       // 垂直居中对齐

    // 如果检测到换行符或长文本，则启用自动换行
    if (m_Str_Tips.Find(L'\n') != -1 || m_Str_Tips.GetLength() > 30) 
    {
        stringFormat.SetFormatFlags(0); // 允许文本自动换行
    }
    else 
    {
        // 单行文本，不自动换行
        stringFormat.SetFormatFlags(StringFormatFlagsNoWrap);
    }

    // 绘制文本
    memGraphics->DrawString(
        m_Str_Tips,            // 文本内容
        -1,                    // 字符串长度 (-1表示自动计算)
        &font,                 // 字体
        textRect,              // 绘制区域
        &stringFormat,         // 字符串格式
        &textBrush             // 文本画刷
    );  
}

void CLarToolsTips::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == m_Uint_Timer)
    {
        KillTimer(m_Uint_Timer);
        ShowWindow(SW_HIDE);
    }
    CWnd::OnTimer(nIDEvent);
}

void CLarToolsTips::InitToolWindow(bool IsMinimalWithMain)
{
    LONG exStyle = ::GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE);
    exStyle &= ~WS_EX_APPWINDOW;
    exStyle |= WS_EX_TOOLWINDOW;
    ::SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, exStyle);
    ::SetWindowPos(
        this->GetSafeHwnd(),
        NULL, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
    );
    if (!IsMinimalWithMain)
    {
        ::SetParent(m_hWnd, ::GetDesktopWindow());
        ::SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)::GetDesktopWindow());
    }
}
