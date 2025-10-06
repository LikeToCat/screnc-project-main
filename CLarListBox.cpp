#include "stdafx.h"
#include "CLarListBox.h"
#include "CDebug.h"
#include "CMessage.h"
extern HANDLE ConsoleHandle;
BEGIN_MESSAGE_MAP(CLarListBoxCtrl, CListBox)
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_PAINT()  // 添加 OnPaint 消息处理
    ON_WM_NCPAINT()
    ON_WM_NCHITTEST()
    ON_WM_NCCALCSIZE()
    ON_WM_MOUSEWHEEL()
    ON_WM_VSCROLL()
    ON_WM_SHOWWINDOW()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()
CLarListBoxCtrl::CLarListBoxCtrl()
    : m_nItemHeight(25)
    , m_TextColor(RGB(209, 209, 209))
    , m_BackColor(RGB(36, 37, 40))
    , m_SelectionColor(RGB(50, 50, 50))
    , m_HoverColor(RGB(50, 50, 50))
    , m_BorderColor(RGB(73, 73, 73))  // 初始化边框颜色
    , m_bHasCustomFont(false)
    , m_nHoverIndex(-1)
    , m_bTracking(false)
    , m_bMouseEntered(false)
    , m_textSize(18)
    , m_Scale(1.0f)
    , m_IsNeedAutoLeaveHide(true)
    , m_GdiFont(nullptr)
    , m_MaxStringWidth(0)
{

}

CLarListBoxCtrl::~CLarListBoxCtrl()
{
}

int CLarListBoxCtrl::GetMaxStrWidth(const CStringArray& ary)
{
    int size = ary.GetSize();
    for (size_t i = 0; i < size; i++)
    {
        CString str = ary.GetAt(i);
        int txtwidth = GetTextWidth(str);
        DBFMT(ConsoleHandle, L"下拉框:%s，计算文本:%s，计算宽度:%d", m_strBoxName.c_str(), str, txtwidth);
        m_MaxStringWidth = (txtwidth > m_MaxStringWidth) ? txtwidth : m_MaxStringWidth;
    }
    return m_MaxStringWidth;
}

