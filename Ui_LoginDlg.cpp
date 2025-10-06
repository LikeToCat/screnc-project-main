// Ui_LoginDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_LoginDlg.h"
#include "afxdialogex.h"
#include "theApp.h"
#include "GlobalFunc.h"
#include "LarStringConversion.h"
#include "ConfigFileHandler.h"
#include "PngLoader.h"
extern theApp App;
// Ui_LoginDlg 对话框
static size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	if (userp == nullptr) return 0;

	size_t realsize = size * nmemb;
	std::string* str = (std::string*)userp;
	str->append(contents, realsize);
	return realsize;
}
IMPLEMENT_DYNAMIC(Ui_LoginDlg, CDialogEx)

Ui_LoginDlg::Ui_LoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LOGINPAGE, pParent)
{
	m_nCornerRadius = 32;
	m_bool_IsAutoLoginChecked = true;
	//编辑框偏移系数
	m_diffX = 9 * m_Scale;
	m_diffY = 9 * m_Scale;
}

Ui_LoginDlg::~Ui_LoginDlg()
{
}

void Ui_LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, LOGINDLG_BTN_LOGO, m_Btn_Logo);
	DDX_Control(pDX, LOGINDLG_STAT_LOGOTEXT, m_Stat_LogoText);
	DDX_Control(pDX, LOGINDLG_EDIT_PHONENUM, m_Edit_PhoneNum);
	DDX_Control(pDX, LOGINDLG_BTN_SEEDVERITYCODE, m_Btn_SendVerityCode);
	DDX_Control(pDX, LOGINDLG_EDIT_VERITYCODE, m_Edit_VerityCode);
	DDX_Control(pDX, LOGINDLG_BTN_LOGIN, m_Btn_Login);
	DDX_Control(pDX, LOGODLG_STAT_LOGINAUTO, m_Stat_AutoLogin);
	DDX_Control(pDX, LOGINDLG_STAT_LOGIN, m_Stat_PhoneNum);
	DDX_Control(pDX, LOGINDLG_STAT_VERITYCODE, m_Stat_VerityCode);
	DDX_Control(pDX, LOGINDLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, LOGINDLG_BTN_LOGINBYPHONE, m_Btn_LoginByPhone);
	DDX_Control(pDX, LOGONDLG_BTN_FEEDBACK, m_Btn_FeedBack);
}

void Ui_LoginDlg::Ui_SetWindowRect(const CRect& rect)
{
	m_CRect_WindowRect.left = rect.left;
	m_CRect_WindowRect.right = rect.right;
	m_CRect_WindowRect.top = rect.top;
	m_CRect_WindowRect.bottom = rect.bottom;

	m_Rect_WindowRect.Width = rect.Width();
	m_Rect_WindowRect.Height = rect.Height();
	m_Rect_WindowRect.X = 0;
	m_Rect_WindowRect.Y = 0;

	m_WindowWidth = m_CRect_WindowRect.Width();
	m_WindowHeight = m_CRect_WindowRect.Height();
}

void Ui_LoginDlg::Ui_UpdateWindowPos(const CRect& rect)
{
	m_CRect_WindowRect.left = rect.left;
	m_CRect_WindowRect.right = rect.right;
	m_CRect_WindowRect.top = rect.top;
	m_CRect_WindowRect.bottom = rect.bottom;

	m_Rect_WindowRect.Width = rect.Width();
	m_Rect_WindowRect.Height = rect.Height();
	m_Rect_WindowRect.X = 0;
	m_Rect_WindowRect.Y = 0;

	m_WindowWidth = m_CRect_WindowRect.Width();
	m_WindowHeight = m_CRect_WindowRect.Height();
	MoveWindow(m_CRect_WindowRect);
}

void Ui_LoginDlg::CleanUpGdiPngRes()
{
	m_Btn_Logo.ClearImages();
}

void Ui_LoginDlg::GetUserDPI()
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
}

