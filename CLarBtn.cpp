#include "stdafx.h"
#include "CLarBtn.h"
#include "CDebug.h"
extern HANDLE ConsoleHandle;
//#define LOGINFO
// Initialize message map
BEGIN_MESSAGE_MAP(CLarBtn, CButton)
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CLarBtn::CLarBtn()
	: m_BkBitmap(nullptr),
	m_IsNoInteraction(false),
	m_bIsHovered(false),
	m_bIsPressed(false),
	m_IsStatic(false),
	m_DpiScale(1),
	m_ClickedSolidBrush(nullptr),
	m_HoverSolidBrush(nullptr),
	m_NormalSolidBrush(nullptr),
	m_FontFamily(_T("微软雅黑")), // 默认字体家族
	m_FontSize(22),               // 默认字体大小
	m_IsBold(false),
	m_IsUnderline(false),
	m_IsItalic(false),
	m_TextColor(Gdiplus::Color(255, 0, 0, 0)),
	m_IsAngleRounded(false),
	m_Textcolor_Hover(128, 0, 0, 0),
	m_Textcolor_Click(235, 0, 0, 0),
	m_showMode(BitmapShowMode::SHOW_ORIGINAL),
	m_IsBitmapsEmpty(true),
	m_IsSetParentBkCalled(false),
	m_IsBtnEnable(true),
	m_IsEraseBkEnable(true),
	m_IsTextCenter(true),
	m_BorderSize(1),
	m_Bitmap_NailImage(nullptr),
	m_Int_TextDiffX(0),
	m_Int_TextDiffY(0),
	m_bool_IsGradualBtn(false),
	m_Color_gc1(Gdiplus::Color(19, 176, 187)),
	m_Color_gc2(Gdiplus::Color(1, 192, 133)),
	m_enum_gcMode(Gdiplus::LinearGradientMode::LinearGradientModeHorizontal),
	m_Color_hoverGc1(Gdiplus::Color(29, 186, 197)),
	m_Color_hoverGc2(Gdiplus::Color(11, 202, 143)),
	m_enum_hoverGcMode(Gdiplus::LinearGradientMode::LinearGradientModeHorizontal),
	m_Color_clickGc1(Gdiplus::Color(255, 234, 212,193)),
	m_Color_clickGc2(Gdiplus::Color(255,204,164,120)),	
	m_enum_clickGcMode(Gdiplus::LinearGradientMode::LinearGradientModeHorizontal),
	m_bool_IsMultline(false),
	m_int_textMaxWidth(-1),
	m_Bitmap_HoverNailImage(nullptr),
	m_bool_EnableBorderDraw(true)
{
	m_Int_NailImageHoverBrn = 1.2f;
	m_Int_NailImageClickBrn = 1.3f;
	m_Rect_NailImage = Gdiplus::Rect();
	m_OpacityAlpha = 255;

	// 初始化默认字体
	int fontSize = m_FontSize * 10;
	m_Font.CreatePointFont(fontSize, m_FontFamily, nullptr);

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
}

CLarBtn::~CLarBtn()
{
	//if (m_BkBitmap)
	//{
	//	delete m_BkBitmap;
	//	m_BkBitmap = nullptr;
	//}
}

BOOL CLarBtn::OnEraseBkgnd(CDC* pDC)
{
	if (m_IsEraseBkEnable && (!m_IsNoInteraction)) 
	{
		// 创建 GDI+ Graphics 对象
		Gdiplus::Graphics graphics(pDC->GetSafeHdc());

		////获取客户区矩形区域
		CRect buttonRect;
		GetClientRect(buttonRect);
		SolidBrush brush(ApplyOpacity(Color(255, 255, 255, 255)));
		if (m_IsAngleRounded) {
			graphics.FillPath(&brush, &m_AngleRoundedpath);
		}
		else {
			graphics.FillRectangle(&brush, 0, 0, buttonRect.Width(), buttonRect.Height());
		}
	}
	return TRUE;
}

BOOL CLarBtn::PreCreateWindow(CREATESTRUCT& cs)
{
	// Set the Owner Draw style
	cs.style |= BS_OWNERDRAW;
	return CButton::PreCreateWindow(cs);
}

void CLarBtn::FillWithTextureBrush(Gdiplus::Graphics& graphics, Gdiplus::TextureBrush& textureBrush,
	const CRect& rect, float opacity)
{
	// 创建颜色矩阵来调整透明度
	Gdiplus::ColorMatrix colorMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 红色通道
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // 绿色通道
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 蓝色通道
		0.0f, 0.0f, 0.0f, opacity, 0.0f, // alpha 通道
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // 偏移量
	};

	Gdiplus::ImageAttributes imgAttr;
	imgAttr.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

	// 绘制带有透明度的 TextureBrush
	graphics.DrawImage(
		m_BkBitmap,
		Gdiplus::Rect(rect.left, rect.top, rect.Width(), rect.Height()),
		0, 0, m_BkBitmap->GetWidth(), m_BkBitmap->GetHeight(),
		Gdiplus::UnitPixel,
		&imgAttr
	);
}

void CLarBtn::LarSetNormalFiilBrush(const SolidBrush& brush)
{
	if (m_NormalSolidBrush)
	{
		delete m_NormalSolidBrush;
	}
	Color color;
	brush.GetColor(&color);
	m_NormalSolidBrush = new SolidBrush(color);
	Invalidate();
}

void CLarBtn::LarSetBtnEnable(bool isEnable)
{
	m_IsBtnEnable = isEnable;
	if (!m_IsBtnEnable) {//如果按钮被禁用
		m_TextColor_BeforeBanned = m_TextColor;
		Color color((m_TextColor.GetAlpha() - 60 < 20 ? 20 : m_TextColor.GetAlpha() - 60),
			m_TextColor.GetR(),
			m_TextColor.GetG(),
			m_TextColor.GetB()
		);
		m_TextColor = color;
	}
	else {
		m_TextColor = m_TextColor_BeforeBanned;
	}
	LarSetBtnIsStatic(!m_IsBtnEnable);
	EnableWindow(m_IsBtnEnable);
	Invalidate(false);
}

