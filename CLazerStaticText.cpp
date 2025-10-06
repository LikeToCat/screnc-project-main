// LazerStaticText.cpp
#include "stdafx.h"
#include "CLazerStaticText.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;
BEGIN_MESSAGE_MAP(CLazerStaticText, CStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// 构造函数：设置默认的字体属性
CLazerStaticText::CLazerStaticText()
	: m_FontFamily(_T("微软雅黑")), // 默认字体家族             
	m_IsBold(false),               // 默认加粗
	m_IsUnderline(false),         // 默认不下划线
	m_IsItalic(false),             // 默认不斜体
	m_IsBreakLine(true),
	m_IsEraseBk(false),
	m_EraseColor(RGB(240, 240, 240)),
	m_IsStrikeOut(false),
	m_Isleft(false),			// 是否左对齐
	m_IsRight(false),			// 是否右对齐
	m_IsCenter(true),		// 是否居中
	m_DrawMode(DrawMode::NormalMode)
{
	//获取用户电脑的逻辑DPI
	HDC screen = ::GetDC(NULL);
	if (!screen)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取用户DPI失败");
		m_DpiScale = 1.0;
	}
	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	::ReleaseDC(NULL, screen);
	// 计算缩放因子（假设水平和垂直DPI相同）
	m_DpiScale = static_cast<double>(dpiX) / 96.0; // 96 DPI 是100%的基准

	m_FontSize = 22 * m_DpiScale;
	m_TextAlpha = 255;

	InitFont(); // 初始化字体
	m_TextColor = RGB(255, 255, 255);
	m_isInSizeMove = nullptr;
	m_isMonitorWindowMove = false;
}

// 析构函数：CFont 对象会在析构时自动释放资源
CLazerStaticText::~CLazerStaticText()
{
	// 无需手动删除 m_Font，因为 CFont 的析构函数会处理
}

BOOL CLazerStaticText::OnEraseBkgnd(CDC* pDC)
{
	if (m_IsEraseBk) {
		CRect rect;
		GetClientRect(&rect);
		pDC->FillSolidRect(rect, m_EraseColor); // 使用一个不透明的背景色
		return TRUE;
	}
	else {
		return TRUE;
	}
}

// 初始化字体，根据当前的字体设置创建 CFont 对象
void CLazerStaticText::InitFont()
{
	// 如果已存在字体对象，则删除它
	if (m_Font.GetSafeHandle())
	{
		m_Font.DeleteObject();
	}

	// 定义 LOGFONT 结构体来设置字体属性
	LOGFONT lf = { 0 };
	lf.lfHeight = m_FontSize; // 设置字体大小
	lf.lfWeight = m_IsBold ? FW_BOLD : FW_NORMAL; // 设置字体重量
	lf.lfItalic = m_IsItalic ? TRUE : FALSE; // 设置斜体
	lf.lfUnderline = m_IsUnderline ? TRUE : FALSE; // 设置下划线
	lf.lfStrikeOut = m_IsStrikeOut ? TRUE : FALSE;	//设置删除线
	wcscpy_s(lf.lfFaceName, m_FontFamily); // 设置字体家族

	// 创建字体对象
	m_Font.CreateFontIndirect(&lf);
}

// 公共接口：设置字体家族
void CLazerStaticText::LarSetTextFamily(LPCTSTR fontFamily)
{
	if (fontFamily && _tcscmp(m_FontFamily, fontFamily) != 0)
	{
		m_FontFamily = fontFamily;
		InitFont();      // 重新初始化字体
		Invalidate();    // 触发repaint
	}
}

void CLazerStaticText::LarSetMonitorWindowMove(bool* Dirtymark)
{
	if (Dirtymark)
	{
		m_isInSizeMove = Dirtymark;
		m_isMonitorWindowMove = true;
	}
}