float CLarListBoxCtrl::GetTextWidth(const CString& text)
{
    if (text.IsEmpty()) return 0.0f;

    // 获取窗口的设备上下文
    CClientDC dc(this);  

    // 创建GDI+ Graphics对象
    Gdiplus::Graphics graphics(dc.GetSafeHdc());
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    // 创建字体（与OnPaint中相同的逻辑）
    Gdiplus::Font* pFont = nullptr;
    Gdiplus::FontFamily fontFamily(L"微软雅黑");

    if (m_bHasCustomFont)
    {
        LOGFONT lf;
        m_Font.GetLogFont(&lf);
        Gdiplus::FontFamily customFamily(lf.lfFaceName);
        pFont = new Gdiplus::Font(&customFamily, abs(lf.lfHeight),
            Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    }
    else
    {
        pFont = new Gdiplus::Font(&fontFamily, m_textSize,
            Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    }

    // 计算文本宽度
    Gdiplus::RectF boundingBox;
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    graphics.MeasureString(text, -1, pFont,
        Gdiplus::RectF(0, 0, 10000, 10000), &format, &boundingBox);
    float width = boundingBox.Width;

    // 清理资源
    delete pFont;
    return width;
}

BOOL CLarListBoxCtrl::PreCreateWindow(CREATESTRUCT& cs)
{
    // 移除焦点边框样式
    cs.style |= LBS_HASSTRINGS;
    cs.style &= ~WS_BORDER;  // 移除标准边框，我们将自己绘制边框

    return CListBox::PreCreateWindow(cs);
}

void CLarListBoxCtrl::OnPaint()
{
    // 防止默认绘制
    CPaintDC paintDC(this);
    CRect clientRect;
    GetClientRect(&clientRect);

    // 创建内存DC用于完全双缓冲
    CDC memDC;
    memDC.CreateCompatibleDC(&paintDC);
    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(&paintDC, clientRect.Width(), clientRect.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

    // 完全填充背景 - 清除所有旧内容
    memDC.FillSolidRect(clientRect, m_BackColor);

    // 选择自定义字体
    CFont* pOldFont = NULL;
    if (m_bHasCustomFont)
    {
        pOldFont = memDC.SelectObject(&m_Font);
    }

    // 手动绘制每个列表项
    int itemCount = GetCount();
    for (int i = 0; i < itemCount; i++)
    {
        CRect itemRect;
        GetItemRect(i, &itemRect);

        // 确定背景颜色
        COLORREF bgColor = m_BackColor;
        if (i == m_nHoverIndex || GetSel(i) > 0)
            bgColor = m_HoverColor;

        // 填充项目背景
        memDC.FillSolidRect(&itemRect, bgColor);

        // 获取文本并绘制 - 添加这段文本绘制代码
        CString itemText;
        GetText(i, itemText);

        // 使用GDI+绘制更平滑的文本
        Gdiplus::Graphics graphics(memDC.GetSafeHdc());
        graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

        // 创建GDI+字体
        Gdiplus::Font* pFont = nullptr;
        Gdiplus::FontFamily fontFamily(L"微软雅黑");

        if (m_bHasCustomFont)
        {
            LOGFONT lf;
            m_Font.GetLogFont(&lf);
            Gdiplus::FontFamily customFamily(lf.lfFaceName);
            pFont = new Gdiplus::Font(&customFamily, abs(lf.lfHeight),
                Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        }
        else
        {
            pFont = new Gdiplus::Font(&fontFamily, m_textSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
        }

        // 绘制文本
        Gdiplus::SolidBrush textBrush(Gdiplus::Color(
            GetRValue(m_TextColor),
            GetGValue(m_TextColor),
            GetBValue(m_TextColor)));

        Gdiplus::StringFormat format;
        format.SetAlignment(Gdiplus::StringAlignmentCenter);
        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
        graphics.DrawString(itemText, -1, pFont,
            Gdiplus::RectF((float)itemRect.left, (float)itemRect.top,
                (float)itemRect.Width(), (float)itemRect.Height()),
            &format, &textBrush);

        // 清理资源
        delete pFont;
    }



    // 一次性将整个内容拷贝到屏幕
    paintDC.BitBlt(0, 0, clientRect.Width(), clientRect.Height(),
        &memDC, 0, 0, SRCCOPY);

    // 清理资源
    if (pOldFont) memDC.SelectObject(pOldFont);
    memDC.SelectObject(pOldBitmap);
    memBitmap.DeleteObject();
}

void CLarListBoxCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    lpMeasureItemStruct->itemHeight = m_nItemHeight;
}

BOOL CLarListBoxCtrl::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(&rect);
    pDC->FillSolidRect(&rect, m_BackColor);
    return TRUE;
}

void CLarListBoxCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    // 启动鼠标跟踪，确保 OnMouseLeave 会被通知
    if (!m_bTracking)
    {
        TRACKMOUSEEVENT tme = { sizeof(tme) };
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = 0;
        _TrackMouseEvent(&tme);
        m_bTracking = true;
    }

    // 判断鼠标在列表项内还是在外
    BOOL bOutside = FALSE;
    UINT idx = ItemFromPoint(point, bOutside);

    // 如果首次进入任何项目区域，标记已进入
    if (!bOutside && !m_bMouseEntered)
    {
        m_bMouseEntered = true;
    }

    // 处理 hover 高亮逻辑（原有）
    if (!bOutside && idx != m_nHoverIndex)
    {
        int old = m_nHoverIndex;
        m_nHoverIndex = idx;
        if (old != -1)
        {
            CRect rc; GetItemRect(old, &rc);
            InvalidateRect(&rc, FALSE);
        }
        CRect rc2; GetItemRect(m_nHoverIndex, &rc2);
        InvalidateRect(&rc2, TRUE);
    }
    else if (bOutside && m_nHoverIndex != -1)
    {
        int old = m_nHoverIndex;
        m_nHoverIndex = -1;
        CRect rc; GetItemRect(old, &rc);
        InvalidateRect(&rc, TRUE);
    }
    
    CListBox::OnMouseMove(nFlags, point);
}

LRESULT CLarListBoxCtrl::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    m_bTracking = false;

    // 如果鼠标曾经进入过列表，且现在离开，隐藏弹出窗口
    if (m_bMouseEntered)
    {
        m_bMouseEntered = false;

        // 隐藏自身
        if (m_IsNeedAutoLeaveHide)
        {
            ShowWindow(SW_HIDE);
            // 隐藏外层承载窗口（CLarListBoxCWnd）
            if (GetParent() && ::IsWindow(GetParent()->GetSafeHwnd()))
                GetParent()->ShowWindow(SW_HIDE);
        }
    }

    // 清除悬停高亮
    if (m_nHoverIndex != -1)
    {
        int old = m_nHoverIndex;
        m_nHoverIndex = -1;
        CRect rc; GetItemRect(old, &rc);
        InvalidateRect(&rc, TRUE);
    }
    
    return 0;
}

void CLarListBoxCtrl::SetMaxDisplayItems(int8_t maxItems)
{
    m_int8_MaxDisplayItem = maxItems; 
    CRect ctrlRect;
    GetWindowRect(ctrlRect);
    ctrlRect.bottom = ctrlRect.top + maxItems * m_nItemHeight;
    MoveWindow(ctrlRect);
}

void CLarListBoxCtrl::SetItemHeight(int nHeight)
{
    m_nItemHeight = nHeight;
    CListBox::SetItemHeight(0, nHeight);
}

void CLarListBoxCtrl::SetFont(CFont* pFont, BOOL bRedraw)
{
    if (pFont != nullptr)
    {
        LOGFONT lf;
        pFont->GetLogFont(&lf);
        m_Font.DeleteObject();
        m_Font.CreateFontIndirect(&lf);
        m_bHasCustomFont = true;
        CListBox::SetFont(&m_Font, bRedraw);
    }
    else
    {
        m_bHasCustomFont = false;
        CListBox::SetFont(pFont, bRedraw);
    }
}

void CLarListBoxCtrl::SetFontSize(int nPointSize)
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
    double scaleX = static_cast<double>(dpiX) / 96.0;
    double scaleY = static_cast<double>(dpiY) / 96.0;
    m_Scale = scaleY;
    m_textSize = nPointSize * m_Scale;
}