void CLarBtn::LarSetBorderColor(const Gdiplus::Color& color)
{
	m_BorderColor = color;
}

void CLarBtn::LarSetEraseBkEnable(bool isEnable)
{
	m_IsEraseBkEnable = isEnable;
	Invalidate();
}

void CLarBtn::LarSetTextCenter(bool isCenter)
{
	m_IsTextCenter = isCenter;
}

void CLarBtn::LarSetParentBkCalled()
{
	m_IsSetParentBkCalled = false;
}

void CLarBtn::LarSetButtonNoInteraction(bool enable)
{
	m_IsNoInteraction = enable;
}

void CLarBtn::LarsetBorderSize(int size)
{
	m_BorderSize = size;
}

void CLarBtn::LarSetBtnNailImage(
	UINT nResourceID, NailImageLayout nailLayout,
	int imageWidth, int imageHeight, int diffx, int diffy, 
	float hoverBrightNess, float clickBrightNess
)
{
	HINSTANCE hInstance = AfxGetInstanceHandle();
	HRSRC hResource = FindResource(
		hInstance,
		MAKEINTRESOURCE(nResourceID),
		L"PNG");
	if (hResource == nullptr)
		return;
	DWORD imageSize = SizeofResource(hInstance, hResource);
	if (imageSize == 0)
		return;
	HGLOBAL hResourceData = LoadResource(hInstance, hResource);
	if (hResourceData == nullptr)
		return;
	const void* pResourceData = LockResource(hResourceData);
	if (pResourceData == nullptr)
		return;
	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (!hBuffer)
		return;
	void* pBuffer = GlobalLock(hBuffer);
	memcpy(pBuffer, pResourceData, imageSize);
	GlobalUnlock(hBuffer);
	IStream* pStream = nullptr;
	if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) != S_OK)
	{
		GlobalFree(hBuffer);
		return;
	}
	m_Bitmap_NailImage = Bitmap::FromStream(pStream);
	pStream->Release();

	CRect btnRect;
	GetWindowRect(btnRect);

	switch (nailLayout)
	{
	case CLarBtn::NailImageLayout::Left:
	{
		m_Rect_NailImage.X = 0 + diffx;
		m_Rect_NailImage.Y = (btnRect.Height() - imageHeight) / 2 + diffy;
		m_Rect_NailImage.Width = imageWidth;
		m_Rect_NailImage.Height = imageHeight;
	}
	break;
	case CLarBtn::NailImageLayout::Right:
	{
		m_Rect_NailImage.X = btnRect.Width() - imageWidth + diffx;
		m_Rect_NailImage.Y = (btnRect.Height() - imageHeight) / 2 + diffy;
		m_Rect_NailImage.Width = imageWidth;
		m_Rect_NailImage.Height = imageHeight;
	}
	break;
	default:
		break;
	}
}

void CLarBtn::LarSetBtnHoverNailImage(UINT nResourceID, NailImageLayout nailLayout, int imageWidth, int imageHeight, int diffx, int diffy, float hoverBrightNess, float clickBrightNess)
{
	HINSTANCE hInstance = AfxGetInstanceHandle();
	HRSRC hResource = FindResource(
		hInstance,
		MAKEINTRESOURCE(nResourceID),
		L"PNG");
	if (hResource == nullptr)
		return;
	DWORD imageSize = SizeofResource(hInstance, hResource);
	if (imageSize == 0)
		return;
	HGLOBAL hResourceData = LoadResource(hInstance, hResource);
	if (hResourceData == nullptr)
		return;
	const void* pResourceData = LockResource(hResourceData);
	if (pResourceData == nullptr)
		return;
	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	if (!hBuffer)
		return;
	void* pBuffer = GlobalLock(hBuffer);
	memcpy(pBuffer, pResourceData, imageSize);
	GlobalUnlock(hBuffer);
	IStream* pStream = nullptr;
	if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) != S_OK)
	{
		GlobalFree(hBuffer);
		return;
	}
	m_Bitmap_HoverNailImage = Bitmap::FromStream(pStream);
	pStream->Release();

	CRect btnRect;
	GetWindowRect(btnRect);

	switch (nailLayout)
	{
	case CLarBtn::NailImageLayout::Left:
	{
		m_Rect_HoverNailImage.X = 0 + diffx;
		m_Rect_HoverNailImage.Y = (btnRect.Height() - imageHeight) / 2 + diffy;
		m_Rect_HoverNailImage.Width = imageWidth;
		m_Rect_HoverNailImage.Height = imageHeight;
	}
	break;
	case CLarBtn::NailImageLayout::Right:
	{
		m_Rect_HoverNailImage.X = btnRect.Width() - imageWidth + diffx;
		m_Rect_HoverNailImage.Y = (btnRect.Height() - imageHeight) / 2 + diffy;
		m_Rect_HoverNailImage.Width = imageWidth;
		m_Rect_HoverNailImage.Height = imageHeight;
	}
	break;
	default:
		break;
	}
}

void CLarBtn::LarAdjustTextDisplayPos(int diffx, int diffy)
{
	m_Int_TextDiffX = diffx;
	m_Int_TextDiffY = diffy;
}

void CLarBtn::LarSetGradualColor(Gdiplus::Color c1, Gdiplus::Color c2, Gdiplus::LinearGradientMode gcMode)
{
	m_Color_gc1 = c1;
	m_Color_gc2 = c2;
	m_enum_gcMode = gcMode;
	m_bool_IsGradualBtn = true;
	Invalidate();
}

void CLarBtn::LarSetHoverGradualColor(Gdiplus::Color c1, Gdiplus::Color c2, Gdiplus::LinearGradientMode gcMode)
{
	m_Color_hoverGc1 = c1;
	m_Color_hoverGc2 = c2;
	m_enum_hoverGcMode = gcMode;
	m_bool_IsGradualBtn = true;
	Invalidate();
}

void CLarBtn::LarSetClickGradualColor(Gdiplus::Color c1, Gdiplus::Color c2, Gdiplus::LinearGradientMode gcMode)
{
	m_Color_clickGc1 = c1;
	m_Color_clickGc2 = c2;
	m_enum_clickGcMode = gcMode;
	m_bool_IsGradualBtn = true;
	Invalidate();
}

