#include "stdafx.h"
#include "AreaSelectDlg.h"
#include "afxdialogex.h"


// AreaSelectDlg 对话框

IMPLEMENT_DYNAMIC(AreaSelectDlg, CDialogEx)
AreaSelectDlg::AreaSelectDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_AREASELECT_DIALOG, pParent)
{
    m_Scale = GetUserDPI();
    m_SaveRect.left = m_SaveRect.right = 0;
    m_SaveRect.top = m_SaveRect.bottom = 0;
    isMouseHover = false; 
    m_MousePos = CPoint(0, 0);
    m_nCrossLineWidth = 1;                 
    m_CrossLineColor = RGB(255, 0, 0);     
    m_TipsBuble.bkColor = RGB(0, 0, 0);
    m_TipsBuble.TextColor = RGB(255, 255, 255);
    m_TipsBuble.TextSize = 24;
    m_TipsBuble.x = 0;
    m_TipsBuble.y = 0;
    m_TipsBuble.width = 230;
    m_TipsBuble.height = 20;
    m_IsMouseMoving = FALSE;
}

AreaSelectDlg::~AreaSelectDlg()
{
}

void AreaSelectDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(AreaSelectDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_ERASEBKGND()
    ON_WM_SETCURSOR()
    ON_WM_RBUTTONUP()
    ON_WM_TIMER()
END_MESSAGE_MAP()

// AreaSelectDlg 消息处理程序

BOOL AreaSelectDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    //窗口全屏
    m_nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    m_nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    MoveWindow(0, 0, m_nScreenWidth, m_nScreenHeight);

    //截取当前桌面，做成一个内存DC
    CaptureDesktop();

    // 获取初始鼠标位置
    GetCursorPos(&m_MousePos);
    ScreenToClient(&m_MousePos);

    // 设置鼠标为十字光标
    SetCursor(LoadCursor(NULL, IDC_CROSS));
    return TRUE;
}

void AreaSelectDlg::OnPaint()
{
    CPaintDC dc(this);
    //创建内存DC
    CDC memDc;
    memDc.CreateCompatibleDC(&dc);
    CBitmap bitmapmemDc;
    bitmapmemDc.CreateCompatibleBitmap(&dc, m_nScreenWidth, m_nScreenHeight);
    memDc.SelectObject(&bitmapmemDc);

    DoubleBufferPaint(&memDc);
    DrawTipBubble(&memDc);
    //渲染完毕，进行贴图
    dc.BitBlt(0, 0, m_nScreenWidth, m_nScreenHeight, &memDc, 0, 0, SRCCOPY);
}

double AreaSelectDlg::GetUserDPI()
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);
    double scale = static_cast<double>(dpiX) / 96.0;
    return scale;
}