void Ui_LoginDlg::UpdateScale()
{
	//编辑框偏移系数
	m_diffX = 9 * m_Scale;
	m_diffY = 9 * m_Scale;

	//关闭按钮
	float CloseBtnWidth = 18 * m_Scale;
	float CloseBtnHeight = 18 * m_Scale;
	float CloseBtnX = m_WindowWidth - CloseBtnWidth - 10 * m_Scale;
	float CloseBtnY = 10 * m_Scale;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	//logo图 60 50 
	float LogoBtnWidth = 24 * m_Scale;
	float LogoBtnHeight = 22 * m_Scale;
	float LogoBtnX = 28.5 * m_Scale;
	float LogoBtnY = 32.9 * m_Scale;
	m_Btn_Logo.MoveWindow(LogoBtnX, LogoBtnY, LogoBtnWidth, LogoBtnHeight);

	//手机号登录按钮95 30 
	float LoginByPhoneWidth = 90 * m_Scale;
	float LoginByPhoneHeight = 20 * m_Scale;
	float LoginByPhoneX = (m_WindowWidth - LoginByPhoneWidth) / 2;
	float LoginByPhoneY = LogoBtnY + 65 * m_Scale;
	m_Btn_LoginByPhone.MoveWindow(LoginByPhoneX, LoginByPhoneY, LoginByPhoneWidth, LoginByPhoneHeight);

	//logo文本 250 50
	float LogoTextWidth = 182 * m_Scale;
	float LogoTextHeight = 28 * m_Scale;
	float LogoTextX = LogoBtnX + LogoBtnWidth - 18 * m_Scale;
	float LogoTextY = LogoBtnY + (LogoBtnHeight - LogoTextHeight) / 2 - 1 * m_Scale;
	m_Stat_LogoText.MoveWindow(LogoTextX, LogoTextY, LogoTextWidth, LogoTextHeight);

	//电话号码提示文本
	float PhoneNumTextWidth = 100 * m_Scale;
	float PhoneNumTextHeight = 25 * m_Scale;
	float PhoneNumTextX = 0.095 * m_WindowWidth;
	float PhoneNumTextY = 0.37 * m_WindowHeight;
	m_Stat_PhoneNum.MoveWindow(PhoneNumTextX, PhoneNumTextY, PhoneNumTextWidth, PhoneNumTextHeight);

	//电话号码编辑框376 36
	float PhoneNumEditWidth = 350 * m_Scale;
	float PhoneNumEditHeight = 24 * m_Scale;
	float PhoneNumEditX = (m_WindowWidth - PhoneNumEditWidth) / 2;
	float PhoneNumEditY = 111 * m_Scale;
	m_Edit_PhoneNum.MoveWindow(PhoneNumEditX, PhoneNumEditY, PhoneNumEditWidth, PhoneNumEditHeight);

	//验证码提示文本
	float VerityCodeTextWidth = 100 * m_Scale;
	float VerityCodeTextHeight = 25 * m_Scale;
	float VerityCodeTextX = PhoneNumTextX - 6 * m_Scale;
	float VerityCodeTextY = PhoneNumEditY + PhoneNumEditHeight;
	m_Stat_VerityCode.MoveWindow(VerityCodeTextX, VerityCodeTextY, VerityCodeTextWidth, VerityCodeTextHeight);

	//验证码编辑框
	float VerityBtnWidth = 0.744 * PhoneNumEditWidth;
	float VerityBtnHeight = 24 * m_Scale ;
	float VerityBtnX = PhoneNumEditX;
	float VerityBtnY = PhoneNumEditY + PhoneNumEditHeight + 46 * m_Scale;
	m_Edit_VerityCode.MoveWindow(VerityBtnX, VerityBtnY, VerityBtnWidth, VerityBtnHeight);

	//发送验证码按钮
	float SendVerityBtnWidth = 80 * m_Scale;
	float SendVerityBtnHeight = 34 * m_Scale + m_diffY;
	float SendVerityBtnX = PhoneNumEditX + PhoneNumEditWidth - SendVerityBtnWidth + m_diffY;
	float SendVerityBtnY = VerityBtnY + (VerityBtnHeight - SendVerityBtnHeight) / 2;
	m_Btn_SendVerityCode.MoveWindow(SendVerityBtnX, SendVerityBtnY, SendVerityBtnWidth, SendVerityBtnHeight);

	//登录按钮
	float LoginBtnWidth = PhoneNumEditWidth;
	float LoginBtnHeight = 30 * m_Scale;
	float LoginBtnX = (m_WindowWidth - LoginBtnWidth) / 2;
	float LoginBtnY = 299 * m_Scale;
	CRect BtnLoginRect;
	BtnLoginRect.SetRect(LoginBtnX, LoginBtnY, LoginBtnX + LoginBtnWidth, LoginBtnY + LoginBtnHeight);
	BtnLoginRect.InflateRect(m_diffX * 1.25, m_diffX);
	m_Btn_Login.MoveWindow(BtnLoginRect);

	//复选框
	float CkBoxWidth = 18 * m_Scale;
	float CkBoxHeight = 18 * m_Scale;
	m_CRect_CkBox.left = VerityBtnX;
	m_CRect_CkBox.top = LoginBtnY + LoginBtnHeight + 38 * m_Scale;
	m_CRect_CkBox.right = m_CRect_CkBox.left + CkBoxWidth;
	m_CRect_CkBox.bottom = m_CRect_CkBox.top + CkBoxHeight;

	//反馈 65 32 
	float FeedBackWidth = 43 * m_Scale;
	float FeedBackHeight = 21 * m_Scale;
	float FeedBackX = BtnLoginRect.left + BtnLoginRect.Width() - FeedBackWidth;
	float FeedBackY = LoginBtnY + LoginBtnHeight + 35 * m_Scale;
	m_Btn_FeedBack.MoveWindow(FeedBackX, FeedBackY, FeedBackWidth, FeedBackHeight);

	//自动登录95 35
	float AutoLoginWidth = 83 * m_Scale;
	float AutoLoginHeight = 20 * m_Scale;
	float AutoLoginX = m_CRect_CkBox.left + CkBoxWidth - 5 * m_Scale;
	float AutoLoginY = m_CRect_CkBox.top + (CkBoxHeight - AutoLoginHeight) / 2;
	m_Stat_AutoLogin.MoveWindow(AutoLoginX, AutoLoginY, AutoLoginWidth, AutoLoginHeight);
}

