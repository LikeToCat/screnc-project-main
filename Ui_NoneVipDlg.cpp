// Ui_MessageModalDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_NoneVipDlg.h"
#include "afxdialogex.h"
#include "CMessage.h"
#include "Ui_ConfigDlg.h"
#include "theApp.h"
#include "ComponentAPI.h"
// Ui_MessageModalDlg 对话框
IMPLEMENT_DYNAMIC(Ui_MessageModalDlg, CDialogEx)

Ui_MessageModalDlg::Ui_MessageModalDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_NONEVIP, pParent)
{
	GetUserDPI();
	m_MessageType = L"";
	m_MessageInfo = L"";
	m_Title = L"";
	m_BtnText = L"";
	m_IsModal = false;
	m_WindowWidth = 420 * m_Scale;
	m_WindowHeight = 220 * m_Scale;
	m_CornerRadius = 0;
	m_CkRectborderWidth = 15 * m_Scale;
	m_CkRectbroderHeight = 15 * m_Scale;
	m_CkEcBorderWidth = m_CkRectborderWidth - 1 * m_Scale;
	m_CkEcBorderHeight = m_CkRectbroderHeight - 1 * m_Scale;
	m_gd1 = Gdiplus::Color(0, 0, 0);
	m_gd2 = Gdiplus::Color(0, 0, 0);
	m_bIsDefaultBtnGradient = false;
	m_hgd1 = Gdiplus::Color(0, 0, 0);
	m_hgd2 = Gdiplus::Color(0, 0, 0);
	m_IsHideClose = false;
}

Ui_MessageModalDlg::~Ui_MessageModalDlg()
{
	for (CLarBtn* pBtn : m_ExtraBtnControls)
	{
		if (pBtn->GetSafeHwnd())
			pBtn->DestroyWindow();
		delete pBtn;
	}
	m_ExtraBtnControls.clear();

	for (CLazerStaticText* pStat : m_ExtraCheckBox)
	{
		if (pStat->GetSafeHwnd())
			pStat->DestroyWindow();
		delete pStat;
	}
	m_ExtraCheckBox.clear();
}

void Ui_MessageModalDlg::Ui_SetWindowRect(const CRect& rect)
{
	m_WindowWidth = rect.Width();
	m_WindowHeight = rect.Height();
}

void Ui_MessageModalDlg::CleanUpGdiPngRes()
{
}

void Ui_MessageModalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, NONEVIPDLG_STAT_APPTEXT, m_Stat_AppText);
	DDX_Control(pDX, NONEVIPDLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, NONEVIPDLG_BTN_ICONLOGO, m_Btn_Icon);
	DDX_Control(pDX, NONEVIPDLG_STAT_MESSAGETYPE, m_Stat_MessageType);
	DDX_Control(pDX, NONEVIPDLG_STAT_MESSAGEINFO, m_Stat_MessageInfo);
	DDX_Control(pDX, NONEVIPDLG_BTN_OPENVIP, m_Btn_OpenVip);
}

void Ui_MessageModalDlg::AddButton(const CString BtnText, Gdiplus::Color BkColoc, Gdiplus::Color hoverColor, Gdiplus::Color clickColor, Gdiplus::Color TextColor, Gdiplus::Color HoverTextColor, Gdiplus::Color ClickTextColor, Gdiplus::Color BorderColor, std::function<void()> BtnCallBack)
{
	ExtraBtn btn;
	btn.BtnText = BtnText;
	btn.BkColoc = BkColoc;
	btn.hoverColor = hoverColor;
	btn.clickColor = clickColor;
	btn.TextColor = TextColor;
	btn.HoverTextColor = HoverTextColor;
	btn.ClickTextColor = ClickTextColor;
	btn.BorderColor = BorderColor;
	btn.BtnCallBack = BtnCallBack;
	btn.Uid = Btn_Identification_Start++;
	m_Map_ExtraBtns.emplace(btn.Uid, std::move(btn));

	
}

