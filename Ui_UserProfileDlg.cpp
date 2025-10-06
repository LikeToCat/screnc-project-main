// Ui_UserProfileDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_UserProfileDlg.h"
#include "afxdialogex.h"
#include "theApp.h"

// Ui_UserProfileDlg 对话框

IMPLEMENT_DYNAMIC(Ui_UserProfileDlg, CDialogEx)

Ui_UserProfileDlg::Ui_UserProfileDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_USERPROFILE, pParent)
{
	m_Scale = GetUserDPI();
	m_WindowWidth = 425 * m_Scale;
	if (App.m_IsVip && App.m_userInfo.level == L"永久会员")
		m_WindowHeight = 110 * m_Scale;
	else
		m_WindowHeight = 140 * m_Scale;
	if (App.m_IsVip && App.m_userInfo.level == L"永久会员")
		m_BoundaryLineY = m_WindowHeight / 3;
	else
		m_BoundaryLineY = m_WindowHeight / 3 - 15 * m_Scale;
	m_bool_isEnterIn = FALSE;
	m_bool_isInitlize = FALSE;
}

Ui_UserProfileDlg::~Ui_UserProfileDlg()
{
}

void Ui_UserProfileDlg::ShowAtXY(int x, int y)
{
	initCtrl();
	if (m_bool_isInitlize)
	{
		CRect rcrect;
		GetWindowRect(rcrect);
		rcrect.MoveToXY(x, y);
		MoveWindow(rcrect);
		ShowWindow(SW_SHOW);
		SetWindowPos(
			&wndTopMost,
			0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW
		);
		SetForegroundWindow();
		return;
	}
	else
	{
		DB(ConsoleHandle, L"错误！调用Ui_UserProfileDlg::ShowAtXY失败！Ui_UserProfileDlg并没有被创建并初始化!");
		return;
	}
}

void Ui_UserProfileDlg::UpdateUserInfo()
{
	initCtrl();
}

void Ui_UserProfileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, UPDLG_BTN_USERICON1, m_btn_UserIcon1);
	DDX_Control(pDX, UPDLG_BTN_USERICON2, m_btn_UserIcon2);
	DDX_Control(pDX, UPDLG_BTN_LOGINOUT, m_btn_logOut);
	DDX_Control(pDX, UPDLG_BTN_CONTACTSERVICE, m_btn_contactservice);
	DDX_Control(pDX, UPDLG_STAT_PHONENUM1, m_stat_phonenum1);
	DDX_Control(pDX, UPDLG_STAT_PHONENUM2, m_stat_phonenum2);
	DDX_Control(pDX, UPDLG_STAT_MEMBERINFO, m_stat_memberinfo);
	DDX_Control(pDX, UPDLG_STAT_MAXBIND, m_stat_maxbind);
	DDX_Control(pDX, UPDLG_STAT_EXPIRETIME, m_stat_expiretime);
	DDX_Control(pDX, UPDLG_BTN_OPENVIP, m_btn_openVip);
}