void CLarBtn::LarSetBtnTextMultLine(bool isMultline)
{
	m_bool_IsMultline = isMultline;
}

void CLarBtn::LarSetTextMaxWidth(int width)
{
	m_int_textMaxWidth = width;
}

void CLarBtn::LarSetBtnBorderEnable(bool isEnable)
{
	m_bool_EnableBorderDraw = isEnable;
}

void CLarBtn::LarSetOpacity(BYTE alpha)
{
	m_OpacityAlpha = alpha;
	Invalidate(FALSE); 
}

void CLarBtn::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	// 创建 GDI+ Graphics 对象
	Gdiplus::Graphics graphics(lpDrawItemStruct->hDC);
	graphics.SetSmoothingMode(SmoothingModeHighQuality);
	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	graphics.SetCompositingQuality(CompositingQualityHighQuality);
	graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
	graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);

	// 获取按钮的矩形区域
	RECT rect = lpDrawItemStruct->rcItem;
	CRect buttonRect(&rect);
	CRect cRect = rect;

	//如果要绘制圆角，首先创建圆角路径
	if (m_IsAngleRounded && m_AngleRoundedpath.GetPointCount() == 0) {
		CreateRoundedRectanglePath(m_AngleRoundedpath, cRect, m_AngleRoundedRadius);
	}

	// 确定按钮的当前状态
	bool isPressed = m_bIsPressed;
	bool isHovered = m_bIsHovered;
	bool isNormal = !isPressed && !isHovered;

	// 绘制背景
	if (!m_bool_IsGradualBtn)//如果未开启渐变模式
	{
		if (m_BkBitmap)
		{
			// 使用 m_BkBitmap 作为背景
			Gdiplus::TextureBrush imageBrush(m_BkBitmap);
			FillWithTextureBrush(graphics, imageBrush, buttonRect, 1.0f); // 绘制图片（可选透明度）

			//处理互动状态下的绘画
			if (isPressed && (!m_IsNoInteraction)) { // 处理按下时绘画
				if (m_ClickedSolidBrush) {//如果设置了按下时的画刷
#ifdef LOGINFO
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘画设置背景，使用按下时设置画刷,是否绘画圆角:%s\n", m_IsAngleRounded ? L"是" : L"否");
#endif // NoLog
					m_IsAngleRounded ?
						graphics.FillPath(m_ClickedSolidBrush, &m_AngleRoundedpath) :
						graphics.FillRectangle(m_ClickedSolidBrush, 0, 0, buttonRect.Width(), buttonRect.Height());
				}
			}
			else if (isHovered && (!m_IsNoInteraction)) {
				if (m_HoverSolidBrush) {
#ifdef LOGINFO
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘画设置背景，使用悬停时设置画刷,是否绘画圆角:%s\n", m_IsAngleRounded ? L"是" : L"否");
#endif
					m_IsAngleRounded ?
						graphics.FillPath(m_HoverSolidBrush, &m_AngleRoundedpath) :
						graphics.FillRectangle(m_HoverSolidBrush, 0, 0, buttonRect.Width(), buttonRect.Height());
				}
			}
		}
		else
		{
			if (isNormal) {
				//绘画纯色背景
				if (m_NormalSolidBrush) {//如果设置了正常状态下的背景画刷
#ifdef LOGINFO
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘画纯色设置背景,是否绘画圆角:%s\n", m_IsAngleRounded ? L"是" : L"否");
#endif
					m_IsAngleRounded ?
						graphics.FillPath(m_NormalSolidBrush, &m_AngleRoundedpath) :
						graphics.FillRectangle(m_NormalSolidBrush, 0, 0, buttonRect.Width(), buttonRect.Height());
				}
				else {
					SolidBrush brush(Color(255, 255, 255, 255));//纯白色背景	
#ifdef LOGINFO
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘画纯色默认背景,是否绘画圆角:%s\n", m_IsAngleRounded ? L"是" : L"否");
#endif
					m_IsAngleRounded ?
						graphics.FillPath(&brush, &m_AngleRoundedpath) :
						graphics.FillRectangle(&brush, 0, 0, buttonRect.Width(), buttonRect.Height());
				}
				}
			//处理互动状态下的绘画
			else if (isPressed && (!m_IsNoInteraction))
			{
				if (m_ClickedSolidBrush) {//如果设置了按下时的画刷
#ifdef LOGINFO
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘画默认背景，使用按下时设置画刷,是否绘画圆角:%s\n", m_IsAngleRounded ? L"是" : L"否");
#endif
					m_IsAngleRounded ?
						graphics.FillPath(m_ClickedSolidBrush, &m_AngleRoundedpath) :
						graphics.FillRectangle(m_ClickedSolidBrush, 0, 0, buttonRect.Width(), buttonRect.Height());
				}
			}
			else if (isHovered && (!m_IsNoInteraction))
			{
				if (m_HoverSolidBrush) {//如果设置了按下时的画刷
#ifdef LOGINFO
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘画默认背景，使用悬停时设置画刷,是否绘画圆角:%s\n", m_IsAngleRounded ? L"是" : L"否");
#endif
					m_IsAngleRounded ?
						graphics.FillPath(m_HoverSolidBrush, &m_AngleRoundedpath) :
						graphics.FillRectangle(m_HoverSolidBrush, 0, 0, buttonRect.Width(), buttonRect.Height());
				}
			}
			}
	}
	else//如果开启了渐变色模式
	{
		Gdiplus::Rect btnRect(
			buttonRect.left, buttonRect.top,
			buttonRect.right - buttonRect.left, buttonRect.bottom - buttonRect.top
		);
		if (!m_IsAngleRounded)
		{
			if (isNormal)
			{
				Gdiplus::LinearGradientBrush gcBrush(btnRect, m_Color_gc1, m_Color_gc2, m_enum_gcMode);
				graphics.FillRectangle(&gcBrush, btnRect);
			}
			else if (isPressed && (!m_IsNoInteraction))
			{
				Gdiplus::LinearGradientBrush gcBrush(btnRect, m_Color_clickGc1, m_Color_clickGc2, m_enum_clickGcMode);
				graphics.FillRectangle(&gcBrush, btnRect);
			}
			else if (isHovered && (!m_IsNoInteraction))
			{
				Gdiplus::LinearGradientBrush gcBrush(btnRect, m_Color_hoverGc1, m_Color_hoverGc2, m_enum_hoverGcMode);
				graphics.FillRectangle(&gcBrush, btnRect);
			}
		}
		else
		{
			if (isNormal)
			{
				Gdiplus::LinearGradientBrush gcBrush(btnRect, m_Color_gc1, m_Color_gc2, m_enum_gcMode);
				graphics.FillPath(&gcBrush, &m_AngleRoundedpath);
			}
			else if (isPressed && (!m_IsNoInteraction))
			{
				Gdiplus::LinearGradientBrush gcBrush(btnRect, m_Color_clickGc1, m_Color_clickGc2, m_enum_clickGcMode);
				graphics.FillPath(&gcBrush, &m_AngleRoundedpath);
			}
			else if (isHovered && (!m_IsNoInteraction))
			{
				Gdiplus::LinearGradientBrush gcBrush(btnRect, m_Color_hoverGc1, m_Color_hoverGc2, m_enum_hoverGcMode);
				graphics.FillPath(&gcBrush, &m_AngleRoundedpath);
			}
		}
	}

	// 获取按钮文本
	CString btnText;
	GetWindowText(btnText);

	// 设置文本绘制属性
	StringFormat format;
	if (m_IsTextCenter) 
		format.SetAlignment(StringAlignmentCenter);
	if (!m_bool_IsMultline)
	{
		format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
		format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
	}
	format.SetLineAlignment(StringAlignmentCenter);

	// 创建GDI+字体
	LOGFONT lf;
	m_Font.GetLogFont(&lf);
	std::unique_ptr<Gdiplus::Font> m_GdiFont;	//字体对象（Gdiplus版本）
	m_GdiFont = make_unique<Gdiplus::Font>(::GetDC(GetSafeHwnd()), &lf);

	// 根据当前按钮状态,设置文本颜色和透明度
	Color textColor;
	if (isPressed) {
#ifdef LOGINFO
		DEBUG_CONSOLE_STR(ConsoleHandle, L"绘制按下字体颜色");
#endif // NOLOG
		textColor = m_Textcolor_Click;
	}
	else if (isHovered) {
#ifdef LOGINFO
		DEBUG_CONSOLE_STR(ConsoleHandle, L"绘制悬停字体颜色");
#endif // NOLOG
		textColor = m_Textcolor_Hover;
	}
	else {
#ifdef LOGINFO
		DEBUG_CONSOLE_STR(ConsoleHandle, L"绘制正常字体颜色");
#endif // NOLOG
		textColor = m_TextColor;
	}
	Color gdiplusColor(textColor);
	SolidBrush brush(gdiplusColor);

	// 绘制文本
	int txtWidth = (m_int_textMaxWidth != -1 ? m_int_textMaxWidth : buttonRect.Width());
	graphics.DrawString(
		btnText,
		-1,
		m_GdiFont.get(),
		RectF((REAL)buttonRect.left + m_Int_TextDiffX, (REAL)buttonRect.top + m_Int_TextDiffY, (REAL)txtWidth, (REAL)buttonRect.Height()),
		&format,
		&brush
	);

	//绘制缩略图
	if (!m_bIsHovered)
	{
		if (m_Bitmap_NailImage)
		{
			if (m_bIsHovered)
			{
				ColorMatrix cm = {
					m_Int_NailImageHoverBrn, 0, 0, 0, 0,
					0, m_Int_NailImageHoverBrn, 0, 0, 0,
					0, 0, m_Int_NailImageHoverBrn, 0, 0,
					0, 0, 0, 1, 0,
					0, 0, 0, 0, 1
				};
				ImageAttributes imgAttr;
				imgAttr.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
				graphics.DrawImage(
					m_Bitmap_NailImage,
					m_Rect_NailImage,
					0, 0, m_Bitmap_NailImage->GetWidth(), m_Bitmap_NailImage->GetHeight(),
					UnitPixel,
					&imgAttr
				);
			}
			else if (m_bIsPressed)
			{
				ColorMatrix cm = {
					m_Int_NailImageClickBrn, 0, 0, 0, 0,
					0, m_Int_NailImageClickBrn, 0, 0, 0,
					0, 0, m_Int_NailImageClickBrn, 0, 0,
					0, 0, 0, 1, 0,
					0, 0, 0, 0, 1
				};
				ImageAttributes imgAttr;
				imgAttr.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
				graphics.DrawImage(
					m_Bitmap_NailImage,
					m_Rect_NailImage,
					0, 0, m_Bitmap_NailImage->GetWidth(), m_Bitmap_NailImage->GetHeight(),
					UnitPixel,
					&imgAttr
				);
			}
			else
			{
				graphics.DrawImage(m_Bitmap_NailImage, m_Rect_NailImage);
			}
		}
	}
	else
	{
		if (m_Bitmap_HoverNailImage)
		{
			if (m_bIsHovered)
			{
				ColorMatrix cm = {
					m_Int_NailImageHoverBrn, 0, 0, 0, 0,
					0, m_Int_NailImageHoverBrn, 0, 0, 0,
					0, 0, m_Int_NailImageHoverBrn, 0, 0,
					0, 0, 0, 1, 0,
					0, 0, 0, 0, 1
				};
				ImageAttributes imgAttr;
				imgAttr.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
				graphics.DrawImage(
					m_Bitmap_HoverNailImage,
					m_Rect_HoverNailImage,
					0, 0, m_Bitmap_HoverNailImage->GetWidth(), m_Bitmap_HoverNailImage->GetHeight(),
					UnitPixel,
					&imgAttr
				);
			}
			else if (m_bIsPressed)
			{
				ColorMatrix cm = {
					m_Int_NailImageClickBrn, 0, 0, 0, 0,
					0, m_Int_NailImageClickBrn, 0, 0, 0,
					0, 0, m_Int_NailImageClickBrn, 0, 0,
					0, 0, 0, 1, 0,
					0, 0, 0, 0, 1
				};
				ImageAttributes imgAttr;
				imgAttr.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
				graphics.DrawImage(
					m_Bitmap_HoverNailImage,
					m_Rect_HoverNailImage,
					0, 0, m_Bitmap_HoverNailImage->GetWidth(), m_Bitmap_HoverNailImage->GetHeight(),
					UnitPixel,
					&imgAttr
				);
			}
			else
			{
				graphics.DrawImage(m_Bitmap_HoverNailImage, m_Rect_HoverNailImage);
			}
		}
		else
		{
			if (m_Bitmap_NailImage)
			{
				if (m_bIsHovered)
				{
					ColorMatrix cm = {
						m_Int_NailImageHoverBrn, 0, 0, 0, 0,
						0, m_Int_NailImageHoverBrn, 0, 0, 0,
						0, 0, m_Int_NailImageHoverBrn, 0, 0,
						0, 0, 0, 1, 0,
						0, 0, 0, 0, 1
					};
					ImageAttributes imgAttr;
					imgAttr.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
					graphics.DrawImage(
						m_Bitmap_NailImage,
						m_Rect_NailImage,
						0, 0, m_Bitmap_NailImage->GetWidth(), m_Bitmap_NailImage->GetHeight(),
						UnitPixel,
						&imgAttr
					);
				}
				else if (m_bIsPressed)
				{
					ColorMatrix cm = {
						m_Int_NailImageClickBrn, 0, 0, 0, 0,
						0, m_Int_NailImageClickBrn, 0, 0, 0,
						0, 0, m_Int_NailImageClickBrn, 0, 0,
						0, 0, 0, 1, 0,
						0, 0, 0, 0, 1
					};
					ImageAttributes imgAttr;
					imgAttr.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
					graphics.DrawImage(
						m_Bitmap_NailImage,
						m_Rect_NailImage,
						0, 0, m_Bitmap_NailImage->GetWidth(), m_Bitmap_NailImage->GetHeight(),
						UnitPixel,
						&imgAttr
					);
				}
				else
				{
					graphics.DrawImage(m_Bitmap_NailImage, m_Rect_NailImage);
				}
			}
		}
	}
	
	// 绘制边框效果
	if (m_bool_EnableBorderDraw)
	{
		Pen BorderPen(m_BorderColor, m_BorderSize);
		m_IsAngleRounded ?
			graphics.DrawPath(&BorderPen, &m_AngleRoundedpath) :
			graphics.DrawRectangle(
				&BorderPen,
				buttonRect.left,
				buttonRect.top,
				buttonRect.Width(),
				buttonRect.Height()
			);
	}
	dc.Detach();
}

