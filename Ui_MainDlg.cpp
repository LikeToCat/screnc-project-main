#include "stdafx.h"
#include "Ui_LevelUpDlg.h"
#include "Ui_MainDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "CDebug.h"
#include "CMessage.h"
#include "theApp.h"
#include "GlobalFunc.h"
#include "LarStringConversion.h"
#include "Ui_NoneVipDlg.h"
#include "ConfigFileHandler.h"
#include "DialogManager.h"
#include "ModalDialogFunc.h"
#include "Ui_UserProfileDlg.h"
#include "FFmpegTest.h"
// Ui_MainDlg 对话框
extern HANDLE ConsoleHandle;
extern theApp App;
IMPLEMENT_DYNAMIC(Ui_MainDlg, CDialogEx)

Ui_MainDlg::Ui_MainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MAINDLG, pParent)
{
	m_bool_IsMoreMenuShow = false;
	m_Dlg_UserPrifile = nullptr;
	m_Bitmap_MouthBill = nullptr;
	m_Rect_MouthBill = Gdiplus::Rect();
}

Ui_MainDlg::~Ui_MainDlg()
{
	if (m_Dlg_Child) {
		m_Dlg_Child->DestroyWindow();
	}
	auto pIns = DialogManager::GetInstance();
	if (pIns)
	{
		pIns->ReleaseInstance();
	}
}

void Ui_MainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, MAINDLG_BTN_APPICON, m_Btn_AppIcon);
	DDX_Control(pDX, MAINDLG_STAT_LOGOTEXT, m_Stat_logoText);
	DDX_Control(pDX, MAINDLG_BTN_PROFILEICON, m_Btn_ProfileIcon);
	DDX_Control(pDX, MAINDLG_BTN_OPENVIP, m_Btn_OpenVIP);
	DDX_Control(pDX, MAINDLG_BTN_CONFIG, m_Btn_Config);
	DDX_Control(pDX, MAINDLG_BTN_MENU, m_Btn_FeedBack);
	DDX_Control(pDX, MAINDLG_BTN_MINIMAL, m_Btn_Minimal);
	DDX_Control(pDX, MAINDLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, MainDlg_Btn_ScreenRecording, m_Btn_ScreenRecord);
	DDX_Control(pDX, MAINDLG_BTN_AREARECORDING, m_Btn_AreaRecord);
	DDX_Control(pDX, MAINDLG_BTN_GAMINGRECORDING, m_Btn_GamingRecord);
	DDX_Control(pDX, MAINDLG_BTN_APPWINDOWRECORDING, m_Btn_WindowRecord);
	DDX_Control(pDX, MAINDLG_BTN_MORE, m_Btn_More);
	DDX_Control(pDX, MAINDLG_BTN_VIDEOLIST, m_Btn_VideoList);
	DDX_Control(pDX, MAINDLG_BTN_LOGIN, m_btn_Login);
	DDX_Control(pDX, MAINDLG_STAT_SCREENRECORD, m_Stat_ScreenRecord);
	DDX_Control(pDX, MAINDLG_STAT_AREARECORD, m_Stat_AreaRecord);
	DDX_Control(pDX, MAINDLG_STAT_GAMINGRECORD, m_Stat_GamingRecord);
	DDX_Control(pDX, MAINDLG_STAT_WINDOWRECORD, m_Stat_WindowRecord);
	DDX_Control(pDX, MAINDLG_BTN_DEVICEBINDING, m_Btn_DeviceBinding);
	DDX_Control(pDX, MAINDLG_BTN_PHONE, m_btn_Phone);
	//DDX_Control(pDX, MAINDLG_STAT_MOUTHPRICE, m_stat_mouthPrice);
}

BEGIN_MESSAGE_MAP(Ui_MainDlg, CDialogEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_NCACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_HOTKEY()
	ON_WM_ACTIVATEAPP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()

	//本窗口控件响应
	ON_BN_CLICKED(MAINDLG_BTN_APPWINDOWRECORDING, &Ui_MainDlg::OnBnClickedBtnAppwindowrecording)
	ON_BN_CLICKED(MAINDLG_BTN_VIDEOLIST, &Ui_MainDlg::OnBnClickedBtnVideolist)
	ON_BN_CLICKED(MAINDLG_BTN_OPENVIP, &Ui_MainDlg::OnBnClickedBtnOpenvip)
	ON_BN_CLICKED(MAINDLG_BTN_CONFIG, &Ui_MainDlg::OnBnClickedBtnConfig)
	ON_BN_CLICKED(MAINDLG_BTN_GAMINGRECORDING, &Ui_MainDlg::OnBnClickedBtnGamingrecording)
	ON_BN_CLICKED(MAINDLG_BTN_AREARECORDING, &Ui_MainDlg::OnBnClickedBtnArearecording)
	ON_BN_CLICKED(MAINDLG_BTN_MORE, &Ui_MainDlg::OnBnClickedBtnMore)
	ON_BN_CLICKED(MAINDLG_BTN_MINIMAL, &Ui_MainDlg::OnBnClickedBtnMinimal)
	ON_BN_CLICKED(MAINDLG_BTN_LOGIN, &Ui_MainDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(MAINDLG_BTN_DEVICEBINDING, &Ui_MainDlg::OnBnClickedBtnDevicebinding)
	ON_BN_CLICKED(MAINDLG_BTN_PHONE, &Ui_MainDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(MainDlg_Btn_ScreenRecording, &Ui_MainDlg::OnBnClickedBtnScreenRecord)
	ON_BN_CLICKED(MAINDLG_BTN_CLOSE, &Ui_MainDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(MAINDLG_BTN_FEEDBACK, &Ui_MainDlg::OnBnClickedBtnFeedback)

	//其他MFC窗口消息的响应
	ON_MESSAGE(MSG_NONEVIPDLG_OPENVIP, &Ui_MainDlg::On_Ui_NoneVipDlg_OpenVipBtnClicked)
	ON_MESSAGE(MSG_NONEVIPDLG_UNBIND, &Ui_MainDlg::On_Ui_NoneVipDlg_UnBindClicked)
	ON_MESSAGE(MSG_REQUESTCONFIGDLG_CLOSE, &Ui_MainDlg::On_Ui_RequestConfigDlg_Close)
	ON_MESSAGE(MSG_NONEVIPDLG_LOGIN, &Ui_MainDlg::On_Ui_NoneVipDlg_Login)
	ON_MESSAGE(WM_UI_CHILDDLG_RETURN, &Ui_MainDlg::On_Ui_ChildDlg_ReturnBtnClicked)
	ON_MESSAGE(WM_TRAYICON, &Ui_MainDlg::OnTrayIcon)
	ON_MESSAGE(MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, &Ui_MainDlg::OnUi_ProfileDlg_WindowHidenByTimer)
	ON_MESSAGE(MSG_UIPROFILE_SIGNOUT, &Ui_MainDlg::On_UiProfile_SignOut)
	ON_MESSAGE(MSG_USERPRIFILEDLG_SHOWLOGOUTSUCCESSFULDLG, &Ui_MainDlg::On_UiProfile_ShowLogoutSuccessDlg)
	ON_MESSAGE(MSG_USERPROFILE_OPENVIP, &Ui_MainDlg::On_UiProfile_OpenVip)
	ON_MESSAGE(MSG_OPENVIP_VIPPRIVILEGECONTRAST, &Ui_MainDlg::On_UiOpenVip_VipPrivilegeContrast)

	//广播消息
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGININ, &Ui_MainDlg::On_BroadCast_UserLogin)
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGINOUT, &Ui_MainDlg::On_BroadCast_UserLogOut)

	//http回传响应
	ON_MESSAGE(HTTPREQUESTCOMPELETE_UPDATE_DEVICEBINGSLIST, &Ui_MainDlg::On_Update_DevicBingList)

	//应用程序行为响应
	ON_MESSAGE(MSG_APPMSG_MINIMALANDTIPSRECORDING, &Ui_MainDlg::OnMinimalAndTipsUserRecording)
	ON_MESSAGE(MSG_APPMSG_FIRSTOPENTOOPENVIP, &Ui_MainDlg::OnFirstOpenToOpenVip)

	//SDL消息响应
	ON_MESSAGE(MSG_UIAREARECORD_UPDATE, &Ui_MainDlg::On_UiRecordArea_WindowChanged)
	ON_MESSAGE(MSG_UIDROPDOWNMENU_CAMERARECORD, &Ui_MainDlg::On_UiDropDowmMeau_CameraRecord)
	ON_MESSAGE(MSG_UIDROPDOWNMENU_MOUSERECORD, &Ui_MainDlg::On_UiDropDowmMeau_MouseRecord)
	ON_MESSAGE(MSG_UIDROPDOWNMENU_SYSTEMAUDIOMICRO, &Ui_MainDlg::On_UiDropDowmMeau_SystemMircroAudioRecord)
	ON_MESSAGE(MSG_UIDEVICEBIND_UNBINDANDCLOSE, &Ui_MainDlg::On_UiDeviceBind_UnBindAndClose)
	ON_MESSAGE(MSG_UIAREARECORD_HIDEMAINDLGTORECORD, &Ui_MainDlg::On_SDLAreaReocrd_HideMainWindow)
	ON_MESSAGE(MSG_UIAREARECORD_STARTRECORD, &Ui_MainDlg::On_SDLAreaReocrd_StartRecord)
	ON_MESSAGE(MSG_UIAREARECORD_STOPRECORD, &Ui_MainDlg::On_SDLAreaReocrd_StopRecord)
	ON_MESSAGE(MSG_UIAREARECORD_CLOSEWINDOW, &Ui_MainDlg::On_SDLAreaReocrd_CloseWindow)
	ON_MESSAGE(MSG_UIAREARECORD_PAUSERECORD, &Ui_MainDlg::On_SDLAreaReocrd_PauseRecord)
	ON_MESSAGE(MSG_UIAREARECORD_RESUMERECORD, &Ui_MainDlg::On_SDLAreaReocrd_ResumeRecord)
	ON_BN_CLICKED(IDC_BUTTON2, &Ui_MainDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

// Ui_MainDlg 消息处理程序

BOOL Ui_MainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetUserDPI();
	UpdateScale();
	InitCtrl();
	LoadRes();

	//if (App.m_IsNonUserPaid)
	//	ShowNoneUserPayUi();

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口阴影效果
	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
	m_Shadow.Create(m_hWnd);

	//创建关闭管理器 
	m_closeManager = std::make_unique<AppCloseManager>(GetSafeHwnd());
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); // 使用应用程序图标
	m_closeManager->SetTrayIconInfo(hIcon, L"极速录屏大师");
	m_closeManager->InitializeTrayIconOnStartup();//显示托盘图标

	// 设置关闭模式
	bool minimizeToTray = false;
	m_closeManager->SetCloseMode(
		minimizeToTray ?
		AppCloseManager::CloseMode::MinimizeToTray :
		AppCloseManager::CloseMode::ExitApplication
	);

	//设置窗口置顶
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, m_WindowWidth, m_WindowHeight, SWP_NOMOVE | SWP_NOZORDER);
	return TRUE;
}

void Ui_MainDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_Dlg_UserPrifile)
	{
		m_Dlg_UserPrifile->ShowWindow(SW_HIDE);
	}
	// 处理点击 MouthBill 区域的逻辑
	{
		RECT rc =
		{
			m_Rect_MouthBill.X,
			m_Rect_MouthBill.Y,
			m_Rect_MouthBill.X + m_Rect_MouthBill.Width,
			m_Rect_MouthBill.Y + m_Rect_MouthBill.Height
		};
		if (::PtInRect(&rc, point))
		{
			BnClickedMouthBill();
			return; 
		}
	}
}

void Ui_MainDlg::GetUserDPI()
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