void Ui_UserProfileDlg::initCtrl()
{
	auto userinfo = App.getUserInfo();

	//顶部手机号
	m_stat_phonenum1.LarSetTextSize(18);
	m_stat_phonenum1.LarSetTextColor(RGB(255, 255, 255));
	m_stat_phonenum1.LarSetTextLeft();
	m_stat_phonenum1.LarSetText(userinfo.nickname);

	//中间手机号
	m_stat_phonenum2.LarSetTextSize(16);
	m_stat_phonenum2.LarSetTextColor(RGB(155, 155, 155));
	m_stat_phonenum2.LarSetTextLeft();
	m_stat_phonenum2.LarSetText(userinfo.nickname);

	//会员信息
	CString memberinfo;
	if (userinfo.isPaid)
		memberinfo = userinfo.level;
	else if(!App.m_IsVip && App.m_IsNonUserPaid)
		memberinfo = L"特权会员";
	else
		memberinfo = L"普通会员";
	m_stat_memberinfo.LarSetTextSize(20);
	m_stat_memberinfo.LarSetTextColor(RGB(255, 255, 255));
	m_stat_memberinfo.LarSetTextLeft();
	m_stat_memberinfo.LarSetText(memberinfo);

	// 可绑定数量
	CString mbtxt;
	mbtxt.Format(L"最大可绑定数量:%d", userinfo.maxBindings);
	m_stat_maxbind.LarSetTextSize(18);
	m_stat_maxbind.LarSetTextColor(RGB(255, 255, 255));
	m_stat_maxbind.LarSetTextRight();
	m_stat_maxbind.LarSetText(mbtxt);
	if (!App.m_IsVip && App.m_IsNonUserPaid)
		m_stat_maxbind.ShowWindow(SW_HIDE);

	// 到期时间
	CString exttxt;
	if (userinfo.isPaid)
		exttxt.Format(L"会员到期时间:%s", userinfo.expiresAt == "" ? L"永久VIP" : userinfo.expiresAt);
	else
		exttxt = L"会员到期时间:未开通";
	m_stat_expiretime.LarSetTextSize(18);
	m_stat_expiretime.LarSetTextColor(RGB(155, 155, 155));
	m_stat_expiretime.LarSetTextRight();
	m_stat_expiretime.LarSetText(exttxt);
	if (!App.m_IsVip && App.m_IsNonUserPaid)
		m_stat_expiretime.ShowWindow(SW_HIDE);

	//用户图标1
	m_btn_UserIcon1.LoadPNG(PROFILEDLG_PNG_USERICON1);
	m_btn_UserIcon1.LoadClickPNG(PROFILEDLG_PNG_USERICON1);
	m_btn_UserIcon1.SetBackgroundColor(RGB(36, 37, 40));

	//用户图标2
	m_btn_UserIcon2.LoadPNG(PROFILEDLG_PNG_USERICON1);
	m_btn_UserIcon2.LoadClickPNG(PROFILEDLG_PNG_USERICON1);
	m_btn_UserIcon2.SetBackgroundColor(RGB(36, 37, 40));

	//退出登录
	m_btn_logOut.LarSetTextSize(19);
	m_btn_logOut.LaSetTextColor(Color(200, 255, 255, 255));
	m_btn_logOut.LaSetTextHoverColor(Color(245, 255, 255, 255));
	m_btn_logOut.LaSetTextClickedColor(Color(255, 255, 255, 255));
	m_btn_logOut.LarSetNormalFiilBrush(SolidBrush(Color(36, 37, 40)));
	m_btn_logOut.LarSetHoverFillBrush(SolidBrush(Color(46, 47, 50)));
	m_btn_logOut.LarSetClickedFillBrush(SolidBrush(Color(56, 57, 50)));
	m_btn_logOut.LarSetBorderColor(Color(36, 37, 40));
	m_btn_logOut.LarSetBtnNailImage(
		PROFILEDLG_PNG_SIGNOUT, CLarBtn::NailImageLayout::Left,
		19 * m_Scale, 19 * m_Scale
	);
	m_btn_logOut.LarAdjustTextDisplayPos(5 * m_Scale, 0);

	//联系客服
	m_btn_contactservice.LarSetTextSize(19);
	m_btn_contactservice.LaSetTextColor(Color(200, 255, 255, 255));
	m_btn_contactservice.LaSetTextHoverColor(Color(245, 255, 255, 255));
	m_btn_contactservice.LaSetTextClickedColor(Color(255, 255, 255, 255));
	m_btn_contactservice.LarSetNormalFiilBrush(SolidBrush(Color(36, 37, 40)));
	m_btn_contactservice.LarSetHoverFillBrush(SolidBrush(Color(46, 47, 50)));
	m_btn_contactservice.LarSetClickedFillBrush(SolidBrush(Color(56, 57, 50)));
	m_btn_contactservice.LarSetBorderColor(Color(36, 37, 40));
	m_btn_contactservice.LarSetBtnNailImage(
		PROFILEDLG_PNG_CONTACT, CLarBtn::NailImageLayout::Left,
		19 * m_Scale, 19 * m_Scale
	);
	m_btn_contactservice.LarAdjustTextDisplayPos(5 * m_Scale, 0);

	//开通VIP
	m_btn_openVip.LarSetTextSize(23);
	m_btn_openVip.LaSetTextColor(Color(255, 63, 34, 1));
	m_btn_openVip.LaSetTextHoverColor(Color(245, 63, 34, 1));
	m_btn_openVip.LaSetTextClickedColor(Color(255, 63, 34, 1));
	m_btn_openVip.LarSetGradualColor(Color(255, 231, 211), Color(244, 186, 141), 
		Gdiplus::LinearGradientModeHorizontal);
	m_btn_openVip.LarSetHoverGradualColor(Color(255, 241, 221), Color(254, 196, 151),
		Gdiplus::LinearGradientModeHorizontal);
	m_btn_openVip.LaSetAngleRounded(5 * m_Scale);
	m_btn_openVip.LarSetBorderColor(Color(36, 37, 40));
	if (App.m_userInfo.level == L"永久会员")
	{
		m_btn_openVip.ShowWindow(SW_HIDE);
	}
	else if (App.m_IsVip || App.m_IsNonUserPaid && App.m_userInfo.level != L"永久会员")
	{
		m_btn_openVip.SetWindowTextW(L"升级会员");
		m_btn_openVip.ShowWindow(SW_SHOW);
	}
	else if (!App.m_IsVip && !App.m_IsNonUserPaid)
	{
		m_btn_openVip.SetWindowTextW(L"开启会员");
		m_btn_openVip.ShowWindow(SW_SHOW);
	}
}

