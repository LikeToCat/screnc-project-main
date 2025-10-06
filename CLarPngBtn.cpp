#include "stdafx.h"
#include "CLarPngBtn.h"
// PngButton.cpp
IMPLEMENT_DYNAMIC(CLarPngBtn, CButton)

CLarPngBtn::CLarPngBtn() :
    m_bUseCustomBgColor(false),
    m_CustomBgColor(RGB(255, 255, 255)),
    m_ClickEffectColor(Gdiplus::Color(80, 0, 0, 0)),
    m_bStretchMode(TRUE),
    m_bToolTipsEnabled(FALSE),
    m_bToolTipsImmediate(FALSE)
{
    m_sToolTips = L"";
}

CLarPngBtn::~CLarPngBtn()
{
    ClearImages();
}

BEGIN_MESSAGE_MAP(CLarPngBtn, CButton)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_SETCURSOR()
END_MESSAGE_MAP()


void CLarPngBtn::SetUseHandCursor(BOOL bUseHandCursor)
{
    m_bUseHandCursor = bUseHandCursor;
}

void CLarPngBtn::SetBackgroundColor(COLORREF color)
{
    m_bUseCustomBgColor = true;
    m_CustomBgColor = color;
    if (GetSafeHwnd()) {
        Invalidate(); // 立即更新显示
    }
}

void CLarPngBtn::OnMouseMove(UINT nFlags, CPoint point)
{
    CButton::OnMouseMove(nFlags, point);

    // 启用悬停效果时，首次进入时置标志并repaint
    if (m_bUseHoverImage && !m_bMouseHover)
    {
        m_bMouseHover = TRUE;
        Invalidate();
    }

    if ((m_bUseHoverImage || (m_bUseHoverEffectColor && !m_bUseHoverImage))
        && !m_bMouseHover)
    {
        if (m_Func_HoverCallBack)//如果设置了回调，则执行
            m_Func_HoverCallBack();
        m_bMouseHover = TRUE;
        Invalidate();
    }

    // 开始鼠标离开跟踪
    if (!m_bMouseTracking)
    {
        TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, GetSafeHwnd(), 0 };
        ::TrackMouseEvent(&tme);
        m_bMouseTracking = TRUE;
    }
}

void CLarPngBtn::OnMouseLeave()
{
    CButton::OnMouseLeave();

    // 鼠标离开时重置悬停标志并repaint
    if (m_bUseHoverImage && m_bMouseHover)
    {
        m_bMouseHover = FALSE;
        Invalidate();
    }

    if ((m_bUseHoverImage || (m_bUseHoverEffectColor && !m_bUseHoverImage))
        && m_bMouseHover)
    {
        if (m_Func_LeaveCallback)//如果设置了回调，则执行
            m_Func_LeaveCallback();
        m_bMouseHover = FALSE;
        Invalidate();
    }

    m_bMouseTracking = FALSE;
}

BOOL CLarPngBtn::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    if (pWnd == this && nHitTest == HTCLIENT && m_bUseHandCursor)
    {
        HCURSOR hHand = ::LoadCursor(NULL, IDC_HAND);
        if (::GetCursor() != hHand)
            ::SetCursor(hHand);
        return TRUE; 
    }
    return CButton::OnSetCursor(pWnd, nHitTest, message);
}

void CLarPngBtn::SetClickEffectColor(BYTE alpha, BYTE red, BYTE green, BYTE blue)
{
    m_ClickEffectColor = Gdiplus::Color(alpha, red, green, blue);
    if (GetSafeHwnd()) {
        Invalidate(); // 如果按钮正在被点击，立即更新显示
    }
}

void CLarPngBtn::SetHighQualityPNG(BOOL bEnable)
{
    m_bHighQualityPNG = (bEnable != FALSE);
    if (GetSafeHwnd())
        Invalidate();
}

void CLarPngBtn::SetBtnHoverCallBack(std::function<void()> hoverCallback)
{
    m_Func_HoverCallBack = hoverCallback;
}

void CLarPngBtn::SetBtnLeaveCallBack(std::function<void()> leaveCallback)
{
    m_Func_LeaveCallback = leaveCallback;
}

void CLarPngBtn::SetCircularImage(BOOL bCircular, int radius)
{
    m_bCircularImage = bCircular;
    m_nCircularRadius = radius;
    if (GetSafeHwnd())
        Invalidate();
}

void CLarPngBtn::SetCircularBorder(BOOL bShowBorder, BYTE borderWidth,
    BYTE alpha, BYTE red, BYTE green, BYTE blue)
{
    m_bShowCircularBorder = bShowBorder;
    m_nCircularBorderWidth = (borderWidth > 0) ? borderWidth : 1;
    m_CircularBorderColor = Gdiplus::Color(alpha, red, green, blue);
    if (GetSafeHwnd() && m_bCircularImage)
        Invalidate();
}