void Ui_MainDlg::UpdateScale()
{
	float CaptionHeight = GetSystemMetrics(SM_CYCAPTION);

	//设定窗口大小
	float WindowHeight = 255 * m_Scale;
	float WindowWidth = 785 * m_Scale;
	CRect WindowRect;
	GetClientRect(WindowRect);
	WindowRect.right = WindowRect.left + WindowWidth;
	WindowRect.bottom = WindowRect.top + WindowHeight;
	m_WindowWidth = WindowWidth;
	m_WindowHeight = WindowHeight;
	m_WindowX = WindowRect.left;
	m_WindowY = WindowRect.top;
	MoveWindow(WindowRect);

	//设定全屏录制按钮
	float ScreenRecordBtnWidth = 72 * m_Scale;
	float ScreenRecordBtnHeight = 72 * m_Scale;
	float ScreenRecordBtnX = 0.097 * WindowWidth;
	float ScreenRecordBtnY = 0.361 * WindowHeight;
	m_Btn_ScreenRecord.MoveWindow(
		ScreenRecordBtnX, ScreenRecordBtnY,
		ScreenRecordBtnWidth, ScreenRecordBtnHeight
	);

	//设定全屏录制文本
	float StatTextHeight = 20 * m_Scale; // 文本控件的高度
	float ScreenRecordStatX = ScreenRecordBtnX;
	float ScreenRecordStatY = ScreenRecordBtnY + ScreenRecordBtnHeight + 5 * m_Scale;
	m_Stat_ScreenRecord.MoveWindow(
		ScreenRecordStatX, ScreenRecordStatY,
		ScreenRecordBtnWidth, StatTextHeight
	);

	//设定选区录制按钮
	float AreaRecordBtnWidth = 72 * m_Scale;
	float AreaRecordBtnHeight = 72 * m_Scale;
	float AreaRecordBtnX = 0.330 * WindowWidth;
	float AreaRecordBtnY = 0.361 * WindowHeight;
	m_Btn_AreaRecord.MoveWindow(
		AreaRecordBtnX, AreaRecordBtnY, AreaRecordBtnWidth, AreaRecordBtnHeight
	);

	//设定选区录制文本
	float AreaRecordStatX = AreaRecordBtnX;
	float AreaRecordStatY = AreaRecordBtnY + AreaRecordBtnHeight + 5 * m_Scale;
	m_Stat_AreaRecord.MoveWindow(
		AreaRecordStatX, AreaRecordStatY,
		AreaRecordBtnWidth, StatTextHeight
	);

	//设定游戏录制按钮
	float GamingRecordBtnWidth = 72 * m_Scale;
	float GamingRecordBtnHeight = 72 * m_Scale;
	float GamingRecordBtnX = 0.563 * WindowWidth;
	float GamingRecordBtnY = 0.361 * WindowHeight;
	m_Btn_GamingRecord.MoveWindow(
		GamingRecordBtnX, GamingRecordBtnY,
		GamingRecordBtnWidth, GamingRecordBtnHeight
	);

	//设定游戏录制文本
	float GamingRecordStatX = GamingRecordBtnX;
	float GamingRecordStatY = GamingRecordBtnY + GamingRecordBtnHeight + 5 * m_Scale;
	m_Stat_GamingRecord.MoveWindow(
		GamingRecordStatX, GamingRecordStatY,
		GamingRecordBtnWidth, StatTextHeight
	);

	//设定应用窗口录制按钮110 112
	float WindowRecordBtnWidth = 73.3 * m_Scale;
	float WindowRecordBtnHeight = 74.6 * m_Scale;
	float WindowRecordBtnX = 0.786 * WindowWidth;
	float WindowRecordBtnY = 0.359 * WindowHeight;
	m_Btn_WindowRecord.MoveWindow(
		WindowRecordBtnX, WindowRecordBtnY,
		WindowRecordBtnWidth, WindowRecordBtnHeight
	);

	//设定应用窗口录制文本
	float WindowRecordStatY = WindowRecordBtnY + WindowRecordBtnHeight + 3 * m_Scale;
	float WindowRecordStatWidth = WindowRecordBtnWidth + 35 * m_Scale;
	float WindowRecordStatX = WindowRecordBtnX + (WindowRecordBtnWidth - WindowRecordStatWidth) / 2;
	m_Stat_WindowRecord.MoveWindow(
		WindowRecordStatX, WindowRecordStatY,
		WindowRecordStatWidth, StatTextHeight
	);

	//更多按钮
	float MoreBtnWidth = 24 * m_Scale;
	float MoreBtnHeight = 72 * m_Scale;
	float MoreBtnX = WindowWidth - MoreBtnWidth - 5 * m_Scale;//最左侧5个单位的边距
	float MoreBtnY = 0.351 * WindowHeight;
	m_Btn_More.MoveWindow(
		MoreBtnX, MoreBtnY,
		MoreBtnWidth, MoreBtnHeight
	);

	//标题栏区域
	float CRect_TopWidth = WindowWidth;
	float CRect_TopHeight = 52 * m_Scale;
	float CRect_TopX = WindowRect.left;
	float CRect_TopY = WindowRect.top;
	m_Rect_Top.X = CRect_TopX;
	m_Rect_Top.Y = CRect_TopY;
	m_Rect_Top.Width = CRect_TopWidth;
	m_Rect_Top.Height = CRect_TopHeight;

	//应用窗口图标
	float AppIconWidth = 27 * m_Scale;
	float AppIconHeight = 25 * m_Scale;
	float AppIconX = 10 * m_Scale;
	float AppIconY = (m_Rect_Top.Height - AppIconHeight) / 2;
	m_Btn_AppIcon.MoveWindow(AppIconX, AppIconY, AppIconWidth, AppIconHeight);

	//极速录屏大师
	float StatLogoTextWidth = 140 * m_Scale;
	float StatLogoTextHeight = 22 * m_Scale;
	float StatLogoTextY = (m_Rect_Top.Height - StatLogoTextHeight) / 2 - 3 * m_Scale;
	float StatLogoTextX = AppIconX + AppIconWidth - 10 * m_Scale;
	m_Stat_logoText.MoveWindow(StatLogoTextX, StatLogoTextY, StatLogoTextWidth, StatLogoTextHeight);

	//用户头像
	float ProfileWidth = 12.6 * m_Scale;
	float ProfileHeight = 14 * m_Scale;
	float ProfileX = m_WindowWidth * 0.505;
	float ProfileY = (m_Rect_Top.Height - ProfileHeight) / 2;
	m_Btn_ProfileIcon.MoveWindow(ProfileX, ProfileY, ProfileWidth, ProfileHeight);

	//VIPlogo
	m_Rect_Viplogo.Width = 22 * m_Scale;
	m_Rect_Viplogo.Height = 22 * m_Scale;
	m_Rect_Viplogo.X = ProfileX - m_Rect_Viplogo.Width - 5 * m_Scale;
	m_Rect_Viplogo.Y = ProfileY + (ProfileHeight - m_Rect_Viplogo.Height) / 2;

	//登录
	CRect LoginTextRect;
	m_btn_Login.GetClientRect(LoginTextRect);
	float LoginTextWidth = LoginTextRect.Width();
	float LoginTextHeight = LoginTextRect.Height();
	float LoginTextX = ProfileX + ProfileWidth + 2 * m_Scale;
	float LoginTextY = ProfileY + (ProfileHeight - LoginTextHeight) / 2;
	m_btn_Login.MoveWindow(LoginTextX, LoginTextY, LoginTextWidth, LoginTextHeight);

	//手机号
	float PhoneBtnWidth = 100 * m_Scale;
	float PhoneBtnHeight = 20 * m_Scale;
	float PhoneBtnX = ProfileX + 15 * m_Scale;
	float PhoneBtnY = ProfileY + (ProfileHeight - PhoneBtnHeight) / 2;
	m_btn_Phone.MoveWindow(PhoneBtnX, PhoneBtnY, PhoneBtnWidth, PhoneBtnHeight);

	//开通会员
	float OpenVipWidth = 98 * m_Scale;
	float OpenVipHeight = 48 * m_Scale;
	float OpenVipX = 0.66 * m_WindowWidth;
	float OpenVipY = (m_Rect_Top.Height - OpenVipHeight) / 2 - 4 * m_Scale;
	m_Btn_OpenVIP.MoveWindow(OpenVipX, OpenVipY, OpenVipWidth, OpenVipHeight);

	//设置
	CRect ConfigBtnRect;
	m_Btn_Config.GetClientRect(ConfigBtnRect);
	float ConfigWidth = ConfigBtnRect.Width();
	float ConfigHeight = ConfigBtnRect.Height();
	float ConfigX = 0.66 * m_WindowWidth + OpenVipWidth + 8 * m_Scale;
	float ConfigY = OpenVipY + (OpenVipHeight - ConfigHeight) / 2 + 5 * m_Scale;
	m_Btn_Config.MoveWindow(ConfigX, ConfigY, ConfigWidth, ConfigHeight);

	//反馈
	CRect FeedBackRect;
	m_Btn_FeedBack.GetClientRect(FeedBackRect);
	float FeedBackWidth = FeedBackRect.Width();
	float FeedBackHeight = FeedBackRect.Height();
	float FeedBackX = ConfigX + ConfigWidth + 5 * m_Scale;
	float FeedBackY = (m_Rect_Top.Height - FeedBackHeight) / 2;
	m_Btn_FeedBack.MoveWindow(FeedBackX, FeedBackY, FeedBackWidth, FeedBackHeight);

	//最小化
	float MinimalBtnWidth = 20.6 * m_Scale * 1.25f;
	float MinimalBtnHeight = 20.6 * m_Scale * 1.25f;
	float MinimalBtnX = FeedBackX + FeedBackWidth + 10 * m_Scale;
	float MinimalBtnY = (m_Rect_Top.Height - MinimalBtnHeight) / 2;
	m_Btn_Minimal.MoveWindow(MinimalBtnX, MinimalBtnY, MinimalBtnWidth, MinimalBtnHeight);

	//关闭按钮31 29
	float CloseBtnWidth = 20.6 * m_Scale * 1.25f;
	float CloseBtnHeight = 20.6 * m_Scale * 1.25f;
	float CloseBtnX = MinimalBtnX + MinimalBtnWidth + 5 * m_Scale;
	float CloseBtnY = (m_Rect_Top.Height - CloseBtnHeight) / 2;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	//视频列表按钮93 24
	float VideoListBtnWidth = 86 * m_Scale;
	float VideoListBtnHeight = 24 * m_Scale;
	float VideoListBtnX = 15 * m_Scale;
	float VideoListBtnY = m_BoundaryLine * m_WindowHeight + (((float)1 - m_BoundaryLine) * m_WindowHeight - VideoListBtnHeight) / 2;
	m_Btn_VideoList.MoveWindow(VideoListBtnX, VideoListBtnY, VideoListBtnWidth, VideoListBtnHeight);

	//设备绑定按钮
	float DeviceBindingWidth = VideoListBtnWidth;
	float DeviceBindingHeight = VideoListBtnHeight;
	float DeviceBindingX = m_WindowWidth - DeviceBindingWidth - 10 * m_Scale;
	float DeviceBindingY = VideoListBtnY;
	if (m_Scale == 2.00)
	{
		m_Btn_DeviceBinding.MoveWindow(DeviceBindingX - 10 * m_Scale, DeviceBindingY,
			DeviceBindingWidth + 10 * m_Scale, DeviceBindingHeight);
	}
	else
	{
		m_Btn_DeviceBinding.MoveWindow(DeviceBindingX, DeviceBindingY,
			DeviceBindingWidth, DeviceBindingHeight);
	}

	//点击全屏录制，区域录制，游戏录制后出来的子界面的位置设置
	float ChildDlgY = m_Rect_Top.Height;
	float ChildDlgX = 0;
	float ChildDlgWidth = m_WindowWidth;
	float ChildDlgHeight = m_BoundaryLine * m_WindowHeight - m_Rect_Top.Height;
	m_CRect_DlgChild.SetRect(ChildDlgX, ChildDlgY, ChildDlgX + ChildDlgWidth, ChildDlgY + ChildDlgHeight);

	//{
	//	int w = 175 * m_Scale;
	//	int h = 28 * m_Scale;
	//	int x = 186 * m_Scale;
	//	int y = (m_Rect_Top.Height - h) / 2;
	//	m_Rect_MouthBill.Width = w;
	//	m_Rect_MouthBill.Height = h;
	//	m_Rect_MouthBill.X = x;
	//	m_Rect_MouthBill.Y = y;
	//	m_stat_mouthPrice.MoveWindow(
	//		x + (w - 40 * m_Scale) / 2 + 10 * m_Scale,
	//		y + 3 * m_Scale, 
	//		80 * m_Scale, 
	//		20 * m_Scale
	//	);
	//}
}