void CLarListBoxCtrl::SetScrollbarColors(COLORREF bgColor, COLORREF thumbColor, COLORREF arrowColor)
{
    m_ScrollbarBgColor = bgColor;
    m_ScrollbarThumbColor = thumbColor;
    m_ScrollbarArrowColor = arrowColor;

    if (m_bCustomScrollbar && ::IsWindow(m_hWnd))
    {
        // 强制repaint非客户区
        SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

void CLarListBoxCtrl::SetScrollbarWidth(int width)
{
    m_nScrollbarWidth = width;

    if (m_bCustomScrollbar && ::IsWindow(m_hWnd))
    {
        // 强制重新计算非客户区
        SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

void CLarListBoxCtrl::EnableCustomScrollbar(bool enable)
{
    if (m_bCustomScrollbar != enable)
    {
        m_bCustomScrollbar = enable;

        if (::IsWindow(m_hWnd))
        {
            // 隐藏系统滚动条
            ShowScrollBar(SB_VERT, FALSE);

            // 移除滚动条样式
            LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
            style &= ~WS_VSCROLL;
            SetWindowLong(m_hWnd, GWL_STYLE, style);

            // 强制重新计算非客户区
            SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

            // 强制repaint
            Invalidate();
        }
    }
}

void CLarListBoxCtrl::OnNcPaint()
{
    if (!m_bCustomScrollbar)
    {
        Default();
        return;
    }

    // 获取窗口DC
    CWindowDC dc(this);

    // 获取非客户区矩形
    CRect rectWnd;
    GetWindowRect(rectWnd);

    // 调整为客户坐标
    rectWnd.OffsetRect(-rectWnd.left, -rectWnd.top);

    // 获取客户区矩形
    CRect rectClient;
    GetClientRect(rectClient);
    ClientToScreen(rectClient);
    ScreenToClient(&rectClient);

    // 计算非客户区矩形
    CRect rectNonClient = rectWnd;
    rectNonClient.SubtractRect(rectNonClient, rectClient);

    // 绘制自定义滚动条
    DrawCustomScrollbar(&dc);
}

LRESULT CLarListBoxCtrl::OnNcHitTest(CPoint point)
{
    if (!m_bCustomScrollbar)
        return CListBox::OnNcHitTest(point);

    // 获取窗口矩形
    CRect rectWnd;
    GetWindowRect(rectWnd);

    // 计算滚动条区域
    CRect rectScrollbar = rectWnd;
    rectScrollbar.left = rectScrollbar.right - m_nScrollbarWidth;

    // 检查点是否在滚动条区域内
    if (rectScrollbar.PtInRect(point))
        return HTHSCROLL;

    return CListBox::OnNcHitTest(point);
}

void CLarListBoxCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
    // 不要调用基类实现，完全自定义非客户区计算
    if (!m_bCustomScrollbar)
    {
        CListBox::OnNcCalcSize(bCalcValidRects, lpncsp);
        return;
    }

    // 不判断滚动条状态，始终保留空间
    if (bCalcValidRects)
    {
        // 我们只需要获取项目数和可见项目数来确定是否需要滚动条
        int nCount = GetCount();
        CRect rectClient(lpncsp->rgrc[0]);
        int nVisibleCount = rectClient.Height() / m_nItemHeight;

        // 如果项目数大于可见数，我们需要显示滚动条，减小客户区宽度
        if (nCount > nVisibleCount)
        {
            lpncsp->rgrc[0].right -= m_nScrollbarWidth;
        }
    }

    // 不调用基类OnNcCalcSize，完全控制非客户区计算
}

void CLarListBoxCtrl::SetLeaveAutoHide(bool isAutoLeaveHide)
{
    m_IsNeedAutoLeaveHide = isAutoLeaveHide;
}

void CLarListBoxCtrl::DrawCustomScrollbar(CDC* pDC)
{
    // 获取项目和可见项目数
    int nCount = GetCount();
    CRect clientRect;
    GetClientRect(&clientRect);
    int nVisibleItems = clientRect.Height() / m_nItemHeight;

    // 如果不需要滚动条，返回
    if (nCount <= nVisibleItems)
        return;

    // 获取窗口矩形
    CRect rectWnd;
    GetWindowRect(rectWnd);
    rectWnd.OffsetRect(-rectWnd.left, -rectWnd.top);

    // 计算滚动条区域
    CRect rectScrollbar = rectWnd;
    rectScrollbar.left = rectScrollbar.right - m_nScrollbarWidth;

    // 使用 GDI 绘制滚动条背景
    pDC->FillSolidRect(rectScrollbar, m_ScrollbarBgColor);

    // 使用 GetTopIndex() 获取当前滚动位置
    int nTopIndex = GetTopIndex();

    // 计算滑块位置和大小
    float ratio = min(1.0f, (float)nVisibleItems / (float)nCount);
    int thumbHeight = max(m_nScrollThumbMinHeight, (int)(rectScrollbar.Height() * ratio));

    // 计算滑块位置
    float position = 0.0f;
    if (nCount > nVisibleItems)
    {
        position = (float)nTopIndex / (float)(nCount - nVisibleItems);
    }

    int thumbTop = (int)(position * (rectScrollbar.Height() - thumbHeight));

    // 确保有效值
    thumbTop = max(0, min(thumbTop, rectScrollbar.Height() - thumbHeight));

    // 使用 GDI+ 绘制圆角矩形滑块
    Gdiplus::Graphics graphics(pDC->GetSafeHdc());

    // 设置像素偏移模式，防止边缘产生抗锯齿伪影
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

    // 设置抗锯齿模式
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    // 滑块矩形 - 减小内边距
    Gdiplus::RectF thumbRect(
        (Gdiplus::REAL)(rectScrollbar.left + 1),  // 从+2改为+1
        (Gdiplus::REAL)(rectScrollbar.top + thumbTop + 1),  // 从+2改为+1
        (Gdiplus::REAL)(rectScrollbar.Width() - 2),  // 从-4改为-2
        (Gdiplus::REAL)(thumbHeight - 2)  // 从-4改为-2
    );

    // 创建圆角矩形路径
    int cornerRadius = min(4, m_nScrollbarWidth / 4); // 圆角半径
    Gdiplus::GraphicsPath path;

    // 左上角圆弧
    path.AddArc(
        (Gdiplus::REAL)thumbRect.X,
        (Gdiplus::REAL)thumbRect.Y,
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)180,
        (Gdiplus::REAL)90);

    // 右上角圆弧
    path.AddArc(
        (Gdiplus::REAL)(thumbRect.X + thumbRect.Width - cornerRadius * 2),
        (Gdiplus::REAL)thumbRect.Y,
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)270,
        (Gdiplus::REAL)90);

    // 右下角圆弧
    path.AddArc(
        (Gdiplus::REAL)(thumbRect.X + thumbRect.Width - cornerRadius * 2),
        (Gdiplus::REAL)(thumbRect.Y + thumbRect.Height - cornerRadius * 2),
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)0,
        (Gdiplus::REAL)90);

    // 左下角圆弧
    path.AddArc(
        (Gdiplus::REAL)thumbRect.X,
        (Gdiplus::REAL)(thumbRect.Y + thumbRect.Height - cornerRadius * 2),
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)(cornerRadius * 2),
        (Gdiplus::REAL)90,
        (Gdiplus::REAL)90);

    // 闭合路径
    path.CloseFigure();

    // 创建完全不透明的画刷 (Alpha = 255)
    Gdiplus::SolidBrush brush(Gdiplus::Color(
        255,  // 完全不透明
        GetRValue(m_ScrollbarThumbColor),
        GetGValue(m_ScrollbarThumbColor),
        GetBValue(m_ScrollbarThumbColor)));

    // 填充路径
    graphics.FillPath(&brush, &path);

    // 不调用DrawPath，避免产生边框
}