void CLarBtn::LaSetTextColor(const Gdiplus::Color& color)
{
	if (m_TextColor.GetValue() != color.GetValue())
	{
		m_TextColor = color;
		Init_Update_Font();      // 重新初始化字体
		Invalidate(); // 触发repaint以应用新的文本颜色
	}
}

// 接口：设置字体家族
void CLarBtn::LarSetTextFamily(const CString& fontFamily)
{
	if (fontFamily && _tcscmp(m_FontFamily, fontFamily) != 0)
	{
		m_FontFamily = fontFamily;
		Init_Update_Font();      // 重新初始化字体
		Invalidate();    // 触发repaint
	}
}

// 接口：设置字体大小
void CLarBtn::LarSetTextSize(int fontSize)
{
	if (fontSize > 0 && m_FontSize != fontSize)
	{
		m_FontSize = fontSize * m_DpiScale;
		Init_Update_Font();      // 重新初始化字体
		Invalidate();    // 触发repaint
	}
}

void CLarBtn::Init_Update_Font()
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
	wcscpy_s(lf.lfFaceName, m_FontFamily); // 设置字体家族

	// 创建字体对象
	m_Font.CreateFontIndirect(&lf);
}

// 接口：设置字体样式（加粗、下划线、斜体）
void CLarBtn::LarSetTextStyle(bool bold, bool underline, bool italic)
{
	// 仅在样式有变化时更新
	if (m_IsBold != bold || m_IsUnderline != underline || m_IsItalic != italic)
	{
		m_IsBold = bold;
		m_IsUnderline = underline;
		m_IsItalic = italic;
		Init_Update_Font();      // 重新初始化字体
		Invalidate();    // 触发repaint
	}
}