void Ui_MainDlg::InitCtrl()
{
	//全屏录制
	m_Btn_ScreenRecord.LoadPNG(MAINDLG_PNG_SCREENRECORD);
	m_Btn_ScreenRecord.LoadClickPNG(MAINDLG_PNG_SCREENRECORD_CLICK);
	m_Btn_ScreenRecord.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_ScreenRecord.SetUseHoverImage(TRUE);

	//区域录制
	m_Btn_AreaRecord.LoadPNG(MAINDLG_PNG_AREARECORD);
	m_Btn_AreaRecord.LoadClickPNG(MAINDLG_PNG_AREARECORD_CLICK);
	m_Btn_AreaRecord.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_AreaRecord.SetUseHoverImage(TRUE);

	//游戏录制
	m_Btn_GamingRecord.LoadPNG(MAINDLG_PNG_GAMINGRECORD);
	m_Btn_GamingRecord.LoadClickPNG(MAINDLG_PNG_GAMINGRECORD_CLICK);
	m_Btn_GamingRecord.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_GamingRecord.SetUseHoverImage(TRUE);

	//应用窗口录制
	m_Btn_WindowRecord.LoadPNG(MAINDLG_PNG_WINDOWRECORD);
	m_Btn_WindowRecord.LoadClickPNG(MAINDLG_PNG_WINDOWRECORD_CLICK);
	m_Btn_WindowRecord.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_WindowRecord.SetUseHoverImage(TRUE);

	//更多
	m_Btn_More.LarSetBtnNailImage(MAINDLG_PNG_MOREDOUBLELINE,
		CLarBtn::NailImageLayout::Right, 12 * m_Scale, 9 * m_Scale,
		-7 * m_Scale, 22 * m_Scale
	);
	m_Btn_More.LarSetBtnHoverNailImage(MAINDLG_PNG_MOREDOUBLELINEHOVER,
		CLarBtn::NailImageLayout::Right, 12 * m_Scale, 9 * m_Scale,
		-7 * m_Scale, 22 * m_Scale);
	m_Btn_More.LarSetTextSize(18);
	m_Btn_More.LarSetBtnTextMultLine(true);
	m_Btn_More.SetWindowTextW(L"更多");
	m_Btn_More.LarSetNormalFiilBrush(SolidBrush(Color(41, 43, 50)));
	m_Btn_More.LarSetHoverFillBrush(SolidBrush(Color(41, 43, 50)));
	m_Btn_More.LarSetClickedFillBrush(SolidBrush(Color(41, 43, 50)));
	m_Btn_More.LaSetTextHoverColor(Color(0, 198, 123));
	m_Btn_More.LaSetTextClickedColor(Color(0, 198, 123));
	m_Btn_More.LaSetTextColor(Color(223, 223, 233));
	m_Btn_More.LarAdjustTextDisplayPos(0, -8 * m_Scale);

	//应用程序图标
	m_Btn_AppIcon.LoadPNG(MAINDLG_PNG_APPICON);
	m_Btn_AppIcon.SetBackgroundColor(RGB(26, 27, 32));

	//极速录屏大师
	m_Stat_logoText.LarSetTextStyle(false, false, false);
	m_Stat_logoText.LarSetTextSize(24);

	//用户头像
	m_Btn_ProfileIcon.LoadPNG(MAINDLG_PNG_PROFILEICON);
	m_Btn_ProfileIcon.SetBackgroundColor(RGB(26, 27, 32));

	//登录
	m_btn_Login.LarSetTextSize(20);
	m_btn_Login.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_btn_Login.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_btn_Login.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_btn_Login.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_btn_Login.LarSetEraseBkEnable(false);
	m_btn_Login.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 27, 32)));

	//手机号
	m_btn_Phone.LarSetTextSize(17);
	m_btn_Phone.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_btn_Phone.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_btn_Phone.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_btn_Phone.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_btn_Phone.LarSetEraseBkEnable(false);
	m_btn_Phone.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 27, 32)));
	m_btn_Phone.ShowWindow(SW_HIDE);
	m_btn_Phone.LarSetTextCenter(false);
	m_btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);

	//设备绑定
	m_Btn_DeviceBinding.LarSetTextSize(20);
	m_Btn_DeviceBinding.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_DeviceBinding.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_DeviceBinding.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_DeviceBinding.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_DeviceBinding.LarSetEraseBkEnable(false);
	m_Btn_DeviceBinding.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 31, 37)));
	if (!App.m_IsVip)
		m_Btn_DeviceBinding.ShowWindow(SW_HIDE);
	else
		m_Btn_DeviceBinding.ShowWindow(SW_SHOW);

	//开通会员
	if (!App.m_IsVip && !App.m_isLoginIn)
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
		DB(ConsoleHandle, L"m_Btn_OpenVIP加载资源MAINDLG_PNG_OPENVIP");
	}
	else
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
		DB(ConsoleHandle, L"m_Btn_OpenVIP加载资源MAINDLG_PNG_PAYFORLONG");
	}
	m_Btn_OpenVIP.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_OpenVIP.SetHighQualityPNG(true);

	//设置
	m_Btn_Config.LarSetTextSize(20);
	m_Btn_Config.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Config.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_Config.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Config.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_Config.LarSetEraseBkEnable(false);
	m_Btn_Config.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 27, 32)));

	//反馈
	m_Btn_FeedBack.LarSetTextSize(20);
	m_Btn_FeedBack.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_FeedBack.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_FeedBack.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_FeedBack.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_FeedBack.LarSetEraseBkEnable(false);
	m_Btn_FeedBack.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 27, 32)));
	m_Btn_FeedBack.ShowWindow(SW_SHOW);

	//最小化按钮 
	m_Btn_Minimal.LoadPNG(MAINDLG_BTN_MINIMAL);
	m_Btn_Minimal.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Minimal.SetHoverEffectColor(30, 255, 255, 200);
	m_Btn_Minimal.SetStretchMode(0.90f);

	//关闭按钮
	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Close.SetHoverEffectColor(30, 255, 255, 200);
	m_Btn_Close.SetStretchMode(0.75f);

	//视频列表
	m_Btn_VideoList.LarSetTextSize(20);
	m_Btn_VideoList.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_VideoList.LaSetTextHoverColor(Gdiplus::Color(245, 245, 245, 245));
	m_Btn_VideoList.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_VideoList.LarSetBorderColor(Gdiplus::Color(255, 26, 31, 37));
	m_Btn_VideoList.LarSetEraseBkEnable(false);
	m_Btn_VideoList.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 31, 37)));
	m_Btn_VideoList.LarSetHoverFillBrush(SolidBrush(Color(255, 26, 31, 37)));
	m_Btn_VideoList.LarSetClickedFillBrush(SolidBrush(Color(255, 26, 31, 37)));
	m_Btn_VideoList.LarSetBtnNailImage(MAINDLG_BTN_VIDEOLIST, 
		CLarBtn::NailImageLayout::Left, 16 * m_Scale, 15 * m_Scale, 
		0, 0
	);
	m_Btn_VideoList.LarAdjustTextDisplayPos(8 * m_Scale, 0);

	//录制文本设置
	m_Stat_ScreenRecord.LarSetTextSize(20);//全屏录制文本
	m_Stat_ScreenRecord.LarSetTextStyle(false, false, false);
	m_Stat_AreaRecord.LarSetTextSize(20);//区域录制文本
	m_Stat_AreaRecord.LarSetTextStyle(false, false, false);
	m_Stat_WindowRecord.LarSetTextSize(20);//应用窗口录制文本
	m_Stat_WindowRecord.LarSetTextStyle(false, false, false);
	m_Stat_GamingRecord.LarSetTextSize(20);//游戏录制文本
	m_Stat_GamingRecord.LarSetTextStyle(false, false, false);

	//m_stat_mouthPrice.LarSetTextColor(RGB(255, 81, 80));
	//m_stat_mouthPrice.LarSetTextSize(20);//月度会员价格
	//m_stat_mouthPrice.LarSetTextStyle(true, false, false);
	//std::wstring mouthPriceStr = L"￥" + std::to_wstring(App.m_CouponPrice.amount);
	//m_stat_mouthPrice.SetWindowTextW(mouthPriceStr.c_str());
	//if (!App.m_IsVip && App.m_IsHasOpenVip && App.m_IsNeedShowMouthBill)
	//{
	//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
	//}
	//else
	//{
	//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
	//}

}

void Ui_MainDlg::SetPhoneBtnImageUp()
{
	m_btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_UP, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);
	Invalidate(false);
}

void Ui_MainDlg::ResetPhoneBtnImage()
{
	m_btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);
	Invalidate(false);
}

void Ui_MainDlg::OnPaint()
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

	//渲染标题栏背景和绘画客户区背景
	SolidBrush CaptionBrush(Color(26, 27, 32));
	SolidBrush ClientBrush(Color(26, 31, 37));
	memGraphics.FillRectangle(&CaptionBrush, m_Rect_Top);//绘画标题栏背景
	memGraphics.FillRectangle(//绘画客户区背景
		&ClientBrush,
		m_Rect_Top.GetLeft(),
		m_Rect_Top.GetBottom(),
		static_cast<INT>(m_WindowWidth),
		static_cast<INT>(m_WindowHeight - m_Rect_Top.GetBottom())
	);

	//渲染下方边界线
	SolidBrush LineBrush(Color(3, 3, 3));
	Pen LinePen(&LineBrush);
	INT py = m_WindowY + m_WindowHeight * m_BoundaryLine;
	Point p1(m_WindowX, py);
	Point p2(m_WindowX + m_WindowWidth, py);
	memGraphics.DrawLine(&LinePen, p1, p2);

	if ((App.m_isLoginIn && App.m_IsVip) || App.m_IsNonUserPaid)
	{
		memGraphics.DrawImage(m_Bitmap_VipLogo, m_Rect_Viplogo);
	}

	//呈现
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));
	DB(ConsoleHandle, L"Ui_MainDlg:repaint..");
}

//需要在GDI+关闭前调用，清理所有的GDI导入的PNG资源
void Ui_MainDlg::CleanUpGdiPngRes()
{
	//清理子对话框GDI资源
	if (m_Dlg_Child)m_Dlg_Child->CleanUpGdiPngRes();
	if (m_Dlg_WindowRecord)m_Dlg_WindowRecord->CleanUpGdiPngRes();
	if (m_Dlg_Videolist)m_Dlg_Videolist->CleanUpGdiPngRes();
	if (m_Dlg_Config)m_Dlg_Config->CleanUpGdiPngRes();
	if (m_Dlg_Gaming)m_Dlg_Gaming->CleanUpGdiPngRes();
	if (m_Dlg_Login)m_Dlg_Login->CleanUpGdiPngRes();

	//清理本对话框GDI资源
	m_Btn_ScreenRecord.ClearImages();
	m_Btn_AreaRecord.ClearImages();
	m_Btn_GamingRecord.ClearImages();
	m_Btn_WindowRecord.ClearImages();
	m_Btn_AppIcon.ClearImages();
	m_Btn_ProfileIcon.ClearImages();
	m_Btn_OpenVIP.ClearImages();
	m_Btn_Minimal.ClearImages();
	m_Btn_Close.ClearImages();
}

void Ui_MainDlg::InitializeApp()
{
}

void Ui_MainDlg::PreCreateAllChildDialogs()
{
	DEBUG_CONSOLE_STR(ConsoleHandle, L"开始预创建所有子对话框...");
	DWORD startTime = GetTickCount();

	// 获取窗口矩形一次，供所有子对话框使用，避免重复调用
	CRect WindowRect;
	GetWindowRect(WindowRect);

	// 预创建 ChildDlg (全屏录制、区域录制等使用的对话框)
	if (!m_Dlg_Child)
	{
		m_Dlg_Child = new Ui_ChildDlg;
		m_Dlg_Child->SetDlgDisplayRect(m_CRect_DlgChild);
		if (!m_Dlg_Child->Create(IDD_DIALOG_CHILDDIALOG, this))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建子界面失败");
			delete m_Dlg_Child;
			m_Dlg_Child = nullptr;
		}
		else
		{
			m_Dlg_Child->MoveWindow(m_CRect_DlgChild);
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建子界面成功");
		}
	}

	// 预创建 ConfigDlg (设置界面)
	if (!m_Dlg_Config)
	{
		int windowWidth = 696 * m_Scale;
		int windowHeight = 686 * m_Scale / 3 * 2;
		CRect configRect = WindowRect;
		configRect.top = configRect.top + (configRect.Height() - windowHeight) / 2;
		configRect.left = configRect.left + (configRect.Width() - windowWidth) / 2;
		configRect.right = configRect.left + windowWidth;
		configRect.bottom = configRect.top + windowHeight;

		m_Dlg_Config = new Ui_ConfigDlg;
		m_Dlg_Config->LoadConfigFromFile();
		m_Dlg_Config->Ui_SetWindowRect(configRect);
		if (!m_Dlg_Config->Create(IDD_DIALOG_CONFIGDLG, this))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建设置界面失败");
			delete m_Dlg_Config;
			m_Dlg_Config = nullptr;
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建设置界面成功");
		}
	}

	// 预创建 VideoListDlg (视频列表界面)
	if (!m_Dlg_Videolist)
	{
		m_Dlg_Videolist = new Ui_VideoListDlg;
		if (!m_Dlg_Videolist->Create(IDD_DIALOG_VIDEOLISTDLG, this))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建视频列表界面失败");
			delete m_Dlg_Videolist;
			m_Dlg_Videolist = nullptr;
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建视频列表界面成功");
		}
		CRect VideoListRect;
		VideoListRect = WindowRect;
		VideoListRect.bottom += 150 * m_Scale;
		m_Dlg_Videolist->Ui_UpdateWindowPos(VideoListRect);
	}

	// 预创建 GamingDlg (游戏录制界面)
	if (!m_Dlg_Gaming)
	{
		int windowWidth = 789 * m_Scale;
		int windowHeight = 474 * m_Scale;
		CRect gamingRect = WindowRect;
		gamingRect.top -= 180 * m_Scale;
		gamingRect.right = gamingRect.left + windowWidth;
		gamingRect.bottom = gamingRect.top + windowHeight;

		m_Dlg_Gaming = new Ui_GamingDlg;
		m_Dlg_Gaming->Ui_SetWindowRect(gamingRect);
		if (!m_Dlg_Gaming->Create(IDD_DIALOG_GAMINGDLG, NULL))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建游戏录制界面失败");
			delete m_Dlg_Gaming;
			m_Dlg_Gaming = nullptr;
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建游戏录制界面成功");
		}
	}

	// 预创建 CameraDlg (摄像头录制界面)
	if (!m_Dlg_Carmera)
	{
		int windowWidth = 771 * m_Scale;
		int windowHeight = 628 * m_Scale;
		CRect cameraRect = WindowRect;
		cameraRect.top -= 180 * m_Scale;
		cameraRect.right = cameraRect.left + windowWidth;
		cameraRect.bottom = cameraRect.top + windowHeight;

		m_Dlg_Carmera = new Ui_CameraDlg;
		m_Dlg_Carmera->Ui_SetWindowRect(cameraRect);
		if (!m_Dlg_Carmera->Create(IDD_DIALOG_CAMERADLG, this))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建摄像头录制界面失败");
			delete m_Dlg_Carmera;
			m_Dlg_Carmera = nullptr;
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"预创建摄像头录制界面成功");
		}
	}

	// 预创建 TopRecDlg (顶部录制小窗口)
	if (!m_Dlg_RecTopDlg)
	{
		m_Dlg_RecTopDlg = new Ui_RecTopDlg;
		m_Dlg_RecTopDlg->Create(IDD_DIALOG_RECTOPWINDOW, NULL);
	}

	// 预创建用户信息Ui_UserProfileDlg(点击用户名出现的小窗口)
	if (!m_Dlg_UserPrifile)
	{
		m_Dlg_UserPrifile = new Ui_UserProfileDlg;
		m_Dlg_UserPrifile->Create(IDD_DIALOG_USERPROFILE, this);
	}

	auto pIns = DialogManager::GetInstance();
	pIns->RegisterDialog(this);
	pIns->RegisterDialog(m_Dlg_Gaming);
	pIns->RegisterDialog(m_Dlg_Carmera);
	pIns->RegisterDialog(m_Dlg_RecTopDlg);

	DWORD endTime = GetTickCount();
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"预创建所有非模态窗口完成，耗时 %d 毫秒", endTime - startTime);
}

