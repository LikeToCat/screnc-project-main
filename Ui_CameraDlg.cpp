// Ui_CameraDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_CameraDlg.h"
#include "Ui_UserProfileDlg.h"
#include "afxdialogex.h"
#include "PngLoader.h"
#include "CDebug.h"
#include "DeviceManager.h"
#include "Ui_MainDlg.h"
#include "CMessage.h"
#include "LarStringConversion.h"
#include "GlobalFunc.h"
#include "WasapiCapture.h"
#include "theApp.h"
#include "ConfigFileHandler.h"
#include "ModalDialogFunc.h"
#include "Ui_ConfigDlg.h"

// Ui_CameraDlg 对话框
// CURL回调函数（退出登录时）

IMPLEMENT_DYNAMIC(Ui_CameraDlg, CDialogEx)

Ui_CameraDlg::Ui_CameraDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CAMERADLG, pParent)
{
	m_IsHasCameraDevice = true;
	m_Dlg_Config = nullptr; 
}

Ui_CameraDlg::~Ui_CameraDlg()
{
}

void Ui_CameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, CAMERADLG_BTN_APPICON, m_Btn_AppIcon);
	DDX_Control(pDX, CAMERADLG_STAT_TITLETEXT, m_Stat_TitleText);
	DDX_Control(pDX, CAMERADLG_BTN_OPENVIP, m_Btn_OpenVIP);
	DDX_Control(pDX, CAMERADLG_BTN_USERICON, m_Btn_UserIcon);
	DDX_Control(pDX, CAMERADLG_BTN_CONFIG, m_Btn_Config);
	DDX_Control(pDX, CAMERADLG_BTN_MENU, m_Btn_FeedBack);
	DDX_Control(pDX, CAMERADLG_BTN_MINMAL, m_Btn_Minimal);
	DDX_Control(pDX, CAMERADLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, CAMERADLG_BTN_RETURN, m_Btn_Return);
	DDX_Control(pDX, CAMERADLG_BTN_RECORDOPT, m_Btn_RecordOpt);
	DDX_Control(pDX, CAMERADLG_STAT_SELECTCAMERA, m_Stat_SelectCarmera);
	DDX_Control(pDX, CAMERADLG_STAT_CAMERAPARAM, m_Stat_CameraParam);
	DDX_Control(pDX, CAMERADLG_STAT_SYSTEMAUDIO, m_Stat_SystemAudio);
	DDX_Control(pDX, CAMERADLG_STAT_MICROAUDIO, m_Stat_MicroAudio);
	DDX_Control(pDX, CAMERADLG_BTN_CBCARMERADEVICE, m_Btn_CameraDevice);
	DDX_Control(pDX, CAMERADLG_BTN_CBCAMERAPARAM, m_Btn_CameraParam);
	DDX_Control(pDX, CAMERADLG_BTN_CBAUDIODEVICE, m_Btn_SystemAudioDevice);
	DDX_Control(pDX, CAMERADLG_BTN_CBMICRODEVICE, m_Btn_MicroAudio);
	DDX_Control(pDX, CAMERADLG_BTN_REFREASH, m_Btn_Refresh);
	DDX_Control(pDX, CAMERADLG_BTN_ADVANCEOPT, m_Btn_SystemAudioAdvanceOpt);
	DDX_Control(pDX, CAMERADLG_BTN_MICROADVANCE, m_Btn_MicroAudioOpt);
	DDX_Control(pDX, CAMERADLG_BTN_STARTRECORD, m_Btn_Rec);
	DDX_Control(pDX, CAMERADLG_BTN_STARTRECORD, m_Btn_Rec);
	DDX_Control(pDX, CAMERADLG_BTN_OPENCARMERA, m_Btn_OpenCamera);
	DDX_Control(pDX, CAMERADLG_BTN_PHONE, m_Btn_Phone);
	DDX_Control(pDX, CAMERDLG_BTN_LOGIN, m_Btn_login);
	DDX_Control(pDX, CAMERADLG_BTN_VIPLOGO, m_Btn_VipLogo);
	DDX_Control(pDX, CAMERADLG_STAT_MICRO, m_stat_hotKeyStartRecord);
}

BEGIN_MESSAGE_MAP(Ui_CameraDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(CAMERADLG_BTN_RETURN, &Ui_CameraDlg::OnBnClickedBtnReturn)
	ON_BN_CLICKED(CAMERADLG_BTN_CLOSE, &Ui_CameraDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(CAMERADLG_BTN_CBCARMERADEVICE, &Ui_CameraDlg::OnBnClickedBtnCbcarmeradevice)
	ON_BN_CLICKED(CAMERADLG_BTN_CBCAMERAPARAM, &Ui_CameraDlg::OnBnClickedBtnCbcameraparam)
	ON_BN_CLICKED(CAMERADLG_BTN_CBAUDIODEVICE, &Ui_CameraDlg::OnBnClickedBtnCbaudiodevice)
	ON_BN_CLICKED(CAMERADLG_BTN_CBMICRODEVICE, &Ui_CameraDlg::OnBnClickedBtnCbmicrodevice)
	ON_BN_CLICKED(CAMERADLG_BTN_RECORDOPT, &Ui_CameraDlg::OnBnClickedBtnRecordopt)
	ON_BN_CLICKED(CAMERADLG_BTN_PHONE, &Ui_CameraDlg::OnBnClickedBtnPhone)
	ON_BN_CLICKED(CAMERADLG_BTN_CONFIG, &Ui_CameraDlg::OnBnClickedBtnConfig)
	ON_BN_CLICKED(CAMERADLG_BTN_ADVANCEOPT, &Ui_CameraDlg::OnBnClickedBtnAdvanceopt)
	ON_BN_CLICKED(CAMERADLG_BTN_MICROADVANCE, &Ui_CameraDlg::OnBnClickedBtnMicroadvance)
	ON_BN_CLICKED(CAMERDLG_BTN_LOGIN, &Ui_CameraDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(CAMERADLG_BTN_USERICON, &Ui_CameraDlg::OnBnClickedBtnUsericon)
	ON_BN_CLICKED(CAMERADLG_BTN_OPENVIP, &Ui_CameraDlg::OnBnClickedBtnOpenvip)
	ON_BN_CLICKED(CAMERADLG_BTN_STARTRECORD, &Ui_CameraDlg::OnBnClickedBtnStartrecord)
	ON_BN_CLICKED(CAMERADLG_BTN_OPENCARMERA, &Ui_CameraDlg::OnBnClickedBtnOpencarmera)
	ON_BN_CLICKED(CAMERADLG_BTN_MINMAL, &Ui_CameraDlg::OnBnClickedBtnMinmal)

	//其他MFC窗口消息
	ON_MESSAGE(MSG_CLARLISTBOX_SELECTED, &Ui_CameraDlg::OnBnClickedBtnListBoxSelected)
	ON_MESSAGE(MSG_UIPROFILE_SIGNOUT, &Ui_CameraDlg::On_SDLBnClick_UserLogOut)
	ON_MESSAGE(MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, &Ui_CameraDlg::On_UserProfileDlg_WindowHidenByTimer)

	//广播消息
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGININ, &Ui_CameraDlg::On_BroadCast_UserLogin)
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGINOUT, &Ui_CameraDlg::On_BroadCast_UserLogOut)

	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOVE()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_EXITSIZEMOVE()
	ON_WM_NCACTIVATE()
	ON_WM_NCLBUTTONDOWN()
	ON_BN_CLICKED(CAMERADLG_BTN_MENU, &Ui_CameraDlg::OnBnClickedBtnMenu)
	ON_BN_CLICKED(CAMERADLG_BTN_VIPLOGO, &Ui_CameraDlg::OnBnClickedBtnViplogo)
	ON_BN_CLICKED(CAMERADLG_BTN_REFREASH, &Ui_CameraDlg::OnBnClickedBtnRefreash)
END_MESSAGE_MAP()

// Ui_CameraDlg 消息处理程序

BOOL Ui_CameraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	LoadRes();
	GetUserDPI();
	UpdateScale();
	InitDropListData();
	InitCtrl();

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口阴影效果
	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
	m_Shadow.Create(m_hWnd);

	SetWindowToAppWindow();
	m_PopupListBox.SetTextSize(14, L"");
	m_PopupListBox.SetScrollbarWidth(6 * m_Scale);
	m_PopupListBox.SetListBoxHideWhenMouseLeave(false);
	return TRUE;
}

void Ui_CameraDlg::SetWindowToAppWindow()
{
	// 获取当前扩展样式
	LONG ex = ::GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE);

	// 加上 WS_EX_APPWINDOW，去掉 WS_EX_TOOLWINDOW
	ex |= WS_EX_APPWINDOW;
	ex &= ~WS_EX_TOOLWINDOW;
	::SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, ex);

	// 通知系统样式改变
	::SetWindowPos(this->GetSafeHwnd(),
		NULL, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);
	::SetWindowTextW(this->GetSafeHwnd(), L"极速录屏大师");
}