void CLarListBoxCtrl::UpdateScrollBar()
{
    if (m_bCustomScrollbar && ::IsWindow(m_hWnd))
    {
        CWindowDC dc(this);
        DrawCustomScrollbar(&dc);
    }
}

BOOL CLarListBoxCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // 处理鼠标滚轮滚动
    int nTopIndex = GetTopIndex();

    if (zDelta > 0) // 向上滚动
    {
        if (nTopIndex > 0)
            SetTopIndex(nTopIndex - 1);
    }
    else // 向下滚动
    {
        CRect rectClient;
        GetClientRect(&rectClient);
        int nVisibleCount = rectClient.Height() / m_nItemHeight;
        int nCount = GetCount();

        if (nTopIndex + nVisibleCount < nCount)
            SetTopIndex(nTopIndex + 1);
    }

    // 强制repaint滚动条
    if (m_bCustomScrollbar)
    {
        CWindowDC dc(this);
        DrawCustomScrollbar(&dc);
    }

    return TRUE; // 已处理
}

void CLarListBoxCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // 调用默认处理
    CListBox::OnVScroll(nSBCode, nPos, pScrollBar);

    // 强制repaint滚动条
    if (m_bCustomScrollbar)
    {
        CWindowDC dc(this);
        DrawCustomScrollbar(&dc);
    }
}