void Ui_MainDlg::ShowLoginInUi()
{
	if (!App.m_IsVip)
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
	else
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	if (App.m_IsOverBinds  && App.m_IsVip)
		m_Btn_DeviceBinding.ShowWindow(SW_SHOW);
	//if (!App.m_IsVip && App.m_IsHasOpenVip)
	//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
	//else
	//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
	m_btn_Login.ShowWindow(SW_HIDE);
	m_btn_Phone.ShowWindow(SW_SHOW);
	m_btn_Phone.SetWindowTextW(App.m_userInfo.nickname);
	Invalidate(false);
}

void Ui_MainDlg::ShowSignOutUi()
{
	if (!App.m_IsVip && App.m_IsHasOpenVip)
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	else
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	}

	//if(App.m_IsNeedShowMouthBill)
	//{
	//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
	//}
	//else
	//{
	//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
	//}
	m_btn_Login.ShowWindow(SW_SHOW);
	m_btn_Phone.ShowWindow(SW_HIDE);
	m_Btn_DeviceBinding.ShowWindow(SW_HIDE);
	Invalidate(false);
}

void Ui_MainDlg::CloseAreaRecordSDL()
{
	DB(ConsoleHandle, L"m_Thread_SDLWindow.get()");
	if (m_Thread_SDLWindow)
	{
		if (m_Thread_SDLWindow->joinable())
		{
			m_Thread_SDLWindow->join();
		}
		m_Thread_SDLWindow.reset();
	}
	DB(ConsoleHandle, L"m_Thread_SDLWindow.get() over");
	if (Ui_AreaRecordingSDL::IsInstanceExist())
	{
		DB(ConsoleHandle, L"Ui_AreaRecordingSDL::IsInstanceExist() in");
		Ui_AreaRecordingSDL::GetInstance()->ReleaseInstance();
		DB(ConsoleHandle, L"Ui_AreaRecordingSDL::GetInstance()->ReleaseInstance() over");
	}
}

void Ui_MainDlg::SetNeedReduceBinds(bool IsNeedReduceBinds)
{
	m_Bool_IsNeedReduceBinds = IsNeedReduceBinds;
	if (m_Bool_IsNeedReduceBinds)
	{//需要提示用户解绑设备
		SetTimer(TIMER_MAINDLG_NEEDREDUCEBINDS, 1500, NULL);//1.5秒后触发
	}
}

void Ui_MainDlg::OnBnClickedBtnScreenRecord()
{
	//隐藏子界面区域的所有控件
	m_Btn_ScreenRecord.ShowWindow(SW_HIDE);
	m_Btn_AreaRecord.ShowWindow(SW_HIDE);
	m_Btn_GamingRecord.ShowWindow(SW_HIDE);
	m_Btn_WindowRecord.ShowWindow(SW_HIDE);
	m_Btn_More.ShowWindow(SW_HIDE);
	m_Stat_ScreenRecord.ShowWindow(SW_HIDE);
	m_Stat_AreaRecord.ShowWindow(SW_HIDE);
	m_Stat_GamingRecord.ShowWindow(SW_HIDE);
	m_Stat_WindowRecord.ShowWindow(SW_HIDE);
	HideUserProfile();
	Invalidate();
	//显示子界面
	if (!m_Dlg_Child) 
	{
		m_Dlg_Child = new Ui_ChildDlg;
		m_Dlg_Child->SetDlgDisplayRect(m_CRect_DlgChild);
		m_Dlg_Child->Create(IDD_DIALOG_CHILDDIALOG, this);
		m_Dlg_Child->MoveWindow(m_CRect_DlgChild);
	}
	m_Dlg_Child->SetForegroundWindow();
	m_Dlg_Child->ShowWindow(SW_SHOW);

	DEBUG_CONSOLE_FMT(ConsoleHandle, L"Dlg_Child:%d,%d,%d,%d",
		(int)m_CRect_DlgChild.left, (int)m_CRect_DlgChild.top,
		(int)m_CRect_DlgChild.right, (int)m_CRect_DlgChild.bottom);

	//显示顶部录制小窗口
	bool IsSDLAreaRecord = false;
	m_Dlg_Child->GetRecordMode(nullptr, &IsSDLAreaRecord, nullptr);
	if (m_Dlg_RecTopDlg && !IsSDLAreaRecord)
	{
		m_Dlg_RecTopDlg->ShowWindow(SW_SHOW);
		m_Dlg_RecTopDlg->SetRecordContext_Child();
	}
	if (m_Dlg_Child->m_IsMouseAreaRecord)
		m_Dlg_Child->SetRecordOptBtnText(L"跟随鼠标");
	else if(m_Dlg_Child->m_IsOnlyAudioRecord)
		m_Dlg_Child->SetRecordOptBtnText(L"录声音");
	else
		m_Dlg_Child->SetRecordOptBtnText(L"录全屏");
}

void Ui_MainDlg::OnBnClickedBtnAppwindowrecording()
{
	//定义窗口显示位置和大小
	CRect WindowRect;
	GetWindowRect(WindowRect);
	float Width = 788 * m_Scale;
	float Height = 662 * m_Scale;
	WindowRect.top -= 180 * m_Scale;
	WindowRect.right = WindowRect.left + Width;
	WindowRect.bottom = WindowRect.top + Height;
	//创建窗口并显示
	if (!m_Dlg_WindowRecord) {
		m_Dlg_WindowRecord = new Ui_WindowRecordDlg;
		m_Dlg_WindowRecord->Ui_SetWindowRect(WindowRect);//设置窗口大小和位置
		m_Dlg_WindowRecord->Create(IDD_DIALOG_WINDOWRECORD, this);
	}
	this->ShowWindow(SW_HIDE);
	m_Dlg_UserPrifile->ShowWindow(SW_HIDE);
	m_Dlg_WindowRecord->Ui_UpdateWindowPos(WindowRect);
	m_Dlg_WindowRecord->SetForegroundWindow();
	m_Dlg_WindowRecord->ShowWindow(SW_SHOW);
	HideUserProfile();
	HideVideoList();
	m_VideoListVisible = false;
	m_Shadow.Show(m_hWnd);

	if (m_Dlg_RecTopDlg)
	{
		m_Dlg_RecTopDlg->SetRecordContext_Window(); //切换上下文为窗口录制
		m_Dlg_RecTopDlg->ShowWindow(SW_SHOW);
	}
}

LRESULT Ui_MainDlg::On_UiProfile_SignOut(WPARAM lp, LPARAM wp)
{
	bool logoutSuccess = App.RequestSignOut();
	if (logoutSuccess)
	{
		if (!App.RequestDeviceInfo())
		{
			DB(ConsoleHandle, L"设备信息获取有异常，user可能为空");
		}

		DEBUG_CONSOLE_STR(ConsoleHandle, L"退出登录请求成功，更新UI状态");
		ShowSignOutUi();
		m_Dlg_Gaming->UpdateLogOutUi();
		m_Dlg_Carmera->UpdateSignOutUi();
		App.m_isLoginIn = false;
		
		//写入自动登录配置
		ConfigFileHandler ConfigHandler(App.m_ConfigPath);
		if (!ConfigHandler.WriteConfigFile("AppConfig", "AutoLogin", "0"))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"警告！写入自动登录参数到配置文件失败");
		}
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"退出登录请求失败，保持当前状态");
	}
	return 1;
}

LRESULT Ui_MainDlg::On_UiProfile_ShowLogoutSuccessDlg(WPARAM lp, LPARAM wp)
{
	ModalDlg_MFC::ShowModal_SignOutSuccess();
	return LRESULT();
}

LRESULT Ui_MainDlg::On_UiProfile_OpenVip(WPARAM lp, LPARAM wp)
{
	if (m_Dlg_UserPrifile && m_Dlg_UserPrifile->IsWindowVisible())
	{
		HideUserProfile();
	}
	this->OnBnClickedBtnOpenvip();
	return LRESULT();
}

LRESULT Ui_MainDlg::On_UiOpenVip_VipPrivilegeContrast(WPARAM lp, LPARAM wp)
{
	if (m_Dlg_VipPay)
	{
		m_Dlg_VipPay->ShowWindow(SW_HIDE);
		::SetWindowPos(m_Dlg_VipPay->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	ModalDlg_SDL::ShowModal_Rai(this, m_Scale);
	if (m_Dlg_VipPay)
	{
		m_Dlg_VipPay->ShowWindow(SW_SHOW);
	}
	return LRESULT();
}

LRESULT Ui_MainDlg::OnUi_ProfileDlg_WindowHidenByTimer(WPARAM lp, LPARAM wp)
{
	HideUserProfile();
	return LRESULT();
}

void Ui_MainDlg::OnBnClickedBtnOpenvip()
{
	if (!this->IsWindowVisible())
		this->ShowWindow(SW_SHOW);
	//if (!App.m_isLoginIn)
	//{
	//	OnBnClickedBtnLogin();
	//}
	//if (!App.m_isLoginIn)
	//	return;
	m_Dlg_Child->HideListBox();
	HideUserProfile();
	::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// 定义窗口显示位置和大小
	CRect WindowRect;
	GetWindowRect(WindowRect);
	float Width = 782 * m_Scale;
	float Height = 570 * m_Scale;
	WindowRect.top = WindowRect.top + (WindowRect.Height() - Height) / 2;
	WindowRect.left = WindowRect.left + (WindowRect.Width() - Width) / 2;
	WindowRect.right = WindowRect.left + Width;
	WindowRect.bottom = WindowRect.top + Height;

	// 创建模态对话框
	if (m_Dlg_VipPay != nullptr)
	{// 确保删除旧对象，避免内存泄漏
		delete m_Dlg_VipPay;
		m_Dlg_VipPay = nullptr;
	}

	m_Dlg_VipPay = new Ui_VipPayDlg(this);
	m_Dlg_VipPay->Ui_SetWindowPos(WindowRect);  // 设置窗口大小和位置
	if (App.m_Dlg_Main.m_Dlg_WindowRecord &&
		::IsWindowVisible(App.m_Dlg_Main.m_Dlg_WindowRecord->GetSafeHwnd()))
	{
		this->ShowWindow(SW_HIDE);
	}
	if (m_Dlg_Videolist->IsWindowVisible())
	{
		m_Dlg_Videolist->ShowWindow(SW_HIDE);
	}

	INT_PTR result = m_Dlg_VipPay->DoModal();
	if (result == IDOK)
	{ 
		// 支付成功，获取用户信息
		App.UserPay(true);
		DEBUG_CONSOLE_STR(ConsoleHandle, L"支付成功，正在获取用户最新信息...");
		//if (!App.RequestToken(App.m_appToken))
		//{
		//	DB(ConsoleHandle, L"获取用户信息失败");
		//}
		if (!App.RequestDeviceInfo())
		{
			DB(ConsoleHandle, L"获取用户信息失败");
		}
		DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
		m_Btn_OpenVIP.LoadClickPNG(MAINDLG_PNG_PAYFORLONG);
		//if (!App.m_IsVip && App.m_IsHasOpenVip)
		//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
		//else
		//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
		if (!App.m_IsOverBinds)
			m_Btn_DeviceBinding.ShowWindow(SW_HIDE);
		else
			m_Btn_DeviceBinding.ShowWindow(SW_SHOW);
		Invalidate(false);
		ModalDlg_MFC::ShowModal_PaySuccess(this);
	}
	else if (result == IDCANCEL)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"用户取消了支付");
		App.UserCloseOpenVip();
		//if (!App.m_IsVip && App.m_IsHasOpenVip)
		//{
		//	std::wstring mouthPriceStr = L"￥" + std::to_wstring(m_Dlg_VipPay->getMouthBillPrice());
		//	m_stat_mouthPrice.SetWindowTextW(mouthPriceStr.c_str());
		//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
		//}
	}
	//if (App.m_IsNeedShowMouthBill)
	//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
	//else
	//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
	Invalidate(FALSE);
}

LRESULT Ui_MainDlg::On_BroadCast_UserLogin(WPARAM lp, LPARAM wp)
{
	ShowLoginInUi();
	return 1;
}

void Ui_MainDlg::UpdateUserInfoDisplay()
{

}

void Ui_MainDlg::CloseSDLAreaRecordWindow()
{
	if (Ui_AreaRecordingSDL::IsInstanceExist())
	{
		Ui_AreaRecordingSDL::GetInstance()->SetRunningState(false);
		if (m_Thread_SDLWindow.get()->joinable())
			m_Thread_SDLWindow.get()->join();
		Ui_AreaRecordingSDL::GetInstance()->ReleaseInstance();
	}
}

void Ui_MainDlg::CreateSDLAreaRecordWindow(const CRect WindowRect, bool IsRecording)
{
	// 创建新的SDL线程
	if (m_Thread_SDLWindow)
	{
		// 尝试等待线程结束
		if (m_Thread_SDLWindow->joinable())
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待之前的SDL线程结束");
			m_Thread_SDLWindow->join();
		}
		// 重置线程指针
		m_Thread_SDLWindow.reset();
	}
	m_Thread_SDLWindow = std::make_unique<std::thread>([=]()
		{
			// 设置线程异常处理
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: 创建SDL录制窗口");

			// 创建SDL窗口实例
			Ui_AreaRecordingSDL* sdlWindow = Ui_AreaRecordingSDL::GetInstance();

			// 初始化和运行窗口
			if (sdlWindow->Initialize(WindowRect))
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口初始化成功，开始运行");
				sdlWindow->Run();
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口已关闭");
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]: SDL窗口初始化失败");
			}
		});
	m_Thread_SDLWindow->detach();	// 分离线程让它独立运行

}