void Ui_CameraDlg::InitDropListData()
{
	//获取摄像头设备列表
	DEBUG_CONSOLE_STR(ConsoleHandle, L"=============摄像头界面设备枚举\n\n");
	DEBUG_CONSOLE_STR(ConsoleHandle, L"---------------摄像头设备列表---------------");
	bool IsHasCameraDevice = false;
	const std::vector<DeviceInfo>& CarmeraDeviceInfos = DeviceManager::GetInstance().GetCameraDevices();
	if (CarmeraDeviceInfos.size() > 0)
	{
		for (auto& CarmeraDeviceInfo : CarmeraDeviceInfos)
		{
			const CString& CamerName = CarmeraDeviceInfo.nameW.c_str();
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s", CamerName);
			m_Array_CameraDeviceList.Add(CamerName);
		}
		IsHasCameraDevice = true;
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"没有查询到可用的摄像头设备");
		m_Array_CameraDeviceList.Add(L"没有查询到可用设备");
		m_IsHasCameraDevice = false;
	}

	//如果有摄像头设备，查询设备支持的录制参数
	if (IsHasCameraDevice)
	{//获取摄像头列表中默认第一个摄像头的录制选项
		DEBUG_CONSOLE_STR(ConsoleHandle, L"---------------摄像头录制选项列表---------------");
		const std::vector<CameraCapability>& CarmeraOptions = DeviceManager::GetInstance().GetCameraCapabilities(CarmeraDeviceInfos.at(0).nameA);
		for (auto& CarmeraOption : CarmeraOptions)
		{
			CString OptionStr;
			OptionStr.Format(L"格式:%s,分辨率:%d*%d,%dFPS", LARSC::s2ws(CarmeraOption.vcodec).c_str(),
				CarmeraOption.width, CarmeraOption.height, static_cast<int>(CarmeraOption.fps));
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s", OptionStr);
			m_Array_CameraParamList.Add(OptionStr);
		}
	}
	else
	{
		m_Array_CameraParamList.Add(L"没有查询到可用设备");
	}


	// 系统声音设备列表
	DEBUG_CONSOLE_STR(ConsoleHandle, L"---------------系统声音设备列表---------------");
	std::vector<CaptureDeviceInfo> AudioCaptures;
	if (WasapiCapture::GetInstance()->getCaptureDevicesInfo(&AudioCaptures))
	{
		if (AudioCaptures.size() > 0)
		{
			for (auto& AudioCapture : AudioCaptures)
			{
				const CString& AudioCaptureName = AudioCapture.deviceName.c_str();
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s", AudioCaptureName);
				if (AudioCapture.isDefault)
				{
					CString defaultDeviceName;
					defaultDeviceName.Format(L"%s(默认)", AudioCaptureName);
					m_Array_SystemAudioList.Add(defaultDeviceName);
				}
				else
				{
					m_Array_SystemAudioList.Add(AudioCaptureName);
				}
			}
		}
		else
		{
			m_Array_SystemAudioList.Add(L"无可用设备");
		}
	}


	// 麦克风设备列表
	DEBUG_CONSOLE_STR(ConsoleHandle, L"---------------麦克风设备列表---------------");
	const std::vector<DeviceInfo>& MicroDeivces = DeviceManager::GetInstance().GetMicrophoneDevices();
	if (MicroDeivces.size() > 0)
	{
		for (auto& MicroDeivce : MicroDeivces)
		{
			const CString& MicroDeivceName = MicroDeivce.nameW.c_str();
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"%s", MicroDeivceName);
			m_Array_MicroAudioList.Add(MicroDeivceName);
		}
	}
	else
	{
		m_Array_MicroAudioList.Add(L"无可用设备");
	}

	DEBUG_CONSOLE_STR(ConsoleHandle, L"\n\n============摄像头界面设备枚举完成\n\n");

	// 录制选项列表
	m_Array_RecordOptList.Add(L"录摄像头");
	m_Array_RecordOptList.Add(L"录全屏");
	m_Array_RecordOptList.Add(L"录区域");
	m_Array_RecordOptList.Add(L"录游戏");
	m_Array_RecordOptList.Add(L"录窗口");
	m_Array_RecordOptList.Add(L"录声音");
	m_Array_RecordOptList.Add(L"跟随鼠标录制");
}

void Ui_CameraDlg::OnLButtonDown(UINT nFlags, CPoint point)
{

	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_CameraDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnCancel();
}

void Ui_CameraDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnOK();
}

LRESULT Ui_CameraDlg::OnNcHitTest(CPoint point)
{
	//// 先调用基类获得原始命中区域
	LRESULT hit = CDialog::OnNcHitTest(point);
	if (hit == HTCLIENT)
	{
		// 如果点击在客户区，则返回 HTCAPTION 使得窗口可拖动
		return HTCAPTION;
	}
	return hit;
}

void Ui_CameraDlg::OnPaint()
{
	CPaintDC dc(this);
	m_Shadow.Show(m_hWnd);
	int WindowWidth = m_Rect_WindowRect.Width;
	int WindowHeight = m_Rect_WindowRect.Height;

	// 预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap membitmap(WindowWidth, WindowHeight);
	Gdiplus::Graphics memGraphice(&membitmap);

	// 绘画背景
	SolidBrush CaptionBrush(Color(37, 39, 46));
	SolidBrush ClientBrush(Color(26, 27, 32));
	SolidBrush CameraDisplayAreaBrush(Color(36, 37, 40));
	memGraphice.FillRectangle(&ClientBrush, m_Rect_MainArea);
	memGraphice.FillRectangle(&CameraDisplayAreaBrush, m_Rect_CameraDisplay);
	memGraphice.FillRectangle(&CaptionBrush, m_Rect_TitleBar);

	// 一次性绘画到窗口上
	Gdiplus::Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&membitmap, 0, 0, WindowWidth, WindowHeight);

	DB(ConsoleHandle, L"Ui_CameraDlg:repaint..");
}

void Ui_CameraDlg::Ui_SetWindowRect(const CRect& rect)
{
	m_CRect_WindowRect.left = rect.left;
	m_CRect_WindowRect.right = rect.right;
	m_CRect_WindowRect.top = rect.top;
	m_CRect_WindowRect.bottom = rect.bottom;

	m_Rect_WindowRect.Width = rect.Width();
	m_Rect_WindowRect.Height = rect.Height();
	m_Rect_WindowRect.X = 0;
	m_Rect_WindowRect.Y = 0;
}

void Ui_CameraDlg::Ui_UpdateWindowPos(const CRect& rect)
{
	m_CRect_WindowRect.left = rect.left;
	m_CRect_WindowRect.right = rect.right;
	m_CRect_WindowRect.top = rect.top;
	m_CRect_WindowRect.bottom = rect.bottom;

	m_Rect_WindowRect.Width = rect.Width();
	m_Rect_WindowRect.Height = rect.Height();
	m_Rect_WindowRect.X = 0;
	m_Rect_WindowRect.Y = 0;

	MoveWindow(m_CRect_WindowRect);
}

void Ui_CameraDlg::CleanUpGdiPngRes()
{
	m_Btn_Refresh.ClearImages();// 刷新
	m_Btn_Rec.ClearImages();	// Rec	
	m_Btn_OpenVIP.ClearImages();// 开通VIP
	m_Btn_UserIcon.ClearImages();// 用户头像
	m_Btn_AppIcon.ClearImages();// 图标
	m_Btn_Minimal.ClearImages();	// 最小化
	m_Btn_Close.ClearImages();// 关闭
	m_Btn_Return.ClearImages();// 返回
}

void Ui_CameraDlg::GetUserDPI()
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

void Ui_CameraDlg::UpdateScale()
{
	// 设定默认窗口大小为 789x474 像素
	int windowWidth = 771 * m_Scale;
	int windowHeight = 628 * m_Scale;

	// 如果 m_CRect_WindowRect 已定义，则使用其尺寸，否则使用默认尺寸
	if (m_CRect_WindowRect.Width() > 0 && m_CRect_WindowRect.Height() > 0) {
		windowWidth = m_CRect_WindowRect.Width();
		windowHeight = m_CRect_WindowRect.Height();
		//设置窗口GDI矩形
		m_Rect_WindowRect.X = 0;
		m_Rect_WindowRect.Y = 0;
		m_Rect_WindowRect.Width = m_CRect_WindowRect.Width();
		m_Rect_WindowRect.Height = m_CRect_WindowRect.Height();
	}
	else {
		//设置窗口GDI矩形
		m_Rect_WindowRect.X = 0;
		m_Rect_WindowRect.Y = 0;
		m_Rect_WindowRect.Width = windowWidth;
		m_Rect_WindowRect.Height = windowHeight;
	}
	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;
	SetWindowPos(NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);

	//设置标题栏区域
	m_Rect_TitleBar.X = 0;
	m_Rect_TitleBar.Y = 0;
	m_Rect_TitleBar.Width = windowWidth;
	m_Rect_TitleBar.Height = windowHeight * 0.076;

	//设置主互动区域
	m_Rect_MainArea.X = 0;
	m_Rect_MainArea.Y = m_Rect_TitleBar.Height;
	m_Rect_MainArea.Width = windowWidth;
	m_Rect_MainArea.Height = windowHeight - m_Rect_TitleBar.Height;

	//设置摄像头显示区域 30 134 963 782
	m_Rect_CameraDisplay.X = windowWidth * 0.031;
	m_Rect_CameraDisplay.Y = windowHeight * 0.171;
	m_Rect_CameraDisplay.Width = 724 * m_Scale;
	m_Rect_CameraDisplay.Height = 308 * m_Scale;

	UpdateTitleBarCtrl();//调整标题栏中的控件
	UpdateMainAreaCtrl();//调整主互动区域中的控件
}