void CLarListBoxCtrl::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CListBox::OnShowWindow(bShow, nStatus);

    // 确保系统滚动条保持隐藏
    if (bShow && m_bCustomScrollbar)
    {
        ShowScrollBar(SB_VERT, FALSE);
    }
}

void CLarListBoxCtrl::OnSize(UINT nType, int cx, int cy)
{
    CListBox::OnSize(nType, cx, cy);

    // 调整大小后确保滚动条状态正确
    if (m_bCustomScrollbar)
    {
        ShowScrollBar(SB_VERT, FALSE);
    }
}

LRESULT CLarListBoxCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = CListBox::WindowProc(message, wParam, lParam);

    if (m_bCustomScrollbar && ::IsWindow(m_hWnd))
    {
        // 捕获更多可能影响滚动位置的消息
        switch (message)
        {
        case LB_ADDSTRING:
        case LB_INSERTSTRING:
        case LB_DELETESTRING:
        case LB_RESETCONTENT:
        case LB_SETTOPINDEX:
        case WM_STYLECHANGED:
        case WM_MOVE:
        case WM_MOVING:
        case WM_SIZE:
        case WM_MOUSEWHEEL:
        case WM_VSCROLL:
            UpdateScrollBar();
            break;
        }
    }

    return result;
}

void CLarListBoxCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 获取列表控件的客户区矩形
    CRect listRect;
    GetWindowRect(&listRect);
    ScreenToClient(&listRect);

    // 检查点击是否在列表控件区域内
    if (listRect.PtInRect(point))
    {
        // 将点坐标转换为列表控件的客户区坐标
        CPoint listPoint = point;
        listPoint.x -= listRect.left;
        listPoint.y -= listRect.top;

        // 使用ClientToScreen和ScreenToClient将坐标完全转换到列表控件坐标系
        CPoint ptClient = listPoint;
        ClientToScreen(&ptClient);
        ScreenToClient(&ptClient);

        // 使用ItemFromPoint获取点击的项目索引 - 更精确的方法
        BOOL bOutside = FALSE;
        int clickedIndex = ItemFromPoint(ptClient, bOutside);

        // 如果点击在有效项目上
        if (!bOutside && clickedIndex != LB_ERR && clickedIndex < GetCount())
        {
            // 获取项目文本
            CString itemText;
            GetText(clickedIndex, itemText);

            // 创建消息数据结构
            MsgParam::LISTBOX_SELECT_INFO selectInfo;
            selectInfo.nIndex = clickedIndex;
            selectInfo.strText = itemText;
            selectInfo.strBoxName = m_strBoxName;

            // 向父窗口发送消息，通知已选择项目
            if (m_Cwnd_MessageTransferDst)
            {
                // 发送自定义消息，携带索引和文本及键值
                ::SendMessage(m_Cwnd_MessageTransferDst->GetSafeHwnd(), MSG_CLARLISTBOX_SELECTED,
                    (WPARAM)&selectInfo,
                    (LPARAM)0);
            }
            else
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"下拉框控件：未设置父窗口，无法发送下拉框项选择消息");
            }

            // 隐藏下拉列表窗口
            ShowWindow(SW_HIDE);
            GetParent()->ShowWindow(SW_HIDE);
            return; // 处理完毕，不调用基类
        }
    }

    // 如果点击在列表控件外部，也隐藏窗口
    CRect wndRect;
    GetClientRect(&wndRect);
    if (!wndRect.PtInRect(point))
    {
        ShowWindow(SW_HIDE);
        return;
    }

    CWnd::OnLButtonDown(nFlags, point);
}