void Ui_MessageModalDlg::AddCkBox(CString CkText, Gdiplus::Color TextColor, int textSize, int borderSize, Gdiplus::Color borderColor, CheckBoxBorderStyle borderType, int diffx, int diffy, int mutualExclusion, bool isSelect)
{
	ExtraCheckBox ck;
	ck.CkText = CkText;
	ck.TextColor = TextColor;
	ck.borderSize = borderSize;
	ck.Uid = Ck_Identification_Start++;
	ck.broderStyle = borderType;
	ck.textSize = textSize;
	ck.CkRect = CRect();
	ck.borderColor = borderColor;
	ck.diffX = diffx;
	ck.diffY = diffy;
	ck.IsSelect = isSelect;
	ck.MutualExclusion = mutualExclusion;
	m_Map_ExtraCk.emplace(ck.Uid, std::move(ck));
}

void Ui_MessageModalDlg::OnDialogOk()
{
	OnOK();
}

void Ui_MessageModalDlg::OnDialogCancel()
{
	OnCancel();
}

void Ui_MessageModalDlg::GetSelectCkUid(std::vector<int>* selectIdvec)
{
	for (auto const& ck : m_Map_ExtraCk)
	{
		if (ck.second.IsSelect)
		{
			selectIdvec->push_back(ck.second.Uid);
		}
	}
}

void Ui_MessageModalDlg::HideCloseBtn()
{
	m_IsHideClose = true;
}

void Ui_MessageModalDlg::GetUserDPI()
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

void Ui_MessageModalDlg::UpdateScale()
{
	//定义标题栏区域
	m_Rect_TitleBarArea.X = 0;
	m_Rect_TitleBarArea.Y = 0;
	m_Rect_TitleBarArea.Width = m_WindowWidth;
	m_Rect_TitleBarArea.Height = 0.151 * m_WindowHeight;

	//定义客户区域
	m_Rect_ClientArea.X = 0;
	m_Rect_ClientArea.Y = m_Rect_TitleBarArea.Height;
	m_Rect_ClientArea.Width = m_WindowWidth;
	m_Rect_ClientArea.Height = m_WindowHeight - m_Rect_TitleBarArea.Height;

	//标题文本165 45
	int TitleTextWidth = 180 * m_Scale;
	int TitleTextHeight = 30 * m_Scale;
	int TitleTextX = 25 * m_Scale;
	int TitleTextY = (m_Rect_TitleBarArea.Height - TitleTextHeight) / 2;
	m_Stat_AppText.MoveWindow(TitleTextX, TitleTextY, TitleTextWidth, TitleTextHeight);

	//关闭按钮
	int CloseBtnWidth = 22.5 * m_Scale;
	int CloseBtnHeight = 22.5 * m_Scale;
	int CloseBtnX = m_WindowWidth - CloseBtnWidth - 10 * m_Scale;
	int CloseBtnY = (m_Rect_TitleBarArea.Height - CloseBtnHeight) / 2;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	//图标
	int iconWidth = 28 * m_Scale;
	int iconHeight = 28 * m_Scale;
	int iconX = 0.079 * m_WindowWidth;
	int iconY = 0.274 * m_WindowHeight;
	m_Btn_Icon.MoveWindow(iconX, iconY, iconWidth, iconHeight);

	//文本类型内容
	int MessageTypeWidth = 230 * m_Scale;
	int MessageTypeHeight = 25 * m_Scale;
	int MessageTypeX = iconX + iconWidth + 15 * m_Scale;
	int MessageTypeY = iconY + (iconHeight - MessageTypeHeight) / 2;
	m_Stat_MessageType.MoveWindow(MessageTypeX, MessageTypeY, MessageTypeWidth, MessageTypeHeight);

	//消息内容
	int MessageWidth = 318 * m_Scale;
	int MessageHeight = 40 * m_Scale;
	int MessageX = MessageTypeX;
	int MessageY = MessageTypeY + 40 * m_Scale;
	m_Stat_MessageInfo.MoveWindow(MessageX, MessageY, MessageWidth, MessageHeight);

	//开通VIP130 50
	int OpenVipBtnWidth = 86 * m_Scale;
	int OpenVipBtnHeight = 33 * m_Scale;
	int OpenVipBtnX = 0.722 * m_WindowWidth;
	int OpenVipBtnY = 0.745 * m_WindowHeight;
	m_Btn_OpenVip.MoveWindow(OpenVipBtnX, OpenVipBtnY, OpenVipBtnWidth, OpenVipBtnHeight);
}