void Ui_MainDlg::OnBnClickedBtnVideolist()
{
	m_Dlg_Child->HideListBox();
	HideUserProfile();
	if (m_VideoListVisible)
		HideVideoList();
	else
		ShowVideoList();
}

void Ui_MainDlg::OnBnClickedBtnConfig()
{
	m_Dlg_Child->HideListBox();
	// 定义窗口显示位置
	int windowWidth = 696 * m_Scale;
	int windowHeight = 686 * m_Scale / 3 * 2;
	CRect WindowRect;
	GetWindowRect(WindowRect);
	WindowRect.top = WindowRect.top + (WindowRect.Height() - windowHeight) / 2;
	WindowRect.left = WindowRect.left + (WindowRect.Width() - windowWidth) / 2;
	WindowRect.right = WindowRect.left + windowWidth;
	WindowRect.bottom = WindowRect.top + windowHeight;

	// 初始化所有窗口
	if (!m_Dlg_Config)
	{
		m_Dlg_Config = new Ui_ConfigDlg;
		m_Dlg_Config->Ui_SetWindowRect(WindowRect);
		if (!m_Dlg_Config->Create(IDD_DIALOG_CONFIGDLG, this))
		{
			delete m_Dlg_Config;
			m_Dlg_Config = nullptr;
			return;
		}
	}
	//m_Shadow.HideShadow();
	//if (m_Dlg_Videolist->IsWindowVisible())
	//	m_Dlg_Videolist->HideShadow();

	this->EnableWindow(FALSE);// 禁用主窗口以模拟模态行为
	// 更新位置并显示配置窗口
	m_Dlg_Config->Ui_UpdateWindowPos(WindowRect);
	m_Dlg_Config->SetForegroundWindow();
	m_Dlg_Config->ShowWindow(SW_SHOW);
	//m_Dlg_Config->OnBnClickedBtnFolderconfig();
	
	m_Dlg_Config->SetTimer(Ui_ConfigDlg::TIMER_DELAYED_REDRAW, 50, NULL);
	HideUserProfile();
	Invalidate();
}

void Ui_MainDlg::OnBnClickedBtnGamingrecording()
{
	// 定义窗口显示位置
	int windowWidth = 789 * m_Scale;
	int windowHeight = 474 * m_Scale;
	CRect WindowRect, GamingRect;
	GetWindowRect(WindowRect);
	GamingRect.left = WindowRect.left + (WindowRect.Width() - windowWidth) / 2;
	GamingRect.top = WindowRect.top + (WindowRect.Height() - windowHeight) / 2;
	GamingRect.right = GamingRect.left + windowWidth;
	GamingRect.bottom = GamingRect.top + windowHeight;

	// 创建窗口并显示
	if (m_Dlg_Gaming)
	{
		m_Dlg_Gaming->DestroyWindow();
		delete m_Dlg_Gaming;
		m_Dlg_Gaming = nullptr;
	}
	if (!m_Dlg_Gaming)
	{
		m_Dlg_Gaming = new Ui_GamingDlg;
		m_Dlg_Gaming->Ui_SetWindowRect(GamingRect);
		if (!m_Dlg_Gaming->Create(IDD_DIALOG_GAMINGDLG, NULL))
		{
			delete m_Dlg_Gaming;
			m_Dlg_Gaming = nullptr;
			return;
		}
	}
	if (App.m_isLoginIn)
		m_Dlg_Gaming->ShowLoginUi();
	if (App.m_IsNonUserPaid)
		m_Dlg_Gaming->ShowNoneUserPayUi();
	this->ShowWindow(SW_HIDE);
	m_Dlg_Gaming->Ui_UpdateWindowPos(GamingRect);
	m_Dlg_Gaming->SetForegroundWindow();
	m_Dlg_Gaming->RegGameDetector();
	m_Dlg_Gaming->ShowWindow(SW_SHOW);
	HideUserProfile();
	if (m_VideoListVisible)
		HideVideoList();
	m_Shadow.Show(m_hWnd);
}

void Ui_MainDlg::OnBnClickedBtnArearecording()
{
	HideUserProfile();
	if (!App.m_IsVip)
	{
		if (!ModalDlg_SDL::ShowModal_Rai(this, m_Scale))
		{
			//点击了开通会员
			App.m_Dlg_Main.OnBnClickedBtnOpenvip();
			return;
		}
	}

	// 创建区域选择对话框
	DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 显示区域选择对话框");
	SetForegroundWindow();
	AreaSelectDlg ModelDlg_AreaSelect;	//区域选择功能窗口 
	if (ModelDlg_AreaSelect.DoModal() == IDOK)
	{
		m_Shadow.Show(m_hWnd);
		// 选择完成，保存区域
		m_CRect_SelectArea = ModelDlg_AreaSelect.GetSelectRect();
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]: 区域选择完成，选择的矩形参数:%d,%d,%d,%d",
			m_CRect_SelectArea.left, m_CRect_SelectArea.top, m_CRect_SelectArea.Width(), m_CRect_SelectArea.Height());
		CreateSDLAreaRecordWindow(m_CRect_SelectArea, false);

		//构造实际录制区域
		CRect RecordRect;
		int PanelHeight = 50;
		int RedLineWidth = 2;
		RecordRect = m_CRect_SelectArea;
		RecordRect.top += PanelHeight + RedLineWidth;
		RecordRect.left += RedLineWidth;
		RecordRect.bottom -= RedLineWidth;
		RecordRect.right -= RedLineWidth;
		m_Dlg_Child->UpdateAreaRecordRect(RecordRect);
		OnBnClickedBtnScreenRecord();//ChildDlg界面显示
	}
	else
	{// 取消选择
		DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 取消选择区域");
	}
	m_Dlg_Child->SetRecordOptBtnText(L"录区域");
}

LRESULT Ui_MainDlg::On_SDLAreaReocrd_StartRecord(WPARAM wParam, LPARAM lParam)
{
	//获取录制接口单例，开始录制
	auto RecordRect = (CRect*)lParam;
	Ui_AreaRecordingSDL::GetInstance()->SetInteractionEnable(false);
	Ui_AreaRecordingSDL::GetInstance()->ShowSDLWindow();
	RecordingParams recordp = CollectRecordingParams();
	this->ShowWindow(SW_MINIMIZE);
	App.m_Dlg_Main.HideVideoList();
	ModalDlg_SDL::ShowModal_CountDown(3, "即将开始录制", [=]()
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"倒计时结束");
		}, true);
	Ui_AreaRecordingSDL::GetInstance()->ShowSDLWindow();
	Ui_AreaRecordingSDL::GetInstance()->SetDashedBorder(true);
	Ui_AreaRecordingSDL::GetInstance()->SetUiModeDuringRecord();

	ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
	pSRIns->SetSystemAudioVolume(recordp.systemAudioVolume);
	pSRIns->SetMicroVolume(recordp.microAudioVolume);
	pSRIns->SetVideoEncoder(GlobalFunc::MapUICodecToEncoderName(recordp.codecText.c_str()));
	pSRIns->SetAudioCaptureDevice(recordp.audioDevice.c_str());
	pSRIns->SetMicroDeviceName(recordp.microDevice.c_str());
	pSRIns->SetRecordMouse(m_Dlg_Config->m_Bool_IsDisplayMouse);
	if (App.m_Dlg_Main.m_Dlg_Config->m_Bool_IsWaterOn)
		pSRIns->SetVideoTextFilter(L"极速录屏大师", "40", "FFFFFF", recordp.logoPath);//设置水印
	pSRIns->SetAreaRecordParam(
		RecordRect->left, RecordRect->top, RecordRect->right, RecordRect->bottom,
		recordp.videoResolution,
		recordp.videoQuality,
		recordp.videoFormat,
		recordp.encodePreset,
		recordp.recordMode,
		recordp.audioSampleRate,
		recordp.audioBitRate,
		recordp.fps
	);
	DBFMT(ConsoleHandle, L"设置录制区域:left%d,top%d,right%d,bottom%d",
		RecordRect->left, RecordRect->top, RecordRect->right, RecordRect->bottom);


	//开始录制 
	CT2A outputfileName(recordp.outputFilePath.c_str());
	std::string filenameStr(outputfileName);
	std::thread([=]() {pSRIns->startRecording(filenameStr.c_str()); }).detach();
	//if (true)
	{   //设置录制按钮为显示“正在录制”
		m_Dlg_Child->m_IsRecording = true;
		m_Dlg_Child->m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_ISRECORDING);
		m_Dlg_Child->initRecordingCtrlPos();
		m_Dlg_Child->SetRecordCountTimer();
		m_Dlg_Child->Invalidate(false);
		App.m_Dlg_Main.m_closeManager.get()->SetTrayDoubleClickCallback([this]()
			{
				this->ShowWindow(SW_RESTORE);
				this->ShowWindow(SW_SHOW);
			});
		if ((!App.m_IsVip || !App.m_isLoginIn) && !App.m_IsNonUserPaid)
			SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, NULL);
		if (App.m_IsOverBinds && !App.m_IsNonUserPaid)//如果超限，1分钟后触发
			SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, NULL);
		Ui_AreaRecordingSDL::GetInstance()->SetInteractionEnable(true);
	}
	delete RecordRect;
	return 1;
}

LRESULT Ui_MainDlg::On_SDLAreaReocrd_StopRecord(WPARAM wParam, LPARAM lParam)
{
	return 1;
}

LRESULT Ui_MainDlg::On_SDLAreaReocrd_CloseWindow(WPARAM wParam, LPARAM lParam)
{
	DB(ConsoleHandle, L"On_SDLAreaReocrd_CloseWindow");
	m_Dlg_Child->KillTimer(TIMER_NONEVIP_RECORDTIME);
	KillTimer(TIMER_NONEVIP_RECORDTIME);
	DB(ConsoleHandle, L"ScreenRecorder::IsRecording()");
	bool IsRecording = ScreenRecorder::IsRecording();
	if (IsRecording)
	{
		DB(ConsoleHandle, L"ScreenRecorder释放");
		ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
		if (pSRIns->isPausing())//如果处于暂停状态，则退出暂停
			pSRIns->ResumeRecording();
		pSRIns->stopRecording();
		pSRIns->ReleaseInstance();
		DB(ConsoleHandle, L"ScreenRecorder释放完成");
	}
	else
	{
		m_Dlg_Child->OnBnClickedBtnReturn();
		return 1;
	}
	DB(ConsoleHandle, L"Ui_AreaRecordingSDL::GetInstance()->ResumeUiModeFromRecord()");
	Ui_AreaRecordingSDL::GetInstance()->ResumeUiModeFromRecord();
	DB(ConsoleHandle, L"Ui_AreaRecordingSDL::GetInstance()->ResumeUiModeFromRecord() over");
	m_Dlg_Child->m_CRect_RecordRect.SetRectEmpty();
	m_Dlg_Child->m_IsRecording = false;
	m_Dlg_Child->m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_STARTRECORD);
	m_Dlg_Child->Invalidate(false);
	m_Dlg_Child->OnBnClickedBtnReturn();
	COMAPI::MFC::SetWindowShowOnTop(this->GetSafeHwnd());
	this->ShowWindow(SW_SHOW);
	this->ShowWindow(SW_RESTORE);
	m_Dlg_Videolist->AddVideoToList(m_RecordP.outputFilePath.c_str());
	ShowVideoList();
	DB(ConsoleHandle, L"CloseAreaRecordSDL");
	CloseAreaRecordSDL();
	DB(ConsoleHandle, L"CloseAreaRecordSDL over");
	m_Dlg_Child->CloseCarmeraPreview();
	if (IsRecording)
		ModalDlg_MFC::ShowModal_Priview(this);

	DB(ConsoleHandle, L"On_SDLAreaReocrd_CloseWindow over");
	return 1;
}

LRESULT Ui_MainDlg::On_SDLAreaReocrd_PauseRecord(WPARAM wParam, LPARAM lParam)
{
	m_Dlg_Child->OnBnClickedBtnPause();
	return LRESULT();
}

LRESULT Ui_MainDlg::On_SDLAreaReocrd_ResumeRecord(WPARAM wParam, LPARAM lParam)
{
	m_Dlg_Child->OnBnClickedBtnResume();
	return LRESULT();
}

void Ui_MainDlg::On_ModalBnClick_Prview()
{
	const std::wstring& fullPath = m_RecordP.outputFilePath;
	HINSTANCE result = ::ShellExecute(
		this->GetSafeHwnd(),
		L"open",
		fullPath.c_str(),
		nullptr,
		nullptr,
		SW_SHOWNORMAL
	);

	if ((INT_PTR)result <= 32)
	{
		// 创建完整命令行，打开文件夹并选中文件
		CString command = L"/select,\"" + CString(fullPath.c_str()) + L"\"";

		// 使用explorer打开文件夹并选中文件
		HINSTANCE result = ShellExecute(
			NULL,                   // 父窗口句柄
			L"open",                // 操作
			L"explorer.exe",        // 资源管理器
			command,                // 参数 - 选中指定文件
			NULL,                   // 默认目录
			SW_SHOWNORMAL           // 显示命令
		);

		// 检查执行结果
		if ((INT_PTR)result <= 32)
		{
			ModalDlg_MFC::ShowModal_OpenVideoFailed();
		}
	}
	return;
}

void Ui_MainDlg::UpdateRecentRecordFile(const CString& filePath)
{
	m_RecordP.outputFilePath = filePath;
}