CLarListBoxCWnd::CLarListBoxCWnd()
{
    m_IsCreated = false;
}

CLarListBoxCWnd::~CLarListBoxCWnd()
{
}

BEGIN_MESSAGE_MAP(CLarListBoxCWnd, CWnd)
    ON_WM_ERASEBKGND()

END_MESSAGE_MAP()


BOOL CLarListBoxCWnd::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

BOOL CLarListBoxCWnd::Create(const int Width, const int ItemHeight, CWnd* pParentWnd, CStringArray& items, const std::wstring string, int maxWidth)
{
    CWnd* pParWnd = nullptr;

    // 注册窗口类
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, _T("PopupListBoxClass"), &wndcls)))
    {
        wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc = ::DefWindowProc;
        wndcls.cbClsExtra = 0;
        wndcls.cbWndExtra = 0;
        wndcls.hInstance = hInst;
        wndcls.hIcon = NULL;
        wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndcls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wndcls.lpszMenuName = NULL;
        wndcls.lpszClassName = _T("PopupListBoxClass");

        if (!AfxRegisterClass(&wndcls))
            return FALSE;
    }

    // 创建顶层窗口
    CRect rect(0, 0, 0, 0);
    int itemHeight = ItemHeight;
    rect.right = Width;
    if (items.GetSize() > 5)
    {
        rect.bottom = rect.top + (5 * itemHeight);
    }
    else
    {
        rect.bottom = rect.top + ItemHeight * items.GetSize();
    }
    if (!CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_COMPOSITED,
        _T("PopupListBoxClass"), _T(""),
        WS_POPUP | WS_VISIBLE,
        rect, pParWnd, 0))
        return FALSE;

    // 创建内部的列表框
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    CRect clientRect;
    GetClientRect(&clientRect);
    clientRect.bottom += itemHeight;
    if (items.GetSize() > 5)
    {
        clientRect.bottom = clientRect.top + (5 * itemHeight) + itemHeight;
        m_Ctrl_listbox.Create(WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_HASSTRINGS | WS_EX_COMPOSITED,
            clientRect, this, (UINT)-1);
        m_Ctrl_listbox.EnableCustomScrollbar(true);
    }
    else
    {
        m_Ctrl_listbox.Create(WS_CHILD | WS_VISIBLE | LBS_NOTIFY | LBS_HASSTRINGS | WS_EX_COMPOSITED,
            clientRect, this, (UINT)-1);
    }
    m_items.Copy(items);
    m_Ctrl_listbox.SetItemHeight(itemHeight);

    // 设置列表框样式和内容
    for (int i = 0; i < items.GetSize(); i++)
        m_Ctrl_listbox.AddString(items[i]);
    m_Ctrl_listbox.SetListBoxName(string);
    m_Ctrl_listbox.SetTransferMessageDstCwnd(pParentWnd);
    int maxtxtWidth = m_Ctrl_listbox.GetMaxStrWidth(items);
    if (maxWidth != -1)
        maxtxtWidth = maxtxtWidth > maxWidth ? maxWidth : maxtxtWidth;
    if (Width < maxtxtWidth)
    {
        CRect lbRect;
        m_Ctrl_listbox.GetWindowRect(lbRect);
        lbRect.right = lbRect.left + maxtxtWidth + 10;
        m_Ctrl_listbox.MoveWindow(lbRect);
        CRect parRect;
        GetWindowRect(parRect);
        parRect.right = parRect.left + maxtxtWidth + 10;
        MoveWindow(parRect);
    }
    m_IsCreated = true;
    return TRUE;
}

CLarPopupListBoxs::CLarPopupListBoxs()
{
    m_maxWidth = -1;
}

CLarPopupListBoxs::~CLarPopupListBoxs()
{
    for (auto& pair : m_Map_ListBox)
    {
        if (pair.second)
        {
            pair.second->DestroyWindow();
        }
    }
}

void CLarPopupListBoxs::SetListBoxHideWhenMouseLeave(bool isLeaveAutoHide)
{
    for (const auto& listbox : m_Map_ListBox)
    {
        (listbox).second->m_Ctrl_listbox.SetLeaveAutoHide(isLeaveAutoHide);
    }
}