void Ui_MessageModalDlg::InitCtrl()
{
	//设置字体
	m_Stat_AppText.LarSetTextSize(20);
	m_Stat_AppText.LarSetTextLeft();

	m_Stat_MessageType.LarSetTextSize(20);
	m_Stat_MessageType.LarSetTextStyle(true, false, false);
	m_Stat_MessageType.LarSetTextLeft();

	//设置消息内容文本
	m_Stat_MessageInfo.SetWindowTextW(L"非VIP用户暂不支持该功能，完整功能需要开通VIP\n后使用");
	m_Stat_MessageInfo.LarSetTextSize(20);
	m_Stat_MessageInfo.LarSetBreakLine(true);
	m_Stat_MessageInfo.LarSetTextLeft();

	//图标
	m_Btn_Icon.LoadPNG(OPENVIPDLG_PNG_QUESTION);
	m_Btn_Icon.LoadClickPNG(OPENVIPDLG_PNG_QUESTION);
	m_Btn_Icon.SetBackgroundColor(RGB(36, 37, 40));
	m_Btn_Icon.SetClickEffectColor(255, 36, 37, 40);

	//关闭按钮
	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.LoadClickPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(36, 37, 40));
	m_Btn_Close.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Close.SetStretchMode(0.75f);
	if (m_IsHideClose)
		m_Btn_Close.ShowWindow(SW_HIDE);

	//开通vip
	m_Btn_OpenVip.LarSetTextSize(18);
	m_Btn_OpenVip.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
	m_Btn_OpenVip.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_OpenVip.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_OpenVip.LarSetBorderColor(Gdiplus::Color(255, 0, 139, 255));
	m_Btn_OpenVip.LarSetEraseBkEnable(false);
	if (m_bIsDefaultBtnGradient)
	{
		int OpenVipBtnWidth = 86 * m_Scale;
		int OpenVipBtnHeight = 35 * m_Scale;
		int OpenVipBtnX = 0.722 * m_WindowWidth;
		int OpenVipBtnY = 0.745 * m_WindowHeight;
		m_Btn_OpenVip.MoveWindow(OpenVipBtnX, OpenVipBtnY, OpenVipBtnWidth, OpenVipBtnHeight);

		m_Btn_OpenVip.LaSetTextColor(Gdiplus::Color(255, 63, 34, 1));
		m_Btn_OpenVip.LaSetTextHoverColor(Gdiplus::Color(255, 63, 34, 1));
		m_Btn_OpenVip.LaSetTextClickedColor(Gdiplus::Color(255, 63, 34, 1));
		m_Btn_OpenVip.LarSetGradualColor(m_gd1, m_gd2, Gdiplus::LinearGradientMode::LinearGradientModeHorizontal);
		m_Btn_OpenVip.LarSetHoverGradualColor(m_hgd1, m_hgd2, Gdiplus::LinearGradientMode::LinearGradientModeHorizontal);
		m_Btn_OpenVip.LarSetClickGradualColor(
			m_cgd1, m_cgd2, Gdiplus::LinearGradientMode::LinearGradientModeForwardDiagonal
		);
		m_Btn_OpenVip.LarSetBtnBorderEnable(false);
		m_Btn_OpenVip.LaSetAngleRounded(2 * m_Scale);
	}
	else
	{
		m_Btn_OpenVip.LarSetNormalFiilBrush(SolidBrush(Color(255, 0, 139, 255)));
		m_Btn_OpenVip.LarSetHoverFillBrush(SolidBrush(Color(245, 7, 118, 212)));
		m_Btn_OpenVip.LarSetClickedFillBrush(SolidBrush(Color(255, 7, 118, 212)));
		m_Btn_OpenVip.LaSetAngleRounded(2 * m_Scale);
	}
}

void Ui_MessageModalDlg::SetModal(CString title, CString Type, CString info, CString BtnText)
{
	m_Title = title;
	m_MessageType = Type;
	m_MessageInfo = info;
	m_BtnText = BtnText;
	m_IsModal = true;
}