void CLarPngBtn::SetStretchMode(BOOL bStretch)
{
    m_bStretchMode = bStretch;
    if (GetSafeHwnd()) {
        Invalidate(); // 立即更新显示
    }
}

void CLarPngBtn::SetStretchMode(float fRatio)
{
    m_bStretchMode = TRUE;
    m_fStretchRatio = (fRatio < 0.0f ? 0.0f : (fRatio > 1.0f ? 1.0f : fRatio));
    if (GetSafeHwnd()) Invalidate();
}

void CLarPngBtn::PreSubclassWindow()
{
    CButton::PreSubclassWindow();

    // 确保按钮是OWNERDRAW风格
    ModifyStyle(0, BS_OWNERDRAW);
    ModifyStyleEx(0, WS_EX_TRANSPARENT);

    // 创建工具提示控件
    m_ToolTip.Create(this);
    m_ToolTip.SetMaxTipWidth(300);  // 支持多行文本
    m_ToolTip.AddTool(this, m_sToolTips);
    m_ToolTip.Activate(m_bToolTipsEnabled);

    // 如果启用了立即显示
    if (m_bToolTipsImmediate && m_bToolTipsEnabled)
    {
        m_ToolTip.SetDelayTime(TTDT_INITIAL, 0);
        m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 0x7FFFFFFF);
        m_ToolTip.SetDelayTime(TTDT_RESHOW, 0);
    }
}

void CLarPngBtn::ClearImages()
{
    m_pImage.reset();
    m_pClickImage.reset();
}

BOOL CLarPngBtn::LoadPNG(UINT resourceID)
{
    // 从资源加载PNG
    HINSTANCE hInstance = AfxGetResourceHandle();
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), L"PNG");
    if (!hResource)
        return FALSE;

    DWORD imageSize = SizeofResource(hInstance, hResource);
    HGLOBAL hGlobal = LoadResource(hInstance, hResource);
    if (!hGlobal)
        return FALSE;

    LPVOID pResourceData = LockResource(hGlobal);
    if (!pResourceData)
        return FALSE;

    // 创建内存流
    IStream* pStream = NULL;
    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (hBuffer)
    {
        void* pBuffer = GlobalLock(hBuffer);
        if (pBuffer)
        {
            CopyMemory(pBuffer, pResourceData, imageSize);
            GlobalUnlock(hBuffer);

            if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK)
            {
                // 从流加载图像
                m_pImage.reset(Gdiplus::Image::FromStream(pStream));
                pStream->Release();

                if (m_pImage)
                    return TRUE;
            }
        }
        GlobalFree(hBuffer);
    }

    return FALSE;
}

BOOL CLarPngBtn::LoadPNG(const CString& filePath)
{
    m_pImage.reset(Gdiplus::Image::FromFile(filePath));
    return (m_pImage != nullptr);
}

BOOL CLarPngBtn::LoadClickPNG(UINT resourceID)
{
    // 从资源加载PNG
    HINSTANCE hInstance = AfxGetResourceHandle();
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), L"PNG");
    if (!hResource)
        return FALSE;

    DWORD imageSize = SizeofResource(hInstance, hResource);
    HGLOBAL hGlobal = LoadResource(hInstance, hResource);
    if (!hGlobal)
        return FALSE;

    LPVOID pResourceData = LockResource(hGlobal);
    if (!pResourceData)
        return FALSE;

    // 创建内存流
    IStream* pStream = NULL;
    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (hBuffer)
    {
        void* pBuffer = GlobalLock(hBuffer);
        if (pBuffer)
        {
            CopyMemory(pBuffer, pResourceData, imageSize);
            GlobalUnlock(hBuffer);

            if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK)
            {
                // 从流加载图像
                m_pClickImage.reset(Gdiplus::Image::FromStream(pStream));
                pStream->Release();

                if (m_pClickImage)
                    return TRUE;
            }
        }
        GlobalFree(hBuffer);
    }

    return FALSE;
}

BOOL CLarPngBtn::LoadPNG(Gdiplus::Bitmap* bitmap)
{
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok)
        return FALSE;

    // 创建原始位图的克隆
    m_pImage.reset(bitmap->Clone(0, 0, bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetPixelFormat()));

    // 检查克隆是否成功
    if (!m_pImage || m_pImage->GetLastStatus() != Gdiplus::Ok)
    {
        m_pImage.reset();
        return FALSE;
    }

    // 如果控件已创建，立即更新显示
    if (GetSafeHwnd())
        Invalidate();

    return TRUE;
}