void CLarPopupListBoxs::SetMaxWidth(int maxWidth)
{
    m_maxWidth = maxWidth;
}

bool CLarPopupListBoxs::addListBox(const int Width, const int Height, CWnd* pParentWnd, CStringArray& items, const std::wstring string)
{
    CLarListBoxCWnd* ListBoxCWnd = new CLarListBoxCWnd;
    if (!ListBoxCWnd->Create(Width, Height, pParentWnd, items, string, m_maxWidth))
    {
        return false;
    }
    m_Map_ListBox.emplace(string, ListBoxCWnd);
    ListBoxCWnd->ShowWindow(SW_HIDE);
    return true;
}

void CLarPopupListBoxs::UpdateDroplistXY(const wchar_t* listBoxName, int X, int Y)
{
    //获取原来下拉框的显示位置
    CRect DisplayRect;
    getDisplayRect(listBoxName, &DisplayRect);

    //更新下拉框的显示位置
    if (DisplayRect.left != X || DisplayRect.top != Y)
    {
        DisplayRect.MoveToXY(X, Y);
    }
    ShowListBox(listBoxName, DisplayRect);
}

void CLarPopupListBoxs::DeleteListBox(std::wstring name)
{
    for (
        std::map<std::wstring, CLarListBoxCWnd*>::iterator it = m_Map_ListBox.begin();
        it != m_Map_ListBox.end();
        )
    {
        if ((*it).first == name)
        {
            if ((*it).second)
            {
                (*it).second->DestroyWindow();
                delete (*it).second;
            }
            it = m_Map_ListBox.erase(it);
        }
        else
        {
            it++;
        }
    }
}

bool CLarPopupListBoxs::ShowListBox(const std::wstring boxName, const CRect& DisplayRect)
{
    bool isFinded = false;
    if (m_SignleMode)
    {
        for (auto& pair : m_Map_ListBox)
        {
            if (pair.first == boxName)
            {
                isFinded = true;
                if (!DisplayRect.IsRectEmpty())pair.second->MoveWindow(DisplayRect);
                pair.second->ShowWindow(SW_SHOW);
                pair.second->m_Ctrl_listbox.ShowWindow(SW_SHOW);
            }
            else if (pair.first != boxName)
            {
                pair.second->ShowWindow(SW_HIDE);
            }
        }
        if (!isFinded)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"没有找到指定的下拉框控件，请检查传值是否正确");
            return false;
        }
    }
    else
    {
        for (auto& pair : m_Map_ListBox)
        {
            if (pair.first == boxName)
            {
                pair.second->ShowWindow(SW_SHOW);
                isFinded = true;
                break;
            }
        }
        if (!isFinded)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"没有找到指定的下拉框控件，请检查传值是否正确");
            return false;
        }
    }
    return true;
}

bool CLarPopupListBoxs::HideListBox()
{
    if (m_Map_ListBox.size() == 0)
        return true;
    for (auto& listbox : m_Map_ListBox)
    {
        listbox.second->ShowWindow(SW_HIDE);
    }
    return true;
}

bool CLarPopupListBoxs::IsListBoxVisiable(const std::wstring boxName)
{
    auto listbox = m_Map_ListBox.find(boxName);
    if ((listbox) != m_Map_ListBox.end())
    {
        if ((*listbox).second)
        {
            return (*listbox).second->IsWindowVisible();
        }
    }
    return false;
}

void CLarPopupListBoxs::SetTextSize(const int size, const std::wstring boxName)
{
    // 不指定名称时应用于所有下拉框
    if (boxName.empty())
    {
        for (auto& pair : m_Map_ListBox)
        {
            if (pair.second)
            {
                pair.second->m_Ctrl_listbox.SetFontSize(size);
                UpdateAdaptTextMaxWidth(pair.second);
            }
        }
    }
    else
    {
        // 查找指定名称的下拉框
        auto it = m_Map_ListBox.find(boxName);
        if (it != m_Map_ListBox.end() && it->second)
        {
            it->second->m_Ctrl_listbox.SetFontSize(size);
            UpdateAdaptTextMaxWidth(it->second);
        }
        else
        {
            // 如果找不到指定名称的下拉框，输出调试信息
            DEBUG_CONSOLE_STR(ConsoleHandle, L"没有找到指定的下拉框控件，无法设置字体大小");
        }
    }
}

void CLarPopupListBoxs::SetItemHeight(int nHeight)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetItemHeight(nHeight);
    }
}

void CLarPopupListBoxs::SetTextColor(COLORREF color)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetTextColor(color);
    }
}