void Ui_LoginDlg::InitCtrl()
{
	//logo图
	m_Btn_Logo.LoadPNG(LOGINDLG_PNG_APPLOGO);
	m_Btn_Logo.LoadClickPNG(LOGINDLG_PNG_APPLOGO);
	m_Btn_Logo.SetHighQualityPNG(TRUE);
	m_Btn_Logo.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_Close.SetHoverEffectColor(20, 255, 255, 255);
	m_Btn_Close.SetStretchMode(0.75f);

	//文字logo
	m_Stat_LogoText.LarSetText(L"极速录屏大师");
	m_Stat_LogoText.LarSetTextSize(27);

	m_Stat_PhoneNum.LarSetTextSize(18);
	m_Stat_PhoneNum.ShowWindow(SW_HIDE);
	m_Stat_VerityCode.LarSetTextSize(18);
	m_Stat_VerityCode.ShowWindow(SW_HIDE);

	//自动登录
	m_Stat_AutoLogin.LarSetText(L"自动登录");
	m_Stat_AutoLogin.LarSetTextSize(20);
	m_Stat_AutoLogin.ShowWindow(SW_HIDE);

	//手机号码编辑框
	m_Edit_PhoneNum.SetBkColor(RGB(31, 36, 37));
	m_Edit_PhoneNum.SetTextColor(RGB(255, 255, 255));
	int TextSize = (float)15 * m_Scale;
	if (m_Scale == 2.25)
		TextSize += 2;
	m_Edit_PhoneNum.SetFontSize(TextSize);
	m_Edit_PhoneNum.SetPlaceholderText(L"手机号");
	m_Edit_PhoneNum.SetPlaceholderColor(RGB(73, 73, 73));

	//发送验证码
	m_Btn_SendVerityCode.LarSetTextSize(18);
	m_Btn_SendVerityCode.LaSetTextColor(Gdiplus::Color(155, 162, 162, 162));
	m_Btn_SendVerityCode.LaSetTextHoverColor(Gdiplus::Color(255, 172, 172, 172));
	m_Btn_SendVerityCode.LaSetTextClickedColor(Gdiplus::Color(255, 182, 182, 182));
	m_Btn_SendVerityCode.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
	m_Btn_SendVerityCode.LarSetEraseBkEnable(false);
	m_Btn_SendVerityCode.LarSetNormalFiilBrush(SolidBrush(Color(155, 62, 65, 82)));
	m_Btn_SendVerityCode.LarSetHoverFillBrush(SolidBrush(Color(255, 72, 75, 92)));
	m_Btn_SendVerityCode.LarSetClickedFillBrush(SolidBrush(Color(255, 82, 85, 102)));
	m_Btn_SendVerityCode.LarSetBtnEnable(false);

	//验证码编辑框
	m_Edit_VerityCode.SetBkColor(RGB(31, 36, 37));
	m_Edit_VerityCode.SetTextColor(RGB(255, 255, 255));
	m_Edit_VerityCode.SetFontSize(TextSize);
	m_Edit_VerityCode.SetPlaceholderText(L"验证码");
	m_Edit_VerityCode.SetPlaceholderColor(RGB(73, 73, 73));

	//登录按钮
	m_Btn_Login.LarSetTextSize(20);
	m_Btn_Login.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Login.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_Login.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Login.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
	m_Btn_Login.LarSetEraseBkEnable(false);
	m_Btn_Login.LarSetNormalFiilBrush(SolidBrush(Color(155, 20, 183, 195)));
	m_Btn_Login.LarSetHoverFillBrush(SolidBrush(Color(255, 30, 193, 205)));
	m_Btn_Login.LarSetClickedFillBrush(SolidBrush(Color(255, 40, 203, 215)));
	m_Btn_Login.LaSetAngleRounded(6 * m_Scale);
	m_Btn_Login.LarSetBtnEnable(false);

	//手机号登录
	m_Btn_LoginByPhone.LarSetTextSize(20);
	m_Btn_LoginByPhone.LaSetTextColor(Gdiplus::Color(255, 0, 139, 255));
	m_Btn_LoginByPhone.LaSetTextHoverColor(Gdiplus::Color(255, 0, 139, 255));
	m_Btn_LoginByPhone.LaSetTextClickedColor(Gdiplus::Color(255, 0, 139, 255));
	m_Btn_LoginByPhone.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
	m_Btn_LoginByPhone.LarSetEraseBkEnable(false);
	m_Btn_LoginByPhone.LarSetNormalFiilBrush(SolidBrush(Color(155, 26, 31, 37)));
	m_Btn_LoginByPhone.LarSetHoverFillBrush(SolidBrush(Color(255, 26, 31, 37)));
	m_Btn_LoginByPhone.LarSetClickedFillBrush(SolidBrush(Color(255, 26, 31, 37)));

	//反馈按钮
	m_Btn_FeedBack.LarSetTextSize(20);
	m_Btn_FeedBack.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_FeedBack.LaSetTextHoverColor(Gdiplus::Color(255, 0, 139, 255));
	m_Btn_FeedBack.LaSetTextClickedColor(Gdiplus::Color(255, 0, 139, 255));
	m_Btn_FeedBack.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
	m_Btn_FeedBack.LarSetEraseBkEnable(false);
	m_Btn_FeedBack.LarSetNormalFiilBrush(SolidBrush(Color(155, 26, 31, 37)));
	m_Btn_FeedBack.LarSetHoverFillBrush(SolidBrush(Color(255, 26, 31, 37)));
	m_Btn_FeedBack.LarSetClickedFillBrush(SolidBrush(Color(255, 26, 31, 37)));
	m_Btn_FeedBack.ShowWindow(SW_HIDE);
}