BOOL CLarPngBtn::LoadClickPNG(Gdiplus::Bitmap* bitmap)
{
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok)
        return FALSE;

    // 创建原始位图的克隆
    m_pClickImage.reset(bitmap->Clone(0, 0, bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetPixelFormat()));

    // 检查克隆是否成功
    if (!m_pClickImage || m_pClickImage->GetLastStatus() != Gdiplus::Ok)
    {
        m_pClickImage.reset();
        return FALSE;
    }

    // 如果控件已创建，立即更新显示
    if (GetSafeHwnd())
        Invalidate();

    return TRUE;
}

BOOL CLarPngBtn::LoadClickPNG(const CString& filePath)
{
    m_pClickImage.reset(Gdiplus::Image::FromFile(filePath));
    return (m_pClickImage != nullptr);
}

void CLarPngBtn::SetToolTips(const CString& tipsText, bool IsDiplayImmediately)
{
    m_sToolTips = tipsText;
    m_bToolTipsEnabled = !tipsText.IsEmpty();
    m_bToolTipsImmediate = IsDiplayImmediately;

    // 如果控件已创建，更新工具提示
    if (::IsWindow(m_hWnd) && ::IsWindow(m_ToolTip.m_hWnd))
    {
        m_ToolTip.UpdateTipText(m_sToolTips, this);
        m_ToolTip.Activate(m_bToolTipsEnabled);

        // 设置工具提示立即显示
        if (IsDiplayImmediately)
        {
            // 将显示延迟设为0毫秒
            m_ToolTip.SetDelayTime(TTDT_INITIAL, 0);

            // 将自动消失时间设为最大值（约24.8天），实际上就不会自动消失
            m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 0x7FFFFFFF);

            // 提示之间切换的延迟也设为0
            m_ToolTip.SetDelayTime(TTDT_RESHOW, 0);
        }
        else
        {
            // 恢复默认延迟时间
            m_ToolTip.SetDelayTime(TTDT_INITIAL, TTDT_AUTOMATIC);
            m_ToolTip.SetDelayTime(TTDT_AUTOPOP, TTDT_AUTOMATIC);
            m_ToolTip.SetDelayTime(TTDT_RESHOW, TTDT_AUTOMATIC);
        }
    }
}

void CLarPngBtn::SetToolTipsDelayTime(int initialDelay, int autoPopDelay, int reshowDelay)
{
    if (!::IsWindow(m_ToolTip.m_hWnd))
        return;

    if (initialDelay >= 0)
        m_ToolTip.SetDelayTime(TTDT_INITIAL, initialDelay);

    if (autoPopDelay >= 0)
        m_ToolTip.SetDelayTime(TTDT_AUTOPOP, autoPopDelay);

    if (reshowDelay >= 0)
        m_ToolTip.SetDelayTime(TTDT_RESHOW, reshowDelay);
}

void CLarPngBtn::SetUseHoverImage(BOOL bUse)
{
    m_bUseHoverImage = bUse;
    // 如果已经创建，立刻刷新一次
    if (GetSafeHwnd())
        Invalidate();
}

void CLarPngBtn::SetHoverEffectColor(BYTE alpha, BYTE red, BYTE green, BYTE blue)
{
    m_bUseHoverEffectColor = TRUE;
    m_HoverEffectColor = Gdiplus::Color(alpha, red, green, blue);
    if (GetSafeHwnd()) {
        Invalidate();

    }
}