LRESULT Ui_MainDlg::On_UiRecordArea_WindowChanged(WPARAM wParam, LPARAM lParam)
{
	//更新录制区域参数
	CRect* UpdatedRecordArea = (CRect*)lParam;
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"录制区域更新，新的录制区域:%d,%d,%d,%d",
		UpdatedRecordArea->left, UpdatedRecordArea->top, UpdatedRecordArea->Width(), UpdatedRecordArea->Height());
	ScreenRecorder::GetInstance()->UpdateRecordArea(UpdatedRecordArea);
	delete UpdatedRecordArea;
	return 1;
}

LRESULT Ui_MainDlg::On_UiDropDowmMeau_CameraRecord(WPARAM wParam, LPARAM lParam)
{
	CloseAreaRecordSDL();
	DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]：处理UiDropDowmMeau摄像头录制消息");
	// 定义窗口显示位置
	int windowWidth = 771 * m_Scale;
	int windowHeight = 628 * m_Scale;
	CRect WindowRect;
	GetWindowRect(WindowRect);
	WindowRect.top -= 180 * m_Scale;
	WindowRect.right = WindowRect.left + windowWidth;
	WindowRect.bottom = WindowRect.top + windowHeight;

	// 创建窗口并显示
	if (!m_Dlg_Carmera) 
	{
		m_Dlg_Carmera = new Ui_CameraDlg;
		m_Dlg_Carmera->Ui_SetWindowRect(WindowRect);
		if (!m_Dlg_Carmera->Create(IDD_DIALOG_CAMERADLG, this)) 
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]：处理UiDropDowmMeau摄像头录制消息时，Create执行失败");
			delete m_Dlg_Carmera;
			m_Dlg_Carmera = nullptr;
			return 0;
		}
	}

	this->ShowWindow(SW_HIDE);
	m_Dlg_Carmera->Ui_UpdateWindowPos(WindowRect);
	m_Dlg_Carmera->SetForegroundWindow();
	m_Dlg_Carmera->ShowWindow(SW_SHOW);
	if (App.m_isLoginIn)
	{
		m_Dlg_Carmera->m_Btn_Phone.SetWindowTextW(App.m_userInfo.nickname);
		m_Dlg_Carmera->UpdateLoginUi();
	}
	else if (App.m_IsNonUserPaid)
	{
		m_Dlg_Carmera->UpdateNoneUserPayUI();
	}
	else
	{
		m_Dlg_Carmera->UpdateSignOutUi();
	}

	//重置ChildDlg对话框录制状态
	if (m_Dlg_Child->m_IsMouseAreaRecord)
		m_Dlg_Child->ResetRecordMouseAreaUi();
	if (m_Dlg_Child->m_IsOnlyAudioRecord)
		m_Dlg_Child->ResetAudioRecord();
	m_Dlg_Child->m_IsMouseAreaRecord = false;
	m_Dlg_Child->m_IsOnlyAudioRecord = false;

	if (m_VideoListVisible)
	{
		HideVideoList();
	}

	m_Shadow.Show(m_hWnd);
	return 1;
}

LRESULT Ui_MainDlg::On_UiDropDowmMeau_MouseRecord(WPARAM wParam, LPARAM lParam)
{
	CloseAreaRecordSDL();
	DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]：处理UiDropDowmMeau跟随鼠标录制消息");
	if (m_Dlg_Child->m_IsOnlyAudioRecord)
		m_Dlg_Child->ResetAudioRecord();
	if (!m_Dlg_Child->m_IsMouseAreaRecord)
		m_Dlg_Child->UpdateRecordMouseAreaUi();
	CString str;
	m_Dlg_Child->m_Btn_MouseRecordAreaPreset.GetWindowTextW(str);

	// 提取分辨率数字
	int width = 0, height = 0;
	int pos = str.Find(L"x");
	if (pos != -1)
	{
		CString resText = str.Mid(pos - 4, 9); // 提取可能包含分辨率的部分
		swscanf_s(resText, L"%dx%d", &width, &height);
	}

	// 调整使得矩形以鼠标为中心
	POINT mousePt;
	GetCursorPos(&mousePt);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int left = mousePt.x - width / 2;
	int top = mousePt.y - height / 2;
	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (left + width > screenWidth) left = screenWidth - width;
	if (top + height > screenHeight) top = screenHeight - height;

	// 构造矩形并赋值
	m_Dlg_Child->m_CRect_RecordRect = CRect(left, top, left + width, top + height);
	m_Dlg_Child->m_IsMouseAreaRecord = true;
	m_Dlg_Child->m_IsOnlyAudioRecord = false;
	m_Dlg_RecTopDlg->SetRecordContext_Child();
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"[主线程]：设置跟随鼠标录制区域 - 位置(%d,%d)，大小%dx%d",
		left, top, width, height);
	OnBnClickedBtnScreenRecord();
	m_Shadow.Show(m_hWnd);
	return 1;
}

LRESULT Ui_MainDlg::On_UiDropDowmMeau_SystemMircroAudioRecord(WPARAM wParam, LPARAM lParam)
{
	CloseAreaRecordSDL();
	DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]：处理UiDropDowmMeau声音录制消息");
	if (m_Dlg_Child->m_IsMouseAreaRecord)
		m_Dlg_Child->ResetRecordMouseAreaUi();
	if (!m_Dlg_Child->m_IsOnlyAudioRecord)
		m_Dlg_Child->UpdateAudioRecord();
	m_Dlg_Child->m_IsOnlyAudioRecord = true;
	m_Dlg_Child->m_IsMouseAreaRecord = false;
	m_Dlg_Child->m_CRect_RecordRect.SetRectEmpty();
	m_Dlg_Child->SetRecordOptBtnText(L"录声音");
	m_Dlg_RecTopDlg->SetRecordContext_Child();
	OnBnClickedBtnScreenRecord();
	m_Shadow.Show(m_hWnd);
	return 1;
}

LRESULT Ui_MainDlg::On_UiDeviceBind_UnBindAndClose(WPARAM wParam, LPARAM lParam)
{
	// 获取解绑的设备信息
	std::string* pDeviceCode = (std::string*)lParam;
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"发送解绑的设备编号:%s",
		LARSC::s2ws(*pDeviceCode).c_str());
	if (App.RequestDeviceUnbind(*pDeviceCode))
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"成功解绑一个设备", L"确认");
		MessageDlg.DoModal();

		if (App.m_IsOverBinds)
		{
			m_Btn_DeviceBinding.ShowWindow(SW_SHOW);
		}
		else
		{
			m_Btn_DeviceBinding.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"Ops!非常抱歉", L"解绑一个设备失败", L"确认");
		MessageDlg.DoModal();
	}
	delete pDeviceCode;
	return 1;
}

void Ui_MainDlg::OnBnClickedBtnMore()
{
	m_Dlg_Child->HideListBox();
	HideUserProfile();
	if (m_bool_IsMoreMenuShow)
	{
		DB(ConsoleHandle, L"返回");
		return;
	}

	// 创建新的SDL线程
	if (m_Thread_DropdownMenu) 
	{
		// 尝试等待线程结束
		if (m_Thread_DropdownMenu->joinable())
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[主线程]: 等待之前的SDL线程结束");
			m_Thread_DropdownMenu->join();
		}
		// 重置线程指针
		m_Thread_DropdownMenu.reset();
	}
	m_Thread_DropdownMenu = std::make_unique<std::thread>([this]() 
		{
		// 设置线程异常处理
		try
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 创建SDL录制窗口");

			// 创建SDL窗口实例
			Ui_DropdownMenuSDL* sdlWindow = Ui_DropdownMenuSDL::GetInstance();
			
			// SDL下拉框消息；摄像头录制
			sdlWindow->SetOnCameraRecordCallback([this]() {
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 摄像头录制消息回调");
				::PostMessage(this->GetSafeHwnd(), MSG_UIDROPDOWNMENU_CAMERARECORD, NULL, NULL);
				m_bool_IsMoreMenuShow = false;
				});

			//SDL下拉框消息：声音录制
			sdlWindow->SetOnMicroSystemAudioCallback([this]() {
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 声音录制回调");
				::PostMessage(this->GetSafeHwnd(), MSG_UIDROPDOWNMENU_SYSTEMAUDIOMICRO, NULL, NULL);
				m_bool_IsMoreMenuShow = false;
				});

			//SDL下拉框消息：跟随鼠标录制
			sdlWindow->SetOnMouseRecordCallback([this]() {
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 跟随鼠标录制回调");
				::PostMessage(this->GetSafeHwnd(), MSG_UIDROPDOWNMENU_MOUSERECORD, NULL, NULL);
				m_bool_IsMoreMenuShow = false;
				});

			// 初始化和运行窗口
			CRect MoreBtnRect, ComboRect;
			m_Btn_More.GetWindowRect(MoreBtnRect);
			ComboRect.left = MoreBtnRect.left + MoreBtnRect.Width() / 2;
			ComboRect.top = MoreBtnRect.top + MoreBtnRect.Height() / 3 * 2;
			ComboRect.right = ComboRect.left + 120 * m_Scale;
			ComboRect.bottom = ComboRect.top + 100 * m_Scale;

			//设定限制区域
			CRect anchorR;
			m_Btn_More.GetWindowRect(anchorR);
			anchorR.right += 120 * m_Scale;
			anchorR.bottom += 100 * m_Scale;
			sdlWindow->SetAnchorRect(anchorR);
			if (sdlWindow->Initialize(ComboRect))
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口初始化成功，开始运行");
				m_bool_IsMoreMenuShow = true;
				// Run方法会阻塞直到窗口关闭
				sdlWindow->Run();
				m_bool_IsMoreMenuShow = false;
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口已关闭");
			}
			else {
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: SDL窗口初始化失败");
			}
		}
		catch (const std::exception& e) {
			// 记录异常信息
			char errorMsg[256] = { 0 };
			sprintf_s(errorMsg, "SDL线程中发生异常: %s", e.what());
			wchar_t wErrorMsg[256] = { 0 };
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, wErrorMsg, _countof(wErrorMsg), errorMsg, _TRUNCATE);
			DEBUG_CONSOLE_STR(ConsoleHandle, wErrorMsg);
		}
		catch (...) {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL下拉框线程]: 发生未知异常");
		}
		});
	m_Thread_DropdownMenu->detach();	// 分离线程让它独立运行
}

LRESULT Ui_MainDlg::On_Ui_ChildDlg_ReturnBtnClicked(WPARAM lp, LPARAM wp)
{
	// 显示之前在OnBnClickedBtnScreenRecord中隐藏的控件
	m_Btn_ScreenRecord.ShowWindow(SW_SHOW);
	m_Btn_AreaRecord.ShowWindow(SW_SHOW);
	m_Btn_GamingRecord.ShowWindow(SW_SHOW);
	m_Btn_WindowRecord.ShowWindow(SW_SHOW);
	m_Btn_More.ShowWindow(SW_SHOW);
	m_Stat_ScreenRecord.ShowWindow(SW_SHOW);
	m_Stat_AreaRecord.ShowWindow(SW_SHOW);
	m_Stat_GamingRecord.ShowWindow(SW_SHOW);
	m_Stat_WindowRecord.ShowWindow(SW_SHOW);

	// 刷新窗口
	DEBUG_CONSOLE_STR(ConsoleHandle, L"子窗口关闭消息响应");
	return 0;
}

LRESULT Ui_MainDlg::On_Ui_NoneVipDlg_OpenVipBtnClicked(WPARAM lp, LPARAM wp)
{
	this->OnBnClickedBtnOpenvip();
	return 1;
}

LRESULT Ui_MainDlg::On_Ui_NoneVipDlg_UnBindClicked(WPARAM lp, LPARAM wp)
{
	this->OnBnClickedBtnDevicebinding();
	return LRESULT();
}

LRESULT Ui_MainDlg::On_Ui_NoneVipDlg_Login(WPARAM lp, LPARAM wp)
{
	this->OnBnClickedBtnLogin();
	return 1;
}

LRESULT Ui_MainDlg::On_Ui_RequestConfigDlg_Close(WPARAM lp, LPARAM wp)
{
	m_Dlg_Config->OnBnClickedBtnClose();
	return 1;
}