// 接口：设置悬停时绘画画刷
void CLarBtn::LarSetHoverFillBrush(const SolidBrush& brush)
{
	if (m_HoverSolidBrush)
	{
		delete m_HoverSolidBrush;
	}
	Color color;
	brush.GetColor(&color);
	m_HoverSolidBrush = new SolidBrush(color);
	Invalidate();
}

// 接口：设置字体悬停时颜色
void CLarBtn::LaSetTextHoverColor(const Gdiplus::Color& color)
{
	m_Textcolor_Hover = color;
}

// 接口：设置字体按下时颜色
void CLarBtn::LaSetTextClickedColor(const Gdiplus::Color& color)
{
	m_Textcolor_Click = color;
}

// 接口：设置按下时绘画画刷
void CLarBtn::LarSetClickedFillBrush(const SolidBrush& brush)
{
	if (m_ClickedSolidBrush)
	{
		delete m_ClickedSolidBrush;
	}
	Color color;
	brush.GetColor(&color);
	m_ClickedSolidBrush = new SolidBrush(color);
	Invalidate();
}

void CLarBtn::LarSetBtnIsStatic(bool IsStatic)
{
	m_IsStatic = IsStatic;
}

void CLarBtn::LaSetAngleRounded(double radius)
{
	m_IsAngleRounded = true;
	m_AngleRoundedRadius = radius * m_DpiScale;
	Invalidate();
}