void Ui_CameraDlg::UpdateTitleBarCtrl()
{
	//极速录屏大师图标
	float TitleIconWidth = 37.8 * m_Scale;
	float TitleIconHeight = 32.4 * m_Scale;
	float TitleIconX = 15 * m_Scale;
	float TitleIconY = (m_Rect_TitleBar.Height - TitleIconHeight) / 2 + 3 * m_Scale;
	m_Btn_AppIcon.MoveWindow(TitleIconX, TitleIconY, TitleIconWidth, TitleIconHeight);

	//极速录屏大师文本
	float TitleTextWidth = 140 * m_Scale;
	float TitleTextHeight = 30 * m_Scale;
	float TitleTextX = TitleIconX + TitleIconWidth + 5 * m_Scale;
	float TitleTextY = TitleIconY + (TitleIconHeight - TitleTextHeight) / 2 - 1 * m_Scale;
	m_Stat_TitleText.MoveWindow(TitleTextX, TitleTextY, TitleTextWidth, TitleTextHeight);

	//开通会员
	float OpenVipWidth = 105.8 * m_Scale;
	float OpenVipHeight = 40.25 * m_Scale;
	float OpenVipX = 0.657 * m_Rect_TitleBar.Width;
	float OpenVipY = (m_Rect_TitleBar.Height - OpenVipHeight) / 2 + 3 * m_Scale;
	m_Btn_OpenVIP.MoveWindow(OpenVipX, OpenVipY - 5 * m_Scale, OpenVipWidth, OpenVipHeight);

	//VIPlogo 0.506
	float ViplogoWidth = 22 * m_Scale;
	float ViplogoHeight = 22 * m_Scale;
	float ViplogoX = 0.47 * m_WindowWidth;
	float ViplogoY = (m_Rect_TitleBar.Height - ViplogoHeight) / 2;
	m_Btn_VipLogo.MoveWindow(ViplogoX, ViplogoY, ViplogoWidth, ViplogoHeight);

	//用户图标
	float UserIconWidth = 16 * m_Scale;
	float UserIconHeight = 16 * m_Scale;
	float UserIconX = ViplogoX + ViplogoWidth + 5 * m_Scale;
	float UserIconY = ViplogoY + (ViplogoHeight - UserIconHeight) / 2;
	m_Btn_UserIcon.MoveWindow(UserIconX, UserIconY, UserIconWidth, UserIconHeight);
	m_Rect_BtnUserIcon_SignOutRect.SetRect(UserIconX, UserIconY, UserIconX + UserIconWidth, UserIconY + UserIconHeight);

	//手机号
	float PhoneWidth = 100 * m_Scale;
	float PhoneHeight = 25 * m_Scale;
	float PhoneX = UserIconX + UserIconWidth;
	float PhoneY = UserIconY + (UserIconHeight - PhoneHeight) / 2;
	m_Btn_Phone.MoveWindow(PhoneX, PhoneY, PhoneWidth, PhoneHeight);

	//登录
	float LoginWidth = 55 * m_Scale;
	float LoginHeight = 25 * m_Scale;
	float LoginX = UserIconX + 10 * m_Scale;
	float LoginY = UserIconY + (UserIconHeight - LoginHeight) / 2 + 1 * m_Scale;
	m_Btn_login.MoveWindow(LoginX, LoginY, LoginWidth, LoginHeight);

	//设置
	float ConfigBtnWidth = 40 * m_Scale;
	float ConfigBtnHeight = 18 * m_Scale;
	float ConfigBtnX = OpenVipX + OpenVipWidth;
	float ConfigBtnY = UserIconY + (UserIconHeight - ConfigBtnHeight) / 2;
	m_Btn_Config.MoveWindow(ConfigBtnX, ConfigBtnY, ConfigBtnWidth, ConfigBtnHeight);

	//反馈
	float FeedBackWidth = ConfigBtnWidth;
	float FeedBackHeight = ConfigBtnHeight;
	float FeedBackX = ConfigBtnX + ConfigBtnWidth + 5 * m_Scale;
	float FeedBackY = ConfigBtnY;
	m_Btn_FeedBack.MoveWindow(FeedBackX, FeedBackY, FeedBackWidth, FeedBackHeight);

	//关闭
	float CloseBtnWidth = 24 * m_Scale;
	float CloseBtnHeight = 24 * m_Scale;
	float CloseBtnX = m_Rect_TitleBar.X + m_Rect_TitleBar.Width - 15 * m_Scale - CloseBtnWidth;
	float CloseBtnY = FeedBackY + (FeedBackHeight - CloseBtnHeight) / 2 - 1 * m_Scale;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	//最小化
	float MinimalBtnWidth = 28 * m_Scale;
	float MinimalBtnHeight = 24 * m_Scale;
	float MinimalX = CloseBtnX - MinimalBtnWidth - 5 * m_Scale;
	float MinimalY = CloseBtnY ;
	m_Btn_Minimal.MoveWindow(MinimalX, MinimalY, MinimalBtnWidth, MinimalBtnHeight);
}

void Ui_CameraDlg::UpdateMainAreaCtrl()
{
	//打开摄像头110 36 
	float OpenCameraWidth = 85 * m_Scale;
	float OpenCameraHeight = 24 * m_Scale;
	float OpenCameraX = m_Rect_CameraDisplay.X + m_Rect_CameraDisplay.Width - OpenCameraWidth;
	float OpenCameraY = m_Rect_CameraDisplay.Y + m_Rect_CameraDisplay.Height + 5 * m_Scale;
	m_Btn_OpenCamera.MoveWindow(OpenCameraX, OpenCameraY, OpenCameraWidth, OpenCameraHeight);

	//返回 36/965 23/726
	float ReturnWidth = 15 * m_Scale;
	float ReturnHeight = 18 * m_Scale;
	float ReturnX = 0.037 * m_WindowWidth;
	float ReturnY = m_Rect_TitleBar.Height + 0.031 * m_Rect_MainArea.Height;
	m_Btn_Return.MoveWindow(ReturnX, ReturnY, ReturnWidth, ReturnHeight);

	//录制选项 120 30
	float RecordOptWidth = 96 * m_Scale;
	float RecordOptHeight = 24 * m_Scale;
	float RecordOptX = ReturnX + ReturnWidth + 10 * m_Scale;
	float RecordOptY = ReturnY + (ReturnHeight - RecordOptHeight) / 2;
	m_Btn_RecordOpt.MoveWindow(RecordOptX, RecordOptY, RecordOptWidth, RecordOptHeight);

	//选择摄像头文本
	float SelectCameraTextWidth = 77 * m_Scale;
	float SelectCameraTextHeight = 18 * m_Scale;
	float SelectCameraX = m_Rect_CameraDisplay.X;
	float SelectCameraY = m_Rect_CameraDisplay.GetBottom() + 40 * m_Scale;
	m_Stat_SelectCarmera.MoveWindow(SelectCameraX, SelectCameraY, SelectCameraTextWidth, SelectCameraTextHeight);

	//摄像头设备选择下拉框 327 29
	float CbCameraDeviceWidth = 262 * m_Scale;
	float CbCameraDeviceHeight = 24 * m_Scale;
	float CbCameraDeviceX = SelectCameraX + SelectCameraTextWidth + 10 * m_Scale;
	float CbCameraDeviceY = SelectCameraY + (SelectCameraTextHeight - CbCameraDeviceHeight) / 2;
	m_Btn_CameraDevice.MoveWindow(CbCameraDeviceX, CbCameraDeviceY, CbCameraDeviceWidth, CbCameraDeviceHeight);

	//刷新按钮 59 23 
	float RefreshWidth = 54 * m_Scale;
	float RefreshHeight = 20.25 * m_Scale;
	float RefreshX = CbCameraDeviceX + CbCameraDeviceWidth + 5 * m_Scale;
	float RefreshY = CbCameraDeviceY + (CbCameraDeviceHeight - RefreshHeight) / 2;
	m_Btn_Refresh.MoveWindow(RefreshX, RefreshY, RefreshWidth, RefreshHeight);

	//摄像头参数
	float CameraParamTextWidth = 77 * m_Scale;
	float CameraParamTextHeight = 18 * m_Scale;
	float CameraParamX = m_Rect_CameraDisplay.X;
	float CameraParamY = SelectCameraY + SelectCameraTextHeight + 20 * m_Scale;
	m_Stat_CameraParam.MoveWindow(CameraParamX, CameraParamY, CameraParamTextWidth, CameraParamTextHeight);

	//摄像头参数下拉框 
	float CbCameraParamWidth = 262 * m_Scale;
	float CbCameraParamHeight = 24 * m_Scale;
	float CbCameraParamX = CbCameraDeviceX;
	float CbCameraParamY = CameraParamY + (CameraParamTextHeight - CbCameraParamHeight) / 2;
	m_Btn_CameraParam.MoveWindow(CbCameraParamX, CbCameraParamY, CbCameraParamWidth, CbCameraParamHeight);

	//系统声音文本
	float SystemAudioTextWidth = 77 * m_Scale;
	float SystemAudioTextHeight = 18 * m_Scale;
	float SystemAudioX = m_Rect_CameraDisplay.X;
	float SystemAudioY = CameraParamY + CameraParamTextHeight + 20 * m_Scale;
	m_Stat_SystemAudio.MoveWindow(SystemAudioX, SystemAudioY, SystemAudioTextWidth, SystemAudioTextHeight);

	//系统声音设备下拉框
	float SystemAudioBtnWidth = 262 * m_Scale;
	float SystemAudioBtnHeight = 24 * m_Scale;
	float SystemAudioBtnX = CbCameraDeviceX;
	float SystemAudioBtnY = SystemAudioY + (SystemAudioTextHeight - SystemAudioBtnHeight) / 2;
	m_Btn_SystemAudioDevice.MoveWindow(SystemAudioBtnX, SystemAudioBtnY, SystemAudioBtnWidth, SystemAudioBtnHeight);

	//高级选项按钮 81 38 
	float AudioAdvanceWidth = 75 * m_Scale;
	float AudioAdvanceHeight = 24 * m_Scale;
	float AudioAdvanceX = SystemAudioBtnX + SystemAudioBtnWidth + 5 * m_Scale;
	float AudioAdvanceY = SystemAudioBtnY + (SystemAudioBtnHeight - AudioAdvanceHeight) / 2;
	m_Btn_SystemAudioAdvanceOpt.MoveWindow(AudioAdvanceX, AudioAdvanceY, AudioAdvanceWidth, AudioAdvanceHeight);

	//麦克风声音
	float MicroTextWidth = 77 * m_Scale;
	float MicroTextHeight = 18 * m_Scale;
	float MicroX = m_Rect_CameraDisplay.X;
	float MicroY = SystemAudioY + SystemAudioTextHeight + 20 * m_Scale;
	m_Stat_MicroAudio.MoveWindow(MicroX, MicroY, MicroTextWidth, MicroTextHeight);

	//麦克风设备下拉框
	float MicroBtnWidth = 262 * m_Scale;
	float MicroBtnHeight = 24 * m_Scale;
	float MicroBtnX = CbCameraDeviceX;
	float MicroBtnY = MicroY + (MicroTextHeight - MicroBtnHeight) / 2;
	m_Btn_MicroAudio.MoveWindow(MicroBtnX, MicroBtnY, MicroBtnWidth, MicroBtnHeight);

	//麦克风设备高级选项
	float MicroAdvanceWidth = 75 * m_Scale;
	float MicroAdvanceHeight = 24 * m_Scale;
	float MicroAdvanceX = MicroBtnX + MicroBtnWidth + 5 * m_Scale;
	float MicroAdvanceY = MicroBtnY + (MicroBtnHeight - MicroAdvanceHeight) / 2;
	m_Btn_MicroAudioOpt.MoveWindow(MicroAdvanceX, MicroAdvanceY, MicroAdvanceWidth, MicroAdvanceHeight);

	//rec
	float RecWidth = 107 * m_Scale;
	float RecHeight = 107 * m_Scale;
	float RecX = 0.756 * m_WindowWidth;
	float RecY = m_Rect_CameraDisplay.GetBottom() + 50 * m_Scale;
	m_Btn_Rec.MoveWindow(RecX, RecY, RecWidth, RecHeight);

	//alt + b
	float HotKeySRW = RecWidth + 100 * m_Scale;
	float HotKeySRH = 20 * m_Scale;
	float HotKeySRX = RecX + (RecWidth - HotKeySRW) / 2;
	float HotKeySRY = RecY + RecHeight + 5 * m_Scale;
	m_stat_hotKeyStartRecord.MoveWindow(HotKeySRX, HotKeySRY, HotKeySRW, HotKeySRH);
}