LRESULT Ui_MainDlg::On_Update_DevicBingList(WPARAM lp, LPARAM wp)
{
	if (Ui_DropdownMenuSDL::IsInsExist())
	{
		Ui_DropdownMenuSDL::GetInstance()->ReleaseInstance();
	}

	auto vec = (std::vector<DeviceBindingInfo>*)wp;
	m_Vec_DeviceBindingList = *vec;
	m_Thread_DeviceBindWindow = std::make_unique<std::thread>([this]()
		{
			// 设置线程异常处理
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL绑定设备窗口]: 创建SDL录制窗口");
			Ui_DeviceBindingSDL* sdlWindow = Ui_DeviceBindingSDL::GetInstance();// 创建SDL窗口实例
			sdlWindow->ClearDevices();
			for (auto& deviceInfo : m_Vec_DeviceBindingList)
			{
				sdlWindow->AddDevice(LARSC::s2ws(deviceInfo.os_name), std::string("code"), deviceInfo.is_current);
			}

			sdlWindow->SetOnCancelBindingCallback([this]()
				{//SDL下拉框消息：点击了取消绑定
					DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL绑定设备窗口]: 点击了解除绑定设备");
				});

			// 初始化和运行窗口
			CRect WindowRect;
			GetWindowRect(WindowRect);
			CRect DeviceBindWindowRect;
			int DeviceBindWindowWidth = 670 * m_Scale;
			int DeviceBindWindowHeight = 430 * m_Scale;
			DeviceBindWindowRect.left = WindowRect.left + (WindowRect.Width() - DeviceBindWindowWidth) / 2;
			DeviceBindWindowRect.top = WindowRect.top + (WindowRect.Height() - DeviceBindWindowHeight) / 2;
			DeviceBindWindowRect.right = DeviceBindWindowRect.left + DeviceBindWindowWidth;
			DeviceBindWindowRect.bottom = DeviceBindWindowRect.top + DeviceBindWindowHeight;
			if (sdlWindow->Initialize(DeviceBindWindowRect, m_Scale))
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL绑定设备窗口]: SDL窗口初始化成功，开始运行");
				sdlWindow->Run();
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL绑定设备窗口]: SDL窗口已关闭");
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL绑定设备窗口]: SDL窗口初始化失败");
			}
			App.m_IsOverBinds = sdlWindow->GetDeviceCount() > App.m_userInfo.maxBindings;

			if (sdlWindow->m_Struct_DeleteDeviceItem.m_Str_Name != L"")
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"发送解绑消息到主窗口");
				std::string* deviceCode = new std::string();
				*deviceCode = sdlWindow->m_Struct_DeleteDeviceItem.m_Str_Id;
				::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_UIDEVICEBIND_UNBINDANDCLOSE, 0, (LPARAM)deviceCode);
				sdlWindow->m_Struct_DeleteDeviceItem.m_Str_Name = L"";
			}
		});
	m_Thread_DeviceBindWindow->detach();	// 分离线程让它独立运行
	return 1;
}

void Ui_MainDlg::EnableShadow()
{
	// 启用非客户区渲染
	DWORD dwPolicy = DWMNCRP_ENABLED;
	HRESULT hr = DwmSetWindowAttribute(this->GetSafeHwnd(), DWMWA_NCRENDERING_POLICY, &dwPolicy, sizeof(dwPolicy));
	if (FAILED(hr))
	{
		TRACE(_T("DwmSetWindowAttribute 调用失败, HRESULT=0x%x\n"), hr);
	}

	// 扩展窗口客户区以显示阴影，margins 可根据需求调整
	MARGINS margins = { 5, 5, 5, 5 };
	hr = DwmExtendFrameIntoClientArea(this->GetSafeHwnd(), &margins);
	if (FAILED(hr))
	{
		TRACE(_T("DwmExtendFrameIntoClientArea 调用失败, HRESULT=0x%x\n"), hr);
	}
}

void Ui_MainDlg::LoadRes()
{
	m_Bitmap_VipLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(MAINDLG_PNG_VIP), 
		L"PNG"
	);
	m_Bitmap_MouthBill = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(MAINDLG_PNG_MOUTHBILL),
		L"PNG"
	);
}

void Ui_MainDlg::SetTrayClickCallback(std::function<void()> callback)
{
	m_closeManager.get()->SetTrayDoubleClickCallback(callback);
}

void Ui_MainDlg::OnBnClickedBtnClose()
{
	m_Dlg_Child->HideListBox();
	HideUserProfile();
	auto pSRIns = ScreenRecorder::GetInstance();
	if (pSRIns->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			if (pSRIns->isPausing())
				pSRIns->ResumeRecording();
			ScreenRecorder::GetInstance()->ReleaseInstance();
		}
		else
		{
			return;
		}
	}
	if (Ui_UserProfileSDL::GetInstance()->IsInsRunning())
	{
		Ui_UserProfileSDL::GetInstance()->Close();	
	}

	if (ModalDlg_MFC::ShowModal_IsCloseToBar() == IDCANCEL)
		return;

	if (Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning())
	{
		Ui_AreaRecordingSDL::GetInstance()->ReleaseInstance();
	}

	if (m_VideoListVisible)
		HideVideoList();

	UpdateQuitWay();
	m_Shadow.Show(m_hWnd);
}

void Ui_MainDlg::UpdateQuitWay()
{
	if (m_Dlg_Config->m_Bool_MInimalTobar)
		m_closeManager->SetCloseMode(AppCloseManager::CloseMode::MinimizeToTray);
	else if (m_Dlg_Config->m_Bool_QuitExe)
		m_closeManager->SetCloseMode(AppCloseManager::CloseMode::ExitApplication);
	if (!m_closeManager->HandleClose())
	{// 如果返回false，则继续正常关闭
		PostQuitMessage(0);
	}
}

void Ui_MainDlg::HandleClose()
{
	if (!m_closeManager->HandleClose())
	{// 如果返回false，则继续正常关闭
		PostQuitMessage(0);
	}
}

BOOL Ui_MainDlg::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//EnableShadow();
	return CDialogEx::OnNcActivate(bActive);
}

LRESULT Ui_MainDlg::OnNcHitTest(CPoint point)
{
	// 先调用基类获得原始命中区域
	LRESULT hit = CDialog::OnNcHitTest(point);

	//MouthBill区域命中测试
	POINT clientPt = point;
	ScreenToClient(&clientPt);
	RECT rc =
	{
		m_Rect_MouthBill.X,
		m_Rect_MouthBill.Y,
		m_Rect_MouthBill.X + m_Rect_MouthBill.Width,
		m_Rect_MouthBill.Y + m_Rect_MouthBill.Height
	};
	
	if (hit == HTCLIENT)
	{
		m_Dlg_Videolist->HideToolWindow();
		return HTCAPTION;// 如果点击在客户区，则返回 HTCAPTION 使得窗口可拖动
	}
	return hit;
}

BOOL Ui_MainDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void Ui_MainDlg::OnBnClickedBtnMinimal()
{
	m_Dlg_Child->HideListBox();
	HideUserProfile();
	m_Shadow.Show(m_hWnd);
	ShowWindow(SW_MINIMIZE);
	m_closeManager.get()->SetTrayDoubleClickCallback([this]()
		{
			this->ShowWindow(SW_SHOW);
			this->SetForegroundWindow();
			m_Dlg_Gaming->ShowWindow(SW_HIDE);
		});
}

void Ui_MainDlg::HideUserProfile()
{
	if (m_Dlg_UserPrifile && m_Dlg_UserPrifile->IsWindowVisible())
	{
		m_Dlg_UserPrifile->ShowWindow(SW_HIDE);
		m_btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);
		Invalidate(false);
	}
}

void Ui_MainDlg::JudgeDeviceBindBtnShowable()
{
	if (App.m_IsVip && App.m_IsOverBinds)
	{
		m_Btn_DeviceBinding.ShowWindow(SW_SHOW);
	}
	else
	{
		m_Btn_DeviceBinding.ShowWindow(SW_HIDE);
	}
}

void Ui_MainDlg::SetWindowShadowSize(int size)
{
	m_Shadow.SetSize(size);
}

void Ui_MainDlg::ShowNoneUserPayUi()
{
	//if (App.m_IsNonUserPaid)
	//{
	//	m_Btn_DeviceBinding.ShowWindow(SW_HIDE);
	//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
	//	m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	//}
	//Invalidate(false);
}

void Ui_MainDlg::BnClickedMouthBill()
{
	if (App.m_IsVip || !App.m_IsHasOpenVip)
		return;

	m_Shadow.HideShadow();
	if (!App.m_isLoginIn)
	{
		OnBnClickedBtnLogin();
	}

	if (!App.m_isLoginIn)
		return;
	if (ModalDlg_MFC::ShowModal_MouthMemberDlg(this) == IDOK)
	{
		// 支付成功，获取用户信息
		DEBUG_CONSOLE_STR(ConsoleHandle, L"支付成功，正在获取用户最新信息...");
		if (!App.RequestDeviceInfo())
		{
			DB(ConsoleHandle, L"获取用户信息失败");
		}
		DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
		m_Btn_OpenVIP.LoadClickPNG(MAINDLG_PNG_PAYFORLONG);
		//if (!App.m_IsVip && App.m_IsHasOpenVip)
		//	m_stat_mouthPrice.ShowWindow(SW_SHOW);
		//else
		//	m_stat_mouthPrice.ShowWindow(SW_HIDE);
		Invalidate(false);
		ModalDlg_MFC::ShowModal_PaySuccess(this);
	}
	m_Shadow.RestoreFromHide();
}

void Ui_MainDlg::OnBnClickedBtnLogin()
{
	m_Dlg_Child->HideListBox();
	if (!App.m_isLoginIn)
	{
		m_Shadow.HideShadow();
		// 确保释放之前的对话框（如果有）
		if (m_Dlg_Login != nullptr)
		{
			DB(ConsoleHandle, L"重新创建登录对话框");
			m_Dlg_Login->DestroyWindow();
			delete m_Dlg_Login;
			m_Dlg_Login = nullptr;
		}

		// 创建新的对话框实例 525 676
		m_Dlg_Login = new Ui_LoginDlg(this);
		int loginWindowWidth = 425 * m_Scale;
		int loginWindowHeight = 397 * m_Scale;

		// 设置位置
		CRect WindowRect, loginWindowRect;
		GetWindowRect(WindowRect);
		loginWindowRect.left = WindowRect.left + (WindowRect.Width() - loginWindowWidth) / 2;
		loginWindowRect.top = WindowRect.top + (WindowRect.Height() - loginWindowHeight) / 2;
		loginWindowRect.right = loginWindowRect.left + loginWindowWidth;
		loginWindowRect.bottom = loginWindowRect.top + loginWindowHeight;
		m_Dlg_Login->Ui_SetWindowRect(loginWindowRect);

		// 以模态方式显示对话框
		DEBUG_CONSOLE_STR(ConsoleHandle, L"显示登录对话框");
		this->SetForegroundWindow();
		if (m_Dlg_Login->DoModal() == IDOK)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"登录成功，开始获取用户信息");
			//if (App.RequestDeviceInfo())
			//{
			//	DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
			//}
			//else
			//{
			//	DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取失败");
			//}
			ShowLoginInUi();
			ModalDlg_MFC::ShowModal_LoginSuccess();
			App.m_isLoginIn = true;
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"用户取消了登录");
		}
		m_Shadow.RestoreFromHide();
	}
	else
	{
		OnBnClickedBtnUserProfile();
	}
}

void Ui_MainDlg::OnBnClickedBtnUserProfile()
{
	m_Dlg_Child->HideListBox();
	if (m_Dlg_UserPrifile && m_Dlg_UserPrifile->IsWindowVisible())
	{
		HideUserProfile();
	}
	else
	{
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"用户名:%s", App.m_userInfo.nickname);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"过期时间:%s", App.m_userInfo.expiresAt);
		DEBUG_CONSOLE_STR(ConsoleHandle, L"开始渲染SDL用户简介窗口");

		//ShowUserProfileSDL();
		if (m_Dlg_UserPrifile)
		{
			m_Dlg_UserPrifile->DestroyWindow();
			m_Dlg_UserPrifile = nullptr;
		}
		m_Dlg_UserPrifile = new Ui_UserProfileDlg;
		m_Dlg_UserPrifile->Create(IDD_DIALOG_USERPROFILE, this);
		
		CRect prect;
		m_btn_Phone.GetWindowRect(prect);

		CRect uprect;
		m_Dlg_UserPrifile->GetWindowRect(uprect);
		int upx = prect.left - uprect.Width() / 2;
		int upy = prect.bottom + 5 * m_Scale;
		uprect.MoveToXY(upx, upy);
		m_Dlg_UserPrifile->MoveWindow(uprect);
		m_Dlg_UserPrifile->UpdateUserInfo();
		m_Dlg_UserPrifile->ShowWindow(SW_SHOW);
		m_btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_UP, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);
		Invalidate();
	}
}

void Ui_MainDlg::ShowUserProfileSDL()
{
	Ui_UserProfileSDL::ReleaseInstance();//保证销毁之前的实例
	m_Thread_UserProfileWindow = std::make_unique<std::thread>([this]()
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 创建SDL用户资料窗口");
			Ui_UserProfileSDL* sdlWindow = Ui_UserProfileSDL::GetInstance();

			sdlWindow->SetOnCloseCallback([this]()
				{
					DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 窗口关闭回调");
				});
			sdlWindow->SetOnLogoutCallback([this]()
				{
					DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 退出登录回调");
					::PostMessage(this->GetSafeHwnd(), MSG_UIPROFILE_SIGNOUT, NULL, NULL);
					//退出登录是异步的，sdlWindow发完消息会立即关闭,为了尽量匹配退出登录逻辑完成并更新ui的过程
					//这里稍微休眠一会
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
				});
			sdlWindow->SetOnContactSupportCallback([this]()
				{
					DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 联系客服回调");
				});

			// 初始化和运行窗口
			CRect BtnRect;
			m_btn_Login.GetWindowRect(BtnRect);
			CRect UserProfileWindowRect;
			int UserProfileWindowWidth = 414 * m_Scale;
			int UserProfileWindowHeight = 119 * m_Scale;
			UserProfileWindowRect.left = BtnRect.left + (BtnRect.Width() - UserProfileWindowWidth) / 2;
			UserProfileWindowRect.top = BtnRect.top + BtnRect.Height() + 5 * m_Scale;
			UserProfileWindowRect.right = UserProfileWindowRect.left + UserProfileWindowWidth;
			UserProfileWindowRect.bottom = UserProfileWindowRect.top + UserProfileWindowHeight;

			if (sdlWindow->Initialize(UserProfileWindowRect, m_Scale))
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口初始化成功，开始运行");
				sdlWindow->Run();
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口已关闭");
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: SDL窗口初始化失败");
			}
		});
	m_Thread_UserProfileWindow->detach();    // 分离线程让它独立运行
}