BEGIN_MESSAGE_MAP(Ui_MessageModalDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_BN_CLICKED(NONEVIPDLG_BTN_OPENVIP, &Ui_MessageModalDlg::OnBnClickedBtnSure)
	ON_BN_CLICKED(NONEVIPDLG_BTN_CLOSE, &Ui_MessageModalDlg::OnBnClickedBtnClose)
END_MESSAGE_MAP()

void Ui_MessageModalDlg::OnPaint()
{
	CPaintDC dc(this);
	m_Shadow.Show(m_hWnd);
	//预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
	Graphics memGraphics(&memBitmap);
	memGraphics.SetSmoothingMode(SmoothingModeHighQuality);
	memGraphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	memGraphics.SetCompositingQuality(CompositingQualityHighQuality);
	memGraphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
	memGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	//绘画标题栏背景和绘画客户区背景
	SolidBrush CaptionBrush(Color(37, 39, 46));
	SolidBrush ClientBrush(Color(36, 37, 40));
	memGraphics.FillRectangle(&CaptionBrush, m_Rect_TitleBarArea);//绘画标题栏背景
	memGraphics.FillRectangle(//绘画客户区背景
		&ClientBrush,
		m_Rect_ClientArea.X,
		m_Rect_ClientArea.Y,
		m_Rect_ClientArea.Width,
		m_Rect_ClientArea.Height
	);

	//绘画多选框边框
	for (const auto& ExtraCk : m_Map_ExtraCk)
	{
		auto ckinfo = ExtraCk.second;
		if (ckinfo.broderStyle == CheckBoxBorderStyle::Rect)
		{
			CRect rect = ckinfo.CkBorderRect;
			SolidBrush borderBrush(ckinfo.borderColor);
			Pen BorderPen(&borderBrush, ckinfo.borderSize);
			memGraphics.DrawRectangle(&BorderPen, rect.left, rect.top, m_CkEcBorderWidth, m_CkEcBorderHeight);
			DBFMT(ConsoleHandle, L"当前遍历状态框:%s，选中状态:%s", ckinfo.CkText, ckinfo.IsSelect ? L"true" : L"false");
			if (ckinfo.IsSelect)
			{
				DBFMT(ConsoleHandle, L"状态框:%s，被选中，绘画选中对勾", ckinfo.CkText);
				COMAPI::MFC::DrawSelectGouByBorderRect(&memGraphics,
					m_CkEcBorderWidth, m_CkEcBorderHeight, 
					rect.left, rect.top, 
					m_Scale
				);
			}
		}
		else if (ExtraCk.second.broderStyle == CheckBoxBorderStyle::Eclipse)
		{
			CRect rect = ckinfo.CkBorderRect;
			SolidBrush borderBrush(ckinfo.borderColor);
			Pen BorderPen(&borderBrush, ckinfo.borderSize);
			memGraphics.DrawEllipse(&BorderPen, rect.left, rect.top, m_CkRectborderWidth, m_CkRectbroderHeight);
			DBFMT(ConsoleHandle, L"当前遍历状态框:%s，选中状态:%s", ckinfo.CkText, ckinfo.IsSelect ? L"true" : L"false");
			if (ckinfo.IsSelect)
			{
				DBFMT(ConsoleHandle, L"状态框:%s，被选中，绘画选中对勾", ckinfo.CkText);
				COMAPI::MFC::DrawSelectGouByBorderRect(&memGraphics,
					m_CkEcBorderWidth, m_CkEcBorderHeight,
					rect.left, rect.top,
					m_Scale
				);
			}
		}
	}

	//一次性绘画到窗口上
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));

	DB(ConsoleHandle, L"Ui_MessageModalDlg:repaint..");
}