void Ui_LoginDlg::LoadRes()
{
	m_Bitmap_Gou = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(LOGINDLG_PNG_GOU),
		L"PNG"
	);
}

BEGIN_MESSAGE_MAP(Ui_LoginDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(LOGINDLG_BTN_SEEDVERITYCODE, &Ui_LoginDlg::OnBnClickedBtnSeedveritycode)
	ON_BN_CLICKED(LOGINDLG_BTN_LOGIN, &Ui_LoginDlg::OnBnClickedBtnLogin)
	ON_WM_TIMER()
	ON_BN_CLICKED(LOGINDLG_BTN_CLOSE, &Ui_LoginDlg::OnBnClickedBtnClose)
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_EN_CHANGE(LOGINDLG_EDIT_PHONENUM, &Ui_LoginDlg::OnEnChangeEditPhoneNum)
	ON_EN_CHANGE(LOGINDLG_EDIT_VERITYCODE, &Ui_LoginDlg::OnEnChangeEditVerityCode)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()
// Ui_LoginDlg 消息处理程序

BOOL Ui_LoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetUserDPI();
	UpdateScale();
	InitCtrl();

	MoveWindow(m_CRect_WindowRect);

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口圆角
	//ApplyRoundedRegion();

	//设置窗口阴影效果
	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
	m_Shadow.Create(m_hWnd);

	SetForegroundWindow();
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	return TRUE;
}

void Ui_LoginDlg::OnOK()
{
	//CDialogEx::OnOK();
}

void Ui_LoginDlg::OnCancel()
{
	//CDialogEx::OnCancel();
}

void Ui_LoginDlg::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(rect);

	// 创建内存DC和位图
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

	// 在内存DC上绘制
	memDC.FillSolidRect(rect, RGB(31, 36, 37));
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);// 将内存DC内容复制到屏幕

	//GDI对象
	Gdiplus::Graphics graphics(dc.GetSafeHdc());

	//边框画笔
	Color BorderColor(73, 73, 73);
	SolidBrush BorderBrush(BorderColor);
	int BorderSize = 2;
	Pen BorderPen(&BorderBrush, BorderSize);

	//手机号编辑边框绘制
	CRect PhoneEditRect;
	m_Edit_PhoneNum.GetWindowRect(PhoneEditRect);
	ScreenToClient(PhoneEditRect);
	PhoneEditRect.InflateRect(m_diffX, m_diffY);
	graphics.DrawRectangle(&BorderPen, Rect(PhoneEditRect.left, PhoneEditRect.top, PhoneEditRect.Width(), PhoneEditRect.Height()));

	//验证码编辑边框绘制
	CRect CodeEditRect;
	m_Edit_VerityCode.GetWindowRect(CodeEditRect);
	ScreenToClient(CodeEditRect);
	CodeEditRect.InflateRect(m_diffX, m_diffY);
	graphics.DrawRectangle(&BorderPen, Rect(CodeEditRect.left, CodeEditRect.top, CodeEditRect.Width(), CodeEditRect.Height()));

	//手机号登录底部蓝色红线
	//int LineSize = 2;
	//SolidBrush lineBrush(Color(0, 139, 255));
	//Pen LinePen(&lineBrush, LineSize);
	//CRect LoginByPhonetRect;
	//m_Btn_LoginByPhone.GetWindowRect(LoginByPhonetRect);
	//ScreenToClient(LoginByPhonetRect);
	//Point p1(LoginByPhonetRect.left + 5 * m_Scale, LoginByPhonetRect.bottom + LineSize);
	//Point p2(LoginByPhonetRect.right - 5 * m_Scale, LoginByPhonetRect.bottom + LineSize);
	//graphics.DrawLine(&LinePen, p1, p2);

	//DrawCheckBox(&graphics);	// 复选框绘制

	// 清理
	memDC.SelectObject(pOldBitmap);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
	m_Shadow.Show(m_hWnd);

	DB(ConsoleHandle, L"Ui_LoginDlg:repaint..");
}