void CLazerStaticText::LarSetTextAlpha(int alpha)
{
	if (alpha < 0) alpha = 0;
	if (alpha > 255) alpha = 255;
	if (m_TextAlpha != alpha)
	{
		m_TextAlpha = alpha;
		Invalidate();
	}
}

// 公共接口：设置字体大小
void CLazerStaticText::LarSetTextSize(int fontSize)
{
	double fSize = static_cast<double>(fontSize) * m_DpiScale;

	if (fSize > 0)
	{
		m_FontSize = fSize;
		InitFont();      // 重新初始化字体
		Invalidate();    // 触发repaint
	}
}

// 公共接口：设置字体样式（加粗、下划线、斜体）
void CLazerStaticText::LarSetTextStyle(bool bold, bool underline, bool italic)
{
	// 仅在样式有变化时更新
	if (m_IsBold != bold || m_IsUnderline != underline || m_IsItalic != italic)
	{
		m_IsBold = bold;
		m_IsUnderline = underline;
		m_IsItalic = italic;
		InitFont();      // 重新初始化字体
		Invalidate();    // 触发repaint
	}
}

void CLazerStaticText::LarSetBreakLine(bool isEnable)
{
	m_IsBreakLine = isEnable;
}

void CLazerStaticText::LarSetTextColor(COLORREF colorRef)
{
	m_TextColor = colorRef;
	Invalidate();
}

void CLazerStaticText::LarSetText(CString Text)
{
	m_CustomText = Text;
	Invalidate();
}

void CLazerStaticText::LarSetIsEraseBk(bool enable)
{
	m_IsEraseBk = enable;
}

void CLazerStaticText::LarSetEraseColor(COLORREF colorRef)
{
	m_EraseColor = colorRef;
}

void CLazerStaticText::LarSetStrikeOut(bool isStrikeOut)
{
	m_IsStrikeOut = isStrikeOut;
	InitFont();
}

void CLazerStaticText::LarSetTextCenter()
{
	m_IsCenter = true;
	m_Isleft = false;
	m_IsRight = false;
}

void CLazerStaticText::LarSetTextLeft()
{
	m_Isleft = true;
	m_IsCenter = false;
	m_IsRight = false;
}

void CLazerStaticText::LarSetTextRight()
{
	m_IsRight = true;
	m_IsCenter = false;
	m_Isleft = false;
}

void CLazerStaticText::LarSetDrawMode(DrawMode drawMode)
{
	m_DrawMode = drawMode;
}