void CLarBtn::LarSetAdaptDpiBitmaps(const std::vector<UINT> resourceIDs, CLarBtn::BitmapShowMode mode)
{
	//如果在这之前调用过LarSetButtonTransparentBitmap,则直接异常，中断退出
	if (!m_bitmapPath.IsEmpty()) {
		AfxMessageBox(L"对象调用过LarSetButtonTransparentBitmap，调用LarSetAdaptDpiBitmaps失败");
		return;
	}

	// 确保资源ID的数量为5
	if (resourceIDs.size() != 5) {
		AfxMessageBox(_T("LarSetAdaptDpiBitmaps 函数需要传入正好5个资源ID，分别对应32x32, 40x40, 48x48, 56x56, 64x64尺寸。"));
		return;
	}

	// 定义一个数组，按照 BitmapSize 的顺序对应资源ID
	BitmapSize sizes[5] = {
		BitmapSize::SIZE_32x32,
		BitmapSize::SIZE_40x40,
		BitmapSize::SIZE_48x48,
		BitmapSize::SIZE_56x56,
		BitmapSize::SIZE_64x64
	};

	// 遍历资源ID并加载位图
	for (size_t i = 0; i < 5; ++i) {
		UINT resID = resourceIDs[i];

		// 加载位图资源
		HBITMAP hBitmap = static_cast<HBITMAP>(::LoadImage(
			AfxGetInstanceHandle(),
			MAKEINTRESOURCE(resID),
			IMAGE_BITMAP,
			0, 0,
			LR_CREATEDIBSECTION | LR_DEFAULTSIZE
		));

		if (!hBitmap) {
			CString errorMsg;
			errorMsg.Format(_T("无法加载资源ID: %u"), resID);
			AfxMessageBox(errorMsg);
			continue;
		}

		// 使用智能指针管理 Gdiplus::Bitmap 对象
		std::unique_ptr<Gdiplus::Bitmap> bitmap(new Gdiplus::Bitmap(hBitmap, NULL));

		// 查位图是否加载成功
		if (bitmap->GetLastStatus() != Gdiplus::Ok) {
			CString errorMsg;
			errorMsg.Format(_T("位图加载失败，资源ID: %u"), resID);
			AfxMessageBox(errorMsg);
			continue;
		}

		// 插入到 m_Bitmaps 映射中
		m_Bitmaps[sizes[i]] = std::move(bitmap);
	}
	m_IsBitmapsEmpty = false;
	if (m_DpiScale == 1.0) {
		EraseAndFitBitmapWithBkBitmap(m_Bitmaps[sizes[0]].get(), mode);
	}
	else if (m_DpiScale == 1.25) {
		EraseAndFitBitmapWithBkBitmap(m_Bitmaps[sizes[1]].get(), mode);
	}
	else if (m_DpiScale == 1.5) {
		EraseAndFitBitmapWithBkBitmap(m_Bitmaps[sizes[2]].get(), mode);
	}
	else if (m_DpiScale == 1.75) {
		EraseAndFitBitmapWithBkBitmap(m_Bitmaps[sizes[3]].get(), mode);
	}
	else if (m_DpiScale == 2.00) {
		EraseAndFitBitmapWithBkBitmap(m_Bitmaps[sizes[4]].get(), mode);
	}
	Invalidate();
}

void CLarBtn::EraseAndFitBitmapWithBkBitmap(Gdiplus::Bitmap* FittedBitmap, CLarBtn::BitmapShowMode mode)
{
	// 加载新位图
	Bitmap* newBitmap = FittedBitmap; // 第二个参数为是否使用透明背景
	// 检查新位图是否加载成功
	if (newBitmap == nullptr || newBitmap->GetLastStatus() != Ok)
	{
		AfxMessageBox(_T("无法加载指定路径的位图。"));
		if (newBitmap)
			delete newBitmap;
		return;
	}
	m_showMode = mode;
	//显示模式为显示设置的位图所有
	if (BitmapShowMode::SHOW_ORIGINAL == mode)
	{
		m_BkBitmap = newBitmap;
		return;
	}

	// 获取父窗口
	CWnd* pParent = GetParent();
	if (!pParent)
		return;

	// 获取按钮相对于父窗口的位置
	CRect btnRect;
	GetWindowRect(&btnRect);
	pParent->ScreenToClient(&btnRect);

	// 赋值给 bitmapRect
	bitmapRect = btnRect;

	// 从父窗口捕获背景
	CDC* pParentDC = pParent->GetDC();
	if (!pParentDC)
		return;

	// 创建兼容位图
	CDC memDC;
	memDC.CreateCompatibleDC(pParentDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pParentDC, bitmapRect.right - bitmapRect.left, bitmapRect.bottom - bitmapRect.top);
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	// 复制背景
	memDC.BitBlt(0, 0, bitmapRect.right - bitmapRect.left, bitmapRect.bottom - bitmapRect.top, pParentDC, bitmapRect.left, bitmapRect.top, SRCCOPY);

	// 将 HBITMAP 转换为 GDI+ Bitmap
	if (m_BkBitmap)
	{
		delete m_BkBitmap;
		m_BkBitmap = nullptr;
	}
	m_BkBitmap = Gdiplus::Bitmap::FromHBITMAP((HBITMAP)bitmap, NULL);
	// 清理
	memDC.SelectObject(&bitmap);
	pParent->ReleaseDC(pParentDC);

	////进行位图合成
	// 遍历每个像素进行合成
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"m_BkBitmap(Width):%d,m_BkBitmap(Height):%d\n",
		m_BkBitmap->GetHeight(), m_BkBitmap->GetWidth());
	for (UINT y = 0; y < m_BkBitmap->GetHeight(); ++y)
	{
		for (UINT x = 0; x < m_BkBitmap->GetWidth(); ++x)
		{
			Color newColor;
			newBitmap->GetPixel(x, y, &newColor);

			// 根据 mode 值决定合成规则
			if (mode == BitmapShowMode::SHOW_TRANSPARENT_BYERASEBLACK)
			{
				// 如果新位图的像素颜色不为黑色（0,0,0）
				if (newColor.GetRed() != 0 || newColor.GetGreen() != 0 || newColor.GetBlue() != 0)
				{
					// 覆盖 m_BkBitmap 的对应像素
					m_BkBitmap->SetPixel(x, y, newColor);
				}
			}
			else if (mode == BitmapShowMode::SHOW_TRANSPARENT_BYERASEWHITE)
			{
				// 如果新位图的像素颜色不为白色（255,255,255）
				if (newColor.GetRed() != 255 || newColor.GetGreen() != 255 || newColor.GetBlue() != 255)
				{
					// 覆盖 m_BkBitmap 的对应像素
					m_BkBitmap->SetPixel(x, y, newColor);
				}
			}
		}
	}
}
// 辅助函数：创建圆角矩形路径并填充到传入的 GraphicsPath 引用中
void CLarBtn::CreateRoundedRectanglePath(GraphicsPath& path, const CRect& rect, int radius)
{
	path.StartFigure();
	// 左上角
	path.AddArc(rect.left + 4 * m_DpiScale, rect.top + 4 * m_DpiScale, radius, radius, 180, 90);
	// 右上角
	path.AddArc(rect.right - radius - 4 * m_DpiScale, rect.top + 4 * m_DpiScale, radius, radius, 270, 90);
	// 右下角
	path.AddArc(rect.right - radius - 4 * m_DpiScale, rect.bottom - radius - 4 * m_DpiScale, radius, radius, 0, 90);
	// 左下角
	path.AddArc(rect.left + 4 * m_DpiScale, rect.bottom - radius - 4 * m_DpiScale, radius, radius, 90, 90);
	path.CloseFigure();
}