void Ui_UserProfileDlg::UpdateScale()
{
	if (App.m_IsVip && App.m_userInfo.level == L"永久会员")
	{
		//顶部用户图标
		float u1icw = 21 * m_Scale;
		float u1ich = 21 * m_Scale;
		float u1icx = 10 * m_Scale;
		float u1icy = (m_BoundaryLineY - u1ich) / 2;
		m_btn_UserIcon1.MoveWindow(u1icx, u1icy, u1icw, u1ich);

		//顶部手机号
		float p1w = 90 * m_Scale;
		float plh = 25 * m_Scale;
		float p1x = u1icx + u1icw + 5 * m_Scale;
		float ply = u1icy + (u1ich - plh) / 2 + 4 * m_Scale;
		m_stat_phonenum1.MoveWindow(p1x, ply, p1w, plh);

		//联系客服
		float ctsw = 90 * m_Scale;
		float ctsh = 25 * m_Scale;
		float ctsx = m_WindowWidth - ctsw - 5 * m_Scale;
		float ctsy = (m_BoundaryLineY - ctsh) / 2;
		m_btn_contactservice.MoveWindow(ctsx, ctsy, ctsw, ctsh);

		//退出登录
		float logoutw = 90 * m_Scale;
		float logouth = 25 * m_Scale;
		float logoutx = ctsx - logoutw - 5 * m_Scale;
		float logouty = (m_BoundaryLineY - logouth) / 2;
		m_btn_logOut.MoveWindow(logoutx, logouty, logoutw, logouth);

		//中间用户图标
		float u2icw = 54 * m_Scale;
		float u2ich = 54 * m_Scale;
		float u2icx = 10 * m_Scale;
		float u2icy = m_BoundaryLineY + (m_WindowHeight - m_BoundaryLineY - u2ich) / 2 - 20 * m_Scale;
		if (App.m_userInfo.level == L"永久会员")
			u2icy += 18 * m_Scale;
		m_btn_UserIcon2.MoveWindow(u2icx, u2icy, u2icw, u2ich);

		//中间手机号
		float p2w = 160 * m_Scale;
		float p2h = 35 * m_Scale;
		float p2x = u2icx + u2icw + 5 * m_Scale;
		float p2y = u2icy + (u2ich - p2h) / 2 + 20 * m_Scale;
		m_stat_phonenum2.MoveWindow(p2x, p2y, p2w, p2h);

		//会员信息
		float uinfow = 160 * m_Scale;
		float uinfoh = 35 * m_Scale;
		float uinfox = p2x + 1 * m_Scale;
		float uinfoy = p2y - uinfoh + 10 * m_Scale;
		m_stat_memberinfo.MoveWindow(uinfox, uinfoy, uinfow, uinfoh);

		//最大绑定数量
		float mbw = 160 * m_Scale;
		float mbh = 35 * m_Scale;
		float mbx = m_WindowWidth - mbw - 10 * m_Scale;
		float mby = uinfoy;
		m_stat_maxbind.MoveWindow(mbx, mby, mbw, mbh);

		//会员到期时间
		float extw = 320 * m_Scale;
		float exth = 35 * m_Scale;
		float extx = m_WindowWidth - extw - 10 * m_Scale;
		float exty = p2y;
		m_stat_expiretime.MoveWindow(extx, exty, extw, exth);

		//开通会员按钮
		float openvipW = m_WindowWidth - 10 * m_Scale;
		float openvipH = 40 * m_Scale;
		if (m_Scale == 1)
			openvipH = 35 * m_Scale;
		else
			openvipH = 40 * m_Scale;
		float openvipX = (m_WindowWidth - openvipW) / 2;
		float openvipY = m_WindowHeight - openvipH - 10 * m_Scale;
		m_btn_openVip.MoveWindow(openvipX, openvipY, openvipW, openvipH);
	}
	else
	{
		//顶部用户图标
		float u1icw = 21 * m_Scale;
		float u1ich = 21 * m_Scale;
		float u1icx = 10 * m_Scale;
		float u1icy = (m_BoundaryLineY - u1ich) / 2;
		m_btn_UserIcon1.MoveWindow(u1icx, u1icy, u1icw, u1ich);

		//顶部手机号
		float p1w = 90 * m_Scale;
		float plh = 25 * m_Scale;
		float p1x = u1icx + u1icw + 5 * m_Scale;
		float ply = u1icy + (u1ich - plh) / 2 + 4 * m_Scale;
		m_stat_phonenum1.MoveWindow(p1x, ply, p1w, plh);

		//联系客服
		float ctsw = 90 * m_Scale;
		float ctsh = 25 * m_Scale;
		float ctsx = m_WindowWidth - ctsw - 5 * m_Scale;
		float ctsy = (m_BoundaryLineY - ctsh) / 2;
		m_btn_contactservice.MoveWindow(ctsx, ctsy, ctsw, ctsh);

		//退出登录
		float logoutw = 90 * m_Scale;
		float logouth = 25 * m_Scale;
		float logoutx = ctsx - logoutw - 5 * m_Scale;
		float logouty = (m_BoundaryLineY - logouth) / 2;
		m_btn_logOut.MoveWindow(logoutx, logouty, logoutw, logouth);

		//中间用户图标
		float u2icw = 54 * m_Scale;
		float u2ich = 54 * m_Scale;
		float u2icx = 10 * m_Scale;
		float u2icy = m_BoundaryLineY + (m_WindowHeight - m_BoundaryLineY - u2ich) / 2 - 23 * m_Scale;
		m_btn_UserIcon2.MoveWindow(u2icx, u2icy, u2icw, u2ich);

		//中间手机号
		float p2w = 160 * m_Scale;
		float p2h = 35 * m_Scale;
		float p2x = u2icx + u2icw + 5 * m_Scale;
		float p2y = u2icy + (u2ich - p2h) / 2 + 20 * m_Scale;
		m_stat_phonenum2.MoveWindow(p2x, p2y, p2w, p2h);

		//会员信息
		float uinfow = 160 * m_Scale;
		float uinfoh = 35 * m_Scale;
		float uinfox = p2x + 1 * m_Scale;
		float uinfoy = p2y - uinfoh + 10 * m_Scale;
		m_stat_memberinfo.MoveWindow(uinfox, uinfoy, uinfow, uinfoh);

		//最大绑定数量
		float mbw = 160 * m_Scale;
		float mbh = 35 * m_Scale;
		float mbx = m_WindowWidth - mbw - 10 * m_Scale;
		float mby = uinfoy;
		m_stat_maxbind.MoveWindow(mbx, mby, mbw, mbh);

		//会员到期时间
		float extw = 320 * m_Scale;
		float exth = 35 * m_Scale;
		float extx = m_WindowWidth - extw - 10 * m_Scale;
		float exty = p2y;
		m_stat_expiretime.MoveWindow(extx, exty, extw, exth);

		//开通会员按钮
		float openvipW = m_WindowWidth - 10 * m_Scale;
		float openvipH = 40 * m_Scale;
		if (m_Scale == 1)
			openvipH = 35 * m_Scale;
		else
			openvipH = 40 * m_Scale;
		float openvipX = (m_WindowWidth - openvipW) / 2;
		float openvipY = m_WindowHeight - openvipH - 10 * m_Scale;
		m_btn_openVip.MoveWindow(openvipX, openvipY, openvipW, openvipH);
	}
}