void CLarPngBtn::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
    if (!m_pImage) return;

    CDC dc; dc.Attach(lpDIS->hDC);
    CDC memDC; memDC.CreateCompatibleDC(&dc);
    CBitmap bmp; bmp.CreateCompatibleBitmap(
        &dc, lpDIS->rcItem.right, lpDIS->rcItem.bottom);
    CBitmap* pOld = memDC.SelectObject(&bmp);

    // 背景填充
    if (m_bUseCustomBgColor)
        memDC.FillSolidRect(&lpDIS->rcItem, m_CustomBgColor);
    else
        memDC.FillSolidRect(&lpDIS->rcItem,
            ::GetSysColor(COLOR_BTNFACE));

    Gdiplus::Graphics g(memDC.GetSafeHdc());
    if (m_bHighQualityPNG)
    {
        g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        //g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    }

    // 选择绘制图片：正常/点击/悬停
    Gdiplus::Image* pImg = m_pImage.get();
    if ((lpDIS->itemState & ODS_SELECTED) && m_pClickImage)
        pImg = m_pClickImage.get();
    else if (m_bUseHoverImage && m_bMouseHover && m_pClickImage)
        pImg = m_pClickImage.get();

    // 计算目标绘制区域
    int btnW = lpDIS->rcItem.right, btnH = lpDIS->rcItem.bottom;
    int imgW = pImg->GetWidth(), imgH = pImg->GetHeight();

    if (m_bCircularImage)
    {
        // 圆形模式 - 计算圆形参数
        int minDim = min(btnW, btnH);
        int radius;

        // 使用用户指定的半径或自动计算
        if (m_nCircularRadius > 0) {
            radius = m_nCircularRadius;
        }
        else {
            radius = minDim / 2;
        }

        int centerX = btnW / 2;
        int centerY = btnH / 2;

        // 创建一个临时位图来绘制圆形
        Gdiplus::Bitmap tempBitmap(btnW, btnH, PixelFormat32bppARGB);
        Gdiplus::Graphics tempG(&tempBitmap);

        // 设置临时图形对象的质量
        if (m_bHighQualityPNG)
        {
            tempG.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            tempG.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            tempG.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
            tempG.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        }

        // 计算图像绘制尺寸
        float scale;
        int drawW, drawH, x, y;

        if (m_bStretchMode)
        {
            // 在圆形模式下，我们希望保持图像比例不变
            float scaleX = (float)(radius * 2) / imgW;
            float scaleY = (float)(radius * 2) / imgH;
            scale = min(scaleX, scaleY) * m_fStretchRatio;

            drawW = (int)(imgW * scale);
            drawH = (int)(imgH * scale);
            x = centerX - drawW / 2;
            y = centerY - drawH / 2;
        }
        else
        {
            // 原始大小时居中
            drawW = imgW;
            drawH = imgH;
            x = centerX - drawW / 2;
            y = centerY - drawH / 2;
        }

        // 创建圆形路径
        Gdiplus::GraphicsPath path;
        path.AddEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);

        // 设置剪切区域为圆形
        tempG.SetClip(&path);

        // 绘制图像
        tempG.DrawImage(pImg, x, y, drawW, drawH);

        // 如果需要绘制边框
        if (m_bShowCircularBorder)
        {
            tempG.ResetClip();
            Gdiplus::Pen borderPen(m_CircularBorderColor, (float)m_nCircularBorderWidth);
            tempG.DrawEllipse(&borderPen, centerX - radius, centerY - radius, radius * 2, radius * 2);
        }

        // 绘制到目标DC
        g.DrawImage(&tempBitmap, 0, 0, btnW, btnH);
    }
    else
    {
        // 原始矩形模式（保持不变）
        if (m_bStretchMode)
        {
            float scaleX = (float)btnW / imgW;
            float scaleY = (float)btnH / imgH;
            float baseScale = min(scaleX, scaleY);

            // 2. 始终用 baseScale * m_fStretchRatio
            float finalScale = baseScale * m_fStretchRatio;

            // 3. 缩放后的尺寸，并水平垂直居中
            int drawW = int(imgW * finalScale);
            int drawH = int(imgH * finalScale);
            int x = (btnW - drawW) / 2;
            int y = (btnH - drawH) / 2;
            g.DrawImage(pImg, x, y, drawW, drawH);
        }
        else
        {
            // 原始大小时居中
            int x = max(0, (btnW - imgW) / 2);
            int y = max(0, (btnH - imgH) / 2);
            g.DrawImage(pImg, x, y, imgW, imgH);
        }
    }

    // 点击半透明遮罩
    if ((lpDIS->itemState & ODS_SELECTED) && !m_pClickImage)
    {
        Gdiplus::SolidBrush brush(m_ClickEffectColor);
        if (m_bCircularImage)
        {
            int radius = (m_nCircularRadius > 0) ? m_nCircularRadius : min(btnW, btnH) / 2;
            int centerX = btnW / 2;
            int centerY = btnH / 2;

            Gdiplus::GraphicsPath path;
            path.AddEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
            g.FillPath(&brush, &path);
        }
        else
        {
            g.FillRectangle(&brush, 0, 0, btnW, btnH);
        }
    }

    // 悬停高亮背景（未启用 HoverImage 且开启了 HoverEffectColor）
    if (m_bUseHoverEffectColor && !m_bUseHoverImage && m_bMouseHover)
    {
        Gdiplus::SolidBrush hoverBrush(m_HoverEffectColor);
        if (m_bCircularImage)
        {
            int radius = (m_nCircularRadius > 0) ? m_nCircularRadius : min(btnW, btnH) / 2;
            int centerX = btnW / 2;
            int centerY = btnH / 2;

            Gdiplus::GraphicsPath path;
            path.AddEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
            g.FillPath(&hoverBrush, &path);
        }
        else
        {
            g.FillRectangle(&hoverBrush, 0, 0, btnW, btnH);
        }
    }

    // 最终 blit
    dc.BitBlt(0, 0, btnW, btnH, &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(pOld);
    dc.Detach();
}

BOOL CLarPngBtn::PreTranslateMessage(MSG* pMsg)
{
    // 如果启用了工具提示并且控件已创建
    if (m_bToolTipsEnabled && ::IsWindow(m_ToolTip.m_hWnd))
    {
        m_ToolTip.RelayEvent(pMsg);
    }

    return CButton::PreTranslateMessage(pMsg);
}