void CLarPopupListBoxs::SetBackColor(COLORREF color)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetBackColor(color);
    }
}

void CLarPopupListBoxs::SetSelectionColor(COLORREF color)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetSelectionColor(color);
    }
}

void CLarPopupListBoxs::SetHoverColor(COLORREF color)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetHoverColor(color);
    }
}

void CLarPopupListBoxs::SetBorderColor(COLORREF color)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetBorderColor(color);
    }
}

void CLarPopupListBoxs::SetFont(CFont* pFont, BOOL bRedraw)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetFont(pFont, bRedraw);
        pair.second->SetFont(pFont, bRedraw);
    }
}

void CLarPopupListBoxs::SetMaxDisplayItems(int8_t maxItems, std::wstring listBoxName)
{
    auto var = m_Map_ListBox.find(listBoxName);
    if ((*var).second)
    {
        auto listbox = (*var).second;

        CRect rcRect;
        listbox->GetWindowRect(rcRect);
        rcRect.bottom = rcRect.top + listbox->m_Ctrl_listbox.GetItemHeight() * maxItems;
        listbox->MoveWindow(rcRect);
        listbox->m_Ctrl_listbox.SetMaxDisplayItems(maxItems);
    }
}

void CLarPopupListBoxs::SetListBoxAdaptWithText(const std::wstring boxName)
{
    for (auto& listbox : m_Map_ListBox)
    {
        
    }
}

void CLarPopupListBoxs::SetScrollbarColors(COLORREF bgColor, COLORREF thumbColor, COLORREF arrowColor)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetScrollbarColors(bgColor, thumbColor, arrowColor);
    }
}

void CLarPopupListBoxs::getDisplayRect(const std::wstring boxName, _Out_ CRect* DisplayGlobalRect)
{
    for (auto& pair : m_Map_ListBox)
    {
        if (pair.first == boxName)
        {
            pair.second->GetWindowRect(DisplayGlobalRect);
        }
    }
}

void CLarPopupListBoxs::SetScrollbarWidth(int width)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.SetScrollbarWidth(width);
    }
}

void CLarPopupListBoxs::setDisplayRect(const std::wstring boxName, _In_ CRect* DisplayGlobalRect)
{
    if (DisplayGlobalRect)
    {
        for (auto& pair : m_Map_ListBox)
        {
            if (pair.first == boxName)
            {
                pair.second->MoveWindow(DisplayGlobalRect);
            }
        }
    }
}

void CLarPopupListBoxs::DeleteAllRow(std::wstring boxName)
{
    if (m_Map_ListBox.size() != 0)
    {
        for (auto it = m_Map_ListBox.begin(); it != m_Map_ListBox.end(); )
        {
            if (it->first == boxName)
            {
                delete it->second;
                it = m_Map_ListBox.erase(it);
            }
            it++;
        }
    }
}

void CLarPopupListBoxs::EnableCustomScrollbar(bool enable)
{
    for (auto& pair : m_Map_ListBox)
    {
        pair.second->m_Ctrl_listbox.EnableCustomScrollbar(enable);
    }
}

void CLarPopupListBoxs::UpdateAdaptTextMaxWidth(CLarListBoxCWnd* cWnd)
{
    if (cWnd)
    {
        CStringArray items;
        items.Copy(cWnd->getItems());
        int maxtxtWidth = cWnd->m_Ctrl_listbox.GetMaxStrWidth(items);
        if(m_maxWidth != -1)
            maxtxtWidth = (maxtxtWidth > m_maxWidth) ? m_maxWidth : maxtxtWidth;
        
        CRect rect;
        cWnd->GetWindowRect(rect);
        if (rect.Width() < maxtxtWidth)
        {
            CRect lbRect;
            cWnd->m_Ctrl_listbox.GetWindowRect(lbRect);
            lbRect.right = lbRect.left + maxtxtWidth + 10;
            cWnd-> m_Ctrl_listbox.MoveWindow(lbRect);
            CRect parRect;
            cWnd->GetWindowRect(parRect);
            parRect.right = parRect.left + maxtxtWidth + 10;
            cWnd->MoveWindow(parRect);
        }
        // 重新应用WS_BORDER样式
        LONG style = GetWindowLong(cWnd->GetSafeHwnd(), GWL_STYLE);
        style |= WS_BORDER;  // 添加边框样式
        SetWindowLong(cWnd->GetSafeHwnd(), GWL_STYLE, style);
        SetWindowPos(cWnd->GetSafeHwnd(), NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        cWnd->Invalidate();
        cWnd->UpdateWindow();
    }
}