float Ui_UserProfileDlg::GetUserDPI()
{
	// 获取系统 DPI
	HDC screen = ::GetDC(NULL);
	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	::ReleaseDC(NULL, screen);

	// 计算缩放因子（基准 DPI 为 96）
	double scaleX = static_cast<double>(dpiX) / 96.0;
	double scaleY = static_cast<double>(dpiY) / 96.0;
	m_Scale = scaleY;
	return scaleY;
}

BEGIN_MESSAGE_MAP(Ui_UserProfileDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_SHOWWINDOW()

	//按钮响应
	ON_BN_CLICKED(UPDLG_BTN_LOGINOUT, &Ui_UserProfileDlg::OnBnClickedBtnLoginout)
	ON_BN_CLICKED(UPDLG_BTN_CONTACTSERVICE, &Ui_UserProfileDlg::OnBnClickedBtnContactservice)

	//应用程序消息响应
	ON_MESSAGE(MSG_APPMSG_LOGOUTSUCCESS,&Ui_UserProfileDlg::On_AppMsg_LogoutSuccess)
	ON_BN_CLICKED(UPDLG_BTN_OPENVIP, &Ui_UserProfileDlg::OnBnClickedBtnOpenvip)
END_MESSAGE_MAP()

// Ui_UserProfileDlg 消息处理程序

BOOL Ui_UserProfileDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL Ui_UserProfileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	UpdateScale();
	initCtrl();

	MoveWindow(0, 0, m_WindowWidth, m_WindowHeight);

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口阴影效果
	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
	m_Shadow.Create(m_hWnd);

	m_bool_isInitlize = TRUE;
	return TRUE; 
}