void CLarBtn::LarSetButtonTransparentBitmapFromResource(UINT nResourceID, int mode)
{
	if (!m_IsBitmapsEmpty)
	{
		AfxMessageBox(_T("对象调用过LarSetAdaptDpiBitmaps设置过位图，LarSetButtonTransparentBitmap调用失败"));
		return;
	}

	// 检查 m_BkBitmap 是否已绑定位图
	if (!m_BkBitmap || m_BkBitmap->GetWidth() == 0 || m_BkBitmap->GetHeight() == 0)
	{
		// 输出错误信息，这里使用调试输出代替AfxMessageBox
		DEBUG_CONSOLE_STR(ConsoleHandle, L"调用对象没有调用SetParentBk设置背景位图,设置全透明位图失败");
		// 从资源中加载位图
		m_BkBitmap = LoadBitmapFromResource(nResourceID);
		m_nResourceID = nResourceID;
		m_mode = mode;
		return;
	}

	// 从资源中加载新位图
	HBITMAP resBitmap = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(nResourceID));
	Bitmap* newbitmap = Bitmap::FromHBITMAP(resBitmap, nullptr);
	m_nResourceID = nResourceID;
	m_mode = mode;
	// 检查新位图是否加载成功
	if (newbitmap == nullptr)
	{
		AfxMessageBox(_T("无法加载指定资源的位图。"));
		return;
	}

	// 遍历每个像素进行合成
	for (UINT y = 0; y < m_BkBitmap->GetHeight(); ++y)
	{
		for (UINT x = 0; x < m_BkBitmap->GetWidth(); ++x)
		{
			Color newColor;
			newbitmap->GetPixel(x, y, &newColor);

			// 根据 mode 值决定合成规则
			if (m_mode == 0)
			{
				// 如果新位图的像素颜色不为黑色（0,0,0）
				if (newColor.GetRed() != 0 || newColor.GetGreen() != 0 || newColor.GetBlue() != 0)
				{
					// 覆盖 m_BkBitmap 的对应像素
					m_BkBitmap->SetPixel(x, y, newColor);
				}
			}
			else
			{
				// 如果新位图的像素颜色不为白色（255,255,255）
				if (newColor.GetRed() != 255 || newColor.GetGreen() != 255 || newColor.GetBlue() != 255)
				{
					// 覆盖 m_BkBitmap 的对应像素
					m_BkBitmap->SetPixel(x, y, newColor);
				}
			}
		}
	}

	// 释放新位图资源
	delete newbitmap;

	// 触发按钮repaint以显示更新后的位图
	Invalidate();
}

Bitmap* CLarBtn::LoadBitmapFromResource(UINT nResourceID)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	// 查找资源
	HRSRC hRes = ::FindResource(hInst, MAKEINTRESOURCE(nResourceID), RT_BITMAP);
	if (hRes == nullptr)
	{
		AfxMessageBox(_T("无法找到资源"));
		return nullptr;
	}
	// 加载资源
	HGLOBAL hGlobal = ::LoadResource(hInst, hRes);
	if (hGlobal == nullptr)
	{
		AfxMessageBox(_T("加载资源失败"));
		return nullptr;
	}
	// 获取资源数据指针和大小
	LPVOID pResourceData = ::LockResource(hGlobal);
	DWORD dwSize = ::SizeofResource(hInst, hRes);

	// 分配内存并复制资源数据，以用于创建IStream
	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, dwSize);
	if (hBuffer == nullptr)
		return nullptr;
	void* pBuffer = GlobalLock(hBuffer);
	memcpy(pBuffer, pResourceData, dwSize);
	GlobalUnlock(hBuffer);

	// 创建IStream对象
	IStream* pStream = nullptr;
	if (FAILED(CreateStreamOnHGlobal(hBuffer, TRUE, &pStream)))
	{
		GlobalFree(hBuffer);
		return nullptr;
	}

	// 通过流加载位图
	Bitmap* pBitmap = Bitmap::FromStream(pStream, FALSE);
	pStream->Release();
	// hBuffer 内存将在IStream释放时自动释放

	return pBitmap;
}

Color CLarBtn::ApplyOpacity(const Color& c) const
{
	// 以原有Alpha为基准，乘以按钮当前不透明度比例
	float k = static_cast<float>(m_OpacityAlpha) / 255.0f;
	BYTE a = static_cast<BYTE>(max(0, min(255, static_cast<int>(c.GetAlpha() * k))));
	return Color(a, c.GetR(), c.GetG(), c.GetB());
}