BOOL Ui_MessageModalDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetUserDPI();
	UpdateScale();
	InitCtrl();

	if (m_IsModal)
	{
		if (m_MessageType != L"")
		{
			m_Stat_MessageType.SetWindowTextW(m_MessageType);
		}

		if (m_MessageInfo != L"")
		{
			m_Stat_MessageInfo.SetWindowTextW(m_MessageInfo);
		}
		else if (m_MessageInfo == L"")
		{
			m_Stat_MessageInfo.ShowWindow(SW_HIDE);
		}

		if (m_Title != L"")
		{
			m_Stat_AppText.SetWindowTextW(m_Title);
		}

		if (m_BtnText != L"")
		{
			m_Btn_OpenVip.SetWindowTextW(m_BtnText);
		}

	}
	if (m_Map_ExtraBtns.size() != 0)
	{
		int i = 0;
		for (auto& extraBtn : m_Map_ExtraBtns)
		{
			CRect BtnRect;
			m_Btn_OpenVip.GetWindowRect(BtnRect);
			ScreenToClient(BtnRect);

			CRect extraBtnRect;
			extraBtnRect.left = BtnRect.left - 10 * m_Scale - BtnRect.Width() * (i + 1);
			extraBtnRect.top = BtnRect.top;
			extraBtnRect.right = extraBtnRect.left + BtnRect.Width();
			extraBtnRect.bottom = extraBtnRect.top + BtnRect.Height();

			const ExtraBtn& info = extraBtn.second;
			CLarBtn* pBtn = new CLarBtn();
			pBtn->Create(
				info.BtnText,
				WS_CHILD | WS_VISIBLE,
				extraBtnRect,
				this,
				info.Uid
			);
			pBtn->MoveWindow(extraBtnRect);
			pBtn->LarSetNormalFiilBrush(SolidBrush(info.BkColoc));
			pBtn->LarSetHoverFillBrush(SolidBrush(info.hoverColor));
			pBtn->LarSetClickedFillBrush(SolidBrush(info.clickColor));
			pBtn->LaSetTextColor(info.TextColor);
			pBtn->LaSetTextHoverColor(info.HoverTextColor);
			pBtn->LaSetTextClickedColor(info.ClickTextColor);
			pBtn->LarSetBorderColor(info.BorderColor);
			pBtn->LaSetAngleRounded(2 * m_Scale);
			pBtn->LarSetTextSize(18);
			m_ExtraBtnControls.push_back(pBtn);
			i++;
		}
	}
	if (m_Map_ExtraCk.size() != 0)
	{
		int i = 0;
		for (auto& extraCK : m_Map_ExtraCk)
		{
			CRect MessageTypeRect;
			m_Stat_MessageType.GetWindowRect(MessageTypeRect);
			ScreenToClient(MessageTypeRect);
			ExtraCheckBox& info = extraCK.second;

			info.CkRect.left = MessageTypeRect.left + 5 * m_Scale + info.diffX;
			info.CkRect.top = MessageTypeRect.top + 15 * m_Scale + MessageTypeRect.Height() * (i + 1) + info.diffY;
			info.CkRect.right = info.CkRect.left + m_Rect_TitleBarArea.Width;
			info.CkRect.bottom = info.CkRect.top + 18 * m_Scale;
			CRect TextRect = info.CkRect;
			TextRect.left += m_CkRectborderWidth;

			CLazerStaticText* pStat = new CLazerStaticText();
			pStat->Create(
				info.CkText,
				WS_CHILD | WS_VISIBLE,
				TextRect,
				this,
				info.Uid
			);
			pStat->MoveWindow(TextRect);
			pStat->LarSetTextLeft();
			pStat->LarSetTextColor(RGB(info.TextColor.GetR(), info.TextColor.GetG(), info.TextColor.GetB()));
			pStat->LarSetTextSize(info.textSize);
			info.stat = pStat;

			CRect rect = info.CkRect;
			int borderWidth = (info.broderStyle == CheckBoxBorderStyle::Rect) ? m_CkRectborderWidth : m_CkEcBorderWidth;
			int borderHeight = (info.broderStyle == CheckBoxBorderStyle::Rect) ? m_CkRectbroderHeight : m_CkEcBorderHeight;
			int borderX = rect.left - 5 * m_Scale;
			int borderY = rect.top + (rect.Height() - borderHeight) / 2;


			rect.left = borderX;
			rect.top = borderY;
			rect.right = rect.left + borderWidth;
			rect.bottom = rect.top + borderHeight;

			info.CkBorderRect = rect;
			m_ExtraCheckBox.push_back(pStat);
			i++;
		}
	}

	if (m_bPosSet)
	{
		// 使用用户指定的屏幕坐标
		SetWindowPos(NULL,
			m_InitPosX, m_InitPosY,
			m_WindowWidth, m_WindowHeight,
			SWP_NOZORDER);
	}
	else
	{
		CRect rect;
		GetWindowRect(&rect);
		SetWindowPos(NULL, rect.left, rect.top, m_WindowWidth, m_WindowHeight, SWP_NOMOVE | SWP_NOZORDER);
	}

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口阴影效果
	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
	m_Shadow.Create(m_hWnd);
	ApplyRoundCorner();

	//设置窗口置顶
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	SetTimer(1001, 500, NULL);
	return TRUE;
}

