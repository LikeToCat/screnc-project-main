// CLarEdit.cpp
#include "stdafx.h"
#include "CLarEdit.h"
#include "CDebug.h"
using namespace Gdiplus;
extern HANDLE ConsoleHandle;
BEGIN_MESSAGE_MAP(CLarEdit, CEdit)
	ON_WM_PAINT()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

CLarEdit::CLarEdit()
	: m_IsEditing(false)
	, m_BorderExpansion(1.0)
	, m_IsReadOnly(false)
	, m_textColor(RGB(0, 0, 0))
{
	m_brush.CreateSolidBrush(RGB(255, 255, 255));
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
	double scale = static_cast<double>(dpiX) / 96.0;
	m_Scale = scale;
}


void CLarEdit::PreSubclassWindow(CREATESTRUCT& cs)
{
	// 移除 WS_BORDER 样式
	ModifyStyle(WS_BORDER, 0, 0);

	// 调用基类的 PreSubclassWindow
	CEdit::PreSubclassWindow();
}

// 设置文本字体
void CLarEdit::LarSetTextFont(const CString fontFamily, int fontSize, int fontStyle)
{
	m_fontSize = fontSize * m_Scale;
	m_fontFamily = fontFamily;
	UpdateControlFont(m_fontFamily, m_fontSize, fontStyle);
	// 应用新字体到控件
	SetFont(&m_font);

	Invalidate();
}

void CLarEdit::LarSetBorderColor(Color color)
{
	m_borderColor = color;
}

void CLarEdit::LarSetBorderThickness(double thickness)
{
	m_borderThickness = thickness;
}

void CLarEdit::LarSetPlaceHolderText(const CString placeHolderText, Gdiplus::Color color, bool SurePlace)
{
	m_placeHolderText = placeHolderText;
	m_PlaceTextColor = color;
	m_SurePlace = SurePlace;
}

void CLarEdit::LarSetBorderExpansion(double BorderExpansion)
{
	m_BorderExpansion = BorderExpansion;
}

void CLarEdit::LarDrawRectBorder()
{
	// 获取父窗口
	CWnd* pParent = GetParent();
	if (pParent == nullptr)
		return;

	// 获取父窗口的设备上下文
	CDC* pCDC = pParent->GetDC();
	if (pCDC == nullptr)
		return;

	// 获取控件的客户区域，准备边框绘制矩形于文本绘制矩形
	CRect rect;
	GetWindowRect(rect);
	pParent->ScreenToClient(&rect);
	double left = 10*m_Scale ;
	double top = 10*m_Scale ;
	double width = 20*m_Scale ;
	double height = 20*m_Scale ;
	Gdiplus::Rect gdipBorderRect((double)((double)rect.left - left * m_BorderExpansion), (double)((double)rect.top - top * m_BorderExpansion),
		(double)((double)rect.Width() + width * m_BorderExpansion), (double)((double)rect.Height() + height * m_BorderExpansion));
	Gdiplus::Rect gdipTextRect(rect.left, rect.top, rect.Width(), rect.Height());
	HDC hdc = pCDC->GetSafeHdc();
	if (hdc != nullptr)
	{//在这里进行自定义逻辑绘画
		Gdiplus::Graphics graphics(hdc);
		CString EditText;
		this->GetWindowText(EditText);
		if (!m_IsEditing && (EditText == "")) {//绘画占位文本
			DEBUG_CONSOLE_STR(ConsoleHandle, L"绘画占位文本");
			if (m_SurePlace)
				SetWindowText(m_placeHolderText);
			DrawPlaceHolderText(gdipTextRect, &graphics);;
		}
		//绘画边框
		Gdiplus::Pen pen(m_borderColor, m_borderThickness);
		graphics.DrawRectangle(&pen, gdipBorderRect);
	}
	pParent->ReleaseDC(pCDC);
}

void CLarEdit::LarSetReadOnly(BOOL isReadOnly)
{
	m_IsReadOnly = isReadOnly;
}

void CLarEdit::UpdateControlFont(const CString& fontFamily, int fontSize, int fontStyle)
{
	LOGFONT lf = { 0 };
	_tcscpy_s(lf.lfFaceName, fontFamily);

	// 使用负值指定字体高度
	double FontSize = static_cast<double>(fontSize);
	lf.lfHeight = -FontSize;
	lf.lfWeight = (fontStyle & FS_BOLD) ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = (fontStyle & FS_ITALIC) ? TRUE : FALSE;
	lf.lfUnderline = (fontStyle & FS_UNDERLINE) ? TRUE : FALSE;
	m_font.DeleteObject();
	m_font.CreateFontIndirect(&lf);
}

void CLarEdit::DrawPlaceHolderText(Rect gdipRect, Graphics* graphics)
{
	Gdiplus::RectF layoutRect(
		static_cast<REAL>(gdipRect.GetLeft()),
		static_cast<REAL>(gdipRect.GetTop()),
		static_cast<REAL>(gdipRect.Width),
		static_cast<REAL>(gdipRect.Height));

	// 创建一个字体（可根据需要调整字体族、大小和样式）
	Gdiplus::FontFamily fontFamily(m_fontFamily);
	Gdiplus::Font font(&fontFamily, m_fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	// 创建一个用于绘制文本的刷子，设置为黑色
	Gdiplus::SolidBrush textBrush(m_PlaceTextColor);

	// m_placeHolderText 为 CString 类型，直接获取字符串指针进行绘制
	graphics->DrawString(m_placeHolderText.GetString(), -1, &font, layoutRect, NULL, &textBrush);
}

BOOL CLarEdit::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类
	return CEdit::PreCreateWindow(cs);
}


void CLarEdit::OnPaint()
{
	CPaintDC dc(this); 
}


void CLarEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	m_IsEditing = false;
	LarDrawRectBorder();
	DEBUG_CONSOLE_STR(ConsoleHandle, L"OnKillFocus");
	// TODO: 在此处添加消息处理程序代码
}


void CLarEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);
	m_IsEditing = true;
	// TODO: 在此处添加消息处理程序代码
	DEBUG_CONSOLE_STR(ConsoleHandle, L"OnSetFocus");
}


BOOL CLarEdit::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return CEdit::OnEraseBkgnd(pDC);
}


void CLarEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_IsReadOnly) {
		return;
	}
	CEdit::OnLButtonDown(nFlags, point);
}

HBRUSH CLarEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_textColor);
	pDC->SetBkColor(RGB(255, 255, 255));
	return m_brush;
}