void Ui_CameraDlg::InitTitleBarCtrl()
{
	//应用程序图标
	m_Btn_AppIcon.LoadPNG(MAINDLG_PNG_APPICON);
	m_Btn_AppIcon.SetBackgroundColor(RGB(37, 39, 46));

	//标题
	m_Stat_TitleText.LarSetTextSize(24);
	m_Stat_TitleText.LarSetTextLeft();

	//开通会员 
	if ((App.m_IsVip || App.m_IsNonUserPaid))
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	else
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
	m_Btn_OpenVIP.SetBackgroundColor(RGB(37, 39, 46));

	//用户头像
	m_Btn_UserIcon.LoadPNG(MAINDLG_PNG_PROFILEICON);
	m_Btn_UserIcon.SetBackgroundColor(RGB(37, 39, 46));

	//VIP标识
	m_Btn_VipLogo.LoadPNG(MAINDLG_PNG_VIP);
	m_Btn_VipLogo.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_VipLogo.SetUseHandCursor(false);
	if ((App.m_IsVip || App.m_IsNonUserPaid))
		m_Btn_VipLogo.ShowWindow(SW_SHOW);
	else
		m_Btn_VipLogo.ShowWindow(SW_HIDE);

	//设置
	SolidBrush BkBrush(Color(37, 39, 46));
	m_Btn_Config.LarSetTextSize(20);
	m_Btn_Config.LarSetNormalFiilBrush(BkBrush);
	m_Btn_Config.LaSetTextColor(Gdiplus::Color(135, 255, 255, 255));
	m_Btn_Config.LaSetTextHoverColor(Gdiplus::Color(255, 255, 255, 255));
	m_Btn_Config.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Config.LarSetBorderColor(Gdiplus::Color(255, 37, 39, 46));
	m_Btn_Config.LarSetEraseBkEnable(false);

	//登录
	m_Btn_login.LarSetTextSize(20);
	m_Btn_login.LarSetNormalFiilBrush(BkBrush);
	m_Btn_login.LaSetTextColor(Gdiplus::Color(135, 255, 255, 255));
	m_Btn_login.LaSetTextHoverColor(Gdiplus::Color(255, 255, 255, 255));
	m_Btn_login.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_login.LarSetBorderColor(Gdiplus::Color(255, 37, 39, 46));
	m_Btn_login.LarSetEraseBkEnable(false);
	m_Btn_login.ShowWindow(SW_SHOW);

	//手机号
	m_Btn_Phone.LarSetTextSize(17);
	m_Btn_Phone.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Phone.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_Phone.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Phone.LarSetBorderColor(Gdiplus::Color(255, 37, 39, 46));
	m_Btn_Phone.LarSetEraseBkEnable(false);
	m_Btn_Phone.LarSetTextCenter(false);
	m_Btn_Phone.LarSetNormalFiilBrush(SolidBrush(Color(255, 37, 39, 46)));
	m_Btn_Phone.ShowWindow(SW_HIDE);
	m_Btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale);
	m_Btn_Phone.LarAdjustTextDisplayPos(3 * m_Scale, 0);

	//菜单
	m_Btn_FeedBack.LarSetTextSize(19);
	m_Btn_FeedBack.LarSetNormalFiilBrush(BkBrush);
	m_Btn_FeedBack.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_FeedBack.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_FeedBack.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_FeedBack.LarSetBorderColor(Gdiplus::Color(255, 37, 39, 46));
	m_Btn_FeedBack.LarSetEraseBkEnable(false);
	m_Btn_FeedBack.ShowWindow(SW_SHOW);

	//最小化 
	m_Btn_Minimal.LoadPNG(MAINDLG_PNG_MINIMAL);
	m_Btn_Minimal.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_Minimal.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Minimal.SetStretchMode(0.90f);

	//关闭
	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_Close.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Close.SetStretchMode(0.87f);
}