void AreaSelectDlg::CaptureDesktop()
{
    HDC ScreenDc = ::GetDC(nullptr);
    HDC memScreenDc = CreateCompatibleDC(ScreenDc);

    // 创建 DIB 位图
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_nScreenWidth;
    bmi.bmiHeader.biHeight = -m_nScreenHeight; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // 32-bit for easy pixel manipulation
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr; // 指向像素数据
    HBITMAP hDIB = CreateDIBSection(ScreenDc, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(memScreenDc, hDIB);

    // 抓取屏幕内容
    BitBlt(memScreenDc, 0, 0, m_nScreenWidth, m_nScreenHeight, ScreenDc, 0, 0, SRCCOPY);

    // 保存原始位图
    m_OriginalScreenBitmap.Attach((HBITMAP)CopyImage(hDIB, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION));

    // 变暗处理
    if (pBits)
    {
        DWORD* pPixel = (DWORD*)pBits; // 32-bit像素数据
        for (int i = 0; i < m_nScreenWidth * m_nScreenHeight; ++i)
        {
            BYTE* pColor = (BYTE*)pPixel; // 访问每个像素的颜色通道
            pColor[0] = pColor[0] / m_iDarkParam;   // Blue 通道
            pColor[1] = pColor[1] / m_iDarkParam;   // Green 通道
            pColor[2] = pColor[2] / m_iDarkParam;   // Red 通道
            ++pPixel;
        }
    }

    // 保存变暗的位图
    m_DimmedScreenBitmap.Attach(hDIB);

    // 清理资源
    SelectObject(memScreenDc, hOldBitmap);
    DeleteDC(memScreenDc);
    ::ReleaseDC(nullptr, ScreenDc);
}

void AreaSelectDlg::DrawGuidelines(CDC* pDC)
{
    // 创建十字线的画笔
    CPen crossPen(PS_SOLID, m_nCrossLineWidth, m_CrossLineColor);
    CPen* pOldPen = pDC->SelectObject(&crossPen);

    int oldROP = pDC->SetROP2(R2_XORPEN); // 使用异或模式，使线条在任何背景上都可见

    // 绘制水平线
    pDC->MoveTo(0, m_MousePos.y);
    pDC->LineTo(m_nScreenWidth, m_MousePos.y);

    // 绘制垂直线
    pDC->MoveTo(m_MousePos.x, 0);
    pDC->LineTo(m_MousePos.x, m_nScreenHeight);

    // 恢复原来的设置
    pDC->SetROP2(oldROP);
    pDC->SelectObject(pOldPen);
}

void AreaSelectDlg::DrawTipBubble(CDC* pDC)
{
    if (m_IsMouseMoving)//如果鼠标正在被移动
        return;

    // 提示文本
    CString tipText = _T("Esc键或鼠标右键退出");

    // 创建高质量字体
    CFont font;
    font.CreateFont(
        m_TipsBuble.TextSize,     // 字体高度
        0,                        // 字体宽度（0表示自动）
        0,                        // 文本角度
        0,                        // 基线角度
        FW_NORMAL,               // 字体粗细
        FALSE,                   // 斜体
        FALSE,                   // 下划线
        FALSE,                   // 删除线
        DEFAULT_CHARSET,         // 字符集
        OUT_TT_PRECIS,           // 使用TrueType字体输出精度
        CLIP_DEFAULT_PRECIS,     // 裁剪精度
        CLEARTYPE_QUALITY,       // 使用ClearType质量（抗锯齿）
        DEFAULT_PITCH | FF_SWISS,// 字体族
        _T("微软雅黑")           // 字体名称
    );

    // 选择字体并设置高质量文本渲染
    CFont* pOldFont = pDC->SelectObject(&font);

    // 设置文本渲染质量
    int oldGraphicsMode = pDC->SetGraphicsMode(GM_ADVANCED);
    pDC->SetTextAlign(TA_LEFT | TA_TOP);

    // 计算文本尺寸
    CSize textSize = pDC->GetTextExtent(tipText);

    // 调整气泡尺寸以适应文本（添加内边距）
    int padding = 8;
    m_TipsBuble.width = textSize.cx + padding * 2;
    m_TipsBuble.height = textSize.cy + padding * 2;

    // 设置气泡位置（在鼠标左下方30个单位）
    m_TipsBuble.x = m_MousePos.x + 30; 
    m_TipsBuble.y = m_MousePos.y + 30;                     

    // 确保气泡不超出屏幕边界
    if (m_TipsBuble.x < 0) {
        m_TipsBuble.x = m_MousePos.x + 30;  // 如果左侧放不下，放到右侧
    }
    if (m_TipsBuble.y + m_TipsBuble.height > m_nScreenHeight) {
        m_TipsBuble.y = m_MousePos.y - m_TipsBuble.height - 30;  // 如果下方放不下，放到上方
    }

    // 确保坐标不为负数
    if (m_TipsBuble.x < 0) m_TipsBuble.x = 5;
    if (m_TipsBuble.y < 0) m_TipsBuble.y = 5;

    // 创建气泡矩形区域
    CRect bubbleRect(
        m_TipsBuble.x,
        m_TipsBuble.y,
        m_TipsBuble.x + m_TipsBuble.width,
        m_TipsBuble.y + m_TipsBuble.height
    );

    // 绘制气泡背景（普通矩形，不带圆角）
    CBrush bgBrush(m_TipsBuble.bkColor);
    CBrush* pOldBrush = pDC->SelectObject(&bgBrush);

    // 创建边框画笔
    CPen borderPen(PS_SOLID, 1, RGB(120, 120, 120));
    CPen* pOldPen = pDC->SelectObject(&borderPen);

    // 绘制普通矩形（不是圆角矩形）
    pDC->Rectangle(bubbleRect);

    // 设置文本颜色和背景模式
    COLORREF oldTextColor = pDC->SetTextColor(m_TipsBuble.TextColor);
    int oldBkMode = pDC->SetBkMode(TRANSPARENT);

    // 计算文本绘制位置（居中）
    CRect textRect = bubbleRect;
    textRect.DeflateRect(padding, padding);

    // 使用高质量文本绘制
    pDC->DrawText(tipText, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    // 恢复原始 GDI 对象和设置
    pDC->SetGraphicsMode(oldGraphicsMode);
    pDC->SetTextColor(oldTextColor);
    pDC->SetBkMode(oldBkMode);
    pDC->SelectObject(pOldFont);
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);
}
 
void AreaSelectDlg::DoubleBufferPaint(CDC* cdc)
{
    // 第一级缓冲 DC
    CDC memcdc1;
    memcdc1.CreateCompatibleDC(cdc);
    CBitmap bmp1;
    bmp1.CreateCompatibleBitmap(cdc, m_nScreenWidth, m_nScreenHeight);
    CBitmap* pOldBmp1 = memcdc1.SelectObject(&bmp1);

    // 变暗图层 DC
    CDC memcdc2;
    memcdc2.CreateCompatibleDC(&memcdc1);
    CBitmap* pOldBmp2 = memcdc2.SelectObject(&m_DimmedScreenBitmap);

    // 原始屏幕图层 DC
    CDC memcdc3;
    memcdc3.CreateCompatibleDC(&memcdc1);
    CBitmap* pOldBmp3 = memcdc3.SelectObject(&m_OriginalScreenBitmap);

    // 绘制变暗全屏
    memcdc1.BitBlt(0, 0,
        m_nScreenWidth, m_nScreenHeight,
        &memcdc2, 0, 0,
        SRCCOPY);

    // 如果存在选区，先正则化坐标，再恢复该区域原始亮度
    CRect normRect = m_SaveRect;
    normRect.NormalizeRect();
    if (normRect.Width() > 0 && normRect.Height() > 0) {
        memcdc1.BitBlt(
            normRect.left, normRect.top,
            normRect.Width(), normRect.Height(),
            &memcdc3,
            normRect.left, normRect.top,
            SRCCOPY
        );
    }

    // 绘制十字参考线
    DrawGuidelines(&memcdc1);

    // 拖拽过程中，绘制红色空心矩形框
    if (isMouseHover && normRect.Width() > 0 && normRect.Height() > 0) {
        // 红色实线 2px，空心
        CPen redPen(PS_SOLID, 2, RGB(255, 0, 0));
        CBrush* pOldBrush = (CBrush*)memcdc1.SelectStockObject(NULL_BRUSH);
        CPen* pOldPen = memcdc1.SelectObject(&redPen);

        memcdc1.Rectangle(normRect);

        // 恢复 GDI 对象
        memcdc1.SelectObject(pOldPen);
        memcdc1.SelectObject(pOldBrush);
    }

    // 最终贴到窗口
    cdc->BitBlt(0, 0,
        m_nScreenWidth, m_nScreenHeight,
        &memcdc1, 0, 0,
        SRCCOPY);

    // 恢复原始位图对象
    memcdc3.SelectObject(pOldBmp3);
    memcdc2.SelectObject(pOldBmp2);
    memcdc1.SelectObject(pOldBmp1);
}

void AreaSelectDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    isMouseHover = true;
    m_SaveRect.left = point.x;
    m_SaveRect.top = point.y;
    m_SaveRect.right = m_SaveRect.left;
    m_SaveRect.bottom = m_SaveRect.top;
    CDialogEx::OnLButtonDown(nFlags, point);
}

void AreaSelectDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
    isMouseHover = false;

    // 确保矩形坐标正确（左上角到右下角）
    CRect rect = m_SaveRect;
    rect.NormalizeRect();
    m_SaveRect = rect;

    //限制最小大小
    if (m_SaveRect.Width() < 470 * m_Scale)
        m_SaveRect.right = m_SaveRect.left + 470 * m_Scale;
    if (m_SaveRect.Height() < 250 * m_Scale)
        m_SaveRect.bottom = m_SaveRect.top + 250 * m_Scale;

    EndDialog(IDOK);
    CDialogEx::OnLButtonUp(nFlags, point);
}

void AreaSelectDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    m_MousePos = point;
    if (isMouseHover)
    {
        m_SaveRect.right = point.x;
        m_SaveRect.bottom = point.y;
    }
    if (!m_IsMouseMoving)
    {
        m_IsMouseMoving = TRUE;
        KillTimer(MOUSE_TIEMR_ID);
        SetTimer(MOUSE_TIEMR_ID, MOUSE_STOP_DELAY, NULL);
    }
    Invalidate(FALSE);
    CDialogEx::OnMouseMove(nFlags, point);
}

BOOL AreaSelectDlg::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void AreaSelectDlg::OnOK()
{
    //CDialogEx::OnOK();
}

void AreaSelectDlg::OnCancel()
{
    CDialogEx::OnCancel();
}

BOOL AreaSelectDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    SetCursor(LoadCursor(NULL, IDC_CROSS));
    return TRUE;
}

void AreaSelectDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
    EndDialog(IDCANCEL);
    CDialogEx::OnRButtonUp(nFlags, point);
}


void AreaSelectDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == MOUSE_TIEMR_ID)
    {
        KillTimer(MOUSE_TIEMR_ID);
        m_IsMouseMoving = FALSE;
        Invalidate();
    }
    CDialogEx::OnTimer(nIDEvent);
}