void Ui_MainDlg::OnBnClickedBtnDevicebinding()
{
	if (App.m_IsNonUserPaid)
		return;
	if (!App.m_IsVip)
		return;

	HideUserProfile();
	if (App.m_isLoginIn)
	{
		if (!App.RequestDeviceList())
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"设备列表请求失败了");
			return;
		}
	}
}

void Ui_MainDlg::OnClose()
{
	CDialog::OnClose();
}

LRESULT Ui_MainDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
	if (m_closeManager) {
		m_closeManager->HandleTrayMessage(wParam, lParam);
	}
	return 0;
}

void Ui_MainDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == TIMER_MAINDLG_NEEDREDUCEBINDS)
	{
		OnBnClickedBtnDevicebinding();
		KillTimer(TIMER_MAINDLG_NEEDREDUCEBINDS);
	}
	 
	if(nIDEvent == TIMER_NONEVIP_RECORDTIME)
	{
		KillTimer(TIMER_NONEVIP_RECORDTIME);
		m_Dlg_Child->KillTimer(1002);
		if (ScreenRecorder::GetInstance()->IsRecording())
		{
			ScreenRecorder::ReleaseInstance();
		}
		this->ShowWindow(SW_SHOW);
		this->ShowWindow(SW_RESTORE);
		::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(),
			MSG_CHILDDLG_STOPTIMERCOUNTANDSHOWNORMALUI, NULL, NULL);
		m_Dlg_Child->OnBnClickedBtnReturn();
		CloseAreaRecordSDL();
		m_Dlg_Child->m_Btn_StartRecording.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
		m_Dlg_Child->m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_STARTRECORD);
		m_Dlg_Videolist->AddVideoToList(m_RecordP.outputFilePath.c_str());
		ShowVideoList();
		m_Dlg_Child->CloseCarmeraPreview();
		m_Dlg_Child->initCtrlPos();
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		App.UserNoneVipRecordSuccess();

		//弹框提示用户无法继续录制的原因
		if ((!App.m_IsVip) && (!App.m_IsNonUserPaid))//不是vip
			ModalDlg_MFC::ShowModal_TrialOver(this);
		else if (App.m_IsOverBinds)//设备绑定超限
			ModalDlg_MFC::ShowModal_OverBindsTips();

	}
	CDialogEx::OnTimer(nIDEvent);
}

LRESULT Ui_MainDlg::OnMinimalAndTipsUserRecording(WPARAM wParam, LPARAM lParam)
{
	//最小化主程序，弹出右下角的消息框，提示正在录制
	::ShowWindow(this->GetSafeHwnd(), SW_MINIMIZE);

	// 创建系统托盘通知
	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = GetSafeHwnd();
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_INFO;
	nid.dwInfoFlags = NIIF_INFO;
	nid.uTimeout = 5000;  // 显示10秒
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);  // 使用程序图标

	wcscpy_s(nid.szTip, L"极速录屏大师");
	wcscpy_s(nid.szInfo, L"录制已开始，程序已最小化至系统托盘");
	wcscpy_s(nid.szInfoTitle, L"正在录制中...");

	Shell_NotifyIcon(NIM_ADD, &nid);
	return 1;
}

LRESULT Ui_MainDlg::OnFirstOpenToOpenVip(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedBtnOpenvip();
	return LRESULT();
}

void Ui_MainDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	// 如果视频列表对话框可见，同步更新其位置
	if (m_Dlg_Videolist && m_VideoListVisible)
	{
		// 获取主窗口矩形
		CRect mainRect;
		GetWindowRect(&mainRect);

		// 获取视频列表窗口矩形
		CRect VideoListRect;
		m_Dlg_Videolist->GetWindowRect(VideoListRect);

		// 将视频列表窗口移动到本窗口底部 
		VideoListRect.MoveToXY(mainRect.left, mainRect.bottom);

		// 更新视频列表位置
		m_Dlg_Videolist->Ui_UpdateWindowPos(VideoListRect);
	}
	//m_Shadow.Show(m_hWnd);
}

void Ui_MainDlg::ShowVideoList()
{
	if (m_VideoListVisible)
	{
		return;
	}

	// 检查视频列表对话框是否存在
	if (!m_Dlg_Videolist)
		return;

	// 获取主窗口矩形
	CRect mainRect;
	GetWindowRect(&mainRect);

	// 获取视频列表窗口矩形
	CRect VideoListRect;
	m_Dlg_Videolist->GetWindowRect(VideoListRect);

	// 将视频列表窗口移动到本窗口底部 
	VideoListRect.MoveToXY(mainRect.left, mainRect.bottom);
	VideoListRect.right = VideoListRect.left + mainRect.Width();
	VideoListRect.bottom = VideoListRect.top + 240 * m_Scale;

	// 更新视频列表位置
	m_Dlg_Videolist->Ui_UpdateWindowPos(VideoListRect);

	// 显示视频列表对话框
	m_Dlg_Videolist->ShowWindow(SW_SHOW);
	m_Dlg_Videolist->SetTimer(Ui_VideoListDlg::TIMER_DELAYED_REDRAW, 50, NULL);
	m_Dlg_Videolist->SetReturnDlg(this);
	m_VideoListVisible = true;
	SetActiveWindow();
	m_Dlg_Videolist->EnableCWndShadow();

	// 隐藏当前窗口的下边框阴影
	m_Shadow.SetHideSingleShadow(CWndShadow::ShadowExMode::exBottom);
}

void Ui_MainDlg::HideVideoList()
{
	if (m_Dlg_Videolist)
	{
		m_Dlg_Videolist->ShowWindow(SW_HIDE);
		m_VideoListVisible = false;
		//恢复显示视频列表窗口时，被隐藏的下边框窗口阴影
		m_Shadow.SetHideSingleShadow(CWndShadow::ShadowExMode::noEx);
	}
}

LRESULT Ui_MainDlg::On_BroadCast_UserLogOut(WPARAM wParam, LPARAM lParam)
{
	ShowSignOutUi();
	return 1;
}

LRESULT Ui_MainDlg::On_SDLAreaReocrd_HideMainWindow(WPARAM wParam, LPARAM lParam)
{
	this->ShowWindow(SW_HIDE);
	return 1;
}

RecordingParams Ui_MainDlg::CollectRecordingParams()
{
	RecordingParams p;
	CString tmp;

	MsgParam::VolumeInfo volumeInfo{};
	::SendMessage(
		m_Dlg_Child->GetSafeHwnd(),
		MSG_MAINDLG_COLLECTVALUME,
		0,
		reinterpret_cast<LPARAM>(&volumeInfo)
	);
	p.microAudioVolume = volumeInfo.MicroAudioVolume;
	p.systemAudioVolume = volumeInfo.SystemAudioVolume;

	// 根据音量信息计算录音模式
	if (p.systemAudioVolume > 0.0f) {
		p.recordMode = (p.microAudioVolume > 0.0f)
			? ScreenRecorder::RecordMode::Both
			: ScreenRecorder::RecordMode::SystemSound;
	}
	else {
		p.recordMode = (p.microAudioVolume > 0.0f)
			? ScreenRecorder::RecordMode::Microphone
			: ScreenRecorder::RecordMode::None;
	}

	// 编码器
	m_Dlg_Config->m_Btn_VideoCodec.GetWindowTextW(tmp);
	p.codecText = (LPCTSTR)tmp;

	// 帧率
	m_Dlg_Config->m_Btn_Fps.GetWindowTextW(tmp);
	p.fps = GlobalFunc::ExtractFpsValue(tmp);

	// 视频格式
	m_Dlg_Config->m_Btn_VideoFormat.GetWindowTextW(tmp);
	if (tmp == L"AVI") p.videoFormat = ScreenRecorder::VideoFormat::AVI;
	else if (tmp == L"FLV") p.videoFormat = ScreenRecorder::VideoFormat::FLV;
	else                  p.videoFormat = ScreenRecorder::VideoFormat::MP4;

	// 视频画质
	m_Dlg_Config->m_Btn_VideoBitRatePercent.GetWindowTextW(tmp);
	if (tmp == L"超清")      p.videoQuality = ScreenRecorder::VideoQuality::SuperDefinition;
	else if (tmp == L"高清") p.videoQuality = ScreenRecorder::VideoQuality::HighDefinition;
	else if (tmp == L"标清") p.videoQuality = ScreenRecorder::VideoQuality::StandardDefinition;
	else                     p.videoQuality = ScreenRecorder::VideoQuality::Origin;

	// 音频采样率
	m_Dlg_Config->m_Btn_AudioSampleRate.GetWindowTextW(tmp);
	if (tmp == L"8000HZ")    p.audioSampleRate = AudioSampleRate::Hz_8000;
	else if (tmp == L"11025HZ") p.audioSampleRate = AudioSampleRate::Hz_11025;
	else if (tmp == L"22050HZ") p.audioSampleRate = AudioSampleRate::Hz_22050;
	else if (tmp == L"48000HZ") p.audioSampleRate = AudioSampleRate::Hz_48000;
	else                        p.audioSampleRate = AudioSampleRate::Hz_44100;

	// 音频比特率
	m_Dlg_Config->m_Btn_BitRate.GetWindowTextW(tmp);
	if (tmp == L"64Kbps")      p.audioBitRate = AudioBitRate::Kbps_64;
	else if (tmp == L"192Kbps") p.audioBitRate = AudioBitRate::Kbps_192;
	else if (tmp == L"256Kbps") p.audioBitRate = AudioBitRate::Kbps_256;
	else if (tmp == L"320Kbps") p.audioBitRate = AudioBitRate::Kbps_320;
	else                        p.audioBitRate = AudioBitRate::Kbps_128;

	// 视频分辨率
	m_Dlg_Config->m_Btn_VideoQuality.GetWindowTextW(tmp);
	if (tmp == L"360P")         p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_360P;
	else if (tmp == L"480P")    p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_480P;
	else if (tmp == L"720P")    p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_720P;
	else if (tmp == L"1080P")   p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_1080P;
	else if (tmp == L"2K")      p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_2K;
	else if (tmp == L"4K")      p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_4k;
	else                        p.videoResolution = ScreenRecorder::ResolutionRatio::Rs_SameAsScreen;

	// 编码预设
	m_Dlg_Config->m_Btn_VideoBitrateMode.GetWindowTextW(tmp);
	if (tmp == L"视频品质优先")       p.encodePreset = ScreenRecorder::EncodingPreset::Slow;
	else if (tmp == L"视频帧率优先") p.encodePreset = ScreenRecorder::EncodingPreset::Fast;
	else                            p.encodePreset = ScreenRecorder::EncodingPreset::Medium;

	// 保存路径、文件格式、设备名称
	m_Dlg_Config->m_Btn_Path.GetWindowTextW(tmp);
	p.savePath = (LPCTSTR)tmp;
	m_Dlg_Config->m_Btn_VideoFormat.GetWindowTextW(tmp);
	p.fileFormat = (LPCTSTR)tmp;
	m_Dlg_Config->m_Btn_AudioDevice.GetWindowTextW(tmp);
	p.audioDevice = (LPCTSTR)tmp;
	m_Dlg_Config->m_Btn_MicroDevice.GetWindowTextW(tmp);
	p.microDevice = (LPCTSTR)tmp;

	// 构造输出文件全路径
	p.outputFilePath = GlobalFunc::BuildVideoFilePath(p.savePath.c_str(), p.fileFormat.c_str());

	// 默认水印路径
	p.logoPath = GlobalFunc::GetExecutablePathFolder() + L"\\filter\\logo.png";

	//是否显示鼠标
	p.RecordMouse = m_Dlg_Config->m_Bool_IsDisplayMouse;
	m_RecordP = p;
	return p;
}

void Ui_MainDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	HideUserProfile();
	if (m_Dlg_Child)
		m_Dlg_Child->HideListBox();
	CDialogEx::OnNcLButtonDown(nHitTest, point);
}

void Ui_MainDlg::OnBnClickedBtnFeedback()
{
	App.OpenFeedBackLink(
		this->GetSafeHwnd(), 
		L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
	);
}

void Ui_MainDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (nHotKeyId == ALT_B_STARTORSTOPRECORD)
	{
		if (m_Dlg_Gaming && m_Dlg_Gaming->IsWindowVisible())
			m_Dlg_Gaming->ShowWindow(SW_HIDE);
		if (m_Dlg_WindowRecord && m_Dlg_WindowRecord->IsWindowVisible())
			m_Dlg_WindowRecord->ShowWindow(SW_HIDE);
		if (m_Dlg_Carmera && m_Dlg_Carmera->IsWindowVisible())
			m_Dlg_Carmera->ShowWindow(SW_HIDE);
		OnBnClickedBtnScreenRecord();
		m_Dlg_Child->OnBnClickedBtnStartrecord();
	}
	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);
}

void Ui_MainDlg::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	CDialogEx::OnActivateApp(bActive, dwThreadID);
	if (!bActive)
	{
		if (m_Dlg_WindowRecord)
		{
			m_Dlg_WindowRecord->m_PopupListBox.HideListBox();
		}
		if (m_Dlg_Carmera)
		{
			m_Dlg_Carmera->m_PopupListBox.HideListBox();
		}
		if (m_Dlg_Config)
		{
			m_Dlg_Config->m_ListBoxs.HideListBox();
		}
	}
	else
	{

	}
}

void Ui_MainDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL Ui_MainDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void Ui_MainDlg::OnBnClickedButton2()
{
	//FFmpegTest f;
	//f.test();

	if (App.RequestDeviceList())
	{
		DB(ConsoleHandle, L"请求成功");
	}
	else
	{
		DB(ConsoleHandle, L"请求失败");
	}
}