void Ui_CameraDlg::InitMainAreaCtrl()
{
	////文本
	m_Stat_SelectCarmera.LarSetTextSize(20);
	m_Stat_CameraParam.LarSetTextSize(20);
	m_Stat_SystemAudio.LarSetTextSize(20);
	m_Stat_MicroAudio.LarSetTextSize(20);
	m_stat_hotKeyStartRecord.LarSetTextSize(20);
	m_stat_hotKeyStartRecord.LarSetTextColor(RGB(255, 255, 255));
	m_stat_hotKeyStartRecord.LarSetTextCenter();

	////下拉框按钮
	Color ComboBox_TextColor(202, 202, 202);//下拉框文本颜色
	Color ComboBox_BorderColor(67, 67, 67); //下拉框边框颜色
	Color ComboBox_BkColor(36, 37, 40);		//下拉框背景颜色
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_CameraDevice);
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_CameraParam);
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_SystemAudioDevice);
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_MicroAudio);

	m_Btn_RecordOpt.SetWindowText(L"录摄像头");
	m_Btn_RecordOpt.LarSetTextSize(20);
	m_Btn_RecordOpt.LarSetTextStyle(false, false, false);
	m_Btn_RecordOpt.LaSetTextColor(Color(155, 255, 255, 255));
	m_Btn_RecordOpt.LarSetBorderColor(Color(73, 73, 73));
	m_Btn_RecordOpt.LarSetEraseBkEnable(false);
	m_Btn_RecordOpt.LarSetButtonNoInteraction(true);
	m_Btn_RecordOpt.LaSetTextHoverColor(Color(255, 255, 255, 255));
	m_Btn_RecordOpt.LaSetTextClickedColor(Color(255, 255, 255, 255));
	m_Btn_RecordOpt.LarSetNormalFiilBrush(SolidBrush(Color(36, 37, 40)));
	m_Btn_RecordOpt.LarSetHoverFillBrush(SolidBrush(Color(26, 27, 32)));
	m_Btn_RecordOpt.LarSetBtnNailImage(MAINDLG_PNG_DOWN,
		CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
	);
	m_Btn_RecordOpt.LarAdjustTextDisplayPos(-8 * m_Scale, 0);

	CRect CameraDevicebtnRect;
	m_Btn_CameraDevice.GetWindowRect(&CameraDevicebtnRect);
	m_PopupListBox.addListBox(CameraDevicebtnRect.Width(), CameraDevicebtnRect.Height(),
		this, m_Array_CameraDeviceList, L"摄像头设备下拉框");

	CRect CameraParambtnRect;
	m_Btn_CameraParam.GetWindowRect(&CameraParambtnRect);
	m_PopupListBox.addListBox(CameraParambtnRect.Width(), CameraParambtnRect.Height(),
		this, m_Array_CameraParamList, L"摄像头参数下拉框");

	CRect SystemAudioDevicebtnRect;
	m_Btn_SystemAudioDevice.GetWindowRect(&SystemAudioDevicebtnRect);
	m_PopupListBox.addListBox(SystemAudioDevicebtnRect.Width(), SystemAudioDevicebtnRect.Height(),
		this, m_Array_SystemAudioList, L"系统音频设备下拉框");

	CRect MicroAudiobtnRect;
	m_Btn_MicroAudio.GetWindowRect(&MicroAudiobtnRect);
	m_PopupListBox.addListBox(MicroAudiobtnRect.Width(), MicroAudiobtnRect.Height(),
		this, m_Array_MicroAudioList, L"麦克风设备下拉框");

	CRect RecordOptionBtnRect;
	m_Btn_RecordOpt.GetWindowRect(&RecordOptionBtnRect);
	m_PopupListBox.addListBox(RecordOptionBtnRect.Width(), RecordOptionBtnRect.Height(),
		this, m_Array_RecordOptList, L"录制选项下拉框");

	m_PopupListBox.SetMaxDisplayItems(7, L"录制选项下拉框");
	m_PopupListBox.SetScrollbarWidth(9);

	//设置参数为下拉框中的第一个
	m_Btn_CameraDevice.SetWindowTextW(m_Array_CameraDeviceList.GetAt(0));
	m_Btn_CameraParam.SetWindowTextW(m_Array_CameraParamList.GetAt(0));
	m_Btn_SystemAudioDevice.SetWindowTextW(m_Array_SystemAudioList.GetAt(0));
	m_Btn_MicroAudio.SetWindowTextW(m_Array_MicroAudioList.GetAt(0));
	m_Btn_RecordOpt.SetWindowTextW(L"录摄像头");

	////图片按钮
	//刷新
	m_Btn_Refresh.LoadPNG(WWDLG_PNG_REFRESH);
	m_Btn_Refresh.SetBackgroundColor(RGB(26, 27, 32));

	//返回
	m_Btn_Return.LoadPNG(WWDLG_PNG_RETURN);
	m_Btn_Return.SetBackgroundColor(RGB(26, 27, 32));

	//REC开始录制按钮
	m_Btn_Rec.LoadPNG(WWDLG_PNG_STARTRECORD);
	m_Btn_Rec.SetBackgroundColor(RGB(26, 27, 32));

	////文本按钮
	//录制选项
	//系统音频高级选项
	m_Btn_SystemAudioAdvanceOpt.SetWindowText(L"高级选项");
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_SystemAudioAdvanceOpt);//摄像头参数设置

	//麦克风音频高级选项
	m_Btn_MicroAudioOpt.SetWindowText(L"高级选项");
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_MicroAudioOpt);//摄像头参数设置
	//打开摄像头
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_OpenCamera);
	m_Btn_OpenCamera.LarSetTextSize(18);
}

void Ui_CameraDlg::InitCtrl()
{
	InitTitleBarCtrl();
	InitMainAreaCtrl();
	InitalizeUi_CameraDisplaySDL();
}

void Ui_CameraDlg::SetButtonStyle(
	Gdiplus::Color BorderColor,
	Gdiplus::Color TextColor,
	Gdiplus::Color BkColor,
	CLarBtn& larbtn,
	int mode)
{
	if (mode == 0) {
		Gdiplus::SolidBrush BkBrush(BkColor);
		larbtn.LarSetTextSize(20);
		larbtn.LarSetTextStyle(false, false, false);
		larbtn.LaSetTextColor(TextColor);
		larbtn.LarSetBorderColor(BorderColor);
		larbtn.LarSetEraseBkEnable(false);
		larbtn.LarSetButtonNoInteraction(true);
		larbtn.LaSetTextHoverColor(TextColor);
		larbtn.LaSetTextClickedColor(TextColor);
		larbtn.LarSetNormalFiilBrush(BkBrush);
	}
}

void Ui_CameraDlg::InitalizeUi_CameraDisplaySDL()
{

}

Ui_CameraDlg::CameraParams Ui_CameraDlg::ParseCameraParamString(const CString& paramStr)
{
	// 提取摄像头下拉框中的参数
	CameraParams params; // 初始化默认值
	params.format = "";
	params.width = 0;
	params.height = 0;
	params.fps = 0;

	int startPos = 0;
	CString token;
	CString delimiter = _T(",");
	CString formatPrefix = _T("格式:");
	CString resolutionPrefix = _T("分辨率:");

	// 分割并处理每个部分
	while (startPos >= 0) {
		int endPos = paramStr.Find(delimiter, startPos);
		if (endPos == -1) {
			token = paramStr.Mid(startPos);
			startPos = -1;
		}
		else {
			token = paramStr.Mid(startPos, endPos - startPos);
			startPos = endPos + 1;
		}

		token.TrimLeft();
		token.TrimRight();

		if (token.Find(formatPrefix) == 0) {
			// CString转std::string
			CString formatValue = token.Mid(formatPrefix.GetLength());

#ifdef UNICODE
			// Unicode版本 - 需要先转换为多字节
			int len = WideCharToMultiByte(CP_ACP, 0, formatValue, -1, NULL, 0, NULL, NULL);
			char* buffer = new char[len];
			WideCharToMultiByte(CP_ACP, 0, formatValue, -1, buffer, len, NULL, NULL);
			params.format = std::string(buffer);
			delete[] buffer;
#else
			// ANSI版本 - 直接转换
			params.format = std::string(formatValue.GetString());
#endif
		}
		else if (token.Find(resolutionPrefix) == 0) {
			CString resolutionStr = token.Mid(resolutionPrefix.GetLength());
			int starPos = resolutionStr.Find('*');
			if (starPos != -1) {
				params.width = _ttoi(resolutionStr.Left(starPos));
				params.height = _ttoi(resolutionStr.Mid(starPos + 1));
			}
		}
		else if (token.Find(_T("FPS")) != -1) {
			int i = 0;
			while (i < token.GetLength() && !isdigit(token[i]))
				i++;

			int j = i;
			while (j < token.GetLength() && isdigit(token[j]))
				j++;

			if (j > i) {
				params.fps = _ttoi(token.Mid(i, j - i));
			}
		}
	}
	return params;
}

void Ui_CameraDlg::OpenCamera()
{
	App.UserOpenCarmerPreview_InCameraDlg(false);
	if (!m_Interface_pCDSDL)
	{
		m_Interface_pCDSDL.reset(Ui_CameraDisplaySDL::GetInstance());
		if (!m_Interface_pCDSDL)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[摄像头界面]: 获取 Ui_CameraDisplaySDL 实例失败");
			return;
		}
	}
	
	//获取摄像头显示区域(全局坐标)
	CRect CameraDisplayRect;
	CameraDisplayRect.SetRect(m_Rect_CameraDisplay.X, m_Rect_CameraDisplay.Y,
		m_Rect_CameraDisplay.GetRight(), m_Rect_CameraDisplay.GetBottom());
	this->ClientToScreen(CameraDisplayRect);

	//获取用户设置的摄像头参数
	CString ParamStr;
	m_Btn_CameraParam.GetWindowTextW(ParamStr);
	CameraParams cameraOption = ParseCameraParamString(ParamStr);
	CString DeviceNameWstr;
	m_Btn_CameraDevice.GetWindowTextW(DeviceNameWstr);
	CT2A pszConvertedAnsiString(DeviceNameWstr);
	const char* deviceName = pszConvertedAnsiString;
	auto deviceInfos = DeviceManager::GetInstance().GetCameraDevices();
	std::string desc = std::string();
	for (auto& deviceInfo : deviceInfos)
	{
		if (deviceInfo.nameA == deviceName)
		{
			desc = deviceInfo.alternateName;
		}
	}

	DBFMT(ConsoleHandle, L"cameraOption.format:%s,cameraOption.width:%d, cameraOption.height:%d,fps:%d",
		LARSC::s2ws(cameraOption.format).c_str(), cameraOption.width, cameraOption.height, cameraOption.fps);

	//初始化SDL摄像头渲染
	if (!m_Interface_pCDSDL.get()->Initialize(
		CameraDisplayRect,
		deviceName,
		desc.c_str(),
		cameraOption.fps,
		cameraOption.format.c_str(),
		cameraOption.width,
		cameraOption.height))
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"[摄像头界面]: SDL摄像头窗口初始化失败，放弃运行渲染线程");
		return;
	}
	m_Interface_pCDSDL.get()->RunSDLWindow();

	App.UserOpenCarmerPreview_InCameraDlg(true);
}

void Ui_CameraDlg::LoadRes()
{

}

BOOL Ui_CameraDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