BOOL Ui_LoginDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void Ui_LoginDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 检查是否点击了自动登录
	if (m_CRect_CkBox.PtInRect(point))
	{
		m_bool_IsAutoLoginChecked = !m_bool_IsAutoLoginChecked;
		Invalidate();
		UpdateWindow();
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_LoginDlg::DrawCheckBox(Gdiplus::Graphics* graphics)
{
	// 复选框外框颜色
	Gdiplus::Color borderColor(255, 188, 188, 188);
	int borderSize = 2;
	Gdiplus::SolidBrush brush(borderColor);
	Gdiplus::Pen pen(&brush, borderSize);

	// 圆角半径
	int radius = 4 * m_Scale;
	int diameter = radius * 2;

	// 矩形参数
	int x = m_CRect_CkBox.left;
	int y = m_CRect_CkBox.top;
	int w = m_CRect_CkBox.Width();
	int h = m_CRect_CkBox.Height();

	// 保存原始抗锯齿状态
	Gdiplus::SmoothingMode oldMode = graphics->GetSmoothingMode();
	// 启用抗锯齿以获得更平滑的边缘
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	// 构造一个圆角矩形路径
	Gdiplus::GraphicsPath path;
	path.AddArc(x, y, diameter, diameter, 180, 90);                  // 左上角
	path.AddLine(x + radius, y, x + w - radius, y);                  // 顶边，修正了这里的width为w
	path.AddArc(x + w - diameter, y, diameter, diameter, 270, 90);   // 右上角
	path.AddLine(x + w, y + radius, x + w, y + h - radius);          // 右边
	path.AddArc(x + w - diameter, y + h - diameter, diameter, diameter, 0, 90);   // 右下角
	path.AddLine(x + w - radius, y + h, x + radius, y + h);          // 底边
	path.AddArc(x, y + h - diameter, diameter, diameter, 90, 90);    // 左下角
	path.AddLine(x, y + h - radius, x, y + radius);                  // 左边
	path.CloseFigure();

	// 绘制圆角边框
	graphics->DrawPath(&pen, &path);

	if (m_bool_IsAutoLoginChecked)
	{
		// 绘制对勾
		Gdiplus::Color checkColor(255, 255, 255, 255);
		Gdiplus::Pen checkPen(checkColor, 2 * m_Scale);

		// 计算对勾的关键点
		float padding = w * 0.2f;
		float midX = x + w / 2.0f;

		// 对勾的三个点：左下，中下，右上
		Gdiplus::PointF pt1(x + padding, y + h / 2.0f);
		Gdiplus::PointF pt2(midX - padding / 2, y + h - padding);
		Gdiplus::PointF pt3(x + w - padding, y + padding);

		// 绘制对勾
		graphics->DrawLine(&checkPen, pt1, pt2);
		graphics->DrawLine(&checkPen, pt2, pt3);
	}

	// 恢复原始抗锯齿状态
	graphics->SetSmoothingMode(oldMode);
}

HBRUSH Ui_LoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

bool Ui_LoginDlg::RequestLogin()
{
	std::string Url = "http://scrnrec.appapr.com/api/user/login-by-phone";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;
	CString PhoneNumber, VerityCode;
	this->m_Edit_PhoneNum.GetWindowTextW(PhoneNumber);
	this->m_Edit_VerityCode.GetWindowTextW(VerityCode);
	Root["phone_number"] = LARSC::CStringToStdString(PhoneNumber);
	Root["code"] = LARSC::CStringToStdString(VerityCode);

	requestHandler.AddHeader("x-api-token", App.m_appToken);
	requestHandler.AddJsonBody(Root);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			if (!Root["data"].isNull() && !Root["data"]["user"].isNull())
			{
				UpdateDeviceInfo(Root);
				auto pIns = DialogManager::GetInstance();
				pIns->BroadcastMessage(BROADCASTMSG_USERLOGIN_ISLOGININ, NULL, NULL);
			}
		}
		else
		{
			Ui_MessageModalDlg MessageDlg;
			MessageDlg.SetModal(L"极速录屏大师", L"请求被拒绝", LARSC::s2ws(Root["message"].asString()).c_str(), L"确认");
			MessageDlg.DoModal();
			return false;
		}
		if (App.m_IsHasOpenVip && (!App.m_IsNonUserPaid && !App.m_IsVip))
			App.m_IsNeedShowMouthBill = true;
		else
			App.m_IsNeedShowMouthBill = false;
		return true;
	}
	else
	{
		//在执行一次登录请求，解决可能存在的新用户第一次登录失败的问题
		{
			std::string Url = "http://scrnrec.appapr.com/api/user/login-by-phone";
			HttpRequestHandler requestHandler(
				HttpRequestHandler::RequestType::POST,
				HttpRequestHandler::ValueEncodeType::UTF8,
				Url
			);
			Json::Value Root;
			CString PhoneNumber, VerityCode;
			this->m_Edit_PhoneNum.GetWindowTextW(PhoneNumber);
			this->m_Edit_VerityCode.GetWindowTextW(VerityCode);
			Root["phone_number"] = LARSC::CStringToStdString(PhoneNumber);
			Root["code"] = LARSC::CStringToStdString(VerityCode);

			requestHandler.AddHeader("x-api-token", App.m_appToken);
			requestHandler.AddJsonBody(Root);
			if (requestHandler.PerformRequest())
			{
				Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
				if (!Root["success"].isNull() && Root["success"].asBool())
				{
					if (!Root["data"].isNull() && !Root["data"]["user"].isNull())
					{
						UpdateDeviceInfo(Root);
						auto pIns = DialogManager::GetInstance();
						pIns->BroadcastMessage(BROADCASTMSG_USERLOGIN_ISLOGININ, NULL, NULL);
					}
				}
				else
				{
					Ui_MessageModalDlg MessageDlg;
					MessageDlg.SetModal(L"极速录屏大师", L"请求被拒绝", LARSC::s2ws(Root["message"].asString()).c_str(), L"确认");
					MessageDlg.DoModal();
					return false;
				}
				if (App.m_IsHasOpenVip && (!App.m_IsNonUserPaid && !App.m_IsVip))
					App.m_IsNeedShowMouthBill = true;
				else
					App.m_IsNeedShowMouthBill = false;
				return true;
			}
			else
			{
				std::string errorMessage = requestHandler.GetErrorMessage();
				Ui_MessageModalDlg MessageDlg;
				MessageDlg.SetModal(L"极速录屏大师", L"Ops！出错了", L"连接不到服务器", L"确认");
				MessageDlg.DoModal();
				return false;
			}
		}
		return false;
	}
}