void CLarBtn::LarSetParentBk()
{
	if (m_IsSetParentBkCalled)
	{
		return;
	}
	m_IsSetParentBkCalled = true;
	DEBUG_CONSOLE_STR(ConsoleHandle, L"设置控件背景为父窗口背景");
	// 获取父窗口
	CWnd* pParent = GetParent();
	if (!pParent)
		return;

	// 获取按钮相对于父窗口的位置
	CRect btnRect;
	GetWindowRect(&btnRect);
	pParent->ScreenToClient(&btnRect);

	// 赋值给 bitmapRect
	bitmapRect = btnRect;

	// 从父窗口捕获背景
	CDC* pParentDC = pParent->GetDC();
	if (!pParentDC)
		return;

	// 创建兼容位图
	CDC memDC;
	memDC.CreateCompatibleDC(pParentDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pParentDC, bitmapRect.right - bitmapRect.left, bitmapRect.bottom - bitmapRect.top);
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	// 复制背景
	memDC.BitBlt(0, 0, bitmapRect.right - bitmapRect.left, bitmapRect.bottom - bitmapRect.top, pParentDC, bitmapRect.left, bitmapRect.top, SRCCOPY);

	// 将 HBITMAP 转换为 GDI+ Bitmap
	if (m_BkBitmap)
	{
		delete m_BkBitmap;
		m_BkBitmap = nullptr;
	}
	m_BkBitmap = Gdiplus::Bitmap::FromHBITMAP((HBITMAP)bitmap, NULL);

	//如果在调用LarSetParentBk之前有调用过LarSetButtonTransparentBitmap
	if (m_nResourceID != 0) {
		// 清理
		memDC.SelectObject(&bitmap);
		pParent->ReleaseDC(pParentDC);
		LarSetButtonTransparentBitmapFromResource(m_nResourceID, m_mode);
		return;
	}

	//如果在调用LarSetParentBk之前有调用过LarSetAdaptDpiBitmaps
	if (!m_IsBitmapsEmpty) {
		DEBUG_CONSOLE_STR(ConsoleHandle, L"Setparent函数开始用背景位图合体LarSetAdaptDpiBitmaps设置的位图");
		// 清理
		memDC.SelectObject(&bitmap);
		pParent->ReleaseDC(pParentDC);
		// 定义一个数组，按照 BitmapSize 的顺序对应资源ID
		BitmapSize sizes[5] = {
			BitmapSize::SIZE_32x32,
			BitmapSize::SIZE_40x40,
			BitmapSize::SIZE_48x48,
			BitmapSize::SIZE_56x56,
			BitmapSize::SIZE_64x64
		};
		if (m_DpiScale == 1.0) {
			EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(m_Bitmaps[sizes[0]].get());
		}
		else if (m_DpiScale == 1.25) {
			EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(m_Bitmaps[sizes[1]].get());
		}
		else if (m_DpiScale == 1.5) {
			EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(m_Bitmaps[sizes[2]].get());
		}
		else if (m_DpiScale == 1.75) {
			EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(m_Bitmaps[sizes[3]].get());
		}
		else if (m_DpiScale == 2.00) {
			EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(m_Bitmaps[sizes[4]].get());
		}
		Invalidate();
		return;
	}
	// 清理
	memDC.SelectObject(&bitmap);
	pParent->ReleaseDC(pParentDC);

	// 触发repaint
	Invalidate();
}

void CLarBtn::EraseAndFitBitmapWithBkBitmapCallInLarSetParentBk(Gdiplus::Bitmap* FittedBitmap)
{
	// 加载新位图
	Bitmap* newBitmap = FittedBitmap; // 第二个参数为是否使用透明背景
	// 检查新位图是否加载成功
	if (newBitmap == nullptr || newBitmap->GetLastStatus() != Ok)
	{
		AfxMessageBox(_T("无法加载指定路径的位图。"));
		if (newBitmap)
			delete newBitmap;
		return;
	}

	////进行位图合成
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"m_BkBitmap(Width):%d,m_BkBitmap(Height):%d\n",
		m_BkBitmap->GetHeight(), m_BkBitmap->GetWidth());
	// 遍历每个像素进行合成
	for (UINT y = 0; y < m_BkBitmap->GetHeight(); ++y)
	{
		for (UINT x = 0; x < m_BkBitmap->GetWidth(); ++x)
		{
			Color newColor;
			newBitmap->GetPixel(x, y, &newColor);

			// 根据 mode 值决定合成规则
			if (m_showMode == BitmapShowMode::SHOW_TRANSPARENT_BYERASEBLACK)
			{
				// 如果新位图的像素颜色不为黑色（0,0,0）
				if (newColor.GetRed() != 0 || newColor.GetGreen() != 0 || newColor.GetBlue() != 0)
				{
					// 覆盖 m_BkBitmap 的对应像素
					m_BkBitmap->SetPixel(x, y, newColor);
				}
			}
			else if (m_showMode == BitmapShowMode::SHOW_TRANSPARENT_BYERASEWHITE)
			{
				// 如果新位图的像素颜色不为白色（255,255,255）
				if (newColor.GetRed() != 255 || newColor.GetGreen() != 255 || newColor.GetBlue() != 255)
				{
					// 覆盖 m_BkBitmap 的对应像素
					m_BkBitmap->SetPixel(x, y, newColor);
				}
			}
		}
	}
}

void CLarBtn::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_IsStatic)return;
	if (!m_bIsHovered)
	{
		m_bIsHovered = true;
		Invalidate(); // repaint以显示悬停状态

		// 开始跟踪鼠标离开事件
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = 0;
		_TrackMouseEvent(&tme);
	}
	//if (m_IsNoInteraction) {
	//	::SetCursor(::LoadCursor(NULL, IDC_HAND));
	//	return;
	//}

	CButton::OnMouseMove(nFlags, point);
}

LRESULT CLarBtn::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	if (m_IsStatic)return 0;
	if (m_bIsHovered)
	{
		m_bIsHovered = false;
		Invalidate(); // repaint以去除悬停状态
	}
	return 0;
}

void CLarBtn::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_IsStatic)return;
	m_bIsPressed = true;
	Invalidate(); // repaint以显示按下状态

	CButton::OnLButtonDown(nFlags, point);
}

void CLarBtn::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_IsStatic)return;
	if (m_bIsPressed)
	{
		m_bIsPressed = false;
		Invalidate(); // repaint以去除按下状态
	}

	CButton::OnLButtonUp(nFlags, point);
}

BOOL CLarBtn::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest == HTCLIENT)
	{
		// If mouse is currently tracking (inside the client area), show hand cursor; otherwise show default
		if (m_bIsHovered)
			::SetCursor(LoadCursor(NULL, IDC_HAND));
		else
			::SetCursor(LoadCursor(NULL, IDC_ARROW));

		return TRUE; // indicate that we have set the cursor
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}