LRESULT Ui_CameraDlg::OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam)
{
	HideUserProfile();
	MsgParam::LISTBOX_SELECT_INFO* pInfo = (MsgParam::LISTBOX_SELECT_INFO*)wParam;
	if (pInfo)
	{
		int nIndex = pInfo->nIndex;
		CString strText = pInfo->strText;
		std::wstring strBoxName = pInfo->strBoxName;

		// 根据不同的下拉框名称进行不同处理
		if (strBoxName == L"摄像头设备下拉框")
		{
			m_Btn_CameraDevice.SetWindowText(strText);
		}
		else if (strBoxName == L"摄像头参数下拉框")
		{
			m_Btn_CameraParam.SetWindowText(strText);
		}
		if (strBoxName == L"系统音频设备下拉框")
		{
			m_Btn_SystemAudioDevice.SetWindowText(strText);
		}
		if (strBoxName == L"麦克风设备下拉框")
		{
			m_Btn_MicroAudio.SetWindowText(strText);
		}
		else if (strBoxName == L"录制选项下拉框")
		{
			m_Btn_RecordOpt.SetWindowText(strText);
		}
	}
	ReturnToPage();
	return 1;
}

void Ui_CameraDlg::ReturnToPage()
{
	CString mode;
	m_Btn_RecordOpt.GetWindowTextW(mode);
	m_PopupListBox.HideListBox();
	if (mode == L"录摄像头")
	{//不需要转到其他窗口
		return;
	}

	if (m_IsRecording)
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		Ui_CameraDisplaySDL::GetInstance()->HideSDLWindow();
		if (MessageDlg.DoModal() == IDOK)
		{
			this->OnBnClickedBtnStartrecord();//停止录制
		}
		else
		{
			return;
		}
	}
	if (m_IsCameraOpening)
	{
		this->OnBnClickedBtnOpencarmera();//关闭摄像头
	}

	if (m_IsRecording)
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			this->OnBnClickedBtnStartrecord();//停止录制
		}
		else
		{
			return;
		}
	}
	if (m_IsCameraOpening)
	{
		this->OnBnClickedBtnOpencarmera();//关闭摄像头
	}

	//需要转到其他窗口
	m_Btn_RecordOpt.SetWindowTextW(L"录摄像头");
	CRect rect, parRect;
	auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	GetWindowRect(rect);
	pDlg->GetWindowRect(parRect);
	int widthpar = parRect.Width();
	int heightpar = parRect.Height();
	parRect.left = rect.left + (rect.Width() - widthpar) / 2;
	parRect.top = rect.top + (rect.Height() - heightpar) / 2;
	parRect.right = parRect.left + widthpar;
	parRect.bottom = parRect.top + heightpar;
	pDlg->MoveWindow(parRect);
	if (mode == L"录全屏")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		pDlg->ShowWindow(SW_SHOW);
		pDlg->m_Dlg_Child->m_IsMouseAreaRecord = false;
		pDlg->m_Dlg_Child->m_IsOnlyAudioRecord = false;
		pDlg->OnBnClickedBtnScreenRecord();
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录区域")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		pDlg->ShowWindow(SW_SHOW);
		pDlg->OnBnClickedBtnArearecording();
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录游戏")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		pDlg->ShowWindow(SW_SHOW);
		pDlg->OnBnClickedBtnGamingrecording();
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录窗口")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		pDlg->OnBnClickedBtnAppwindowrecording();
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录声音")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		this->GetParent()->ShowWindow(SW_SHOW);
		::PostMessage(this->GetParent()->GetSafeHwnd(), MSG_UIDROPDOWNMENU_SYSTEMAUDIOMICRO, NULL, NULL);
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"跟随鼠标录制")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		this->GetParent()->ShowWindow(SW_SHOW);
		::PostMessage(this->GetParent()->GetSafeHwnd(), MSG_UIDROPDOWNMENU_MOUSERECORD, NULL, NULL);
		m_Shadow.Show(m_hWnd);
	}
}

void Ui_CameraDlg::OpenConfigDlg()
{
	m_Shadow.Show(m_hWnd);
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
	if (!m_Dlg_Config) {
		m_Dlg_Config = new Ui_ConfigDlg;
		m_Dlg_Config->Ui_SetWindowRect(WindowRect);
		if (!m_Dlg_Config->Create(IDD_DIALOG_CONFIGDLG, this)) {
			delete m_Dlg_Config;
			m_Dlg_Config = nullptr;
			return;
		}
	}
	::SetWindowPos(m_Dlg_Config->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	this->EnableWindow(FALSE);// 禁用主窗口以模拟模态行为
	// 更新位置并显示配置窗口
	m_Dlg_Config->Ui_UpdateWindowPos(WindowRect);
	m_Dlg_Config->SetForegroundWindow();
	m_Dlg_Config->ShowWindow(SW_SHOW);
	m_Dlg_Config->m_bool_IsEraseTopMost = false;
	m_Dlg_Config->SetTimer(Ui_ConfigDlg::TIMER_DELAYED_REDRAW, 50, NULL);
}

void Ui_CameraDlg::HideUserProfile()
{
	auto pDlg = App.m_Dlg_Main.getProfileDlg();
	pDlg->ShowWindow(SW_HIDE);
	m_Btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale);
	Invalidate();
}

void Ui_CameraDlg::UpdateLoginUi()
{
	if (m_IsloginUi)
		return;
	//显示控件
	m_Btn_Phone.SetWindowTextW(App.m_userInfo.nickname);
	m_Btn_login.ShowWindow(SW_HIDE);
	m_Btn_Phone.ShowWindow(SW_SHOW);
	if (App.m_IsVip || App.m_IsNonUserPaid)
	{
		m_Btn_VipLogo.ShowWindow(SW_SHOW);
	}
	if ((!App.m_IsVip && !App.m_IsNonUserPaid))
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	else
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	}
	m_IsloginUi = true;
	Invalidate();
}

void Ui_CameraDlg::UpdateNoneUserPayUI()
{
	//显示控件
	if (App.m_IsNonUserPaid || App.m_IsVip)
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
		m_Btn_VipLogo.ShowWindow(SW_SHOW);
	}
	else
	{
		m_Btn_VipLogo.ShowWindow(SW_HIDE);
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	Invalidate(FALSE);
}

void Ui_CameraDlg::UpdateSignOutUi()
{
	if (!m_IsloginUi)
		return;
	m_Btn_login.ShowWindow(SW_SHOW);
	m_Btn_Phone.ShowWindow(SW_HIDE);
	m_Btn_VipLogo.ShowWindow(SW_HIDE);
	m_Btn_UserIcon.MoveWindow(m_Rect_BtnUserIcon_SignOutRect);
	if ((!App.m_IsVip && !App.m_IsNonUserPaid))
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	else
	{
		m_Btn_OpenVIP.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	}
	m_IsloginUi = false;
	Invalidate();
}

void Ui_CameraDlg::OnBnClickedBtnReturn()
{
	HideUserProfile();
	m_PopupListBox.HideListBox();
	if (m_IsRecording)
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		Ui_CameraDisplaySDL::GetInstance()->HideSDLWindow();
		if (MessageDlg.DoModal() == IDOK)
		{
			this->OnBnClickedBtnStartrecord();//停止录制
		}
		else
		{
			return;
		}
	}
	if (m_IsCameraOpening)
	{
		this->OnBnClickedBtnOpencarmera();//关闭摄像头
	}


	//出现父亲对话框，本对话隐藏
	ShowWindow(SW_HIDE);
	App.m_Dlg_Main.ShowWindow(SW_SHOW);
	App.m_Dlg_Main.SetTrayClickCallback([=]()
		{
			App.m_Dlg_Main.ShowWindow(SW_SHOW);
			App.m_Dlg_Main.SetForegroundWindow();
		});

	m_Shadow.Show(this->GetSafeHwnd());
}

void Ui_CameraDlg::OnBnClickedBtnClose()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	//关闭程序
	reinterpret_cast<Ui_MainDlg*>(GetParent())->OnBnClickedBtnClose();
}