void Ui_LoginDlg::OnBnClickedBtnLogin()
{
	if (App.RequestSignIn())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"登录请求成功");
		ConfigFileHandler ConfigHandler(App.m_ConfigPath);
		if (!ConfigHandler.WriteConfigFile("AppConfig", "AutoLogin", m_bool_IsAutoLoginChecked ? "1" : "0"))
			DEBUG_CONSOLE_STR(ConsoleHandle, L"-----警告，写入自动登录到配置文件失败!");
		EndDialog(IDOK);
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"登录请求失败");
	}
}

void Ui_LoginDlg::OnBnClickedBtnSeedveritycode()
{
	bool retflag;
	IsPhoneInLegal(retflag);
	if (retflag) return;

	// 创建JSON请求体
	if (App.RequestSeedCode())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"发送验证码请求成功");
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"发送验证码请求失败");
	}
}

void Ui_LoginDlg::IsPhoneInLegal(bool& retflag)
{
	retflag = true;
	// 获取手机号
	CString phoneNumber;
	m_Edit_PhoneNum.GetWindowText(phoneNumber);

	// 验证手机号是否为11位数字
	if (phoneNumber.GetLength() != 11)
	{
		MessageBox(_T("请输入11位手机号码"), _T("提示"), MB_OK);
		return;
	}

	// 验证是否全部为数字
	for (int i = 0; i < phoneNumber.GetLength(); i++)
	{
		if (phoneNumber[i] < '0' || phoneNumber[i] > '9')
		{
			MessageBox(_T("手机号码只能包含数字"), _T("提示"), MB_OK);
			return;
		}
	}

	// 禁用发送按钮
	m_Btn_SendVerityCode.LarSetBtnEnable(false);

	// 设置定时器和初始按钮文字
	static int remainingSeconds = 60;
	remainingSeconds = 60;
	CString btnText;
	btnText.Format(_T("%d秒"), remainingSeconds);
	m_Btn_SendVerityCode.SetWindowText(btnText);
	SetTimer(1001, 1000, NULL);
	retflag = false;
}

void Ui_LoginDlg::ApplyRoundedRegion()
{
	// 计算整个窗口客户区和非客户区的总大小
	CRect rcWindow;
	GetWindowRect(&rcWindow);

	// CreateRoundRectRgn 的右/下参数需要 +1 才能完整覆盖
	HRGN hRgn = ::CreateRoundRectRgn(
		0, 0,
		rcWindow.Width() + 1,
		rcWindow.Height() + 1,
		m_nCornerRadius,
		m_nCornerRadius
	);
	// 应用到窗口
	SetWindowRgn(hRgn, TRUE);
}