void Ui_UserProfileDlg::OnPaint()
{
	CPaintDC dc(this);
	m_Shadow.Show(m_hWnd);
	//预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
	Graphics memGraphics(&memBitmap);
	memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	memGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

	//绘画背景
	SolidBrush bkbrush(Color(36, 37, 40));
	memGraphics.FillRectangle(&bkbrush, 0, 0, m_WindowWidth, m_WindowHeight);

	//绘画分界线
	SolidBrush lineBrush(Color(73, 73, 73));
	Pen penBrush(&lineBrush);
	memGraphics.DrawLine(&penBrush, Point(0, m_BoundaryLineY), Point(m_WindowWidth, m_BoundaryLineY));

	//绘画边框
	SolidBrush boardBrush(Color(73, 73, 73));
	Pen boardpen(&lineBrush, 4);
	memGraphics.DrawRectangle(&boardpen, 0, 0, m_WindowWidth, m_WindowHeight);

	//一次性绘画到窗口上
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));

	DB(ConsoleHandle, L"Ui_UserProfileDlg:repaint..");
}

void Ui_UserProfileDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_MOUSE_CHECK)
	{//判断鼠标是否还在当前窗口里，不在则隐藏
		CPoint pt;
		::GetCursorPos(&pt);
		CRect rc;
		GetWindowRect(&rc);
		if (!rc.PtInRect(pt))
		{
			KillTimer(IDT_MOUSE_CHECK);
			ShowWindow(SW_HIDE);
			m_bool_isEnterIn = FALSE;
			if (App.m_Dlg_Main)
				::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, NULL, NULL);
			if (App.m_Dlg_Main.m_Dlg_Gaming)
				::PostMessage(App.m_Dlg_Main.m_Dlg_Gaming->GetSafeHwnd(), MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, NULL, NULL);
			if (App.m_Dlg_Main.m_Dlg_Carmera)
				::PostMessage(App.m_Dlg_Main.m_Dlg_Carmera->GetSafeHwnd(), MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, NULL, NULL);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void Ui_UserProfileDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_bool_isEnterIn)
	{
		SetTimer(IDT_MOUSE_CHECK, 100, nullptr);
		m_bool_isEnterIn = TRUE;
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void Ui_UserProfileDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnNcLButtonDown(nHitTest, point);
}

void Ui_UserProfileDlg::OnBnClickedBtnLoginout()
{
	if (App.m_Dlg_Main)
	{
		::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, NULL, NULL);
		::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_UIPROFILE_SIGNOUT, NULL, NULL);
	}
}

void Ui_UserProfileDlg::OnBnClickedBtnContactservice()
{
	App.OpenFeedBackLink(
		this->GetSafeHwnd(),
		L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
	);
}

LRESULT Ui_UserProfileDlg::On_AppMsg_LogoutSuccess(WPARAM wp, LPARAM lp)
{
	::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_USERPRIFILEDLG_SHOWLOGOUTSUCCESSFULDLG, NULL, NULL);
	return LRESULT();
}

void Ui_UserProfileDlg::OnBnClickedBtnOpenvip()
{
	::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_USERPROFILE_OPENVIP, NULL, NULL);
}


void Ui_UserProfileDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		App.m_Dlg_Main.SetPhoneBtnImageUp();
	}
	else
	{
		App.m_Dlg_Main.ResetPhoneBtnImage();
	}
}