BOOL Ui_MessageModalDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

HBRUSH Ui_MessageModalDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->GetDlgCtrlID() == NONEVIPDLG_STAT_MESSAGEINFO && nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetTextColor(RGB(168, 168, 168));
		static CBrush brush(RGB(36, 37, 40));
		pDC->SetBkColor(RGB(36, 37, 40));
		return (HBRUSH)brush.GetSafeHandle();
	}
	return hbr;
}

void Ui_MessageModalDlg::OnBnClickedBtnSure()
{
	if (!m_IsModal)//如果是非模态窗口的确认请求
	{
		KillTimer(1001);
		CString btnText;
		m_Btn_OpenVip.GetWindowTextW(btnText);
		if (btnText == L"解绑设备")
		{
			GetParent()->EnableWindow(true);
			this->ShowWindow(SW_HIDE);
			auto pConfigDlg = (Ui_ConfigDlg*)GetParent();
			pConfigDlg->OnBnClickedBtnClose();
			m_Shadow.Show(m_hWnd);
			::PostMessage(GetParent()->GetParent()->GetSafeHwnd(), MSG_NONEVIPDLG_UNBIND, NULL, NULL);
		}
		else if (btnText == L"开通VIP")
		{
			GetParent()->EnableWindow(true);
			this->ShowWindow(SW_HIDE);
			auto pConfigDlg = (Ui_ConfigDlg*)GetParent();
			if(pConfigDlg)
				pConfigDlg->OnBnClickedBtnClose();
			m_Shadow.Show(m_hWnd);
			if (::IsWindowVisible(App.m_Dlg_Main.m_Dlg_Carmera->GetSafeHwnd()))
				::PostMessage(App.m_Dlg_Main.m_Dlg_Carmera->GetSafeHwnd(), CAMERADLG_BTN_OPENVIP, NULL, NULL);
			else
				::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_NONEVIPDLG_OPENVIP, NULL, NULL);
		}
		else if (btnText == L"登录")
		{
			GetParent()->EnableWindow(true);
			this->ShowWindow(SW_HIDE);
			m_Shadow.Show(m_hWnd);
			::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_NONEVIPDLG_LOGIN, NULL, NULL);
			::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_REQUESTCONFIGDLG_CLOSE, NULL, NULL);
		}
		else if (btnText == L"取消")
		{

		}
		else if (btnText == L"联系客服")
		{
			App.m_Dlg_Main.OnBnClickedBtnFeedback();
		}
		this->ShowWindow(SW_HIDE);
	}
	else//如果是模态窗口的确认
	{
		m_Shadow.Show(m_hWnd);
		if (m_defaultBtnCallBack)
			m_defaultBtnCallBack();
		else
			OnOK();
	}
}

void Ui_MessageModalDlg::OnBnClickedBtnClose()
{
	if (!m_IsModal)//如果是非模态窗口关闭
	{
		KillTimer(1001);
		GetParent()->EnableWindow(true);
		this->ShowWindow(SW_HIDE);
		m_Shadow.Show(m_hWnd);
	}
	else//如果是模态窗口的关闭
	{
		OnCancel();
	}
}

void Ui_MessageModalDlg::SetCornerRadius(int radius)
{
	m_CornerRadius = radius;
}

void Ui_MessageModalDlg::SetMessageTypeIcon(UINT uId)
{
	m_Btn_Icon.LoadPNG(uId);
	m_Btn_Icon.LoadClickPNG(uId);
	m_Btn_Icon.SetBackgroundColor(RGB(36, 37, 40));
	m_Btn_Icon.SetClickEffectColor(255, 36, 37, 40);
}

void Ui_MessageModalDlg::SetDefaultBtnCallback(std::function<void()> SureCallBack)
{
	m_defaultBtnCallBack = SureCallBack;
}

void Ui_MessageModalDlg::SetDefaultBtnGradientColor(
	Gdiplus::Color c1, Gdiplus::Color c2,
	Gdiplus::Color hc1, Gdiplus::Color hc2,
	Gdiplus::Color cc1, Gdiplus::Color cc2
)
{
	m_gd1 = c1;
	m_gd2 = c2;
	m_hgd1 = hc1;
	m_hgd2 = hc2;
	m_cgd1 = cc1;
	m_cgd2 = cc2;
	m_bIsDefaultBtnGradient = true;
}