void Ui_LoginDlg::SaveUserLoginInfo()
{
	// 获取应用程序可执行文件的路径
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 获取目录路径
	CString strPath(szPath);
	int pos = strPath.ReverseFind(_T('\\'));
	if (pos != -1)
		strPath = strPath.Left(pos + 1);

	// 配置文件完整路径
	CString configFilePath = strPath + _T("userconfig.ini");

	// 写入用户信息
	WritePrivateProfileString(_T("UserInfo"), _T("nickname"), App.m_userInfo.nickname, configFilePath);
	WritePrivateProfileString(_T("UserInfo"), _T("level"), App.m_userInfo.level, configFilePath);
	WritePrivateProfileString(_T("UserInfo"), _T("expires_at"), App.m_userInfo.expiresAt, configFilePath);

	// 保存自动登录设置
	WritePrivateProfileString(_T("Settings"), _T("AutoLogin"), _T("1"), configFilePath);

	// 可选：保存登录时间
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString loginTime;
	loginTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	WritePrivateProfileString(_T("UserInfo"), _T("last_login"), loginTime, configFilePath);
}

void Ui_LoginDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1001)
	{
		static int seconds = 60;
		seconds--;

		CString btnText;
		btnText.Format(_T("%d秒"), seconds);
		m_Btn_SendVerityCode.SetWindowText(btnText);

		if (seconds <= 0)
		{
			KillTimer(1001);
			m_Btn_SendVerityCode.SetWindowText(_T("发送验证码"));
			m_Btn_SendVerityCode.LarSetBtnEnable(true);
			seconds = 60;  // 重置倒计时
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void Ui_LoginDlg::OnBnClickedBtnClose()
{
	this->EndDialog(IDCANCEL);
	m_Shadow.Show(this->GetSafeHwnd());
}

void Ui_LoginDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	// m_Shadow.Show(m_hWnd);
	// TODO: 在此处添加消息处理程序代码
}

void Ui_LoginDlg::UpdateDeviceInfo(Json::Value& data_user_Root)
{
	UserInfo userInfo;
	Json::Value& userRoot = data_user_Root["data"]["user"];
	userInfo.currentBindings =
		!userRoot["current_bindings"].isNull() ? userRoot["current_bindings"].asInt() : -1;
	userInfo.isPaid =
		!userRoot["is_paid"].isNull() ? userRoot["is_paid"].asBool() : false;
	userInfo.nickname =
		!userRoot["nickname"].isNull() ? CString(userRoot["nickname"].asCString()) : _T("");
	userInfo.level =
		!userRoot["level"].isNull() ? CString(userRoot["level"].asCString()) : _T("");
	userInfo.expiresAt =
		!userRoot["expires_at"].isNull() ? CString(userRoot["expires_at"].asCString()) : _T("");
	userInfo.maxBindings =
		!userRoot["max_bindings"].isNull() ? userRoot["max_bindings"].asInt() : 0;

	DB(ConsoleHandle, L"开始更新用户信息成员");
	App.UpdateUserInfo(userInfo);
}

void Ui_LoginDlg::DenyMessageDlg(std::string DenyMessage)
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师", L"请求被拒绝", LARSC::s2ws(DenyMessage).c_str(), L"确认");
	MessageDlg.DoModal();
}

void Ui_LoginDlg::ErrorMessageDlg(HttpRequestHandler& requestHandler)
{
	std::string errorMessage = requestHandler.GetErrorMessage();
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师", L"Ops！出错了", LARSC::s2ws(errorMessage).c_str(), L"确认");
	MessageDlg.DoModal();
}

bool Ui_LoginDlg::RequestVerityCode()
{
	std::string Url = "http://scrnrec.appapr.com/api/user/send-code";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;
	CString PhoneNumber;
	this->m_Edit_PhoneNum.GetWindowTextW(PhoneNumber);
	Root["phone_number"] = LARSC::CStringToStdString(PhoneNumber);

	requestHandler.AddHeader("x-api-token", App.m_appToken);
	requestHandler.AddJsonBody(Root);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"验证码发送成功");
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
		}
		return true;
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return false;
	}
}

void Ui_LoginDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