void Ui_CameraDlg::OnBnClickedBtnCbcarmeradevice()
{
	HideUserProfile();
	if (m_PopupListBox.IsListBoxVisiable(L"摄像头设备下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_CameraDevice.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"摄像头设备下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_CameraDlg::OnBnClickedBtnCbcameraparam()
{
	HideUserProfile();
	if (m_PopupListBox.IsListBoxVisiable(L"摄像头参数下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_CameraParam.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"摄像头参数下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_CameraDlg::OnBnClickedBtnCbaudiodevice()
{
	HideUserProfile();
	if (m_PopupListBox.IsListBoxVisiable(L"系统音频设备下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_SystemAudioDevice.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"系统音频设备下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_CameraDlg::OnBnClickedBtnCbmicrodevice()
{
	HideUserProfile();
	if (m_PopupListBox.IsListBoxVisiable(L"麦克风设备下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_MicroAudio.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"麦克风设备下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_CameraDlg::OnBnClickedBtnRecordopt()
{
	HideUserProfile();
	if (m_PopupListBox.IsListBoxVisiable(L"录制选项下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	CRect Rect;
	m_Btn_RecordOpt.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"录制选项下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_CameraDlg::OnBnClickedBtnStartrecord()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	if (m_IsHasCameraDevice)
	{
		if (!m_IsRecording)
		{
			App.UserTriggerRecord();
			if (App.m_IsOverBinds)
			{
				if (ModalDlg_MFC::ShowModal_OverBindsTipsBeforeRecord() == IDCANCEL)
					return;
			}

			if (!App.m_IsVip && !App.m_IsNonUserPaid)
			{
				if (m_IsCameraOpening)
					OnBnClickedBtnOpencarmera();
				if (!ModalDlg_SDL::ShowModal_Rai(this, m_Scale))
				{
					//点击了开通会员
					App.m_Dlg_Main.OnBnClickedBtnOpenvip();
					return;
				}
			}
			App.UserCarmeraRecord(false);
			if (!m_IsCameraOpening)
			{   //先打开摄像头
				OpenCamera();
			}
			//等待摄像头渲染开始
			if (m_Interface_pCDSDL)
			{
				if (m_Interface_pCDSDL.get()->WaitForRenderingToStart())
				{
					auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
					//获取用户选择的编码器
					CString codecText;
					pDlg->m_Dlg_Config->m_Btn_VideoCodec.GetWindowTextW(codecText);
					std::string encoderName = GlobalFunc::MapUICodecToEncoderName(codecText);
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"选择编码器: %s -> %hs",
						codecText, encoderName.empty() ? "软件编码器" : encoderName.c_str());

					//获取用户选择的麦克风
					CString MicroDevice;
					Ui_CameraDisplaySDL::RecordMode recordMode;
					pDlg->m_Dlg_Config->m_Btn_MicroDevice.GetWindowTextW(MicroDevice);
					if (MicroDevice == L"无可用设备")
					{
						recordMode = Ui_CameraDisplaySDL::RecordMode::VideoOnly;
					}
					recordMode = Ui_CameraDisplaySDL::RecordMode::Both;

					//构造保存的文件路径，文件名，音频设备
					CString SavePath, fileFormat, deviceNameW, microNameW;
					pDlg->m_Dlg_Config->m_Btn_Path.GetWindowTextW(SavePath);
					//pDlg->m_Dlg_Config->m_Btn_VideoFormat.GetWindowTextW(fileFormat);
					fileFormat = L"MP4";
					pDlg->m_Dlg_Config->m_Btn_AudioDevice.GetWindowTextW(deviceNameW);
					pDlg->m_Dlg_Config->m_Btn_MicroDevice.GetWindowTextW(microNameW);
					CString filePath = GlobalFunc::BuildVideoFilePath(SavePath, fileFormat);
					std::wstring filePathStringW = filePath;
					std::string filePathString = LARSC::ws2s(filePathStringW);

					// 设置编码器后开始录制
					m_Interface_pCDSDL.get()->SetVideoEncoder(encoderName);
					if (m_Interface_pCDSDL.get()->StartRecording(filePathString, recordMode))
					{
						DEBUG_CONSOLE_STR(ConsoleHandle, L"摄像头开始录制");
						m_Str_LastRecordFilePath = LARSC::s2ws(filePathString).c_str();
						m_IsRecording = true;
						m_IsCameraOpening = true;

						SetUiModeDuringRecord();
						if (!App.m_IsVip && !App.m_IsNonUserPaid)
							SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, NULL);
						if (App.m_IsOverBinds && !App.m_IsNonUserPaid)//如果超限，1分钟后触发
							SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, NULL);
					}
				}
			}
			else
			{// 渲染未开始，处理错误
				DEBUG_CONSOLE_STR(ConsoleHandle, L"摄像头渲染未开始，无法录制");
			}
		}
		else
		{
			App.UserCarmeraRecord(true);
			if (m_Interface_pCDSDL.get())
			{
				m_Interface_pCDSDL.get()->StopRecording();
				m_Interface_pCDSDL.reset();
			}
			KillTimer(TIMER_NONEVIP_RECORDTIME);
			m_IsCameraOpening = false;
			m_IsRecording = false;
			App.m_Dlg_Main.UpdateRecentRecordFile(m_Str_LastRecordFilePath);
			App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(m_Str_LastRecordFilePath);
			ModalDlg_MFC::ShowModal_Priview(this);
		}

		if (m_IsRecording)
		{
			m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_ISRECORDING);
			m_Btn_Rec.LoadPNG(CHILDDLG_PNG_ISRECORDING);
			Invalidate();
		}
		else
		{
			ResumeUiDuringNormal();
			Invalidate();
		}
	}
	else
	{
		ModalDlg_MFC::ShowModal_NoDeviceAvailable();
	}
}

void Ui_CameraDlg::SetUiModeDuringRecord()
{
	m_Btn_Config.LarSetBtnEnable(false);
	m_Btn_Config.LaSetTextHoverColor(Color(155, 255, 255));
	m_Btn_login.LarSetBtnEnable(false);
	m_Btn_login.LaSetTextHoverColor(Color(155, 255, 255));
	m_Btn_FeedBack.LarSetBtnEnable(false);
	m_Btn_FeedBack.LaSetTextHoverColor(Color(155, 255, 255));

	m_Btn_OpenCamera.LarSetBtnEnable(false);
	m_Btn_OpenCamera.LarSetNormalFiilBrush(SolidBrush(Color(50, 26, 37, 40)));
	m_Btn_SystemAudioAdvanceOpt.LarSetBtnEnable(false);
	m_Btn_SystemAudioAdvanceOpt.LarSetNormalFiilBrush(SolidBrush(Color(50, 26, 37, 40)));
	m_Btn_MicroAudioOpt.LarSetBtnEnable(false);
	m_Btn_MicroAudioOpt.LarSetNormalFiilBrush(SolidBrush(Color(50, 26, 37, 40)));
	m_Btn_OpenCamera.SetWindowTextW(L"关闭摄像头");

	m_Btn_OpenVIP.ShowWindow(SW_HIDE);
}

void Ui_CameraDlg::ResumeUiDuringNormal()
{
	m_Btn_OpenCamera.SetWindowTextW(L"打开摄像头");
	m_Btn_OpenCamera.LarSetBtnEnable(true);
	m_Btn_MicroAudioOpt.LarSetBtnEnable(true);
	m_Btn_SystemAudioAdvanceOpt.LarSetBtnEnable(true);

	Color ComboBox_TextColor(202, 202, 202);//下拉框文本颜色
	Color ComboBox_BorderColor(67, 67, 67); //下拉框边框颜色
	Color ComboBox_BkColor(36, 37, 40);		//下拉框背景颜色
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_OpenCamera);
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_SystemAudioAdvanceOpt);
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor,
		m_Btn_MicroAudioOpt);
	m_Btn_OpenCamera.LarSetTextSize(16);
	m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);

	m_Btn_Config.LarSetBtnEnable(true);
	m_Btn_Config.LaSetTextColor(Color(155, 255, 255, 255));
	m_Btn_Config.LaSetTextHoverColor(Color(255, 255, 255, 255));

	m_Btn_FeedBack.LarSetBtnEnable(true);
	m_Btn_FeedBack.LaSetTextColor(Color(155, 255, 255, 255));
	m_Btn_FeedBack.LaSetTextHoverColor(Color(255, 255, 255, 255));

	m_Btn_login.LarSetBtnEnable(true);
	m_Btn_login.LaSetTextColor(Color(155, 255, 255, 255));
	m_Btn_login.LaSetTextHoverColor(Color(255, 255, 255, 255));

	m_Btn_OpenVIP.ShowWindow(SW_SHOW);
}

void Ui_CameraDlg::OnBnClickedBtnOpencarmera()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	if (m_IsHasCameraDevice)
	{
		if (m_IsCameraOpening)
		{
			App.UserCloseCarmerPreview_InCameraDlg(false);
			if (m_Interface_pCDSDL.get())
			{
				m_Interface_pCDSDL.reset();
			}
			m_IsCameraOpening = false;
			m_Btn_OpenCamera.SetWindowTextW(L"打开摄像头");
			App.UserCloseCarmerPreview_InCameraDlg(true);
		}
		else
		{
			OpenCamera();
			m_IsCameraOpening = true;
			m_Btn_OpenCamera.SetWindowTextW(L"关闭摄像头");
		}
	}
	else
	{
		ModalDlg_MFC::ShowModal_NoDeviceAvailable();
	}
}

void Ui_CameraDlg::OnBnClickedBtnMinmal()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	ShowWindow(SW_MINIMIZE);
	if (m_Interface_pCDSDL.get())
	{
		m_Interface_pCDSDL.get()->MinimalSDLWindow();
	}
	App.m_Dlg_Main.SetTrayClickCallback([this]()
		{
			this->ShowWindow(SW_SHOW);
			this->ShowWindow(SW_RESTORE);
			this->SetForegroundWindow();
		});
}

void Ui_CameraDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

void Ui_CameraDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_NONEVIP_RECORDTIME)
	{
		KillTimer(TIMER_NONEVIP_RECORDTIME);
		if (m_Interface_pCDSDL.get())
		{
			m_Interface_pCDSDL.get()->StopRecording();
			m_Interface_pCDSDL.reset();
		}
		m_IsCameraOpening = false;
		m_IsRecording = false;
		App.m_Dlg_Main.UpdateRecentRecordFile(m_Str_LastRecordFilePath);
		App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(m_Str_LastRecordFilePath);
		ResumeUiDuringNormal();
		App.UserNoneVipRecordSuccess();
		//弹框提示用户无法继续录制的原因
		if (!App.m_IsVip && !App.m_IsNonUserPaid)//不是vip
			ModalDlg_MFC::ShowModal_TrialOver(this);
		else if (App.m_IsOverBinds)//设备绑定超限
			ModalDlg_MFC::ShowModal_OverBindsTips();
	}
	CDialogEx::OnTimer(nIDEvent);
}

LRESULT Ui_CameraDlg::On_BroadCast_UserLogin(WPARAM wParam, LPARAM lParam)
{
	UpdateLoginUi();
	return 1;
}

LRESULT Ui_CameraDlg::On_BroadCast_UserLogOut(WPARAM wParam, LPARAM lParam)
{
	UpdateSignOutUi();
	return 1;
}

LRESULT Ui_CameraDlg::On_SDLBnClick_UserLogOut(WPARAM wParam, LPARAM lParam)
{
	bool logoutSuccess = App.RequestSignOut();
	if (logoutSuccess)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"退出登录请求成功，更新UI状态");
		UpdateSignOutUi();
		App.m_isLoginIn = false;

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

LRESULT Ui_CameraDlg::On_UserProfileDlg_WindowHidenByTimer(WPARAM wParam, LPARAM lParam)
{
	HideUserProfile();
	return LRESULT();
}

void Ui_CameraDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	//m_Shadow.Show(m_hWnd);
}

void Ui_CameraDlg::OnEnterSizeMove()
{
	if (m_Interface_pCDSDL.get())
	{//暂停摄像头渲染
		m_Interface_pCDSDL->PauseRendering();
		m_Interface_pCDSDL->HideSDLWindow();
	}
	//m_Shadow.Show(m_hWnd);
}

void Ui_CameraDlg::OnExitSizeMove()
{
	if (m_Interface_pCDSDL.get())
	{//恢复摄像头渲染
		m_Interface_pCDSDL->ResumeRendering();
		m_Interface_pCDSDL->ShowSDLWindow();

		//更新SDL窗口位置
		CRect cameraDisplayRect;
		cameraDisplayRect.SetRect(m_Rect_CameraDisplay.X, m_Rect_CameraDisplay.Y,
			m_Rect_CameraDisplay.GetRight(), m_Rect_CameraDisplay.GetBottom());
		ClientToScreen(cameraDisplayRect);
		m_Interface_pCDSDL->UpdateSDLWindowPos(cameraDisplayRect);
	}
	//m_Shadow.Show(m_hWnd);
}

void Ui_CameraDlg::EnableShadow()
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

BOOL Ui_CameraDlg::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//EnableShadow();
	return CDialogEx::OnNcActivate(bActive);
}