void Ui_MessageModalDlg::SetInitialPosition(int x, int y)
{
	m_bPosSet = true;
	m_InitPosX = x;
	m_InitPosY = y;
}

void Ui_MessageModalDlg::ApplyRoundCorner()
{
	if (m_CornerRadius > 0)
	{
		HRGN hRgn = ::CreateRoundRectRgn(
			0, 0,
			m_WindowWidth + 1, m_WindowHeight + 1,
			m_CornerRadius, m_CornerRadius
		);
		SetWindowRgn(hRgn, TRUE);
	}
}

BOOL Ui_MessageModalDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT id = LOWORD(wParam);
	UINT code = HIWORD(wParam);
	if (code == BN_CLICKED)
	{
		auto it = m_Map_ExtraBtns.find(id);
		if (it != m_Map_ExtraBtns.end())
		{
			(*it).second.BtnCallBack();
			return TRUE;
		}
	}
	return CDialogEx::OnCommand(wParam, lParam);
}

void Ui_MessageModalDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	bool IsClickCk = false;
	for (auto& ck : m_Map_ExtraCk)
	{
		auto& ckInfo = ck.second;
		DBFMT(ConsoleHandle, L"判断CkBorderRect:left:%d,top:%d,right:%d,bottom:%d,posx:%d,posy:%d",
			ckInfo.CkBorderRect.left, ckInfo.CkBorderRect.top, ckInfo.CkBorderRect.right, ckInfo.CkBorderRect.bottom,
			point.x, point.y);
		if (ckInfo.CkRect.PtInRect(point))
		{
			IsClickCk = true;
			DBFMT(ConsoleHandle, L"选中状态框MutualExclusion:%d", ckInfo.MutualExclusion);
			if (ckInfo.IsSelect)
			{
				DBFMT(ConsoleHandle, L"状态框%s  已经选中", ckInfo.CkText);
				ckInfo.IsSelect = false;
				if(IsNoMutalSelect(ckInfo.MutualExclusion))
					ckInfo.IsSelect = true;//当前项为唯一选中的互斥项
				break;
			}
			if (ckInfo.MutualExclusion != -1)
			{
				MutualExCk(ckInfo.MutualExclusion);//重置MutualExclusion值相同的其他复选框选中状态为false
			}
			DBFMT(ConsoleHandle, L"状态框:%s，被设置为选中", ckInfo.CkText);
			ckInfo.IsSelect = true;
			break;
		}
	}

	if (IsClickCk)
	{
		DB(ConsoleHandle, L"repaint");
		Invalidate();
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_MessageModalDlg::MutualExCk(int mutualEx)
{
	for (auto& ck : m_Map_ExtraCk)
	{
		auto& ckInfo = ck.second;
		if (mutualEx == ckInfo.MutualExclusion)
		{
			DBFMT(ConsoleHandle, L"重置复选框互斥值为%d的状态为未选中", mutualEx);
			ckInfo.IsSelect = false;
		}
	}
}

bool Ui_MessageModalDlg::IsNoMutalSelect(int mutualEx)
{
	bool IsNoMutalSelect = true;
	for (const auto& ckInfo : m_Map_ExtraCk)
	{
		if (ckInfo.second.IsSelect)
		{
			IsNoMutalSelect = false;
			break;
		}
	}
	return IsNoMutalSelect;
}

void Ui_MessageModalDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1001)
	{
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		UpdateWindow();
		::BringWindowToTop(m_hWnd);
		this->SetForegroundWindow();
		KillTimer(1001);
		ShowWindow(SW_SHOW);
	}
	CDialogEx::OnTimer(nIDEvent);
}

void Ui_MessageModalDlg::OnOK()
{
	KillTimer(1001);
	CDialogEx::OnOK();
}

void Ui_MessageModalDlg::OnCancel()
{
	KillTimer(1001);
	CDialogEx::OnCancel();
}

void Ui_MessageModalDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		m_Shadow.RestoreFromHide();
	}
	else
	{
		m_Shadow.HideShadow();
	}
}