void CLazerStaticText::OnPaint()
{
	if (m_isMonitorWindowMove && *m_isInSizeMove)//如果父窗口正在被拖动，则直接返回
		return;

	if (m_DrawMode == DrawMode::NormalMode)
	{
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);

		Gdiplus::Graphics graphics(dc.GetSafeHdc());
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

		dc.SetTextColor(m_TextColor);
		dc.SetBkMode(TRANSPARENT);
		CFont* pOldFont = dc.SelectObject(&m_Font);

		CString strText;
		GetWindowText(strText);
		if (!m_CustomText.IsEmpty())
			strText = m_CustomText;

		if (m_TextAlpha >= 255)
		{
			dc.SetTextColor(m_TextColor);
			dc.SetBkMode(TRANSPARENT);
			CFont* pOldFont = dc.SelectObject(&m_Font);

			UINT uFormat = DT_VCENTER | DT_NOPREFIX;
			uFormat |= (m_IsBreakLine ? DT_WORDBREAK : DT_SINGLELINE);
			if (m_Isleft)      uFormat |= DT_LEFT;
			else if (m_IsCenter) uFormat |= DT_CENTER;
			else if (m_IsRight)  uFormat |= DT_RIGHT;

			dc.DrawText(strText, &rect, uFormat);
			dc.SelectObject(pOldFont);
		}
		else
		{
			using namespace Gdiplus;
			FontFamily fontFamily(m_FontFamily);
			FontStyle style = FontStyleRegular;
			if (m_IsBold)      style = (FontStyle)(style | FontStyleBold);
			if (m_IsItalic)    style = (FontStyle)(style | FontStyleItalic);
			if (m_IsUnderline) style = (FontStyle)(style | FontStyleUnderline);

			Gdiplus::Font font(&fontFamily, (REAL)m_FontSize - 8, style, UnitPixel);

			Color textColor(
				(BYTE)m_TextAlpha,
				GetRValue(m_TextColor),
				GetGValue(m_TextColor),
				GetBValue(m_TextColor));
			SolidBrush textBrush(textColor);

			StringFormat format;
			format.SetLineAlignment(StringAlignmentCenter);
			if (m_Isleft)        format.SetAlignment(StringAlignmentNear);
			else if (m_IsCenter) format.SetAlignment(StringAlignmentCenter);
			else if (m_IsRight)  format.SetAlignment(StringAlignmentFar);
			if (m_IsBreakLine)   format.SetFormatFlags(0);
			else                 format.SetFormatFlags(StringFormatFlagsNoWrap);

			RectF layoutRect(
				(REAL)rect.left, (REAL)rect.top,
				(REAL)rect.Width(), (REAL)rect.Height());

			graphics.DrawString(strText, strText.GetLength(), &font, layoutRect, &format, &textBrush);
		}
	}
	else if (m_DrawMode == DrawMode::GdiplusHightQualityMode)
	{
		using namespace Gdiplus;
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(&rect);

		Gdiplus::Graphics graphics(dc.GetSafeHdc());
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

		// 背景透明或自定义背景
		if (m_IsEraseBk)
		{
			Gdiplus::SolidBrush bkBrush(Gdiplus::Color(
				255,
				GetRValue(m_EraseColor),
				GetGValue(m_EraseColor),
				GetBValue(m_EraseColor)
				));
			graphics.FillRectangle(&bkBrush, rect.left, rect.top, rect.Width(), rect.Height());
		}

		CString strText;
		GetWindowText(strText);
		if (!m_CustomText.IsEmpty())
			strText = m_CustomText;

		// 字体属性
		Gdiplus::FontFamily fontFamily(m_FontFamily);
		Gdiplus::FontStyle style = Gdiplus::FontStyleRegular;
		if (m_IsBold) style = (Gdiplus::FontStyle)(style | Gdiplus::FontStyleBold);
		if (m_IsItalic) style = (Gdiplus::FontStyle)(style | Gdiplus::FontStyleItalic);
		if (m_IsUnderline) style = (Gdiplus::FontStyle)(style | Gdiplus::FontStyleUnderline);

		Gdiplus::Font font(
			&fontFamily,
			(REAL)m_FontSize,
			style,
			Gdiplus::UnitPixel
			);

		Gdiplus::Color textColor(
			(BYTE)m_TextAlpha,
			GetRValue(m_TextColor),
			GetGValue(m_TextColor),
			GetBValue(m_TextColor)
		);
		Gdiplus::SolidBrush textBrush(textColor);

		// 对齐方式
		Gdiplus::StringFormat format;
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		if (m_Isleft)
			format.SetAlignment(Gdiplus::StringAlignmentNear);
		else if (m_IsCenter)
			format.SetAlignment(Gdiplus::StringAlignmentCenter);
		else if (m_IsRight)
			format.SetAlignment(Gdiplus::StringAlignmentFar);

		if (m_IsBreakLine)
			format.SetFormatFlags(0); // 允许换行
		else
			format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

		Gdiplus::RectF layoutRect(
			(Gdiplus::REAL)rect.left,
			(Gdiplus::REAL)rect.top,
			(Gdiplus::REAL)rect.Width(),
			(Gdiplus::REAL)rect.Height()
			);

		graphics.DrawString(
			strText, strText.GetLength(),
			&font,
			layoutRect,
			&format,
			&textBrush
			);
	}
}