void Ui_CameraDlg::OnBnClickedBtnPhone()
{
	m_PopupListBox.HideListBox();
	auto pDlg = (Ui_UserProfileDlg*)(App.m_Dlg_Main.getProfileDlg());//获取对话框指针
	if (!pDlg)
	{
		pDlg = new Ui_UserProfileDlg;
		pDlg->Create(IDD_DIALOG_USERPROFILE, NULL);
	}

	if (pDlg && (!pDlg->IsWindowVisible()))
	{//显示窗口 
		//定义窗口显示位置 
		CRect prect;
		m_Btn_Phone.GetWindowRect(prect);
		CRect dlgrect;
		pDlg->GetWindowRect(dlgrect);
		int x = prect.left - dlgrect.Width() / 2;
		int y = prect.bottom + 10 * m_Scale;

		pDlg->ShowAtXY(x, y);//显示用户信息窗口
		pDlg->UpdateUserInfo();

		//更新控件状态 
		m_Btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_UP, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale);
		Invalidate();
		return;
	}
	else if (pDlg && (pDlg->IsWindowVisible()))
	{//隐藏窗口
		HideUserProfile();
		return;
	}
	else if (!pDlg)
	{
		DB(ConsoleHandle, L"错误！获取的Ui_UserProfileDlg窗口指针为空！调用OnBnClickedBtnPhone失败！");
		return;
	}
	else
	{
		DB(ConsoleHandle, L"未知错误！调用OnBnClickedBtnPhone失败！");
		return;
	}
	//ShowUserProfileSDL();
}

void Ui_CameraDlg::ShowUserProfileSDL()
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
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
				});
			sdlWindow->SetOnContactSupportCallback([this]()
				{
					DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL用户资料窗口]: 联系客服回调");
				});

			// 初始化和运行窗口
			CRect BtnRect;
			m_Btn_login.GetWindowRect(BtnRect);
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

void Ui_CameraDlg::OnBnClickedBtnConfig()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	OpenConfigDlg();
}

void Ui_CameraDlg::OnBnClickedBtnAdvanceopt()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	OpenConfigDlg();
}

void Ui_CameraDlg::OnBnClickedBtnMicroadvance()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	OpenConfigDlg();
}

void Ui_CameraDlg::OnBnClickedBtnLogin()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	if (!App.m_isLoginIn)
	{
		// 确保释放之前的对话框（如果有）
		if (pDlg->m_Dlg_Login != nullptr)
		{
			pDlg->m_Dlg_Login->DestroyWindow();
			delete pDlg->m_Dlg_Login;
			pDlg->m_Dlg_Login = nullptr;
		}

		// 创建新的对话框实例
		pDlg->m_Dlg_Login = new Ui_LoginDlg(this);
		int loginWindowWidth = 350 * m_Scale;
		int loginWindowHeight = 390 * m_Scale;

		// 设置位置
		CRect WindowRect, loginWindowRect;
		GetWindowRect(WindowRect);
		loginWindowRect.left = WindowRect.left + (WindowRect.Width() - loginWindowWidth) / 2;
		loginWindowRect.top = WindowRect.top + (WindowRect.Height() - loginWindowHeight) / 2;
		loginWindowRect.right = loginWindowRect.left + loginWindowWidth;
		loginWindowRect.bottom = loginWindowRect.top + loginWindowHeight;
		pDlg->m_Dlg_Login->Ui_SetWindowRect(loginWindowRect);

		// 以模态方式显示对话框
		DEBUG_CONSOLE_STR(ConsoleHandle, L"显示登录对话框");
		INT_PTR result = pDlg->m_Dlg_Login->DoModal();
		// 处理结果
		if (result == IDOK)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"登录成功，开始获取用户信息");
			DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
			::PostMessage(App.m_pMainWnd->GetSafeHwnd(), BROADCASTMSG_USERLOGIN_ISLOGININ, NULL, NULL);
			ModalDlg_MFC::ShowModal_LoginSuccess();
		}
		else if (result == IDCANCEL)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"用户取消了登录");
		}
	}
}

void Ui_CameraDlg::OnBnClickedBtnUsericon()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	if (App.m_isLoginIn)
	{
		OnBnClickedBtnPhone();
	}
	else
	{
		OnBnClickedBtnLogin();
	}
}

void Ui_CameraDlg::OnBnClickedBtnOpenvip()
{
	m_PopupListBox.HideListBox();
	HideUserProfile();
	if (App.m_isLoginIn)
	{
		// 定义窗口显示位置和大小
		CRect WindowRect;
		GetWindowRect(WindowRect);
		float Width = 762 * m_Scale;
		float Height = 460 * m_Scale;
		WindowRect.top = WindowRect.top + (WindowRect.Height() - Height) / 2;
		WindowRect.left = WindowRect.left + (WindowRect.Width() - Width) / 2;
		WindowRect.right = WindowRect.left + Width;
		WindowRect.bottom = WindowRect.top + Height;

		// 创建模态对话框
		if (m_Dlg_VipPay != nullptr) {
			delete m_Dlg_VipPay;  // 确保删除旧对象，避免内存泄漏
			m_Dlg_VipPay = nullptr;
		}

		m_Dlg_VipPay = new Ui_VipPayDlg(this);
		m_Dlg_VipPay->Ui_SetWindowPos(WindowRect);  // 设置窗口大小和位置

		INT_PTR result = m_Dlg_VipPay->DoModal();
		if (result == IDOK)
		{
			// 支付成功，获取用户信息
			DEBUG_CONSOLE_STR(ConsoleHandle, L"支付成功，正在获取用户最新信息...");
			if (App.RequestDeviceInfo())
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
				Invalidate(false);
				ModalDlg_MFC::ShowModal_PaySuccess(this);
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取失败");
			}
		}
		else if (result == IDCANCEL)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"用户取消了支付");
		}
		m_Dlg_VipPay = nullptr;
	}
	else
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"抱歉，请登录后进行此操作", L"确认");
		MessageDlg.DoModal();
	}
}

void Ui_CameraDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	HideUserProfile();
	m_PopupListBox.HideListBox();
	CDialogEx::OnNcLButtonDown(nHitTest, point);
}

BOOL Ui_CameraDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::PreCreateWindow(cs);
}

void Ui_CameraDlg::OnBnClickedBtnMenu()
{
	m_PopupListBox.HideListBox();
	App.OpenFeedBackLink(
		this->GetSafeHwnd(),
		L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
	);
}

void Ui_CameraDlg::OnBnClickedBtnViplogo()
{
	m_PopupListBox.HideListBox();
	// TODO: 在此添加控件通知处理程序代码
}

void Ui_CameraDlg::OnBnClickedBtnRefreash()
{
	m_PopupListBox.HideListBox();
	// TODO: 在此添加控件通知处理程序代码
}