void Ui_LoginDlg::OnEnChangeEditPhoneNum()
{
	// 处理m_Edit_PhoneNum编辑
	CString phoneNum;
	m_Edit_PhoneNum.GetWindowText(phoneNum); // 获取编辑框中的文本

	// 检查是否是11位数字
	bool isValid = true;
	if (phoneNum.GetLength() == 11) 
	{
		// 验证全部为数字
		for (int i = 0; i < 11; i++)
		{
			if (!isdigit(phoneNum[i]))
			{
				isValid = false;
				break;
			}
		}
		if (isValid) 
		{
			// 如果是11位数字，启用发送验证码按钮
			m_Btn_SendVerityCode.LarSetBtnEnable(true);
			m_Btn_SendVerityCode.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
			m_Btn_SendVerityCode.LaSetTextHoverColor(Gdiplus::Color(155, 245, 245, 245));
			m_Btn_SendVerityCode.LaSetTextClickedColor(Gdiplus::Color(155, 235, 235, 235));
			m_Btn_SendVerityCode.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
			m_Btn_SendVerityCode.LarSetNormalFiilBrush(SolidBrush(Color(255, 62, 65, 82)));
			m_Btn_SendVerityCode.LarSetHoverFillBrush(SolidBrush(Color(155, 72, 75, 92)));
			m_Btn_SendVerityCode.LarSetClickedFillBrush(SolidBrush(Color(155, 82, 85, 102)));

			// 立即repaint按钮
			m_Btn_SendVerityCode.InvalidateRect(NULL, FALSE);
			m_Btn_SendVerityCode.UpdateWindow();
		}
	}
	else 
	{
		// 如果不是11位数字，禁用发送验证码按钮
		m_Btn_SendVerityCode.LarSetBtnEnable(false);
		m_Btn_SendVerityCode.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
		m_Btn_SendVerityCode.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
		m_Btn_SendVerityCode.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
		m_Btn_SendVerityCode.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
		m_Btn_SendVerityCode.LarSetNormalFiilBrush(SolidBrush(Color(155, 62, 65, 82)));
		m_Btn_SendVerityCode.LarSetHoverFillBrush(SolidBrush(Color(255, 72, 75, 92)));
		m_Btn_SendVerityCode.LarSetClickedFillBrush(SolidBrush(Color(255, 82, 85, 102)));

		// 立即repaint按钮
		m_Btn_SendVerityCode.InvalidateRect(NULL, FALSE);
		m_Btn_SendVerityCode.UpdateWindow();
	}
}

void Ui_LoginDlg::OnEnChangeEditVerityCode()
{
	// m_Edit_VerityCode处理
	CString phoneNum;
	CString verityCode;
	m_Edit_PhoneNum.GetWindowText(phoneNum); // 获取手机号文本
	m_Edit_VerityCode.GetWindowText(verityCode); // 获取验证码文本

	// 检查手机号是否是11位数字
	bool isPhoneValid = true;
	if (phoneNum.GetLength() == 11)
	{
		for (int i = 0; i < 11; i++)
		{
			if (!isdigit(phoneNum[i]))
			{
				isPhoneValid = false;
				break;
			}
		}
	}
	else
	{
		isPhoneValid = false;
	}

	// 检查验证码是否是6位数字
	bool isCodeValid = true;
	if (verityCode.GetLength() == 6)
	{
		for (int i = 0; i < 6; i++)
		{
			if (!isdigit(verityCode[i]))
			{
				isCodeValid = false;
				break;
			}
		}
	}
	else
	{
		isCodeValid = false;
	}

	// 如果手机号和验证码都有效，启用登录按钮
	if (isPhoneValid && isCodeValid)
	{
		// 启用登录按钮并设置样式
		m_Btn_Login.LarSetBtnEnable(true);
		m_Btn_Login.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
		m_Btn_Login.LaSetTextHoverColor(Gdiplus::Color(155, 245, 245, 245));
		m_Btn_Login.LaSetTextClickedColor(Gdiplus::Color(155, 235, 235, 235));
		m_Btn_Login.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
		m_Btn_Login.LarSetNormalFiilBrush(SolidBrush(Color(255, 20, 183, 195)));
		m_Btn_Login.LarSetHoverFillBrush(SolidBrush(Color(155, 30, 193, 205)));
		m_Btn_Login.LarSetClickedFillBrush(SolidBrush(Color(155, 40, 203, 215)));

		// 立即repaint按钮
		m_Btn_Login.InvalidateRect(NULL, FALSE);
		m_Btn_Login.UpdateWindow();
	}
	else
	{
		// 禁用登录按钮并设置样式
		m_Btn_Login.LarSetBtnEnable(false);
		m_Btn_Login.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
		m_Btn_Login.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
		m_Btn_Login.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
		m_Btn_Login.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
		m_Btn_Login.LarSetNormalFiilBrush(SolidBrush(Color(155, 20, 183, 195)));
		m_Btn_Login.LarSetHoverFillBrush(SolidBrush(Color(255, 30, 193, 205)));
		m_Btn_Login.LarSetClickedFillBrush(SolidBrush(Color(255, 40, 203, 215)));

		// 立即repaint按钮
		m_Btn_Login.InvalidateRect(NULL, FALSE);
		m_Btn_Login.UpdateWindow();
	}
}



LRESULT Ui_LoginDlg::OnNcHitTest(CPoint point)
{
	// 将屏幕坐标转换为客户区坐标
	CPoint clientPoint = point;
	ScreenToClient(&clientPoint);

	// 检查是否在复选框区域内
	if (m_CRect_CkBox.PtInRect(clientPoint))
	{
		// 在复选框区域内，让基类处理
		return CDialogEx::OnNcHitTest(point);
	}

	// 检查是否在控件区域内
	CWnd* pWnd = ChildWindowFromPoint(clientPoint, CWP_ALL);
	if (pWnd != NULL && pWnd != this)
	{
		// 在控件区域内，让基类处理
		return CDialogEx::OnNcHitTest(point);
	}
	return HTCAPTION;
}
