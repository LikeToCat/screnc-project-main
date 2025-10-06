// Ui_WindowRecordDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_WindowRecordDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "WindowHandleManager.h"
#include "CDebug.h"
#include "CMessage.h"
#include "DeviceManager.h"
#include "ScreenRecorder.h"
#include "GlobalFunc.h"
#include "Ui_MainDlg.h"
#include "PngLoader.h"
#include "theApp.h"
#include "ModalDialogFunc.h"
#include "Ui_VipPayDlg.h"

#ifdef TARGET_WIN10
#define PW_RENDERFULLCONTENT 0x00000002
#elif defined TARGET_WIN7
#define PW_RENDERFULLCONTENT 0

#endif

extern HANDLE ConsoleHandle;
// Ui_WindowRecordDlg 对话框
static int kThumbnailTitleHeight = 20;						// 每个缩略图底部标题区高度
static int kImagePadding = 5;								// 缩略图内部图片与边框的留白
static const UINT WINDOWRECDLG_TIMER_TIMERECORDED = 5001;
IMPLEMENT_DYNAMIC(Ui_WindowRecordDlg, CDialogEx)

Ui_WindowRecordDlg::Ui_WindowRecordDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_WINDOWRECORD, pParent)
{
	m_nonVipRemainingMs = 0;
	m_nonVipTimerStartTick = 0;
	m_nonVipTimerActive = false;

	m_Bool_IsMouseInThumbnail = false;
	m_Rect_CurrnetHovertTumbnailRect.X = 0;
	m_Rect_CurrnetHovertTumbnailRect.Y = 0;
	m_Rect_CurrnetHovertTumbnailRect.Width = 0;
	m_Rect_CurrnetHovertTumbnailRect.Height = 0;

	m_Rect_LastHovertTumbnailRect.X = 0;
	m_Rect_LastHovertTumbnailRect.Y = 0;
	m_Rect_LastHovertTumbnailRect.Width = 0;
	m_Rect_LastHovertTumbnailRect.Height = 0;

	m_Hwnd_CurSelect = nullptr;
	m_Dlg_VipPay = nullptr;

	if (App.IsWin7Over())
	{
		m_captureflag = 2;
	}
	else
	{
		m_captureflag = 0;
	}
}

Ui_WindowRecordDlg::~Ui_WindowRecordDlg()
{
	CleanUpWindowImage();
	if (m_Bitmap_AddWindow)
	{
		delete m_Bitmap_AddWindow;
	}
	if (m_windowRecordUI.bkBrush)        delete m_windowRecordUI.bkBrush;
	if (m_windowRecordUI.borderPen)      delete m_windowRecordUI.borderPen;
	if (m_windowRecordUI.timeTextBrush)  delete m_windowRecordUI.timeTextBrush;
	if (m_windowRecordUI.sizeTextBrush)  delete m_windowRecordUI.sizeTextBrush;
	if (m_windowRecordUI.timerFont)      delete m_windowRecordUI.timerFont;
	if (m_windowRecordUI.sizeFont)       delete m_windowRecordUI.sizeFont;
}

void Ui_WindowRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, WWDLG_BTN_APPICON, m_Btn_AppIcon);
	DDX_Control(pDX, WWDLG_STAT_LOGOTEXT, m_Stat_LogoText);
	DDX_Control(pDX, WWDLG_BTN_PROFILEICON, m_Btn_PrifileIcon);
	DDX_Control(pDX, WWDLG_BTN_USERNAME, m_Btn_Username);
	DDX_Control(pDX, WWDLG_BTN_OPENVIP, m_Btn_OpenVip);
	DDX_Control(pDX, WWDLG_BTN_CONFIG, m_Btn_Config);
	DDX_Control(pDX, WWDLG_BTN_MENU, m_Btn_FeedBack);
	DDX_Control(pDX, WWDLG_BTN_MINIMAL, m_Btn_Minimal);
	DDX_Control(pDX, WWDLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, WWDLG_BTN_RETURN, m_Btn_Return);
	DDX_Control(pDX, WWDLG_BTN_COMBOX, m_Btn_ComBox);
	DDX_Control(pDX, WWDLG_BTN_REFRESH, m_Btn_Refresh);
	DDX_Control(pDX, WWDLG_BTN_STARTRECORD, m_Btn_StartRecord);
	DDX_Control(pDX, WWDLG_BTN_COMBOXOPTION1, m_Btn_RecordWindowOption);
	DDX_Control(pDX, WWDLG_BTN_COMBOXOPTION2, m_Btn_RecordMode);
	DDX_Control(pDX, WWDLG_BTN_COMBOXOPTION3, m_Btn_SystemAudioOption);
	DDX_Control(pDX, WWDLG_BTN_COMBOXOPTION4, m_Btn_MicroAudioOption);
	DDX_Control(pDX, WWDLG_STAT_RECORDWINDOW, m_Stat_RecordWindow);
	DDX_Control(pDX, WWDLG_STAT_RECORDMODE, m_Stat_RecordMode);
	DDX_Control(pDX, WWDLG_STAT_SYSTEMAUDIO, m_Stat_SystemAudio);
	DDX_Control(pDX, WWDLG_STAT_MICROAUDIO, m_Stat_MicroAudio);
	DDX_Control(pDX, WWDLG_BTN_RESELECTRECORDWINDOW, m_Btn_RecordWindowReselect);
	DDX_Control(pDX, WWDLG_BTN_TIPSOFRECORDMODE, m_Btn_TipsOfRecordMode);
	DDX_Control(pDX, WWDLG_BTN_ADVANCEOPTIONSYSTEMAUDIO, m_Btn_SystemAudioAdvanceOption);
	DDX_Control(pDX, WWDLG_BTN_ADVANCEMICROOPTION, m_Btn_MicroAudioAdvanceOption);
	DDX_Control(pDX, WWDLG_BTN_NOSELECT, m_Btn_NoSelect);
	DDX_Control(pDX, WINDOWRECDLG_STAT_HOTKEYSTARTRECORD, m_stat_hotKeyStartRecord);
	DDX_Control(pDX, WWDLG_BTN_PAUSE, m_Btn_WindowRecPause);
	DDX_Control(pDX, WWDLG_BTN_STOPRECORDING, m_Btn_WindowRecStop);
	DDX_Control(pDX, WWDLG_BTN_RESUME, m_Btn_WindowRecResume);
}

BEGIN_MESSAGE_MAP(Ui_WindowRecordDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	ON_WM_NCACTIVATE()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(WWDLG_BTN_RETURN, &Ui_WindowRecordDlg::OnBnClickedBtnReturn)
	ON_WM_PAINT()
	ON_MESSAGE(WM_USER + 100, OnNavButtonAction)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(WWDLG_BTN_CLOSE, &Ui_WindowRecordDlg::OnBnClickedBtnClose)

	//下拉框响应相关
	ON_BN_CLICKED(WWDLG_BTN_COMBOXOPTION1, &Ui_WindowRecordDlg::OnBnClickedBtnRecordwindowoption)
	ON_BN_CLICKED(WWDLG_BTN_COMBOXOPTION2, &Ui_WindowRecordDlg::OnBnClickedBtnRecordmode)
	ON_BN_CLICKED(WWDLG_BTN_COMBOXOPTION3, &Ui_WindowRecordDlg::OnBnClickedBtnSystemaudiooption)
	ON_BN_CLICKED(WWDLG_BTN_COMBOXOPTION4, &Ui_WindowRecordDlg::OnBnClickedBtnMicroaudiooption)
	ON_BN_CLICKED(WWDLG_BTN_REFRESH, &Ui_WindowRecordDlg::OnBnClickedBtnRefresh)
	ON_BN_CLICKED(WWDLG_BTN_MINIMAL, &Ui_WindowRecordDlg::OnBnClickedBtnMinimal)
	ON_BN_CLICKED(WWDLG_BTN_COMBOX, &Ui_WindowRecordDlg::OnBnClickedBtnCombox)
	ON_BN_CLICKED(WWDLG_BTN_ADVANCEOPTIONSYSTEMAUDIO, &Ui_WindowRecordDlg::OnBnClickedBtnAdvanceoptionsystemaudio)
	ON_BN_CLICKED(WWDLG_BTN_ADVANCEMICROOPTION, &Ui_WindowRecordDlg::OnBnClickedBtnAdvancemicrooption)
	ON_BN_CLICKED(WWDLG_BTN_RESELECTRECORDWINDOW, &Ui_WindowRecordDlg::OnBnClickedBtnReselectrecordwindow)
	ON_BN_CLICKED(WWDLG_BTN_STARTRECORD, &Ui_WindowRecordDlg::OnBnClickedBtnStartrecord)
	ON_BN_CLICKED(WWDLG_BTN_CONFIG, &Ui_WindowRecordDlg::OnBnClickedBtnConfig)
	ON_BN_CLICKED(WWDLG_BTN_PAUSE, &Ui_WindowRecordDlg::OnBnClickedBtnPauseWindowRecord)
	ON_BN_CLICKED(WWDLG_BTN_STOPRECORDING, &Ui_WindowRecordDlg::OnBnClickedBtnStopWindowRecord)
	ON_BN_CLICKED(WWDLG_BTN_RESUME, &Ui_WindowRecordDlg::OnBnClickedBtnResumeWindowRecord)

	//其他消息
	ON_MESSAGE(MSG_CLARLISTBOX_SELECTED, &Ui_WindowRecordDlg::OnBnClickedBtnListBoxSelected)
	ON_MESSAGE(MSG_WINDOWRECORDPAGE_CLICKAREARECORD, &Ui_WindowRecordDlg::OnBnClickBtnAreaRecord)
	ON_MESSAGE(MSG_WINDOWIMAGECAPTURE_COMPELETE, &Ui_WindowRecordDlg::OnCaptureCompelete)
	ON_MESSAGE(MSG_WINDOWRECORDPAGE_RECORDWINDOWMINIMAL, &Ui_WindowRecordDlg::OnRecordWindowMinimal)
	ON_MESSAGE(MSG_SCRENC_FILESIZEUPDATE, &Ui_WindowRecordDlg::On_WindowRecFileSizeUpdate)
	ON_MESSAGE(MSG_TOPRECDLG_STARTRECORD_WINDOW, &Ui_WindowRecordDlg::On_TopRec_StartRecord) 
	ON_MESSAGE(MSG_TOPRECDLG_STOPRECORD_WINDOW, &Ui_WindowRecordDlg::On_TopRec_StopRecord) 
	ON_MESSAGE(MSG_TOPRECDLG_PAUSE_WINDOW, &Ui_WindowRecordDlg::On_TopRec_PauseRecord)
	ON_MESSAGE(MSG_TOPRECDLG_RESUME_WINDOW, &Ui_WindowRecordDlg::On_TopRec_ResumeRecord)

	//广播
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGININ, &Ui_WindowRecordDlg::On_BroadCast_UserLogin)
	
	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SHOWWINDOW()
	ON_WM_EXITSIZEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_NCMOUSELEAVE()
	ON_WM_TIMER()
	ON_BN_CLICKED(WWDLG_BTN_MENU, &Ui_WindowRecordDlg::OnBnClickedBtnMenu)
	ON_BN_CLICKED(WWDLG_BTN_PROFILEICON, &Ui_WindowRecordDlg::OnBnClickedBtnProfileicon)
	ON_BN_CLICKED(WWDLG_BTN_USERNAME, &Ui_WindowRecordDlg::OnBnClickedBtnUsername)
	ON_BN_CLICKED(WWDLG_BTN_OPENVIP, &Ui_WindowRecordDlg::OnBnClickedBtnOpenvip)
	ON_BN_CLICKED(WWDLG_BTN_TIPSOFRECORDMODE, &Ui_WindowRecordDlg::OnBnClickedBtnTipsofrecordmode)
	ON_BN_CLICKED(WWDLG_BTN_NOSELECT, &Ui_WindowRecordDlg::OnBnClickedBtnNoselect)
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

BOOL Ui_WindowRecordDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return TRUE;
}

void Ui_WindowRecordDlg::GetUserDPI()
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

void Ui_WindowRecordDlg::UpdateScale()
{
	//设置窗口大小
	MoveWindow(m_CRect_WindowRect);
	float WindowWidth = m_CRect_WindowRect.Width();

	//标题栏区域
	float CRect_TopWidth = WindowWidth;
	float CRect_TopHeight = 52 * m_Scale;
	float CRect_TopX = 0;
	float CRect_TopY = 0;
	m_Rect_Top.X = CRect_TopX;
	m_Rect_Top.Y = CRect_TopY;
	m_Rect_Top.Width = CRect_TopWidth;
	m_Rect_Top.Height = CRect_TopHeight;

	//枚举出来的窗口显示区域 932 445
	m_Rect_WindowsDiplayArea.Width = 746 * m_Scale;
	m_Rect_WindowsDiplayArea.Height = 336 * m_Scale;
	m_Rect_WindowsDiplayArea.Y = 0.159 * m_WindowHeight;
	m_Rect_WindowsDiplayArea.X = (m_WindowWidth - m_Rect_WindowsDiplayArea.Width) / 2;

	//应用窗口图标
	CRect AppIconRect;
	m_Btn_AppIcon.GetClientRect(AppIconRect);
	float AppIconWidth = 35 * m_Scale;
	float AppIconHeight = 35 * m_Scale;
	float AppIconX = 5 * m_Scale;
	float AppIconY = 10 * m_Scale;
	m_Btn_AppIcon.MoveWindow(AppIconX, AppIconY, AppIconWidth, AppIconHeight);

	//极速录屏大师
	float StatLogoTextWidth = 140 * m_Scale;
	float StatLogoTextHeight = 22 * m_Scale;
	float StatLogoTextY = AppIconY + 3 * m_Scale;
	float StatLogoTextX = AppIconX + AppIconWidth;
	m_Stat_LogoText.MoveWindow(StatLogoTextX, StatLogoTextY, StatLogoTextWidth, StatLogoTextHeight);

	//用户头像
	CRect ProfileIconRect;
	m_Btn_PrifileIcon.GetClientRect(ProfileIconRect);
	float ProfileWidth = ProfileIconRect.Width();
	float ProfileHeight = ProfileIconRect.Height();
	float ProfileX = m_CRect_WindowRect.Width() * 0.541;
	float ProfileY = AppIconY;
	m_Btn_PrifileIcon.MoveWindow(ProfileX, ProfileY, ProfileWidth, ProfileHeight);

	//用户名
	CRect LoginTextRect;
	m_Btn_Username.GetClientRect(LoginTextRect);
	float LoginTextWidth = LoginTextRect.Width();
	float LoginTextHeight = LoginTextRect.Height();
	float LoginTextX = ProfileX + ProfileWidth;
	float LoginTextY = ProfileY + (ProfileHeight - LoginTextHeight) / 2 + 2 * m_Scale;
	m_Btn_Username.MoveWindow(LoginTextX, LoginTextY, LoginTextWidth, LoginTextHeight);

	//开通会员
	float OpenVipWidth = 93 * m_Scale;
	float OpenVipHeight = 43 * m_Scale;
	float OpenVipX = 0.66 * m_CRect_WindowRect.Width();
	float OpenVipY = (m_Rect_Top.Height - OpenVipHeight) / 2;
	m_Btn_OpenVip.MoveWindow(OpenVipX, OpenVipY, OpenVipWidth, OpenVipHeight);

	//设置
	CRect ConfigBtnRect;
	m_Btn_Config.GetClientRect(ConfigBtnRect);
	float ConfigWidth = ConfigBtnRect.Width();
	float ConfigHeight = ConfigBtnRect.Height();
	float ConfigX = OpenVipX + OpenVipWidth + 5 * m_Scale;
	float ConfigY = OpenVipY + (OpenVipHeight - ConfigHeight) / 2;
	m_Btn_Config.MoveWindow(ConfigX, ConfigY, ConfigWidth, ConfigHeight);

	//菜单
	CRect MenuBtnRect;
	m_Btn_FeedBack.GetClientRect(MenuBtnRect);
	float MenuBtnWidth = MenuBtnRect.Width();
	float MenuBtnHeight = MenuBtnRect.Height();
	float MenuBtnX = ConfigX + ConfigWidth + 5 * m_Scale;
	float MenuBtnY = (m_Rect_Top.Height - MenuBtnHeight) / 2;
	m_Btn_FeedBack.MoveWindow(MenuBtnX, MenuBtnY, MenuBtnWidth, MenuBtnHeight);

	//最小化
	float MinimalBtnWidth = 27 * m_Scale;
	float MinimalBtnHeight = 27 * m_Scale;
	float MinimalBtnX = MenuBtnX + MenuBtnWidth + 10 * m_Scale;
	float MinimalBtnY = (m_Rect_Top.Height - MinimalBtnHeight) / 2;
	m_Btn_Minimal.MoveWindow(MinimalBtnX, MinimalBtnY, MinimalBtnWidth, MinimalBtnHeight);

	//关闭按钮
	float CloseBtnWidth = 24 * m_Scale;
	float CloseBtnHeight = 24 * m_Scale;
	float CloseBtnX = MinimalBtnX + MinimalBtnWidth + 5 * m_Scale;
	float CloseBtnY = (m_Rect_Top.Height - CloseBtnHeight) / 2;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	//返回按钮
	float ReturnBtnWidth = 22.5 * m_Scale;
	float ReturnBtnHeight = 22.5 * m_Scale;
	float ReturnBtnX = 0.028 * m_WindowWidth;
	float ReturnBtnY = 0.105 * m_WindowHeight;
	m_Btn_Return.MoveWindow(ReturnBtnX, ReturnBtnY, ReturnBtnWidth, ReturnBtnHeight);

	//刷新按钮
	float RefreshBtnX = 0.902 * m_WindowWidth;
	float RefreshBtnY = ReturnBtnY;
	float RefreshBtnWidth = 65 * m_Scale;
	float RefreshBtnHeight = 25 * m_Scale;
	m_Btn_Refresh.MoveWindow(RefreshBtnX, RefreshBtnY, RefreshBtnWidth, RefreshBtnHeight);

	//录制选项 66 79 
	float RecordOptionWidth = 75 * m_Scale;
	float RecordOptionHeight = 26 * m_Scale;
	float RecordOptionX = 56 * m_Scale;
	float RecordOptionY = 68 * m_Scale;
	m_Btn_ComBox.MoveWindow(RecordOptionX, RecordOptionY, RecordOptionWidth, RecordOptionHeight);

	//录制窗口文本0.751
	float RecordWindowTextY = 0.721 * m_WindowHeight;
	float RecordWindowTextX = 0.032 * m_WindowWidth;
	float RecordWindowTextWidth = 85 * m_Scale;
	float RecordWindowTextHeight = 20 * m_Scale;
	m_Stat_RecordWindow.MoveWindow(RecordWindowTextX, RecordWindowTextY, RecordWindowTextWidth, RecordWindowTextHeight);

	//录制模式文本
	float RecordModeTextY = 0.791 * m_WindowHeight;
	float RecordModeTextX = RecordWindowTextX;
	float RecordModeTextWidth = 85 * m_Scale;
	float RecordModeTextHeight = 20 * m_Scale;
	m_Stat_RecordMode.MoveWindow(RecordModeTextX, RecordModeTextY, RecordModeTextWidth, RecordModeTextHeight);

	//系统声音文本
	float SystemAudioTextY = 0.861 * m_WindowHeight;
	float SystemAudioTextX = RecordWindowTextX;
	float SystemAudioTextWidth = 85 * m_Scale;
	float SystemAudioTextHeight = 20 * m_Scale;
	m_Stat_SystemAudio.MoveWindow(SystemAudioTextX, SystemAudioTextY, SystemAudioTextWidth, SystemAudioTextHeight);

	//麦克风声音文本
	float MicroAudioTextY = 0.931 * m_WindowHeight;
	float MicroAudioTextX = RecordWindowTextX;
	float MicroAudioTextWidth = 85 * m_Scale;
	float MicroAudioTextHeight = 20 * m_Scale;
	m_Stat_MicroAudio.MoveWindow(MicroAudioTextX, MicroAudioTextY, MicroAudioTextWidth, MicroAudioTextHeight);

	//录制窗口下拉框
	float RecordWindowBtnWidth = 286 * m_Scale;
	float RecordWindowBtnHeight = 32 * m_Scale;
	float RecordWindowBtnY = RecordWindowTextY + (RecordWindowTextHeight - RecordWindowBtnHeight) / 2;
	float RecordWindowBtnX = RecordWindowTextX + RecordWindowTextWidth + 10 * m_Scale;
	m_Btn_RecordWindowOption.MoveWindow(RecordWindowBtnX, RecordWindowBtnY, RecordWindowBtnWidth, RecordWindowBtnHeight);

	//录制模式下拉框
	float RecordModeBtnWidth = 286 * m_Scale;
	float RecordModeBtnHeight = 32 * m_Scale;
	float RecordModeBtnY = RecordModeTextY + (RecordModeTextHeight - RecordModeBtnHeight) / 2;
	float RecordModeBtnX = RecordModeTextX + RecordModeTextWidth + 10 * m_Scale;
	m_Btn_RecordMode.MoveWindow(RecordModeBtnX, RecordModeBtnY, RecordModeBtnWidth, RecordModeBtnHeight);

	//系统声音选项下拉框
	float SysteamAudioBtnWidth = 286 * m_Scale;
	float SysteamAudioBtnHeight = 32 * m_Scale;
	float SysteamAudioBtnY = SystemAudioTextY + (SystemAudioTextHeight - SysteamAudioBtnHeight) / 2;
	float SysteamAudioBtnX = SystemAudioTextX + SystemAudioTextWidth + 10 * m_Scale;
	m_Btn_SystemAudioOption.MoveWindow(SysteamAudioBtnX, SysteamAudioBtnY, SysteamAudioBtnWidth, SysteamAudioBtnHeight);

	//麦克风选项下拉框
	float MicroAudioBtnWidth = 286 * m_Scale;
	float MicroAudioBtnHeight = 32 * m_Scale;
	float MicroAudioBtnY = MicroAudioTextY + (MicroAudioTextHeight - MicroAudioBtnHeight) / 2;
	float MicroAudioBtnX = SysteamAudioBtnX;
	m_Btn_MicroAudioOption.MoveWindow(MicroAudioBtnX, MicroAudioBtnY, MicroAudioBtnWidth, MicroAudioBtnHeight);

	//开始录制按钮
	float StartRecordBtnX = 0.797 * m_WindowWidth;
	float StartRecordBtnY = 0.770 * m_WindowHeight;
	float StartRecordBtnWidth = 107 * m_Scale;
	float StartRecordBtnHeight = 107 * m_Scale;
	m_Btn_StartRecord.MoveWindow(StartRecordBtnX, StartRecordBtnY, StartRecordBtnWidth, StartRecordBtnHeight);

	//快捷键文本提示（Alt + B）
	float HotKeySRW = StartRecordBtnWidth + 100 * m_Scale;
	float HotKeySRH = 20 * m_Scale;
	float HotKeySRX = StartRecordBtnX + (StartRecordBtnWidth - HotKeySRW) / 2;
	float HotKeySRY = StartRecordBtnY + StartRecordBtnHeight + 5 * m_Scale;
	m_stat_hotKeyStartRecord.MoveWindow(HotKeySRX, HotKeySRY, HotKeySRW, HotKeySRH);

	//录制窗口重新选择按钮 96 40
	float ReSelectWindowRecordWidth = 77 * m_Scale;
	float ReSelectWindowRecordHeight = 32 * m_Scale;
	float ReSelectWindowRecordX = RecordWindowBtnX + RecordWindowBtnWidth + 5 * m_Scale;
	float ReSelectWindowRecordY = RecordWindowBtnY;
	m_Btn_RecordWindowReselect.MoveWindow(ReSelectWindowRecordX, ReSelectWindowRecordY, ReSelectWindowRecordWidth, ReSelectWindowRecordHeight);
	m_Btn_NoSelect.MoveWindow(ReSelectWindowRecordX, ReSelectWindowRecordY,
		ReSelectWindowRecordWidth, ReSelectWindowRecordHeight);

	//录制模式提示按钮
	float RecordModeTipWidth = 16 * m_Scale;
	float RecordModeTipHeight = 16 * m_Scale;
	float RecordModeTipX = RecordModeBtnX + RecordModeBtnWidth + 5 * m_Scale;
	float RecordModeTipY = RecordModeBtnY + (RecordModeBtnHeight - RecordModeTipHeight) / 2;
	m_Btn_TipsOfRecordMode.MoveWindow(RecordModeTipX, RecordModeTipY, RecordModeTipWidth, RecordModeTipHeight);

	//系统声音高级选项按钮
	float SystemAudioAdvanceOptX = SysteamAudioBtnX + SysteamAudioBtnWidth + 5 * m_Scale;
	float SystemAudioAdvanceOptY = SysteamAudioBtnY;
	float SystemAudioAdvanceOptWidth = 77 * m_Scale;
	float SystemAudioAdvanceOptHeight = 32 * m_Scale;
	m_Btn_SystemAudioAdvanceOption.MoveWindow(
		SystemAudioAdvanceOptX, SystemAudioAdvanceOptY,
		SystemAudioAdvanceOptWidth, SystemAudioAdvanceOptHeight);

	//麦克风声音高级选项按钮
	float MicroAduioOptX = MicroAudioBtnX + MicroAudioBtnWidth + 5 * m_Scale;
	float MicroAduioOptY = MicroAudioBtnY;
	float MicroAduioOptWidth = 77 * m_Scale;
	float MicroAduioOptHeight = 32 * m_Scale;
	m_Btn_MicroAudioAdvanceOption.MoveWindow(
		MicroAduioOptX, MicroAduioOptY,
		MicroAduioOptWidth, MicroAduioOptHeight);

	// 恢复，暂停，停止
	int WidthHeightPng = 61 * m_Scale;
	int WidthHeightPngResume = 70 * m_Scale;
	int x1 = StartRecordBtnX + 20 * m_Scale;
	int x2 = x1 + WidthHeightPng + 10 * m_Scale;
	int y = StartRecordBtnY + 13 * m_Scale;
	int x3 = x1 - 4.5 * m_Scale;
	int y3 = y - 4.5 * m_Scale;
	m_Btn_WindowRecPause.MoveWindow(x1, y, WidthHeightPng, WidthHeightPng);
	m_Btn_WindowRecStop.MoveWindow(x2, y, WidthHeightPng, WidthHeightPng);
	m_Btn_WindowRecResume.MoveWindow(x3, y3, WidthHeightPngResume, WidthHeightPngResume);
}

void Ui_WindowRecordDlg::InitDropListData()
{
	// 初始化录制模式列表
	m_Array_RecordModeList.Add(L"录制系统音频");
	m_Array_RecordModeList.Add(L"录制麦克风");
	m_Array_RecordModeList.Add(L"录制系统音频和麦克风");

	std::vector<CaptureDeviceInfo> AudioCaptures;
	WasapiCapture::GetInstance()->getCaptureDevicesInfo(&AudioCaptures);
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

	// 获取麦克风设备列表
	const std::vector<DeviceInfo>& MicroDevices = DeviceManager::GetInstance().GetMicrophoneDevices();
	for (auto& MicroDevice : MicroDevices) {
		m_Array_MicroAudioList.Add(MicroDevice.nameW.c_str());
	}

	m_Array_RecordOptions.Add(L"录全屏");
	m_Array_RecordOptions.Add(L"录区域");
	m_Array_RecordOptions.Add(L"录游戏");
	m_Array_RecordOptions.Add(L"录窗口");
	m_Array_RecordOptions.Add(L"录摄像头");
	m_Array_RecordOptions.Add(L"录声音");
	m_Array_RecordOptions.Add(L"跟随鼠标");
}

void Ui_WindowRecordDlg::InitDroplistCtrl()
{
	// 创建下拉列表框
	CRect RecordModeRect;
	m_Btn_RecordMode.GetWindowRect(&RecordModeRect);
	m_PopupListBox.addListBox(RecordModeRect.Width(), RecordModeRect.Height(),
		this, m_Array_RecordModeList, L"录制模式下拉框");
	m_Btn_RecordMode.LarSetBtnNailImage(MAINDLG_PNG_DOWN,
		CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
	);

	CRect SystemAudioRect;
	m_Btn_SystemAudioOption.GetWindowRect(&SystemAudioRect);
	m_PopupListBox.addListBox(SystemAudioRect.Width(), SystemAudioRect.Height(),
		this, m_Array_SystemAudioList, L"系统声音下拉框");
	m_Btn_SystemAudioOption.LarSetBtnNailImage(MAINDLG_PNG_DOWN,
		CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
	);

	CRect MicroAudioRect;
	m_Btn_MicroAudioOption.GetWindowRect(&MicroAudioRect);
	m_PopupListBox.addListBox(MicroAudioRect.Width(), MicroAudioRect.Height(),
		this, m_Array_MicroAudioList, L"麦克风声音下拉框");
	m_Btn_MicroAudioOption.LarSetBtnNailImage(MAINDLG_PNG_DOWN,
		CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
	);

	CRect RecordOptionRect;
	m_Btn_ComBox.GetWindowRect(RecordOptionRect);
	m_PopupListBox.addListBox(RecordOptionRect.Width(), RecordOptionRect.Height(),
		this, m_Array_RecordOptions, L"录制选项下拉框");


	// 设置滚动条宽度
	m_PopupListBox.SetScrollbarWidth(9);
	m_PopupListBox.SetMaxDisplayItems(7, L"录制选项下拉框");

	// 设置默认选项（如果列表不为空）
	if (m_Array_RecordOptions.GetSize())
		m_Btn_ComBox.SetWindowTextW(L"录窗口");
	if (m_Array_RecordModeList.GetSize() > 0)
		m_Btn_RecordMode.SetWindowTextW(m_Array_RecordModeList.GetAt(0));
	if (m_Array_SystemAudioList.GetSize() > 0)
		m_Btn_SystemAudioOption.SetWindowTextW(m_Array_SystemAudioList.GetAt(0));
	if (m_Array_MicroAudioList.GetSize() > 0)
		m_Btn_MicroAudioOption.SetWindowTextW(m_Array_MicroAudioList.GetAt(0));
}

void Ui_WindowRecordDlg::CaptureWindowImageThread()
{
	HWND hwnd = this->GetSafeHwnd(); // 在主线程保存窗口句柄
	std::thread captureThread([this, hwnd]() {
		CaptureAllWindowImages();
		::PostMessage(hwnd, MSG_WINDOWIMAGECAPTURE_COMPELETE, NULL, NULL);
		DEBUG_CONSOLE_STR(ConsoleHandle, L"触发repaint");
		});
	captureThread.detach();
}

void Ui_WindowRecordDlg::InitCtrl()
{
	//应用程序名字
	m_Stat_LogoText.LarSetTextSize(24);
	m_Stat_LogoText.LarSetTextStyle(false, false, false);

	m_stat_hotKeyStartRecord.LarSetTextSize(20);
	m_stat_hotKeyStartRecord.LarSetTextColor(RGB(255, 255, 255));
	m_stat_hotKeyStartRecord.LarSetTextCenter();

	//用户头像
	m_Btn_PrifileIcon.LoadPNG(MAINDLG_PNG_PROFILEICON);
	m_Btn_PrifileIcon.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_PrifileIcon.ShowWindow(SW_HIDE);

	//应用程序图标
	m_Btn_AppIcon.LoadPNG(WWDLG_PNG_APPICON);
	m_Btn_AppIcon.SetBackgroundColor(RGB(37, 39, 46));

	//用户名
	m_Btn_Username.LarSetTextSize(20);
	m_Btn_Username.LarSetTextStyle(false, false, false);
	m_Btn_Username.LaSetTextColor(Color(255, 255, 255, 255));
	m_Btn_Username.LarSetBorderColor(Color(37, 39, 46));
	m_Btn_Username.ShowWindow(SW_HIDE);

	//开通会员
	m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_OPENVIP);
	m_Btn_OpenVip.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_OpenVip.ShowWindow(SW_HIDE);

	//设置
	m_Btn_Config.LarSetTextSize(20);
	m_Btn_Config.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Config.LaSetTextHoverColor(Gdiplus::Color(255, 255, 255, 255));
	m_Btn_Config.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Config.LarSetBorderColor(Gdiplus::Color(255, 37, 39, 46));
	m_Btn_Config.LarSetEraseBkEnable(false);
	m_Btn_Config.LarSetEraseBkEnable(false);

	//菜单
	m_Btn_FeedBack.LarSetTextSize(20);
	m_Btn_FeedBack.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_FeedBack.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_FeedBack.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_FeedBack.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_FeedBack.LarSetEraseBkEnable(false);
	m_Btn_FeedBack.LarSetEraseBkEnable(false);
	m_Btn_FeedBack.ShowWindow(SW_SHOW);

	//录制选项下拉框
	m_Btn_ComBox.SetWindowText(L"录窗口");
	m_Btn_ComBox.LarSetTextSize(20);
	m_Btn_ComBox.LarSetTextStyle(false, false, false);
	m_Btn_ComBox.LaSetTextColor(Color(155, 255, 255, 255));
	m_Btn_ComBox.LarSetBorderColor(Color(73, 73, 73));
	m_Btn_ComBox.LarSetEraseBkEnable(false);
	m_Btn_ComBox.LarSetButtonNoInteraction(true);
	m_Btn_ComBox.LaSetTextHoverColor(Color(255, 255, 255, 255));
	m_Btn_ComBox.LaSetTextClickedColor(Color(255, 255, 255, 255));
	m_Btn_ComBox.LarSetNormalFiilBrush(SolidBrush(Color(36, 37, 40)));
	m_Btn_ComBox.LarSetHoverFillBrush(SolidBrush(Color(26, 27, 32)));
	m_Btn_ComBox.LarSetBtnNailImage(MAINDLG_PNG_DOWN, 
		CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
	);
	m_Btn_ComBox.LarAdjustTextDisplayPos(-8 * m_Scale, 0);

	//返回按钮
	m_Btn_Return.LoadPNG(WWDLG_PNG_RETURN);
	m_Btn_Return.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Return.SetHoverEffectColor(10, 255, 255, 255);
	m_Btn_Return.SetStretchMode(0.75f);

	//刷新按钮
	m_Btn_Refresh.LoadPNG(WWDLG_PNG_REFRESH);
	m_Btn_Refresh.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Refresh.SetHoverEffectColor(10, 255, 255, 255);
	m_Btn_Refresh.SetStretchMode(0.90f);

	//最小化按钮 
	m_Btn_Minimal.LoadPNG(MAINDLG_BTN_MINIMAL);
	m_Btn_Minimal.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_Minimal.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Minimal.SetStretchMode(0.75f);

	//关闭按钮
	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_Close.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Close.SetStretchMode(0.75f);

	//赛罗奥特曼(BkBrush, m_Btn_RecordWindowReselect);
	m_Btn_TipsOfRecordMode.LoadPNG(WWDLG_BTN_TIPSOFRECORDMODE);
	m_Btn_TipsOfRecordMode.SetBackgroundColor(RGB(26, 27, 32));

	SolidBrush BkBrush(Color(36, 37, 40));//准备背景画刷 
	SetBtnStyle(BkBrush, m_Btn_RecordWindowOption, L"未选择");	//录制窗口选项按钮下拉框
	SetBtnStyle(BkBrush, m_Btn_RecordWindowReselect, L"重新选择");	//迪迦奥特曼
	SetBtnStyle(BkBrush, m_Btn_SystemAudioAdvanceOption, L"高级选项");	//系统声音高级选项
	SetBtnStyle(BkBrush, m_Btn_MicroAudioAdvanceOption, L"高级选项");	//麦克风声音高级选项
	SetBtnStyle(BkBrush, m_Btn_RecordMode, L"录窗口");	//录制模式选项按钮下拉框
	SetBtnStyle(BkBrush, m_Btn_NoSelect, L"未选择");	//未选择按钮
	m_Btn_NoSelect.LarSetBtnEnable(false);
	m_Btn_NoSelect.LaSetTextColor(Color(65, 255, 255, 255));
	m_Btn_NoSelect.ShowWindow(SW_SHOW);
	m_Btn_RecordWindowReselect.ShowWindow(SW_HIDE);

	m_Btn_RecordWindowOption.LarSetBtnNailImage(MAINDLG_PNG_DOWN,
		CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
	);

	CString AudioDeviceText;
	App.m_Dlg_Main.m_Dlg_Config->m_Btn_AudioDevice.GetWindowTextW(AudioDeviceText);
	SetBtnStyle(BkBrush, m_Btn_SystemAudioOption, AudioDeviceText);	//系统声音选项按钮下拉框

	CString MicroDeviceText;
	App.m_Dlg_Main.m_Dlg_Config->m_Btn_MicroDevice.GetWindowTextW(MicroDeviceText);
	SetBtnStyle(BkBrush, m_Btn_MicroAudioOption, MicroDeviceText);	//麦克风声音选项按钮下拉框

	//录制窗口文本
	m_Stat_RecordWindow.LarSetTextSize(20);
	m_Stat_RecordWindow.LarSetTextStyle(false, false, false);

	//录制模式文本
	m_Stat_RecordMode.LarSetTextSize(20);
	m_Stat_RecordMode.LarSetTextStyle(false, false, false);

	//系统声音
	m_Stat_SystemAudio.LarSetTextSize(20);
	m_Stat_SystemAudio.LarSetTextStyle(false, false, false);

	//麦克风声音文本
	m_Stat_MicroAudio.LarSetTextSize(20);
	m_Stat_MicroAudio.LarSetTextStyle(false, false, false);

	//开始录制按钮
	m_Btn_StartRecord.LoadPNG(WWDLG_PNG_STARTRECORD);
	m_Btn_StartRecord.SetBackgroundColor(RGB(26, 27, 32));

	m_Btn_TipsOfRecordMode.ShowWindow(SW_HIDE);

	// 初始化窗口录制暂停/停止/恢复按钮
	m_Btn_WindowRecPause.LoadPNG(CHILDDLG_PNG_PAUSE);
	m_Btn_WindowRecPause.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_WindowRecPause.ShowWindow(SW_HIDE);

	m_Btn_WindowRecStop.LoadPNG(CHILDDLG_PNG_STOPRECORDING);
	m_Btn_WindowRecStop.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_WindowRecStop.ShowWindow(SW_HIDE);

	m_Btn_WindowRecResume.LoadPNG(CHILDDLG_PNG_RESUME);
	m_Btn_WindowRecResume.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_WindowRecResume.ShowWindow(SW_HIDE);

	m_Btn_WindowRecPause.LoadClickPNG(CHILDDLG_PNG_STOPRECORDINGHOV);
	m_Btn_WindowRecResume.LoadClickPNG(CHILDDLG_PNG_RESUMEHOV);
	m_Btn_WindowRecStop.LoadClickPNG(CHILDDLG_PNG_STOPHOV);
	InitWindowRecordRuntimeRect(); //初始化计时/大小显示区域
}

void Ui_WindowRecordDlg::SetBtnStyle(Gdiplus::SolidBrush& BkBrush, CLarBtn& Btn, CString BtnText)
{
	Btn.SetWindowText(BtnText);
	Btn.LarSetTextSize(20);
	Btn.LarSetTextStyle(false, false, false);
	Btn.LaSetTextColor(Color(255, 255, 255));
	Btn.LarSetBorderColor(Color(73, 73, 73));
	Btn.LarSetEraseBkEnable(false);
	Btn.LarSetButtonNoInteraction(true);
	Btn.LaSetTextHoverColor(Color(235, 104, 115, 136));
	Btn.LaSetTextClickedColor(Color(245, 104, 115, 136));
	Btn.LarSetNormalFiilBrush(BkBrush);
	Btn.LarSetHoverFillBrush(BkBrush);
	Btn.LarSetClickedFillBrush(BkBrush);
	Btn.LarsetBorderSize(2);
}

void Ui_WindowRecordDlg::LoadRes()
{
	m_Bitmap_AddWindow = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(WWDLG_PNG_ADDWINDOW),
		L"PNG"
	);
}

void Ui_WindowRecordDlg::HandleNavButtonClick(int action)
{
	std::lock_guard<std::mutex> lk(m_imagesMutex);
	int totalPages = (m_WindowImages.size() + 8 - 1) / 8; // 每页最多8个(4x2)
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"Handle nav button click, action: %d, current page: %d, total pages: %d\n",
		action, m_currentPage, totalPages);

	int newPage = m_currentPage;

	// 处理特殊动作
	if (action == NAV_PREV_TEXT || action == NAV_PREV_ARROW) {
		if (m_currentPage > 0) {
			newPage = m_currentPage - 1;
		}
	}
	else if (action == NAV_NEXT_TEXT || action == NAV_NEXT_ARROW) {
		if (m_currentPage < totalPages - 1) {
			newPage = m_currentPage + 1;
		}
	}
	else if (action >= 0 && action < totalPages) {
		// 直接跳转到指定页码
		newPage = action;
	}

	// 如果页码发生变化，更新并repaint
	if (newPage != m_currentPage) {
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"Page changed from %d to %d\n", m_currentPage, newPage);
		m_currentPage = newPage;
		Invalidate();
	}
}

void Ui_WindowRecordDlg::DrawBorder(Gdiplus::Pen* pen, const CRect& rect, Gdiplus::Graphics* graphics)
{
	graphics->DrawRectangle(
		pen,
		(INT)rect.left,
		(INT)rect.top,
		(INT)rect.Width(),
		(INT)rect.Height()
	);
}

void Ui_WindowRecordDlg::DrawAngleRect(
	Gdiplus::Rect& rect,
	Gdiplus::Graphics* graphics,
	float cornerRadius,
	float borderWidth,
	Gdiplus::Color borderColor,
	Gdiplus::Color fillColor)
{
	// 创建画刷用于填充
	Gdiplus::SolidBrush fillBrush(fillColor);

	// 创建画笔用于边框
	Gdiplus::Pen borderPen(borderColor, borderWidth);

	// 创建GraphicsPath对象用于绘制圆角矩形
	Gdiplus::GraphicsPath path;

	// 将Rect转换为RectF
	Gdiplus::RectF rectf(
		static_cast<Gdiplus::REAL>(rect.X),
		static_cast<Gdiplus::REAL>(rect.Y),
		static_cast<Gdiplus::REAL>(rect.Width),
		static_cast<Gdiplus::REAL>(rect.Height)
	);

	// 添加圆角矩形的四个圆弧到路径
	path.AddArc(rectf.X, rectf.Y, cornerRadius * 2, cornerRadius * 2, 180, 90); // 左上角
	path.AddArc(rectf.X + rectf.Width - cornerRadius * 2, rectf.Y, cornerRadius * 2, cornerRadius * 2, 270, 90); // 右上角
	path.AddArc(rectf.X + rectf.Width - cornerRadius * 2, rectf.Y + rectf.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 0, 90); // 右下角
	path.AddArc(rectf.X, rectf.Y + rectf.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 90, 90); // 左下角

	// 关闭路径，确保连接所有点
	path.CloseFigure();

	// 然后填充路径
	graphics->FillPath(&fillBrush, &path);

	// 如果边框宽度大于0，则绘制边框
	if (borderWidth > 0.0f) {
		graphics->DrawPath(&borderPen, &path);
	}
}

bool Ui_WindowRecordDlg::DrawWindowThumbnails(Gdiplus::Graphics* graphics)
{
	if (!m_Bool_IsWindowImageReady.load())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"窗口截图还没有准备好，缩略图返回绘画");
		return false;
	}

	DEBUG_CONSOLE_STR(ConsoleHandle, L"开始绘画缩略图");
	using namespace Gdiplus;
	m_NavButtons.clear();
	// 清空缩略图位置信息
	m_thumbnailRects.clear();
	DrawAngleRect(m_Rect_WindowsDiplayArea,
		graphics,
		30.0f,                                // 圆角半径
		2.0f,                                 // 边框宽度
		Gdiplus::Color(60, 62, 66),           // 边框颜色
		Gdiplus::Color(36, 37, 40)            // 填充颜色
	);

	// 如果没有窗口截图，则直接返回
	if (m_WindowImages.empty())
	{
		DB(ConsoleHandle, L"没有获取到任何窗口截图");
		return false;
	}

	// 启用高质量绘图和抗锯齿
	graphics->SetSmoothingMode(SmoothingModeAntiAlias);
	graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	// 保存当前图形状态
	GraphicsState state = graphics->Save();

	// 获取圆角矩形区域内可用的显示区域（无内边距）
	Rect displayArea(
		m_Rect_WindowsDiplayArea.X,
		m_Rect_WindowsDiplayArea.Y,
		m_Rect_WindowsDiplayArea.Width,
		m_Rect_WindowsDiplayArea.Height
	);

	// 创建裁剪路径，确保图像不会超出圆角矩形区域
	GraphicsPath clipPath;
	float cornerRadius = 25.0f;  // 略小于外框的圆角
	RectF rectf(
		static_cast<REAL>(displayArea.X),
		static_cast<REAL>(displayArea.Y),
		static_cast<REAL>(displayArea.Width),
		static_cast<REAL>(displayArea.Height)
	);

	clipPath.AddArc(rectf.X, rectf.Y, cornerRadius * 2, cornerRadius * 2, 180, 90);
	clipPath.AddArc(rectf.X + rectf.Width - cornerRadius * 2, rectf.Y, cornerRadius * 2, cornerRadius * 2, 270, 90);
	clipPath.AddArc(rectf.X + rectf.Width - cornerRadius * 2, rectf.Y + rectf.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 0, 90);
	clipPath.AddArc(rectf.X, rectf.Y + rectf.Height - cornerRadius * 2, cornerRadius * 2, cornerRadius * 2, 90, 90);
	clipPath.CloseFigure();

	// 绘制边框 - 先绘制边框再设置裁剪区域
	Pen borderPen(Color(60, 62, 66), 2.0f);  // 设置边框颜色为RGB(60,62,66)，线宽为2.0
	graphics->DrawPath(&borderPen, &clipPath);

	// 设置裁剪区域
	graphics->SetClip(&clipPath);

	// 绘制整个背景为RGB(36,37,40)
	SolidBrush backgroundBrush(Color(36, 37, 40));
	graphics->FillPath(&backgroundBrush, &clipPath);

	// 计算每行显示的缩略图数量和每个缩略图的尺寸
	const int thumbnailsPerRow = 4;
	const int rowCount = 2;
	const int maxThumbnailsPerPage = thumbnailsPerRow * rowCount;

	// 缩略图之间的间距
	const int horizontalSpacing = 20;
	const int verticalSpacing = 10;

	// 为底部导航栏+外部间距预留总高度（这里 30 是导航区高度，15 是底部额外留白）
	const int navReserveHeight = 30 + 15;

	// 计算缩略图的可用区尺寸
	int availableWidth = displayArea.Width - horizontalSpacing * (thumbnailsPerRow + 1);
	int availableHeight = displayArea.Height - verticalSpacing * (rowCount + 1) - navReserveHeight;

	// 计算每个缩略图尺寸
	int thumbnailWidth = availableWidth / thumbnailsPerRow;
	// 去掉额外的 “+8*m_Scale”，直接平分两行剩余高度
	int thumbnailHeight = availableHeight / rowCount;

	// 计算本页开始的索引
	int startIndex = m_currentPage * maxThumbnailsPerPage;

	// 字体和画刷，用于绘制窗口标题 - 统一使用微软雅黑
	Gdiplus::Font titleFont(L"微软雅黑", 8, FontStyleRegular);
	SolidBrush textBrush(Color(210, 210, 210));
	StringFormat format;
	format.SetAlignment(StringAlignmentCenter);
	format.SetLineAlignment(StringAlignmentCenter);
	format.SetTrimming(StringTrimmingEllipsisCharacter);
	format.SetFormatFlags(StringFormatFlagsNoWrap);  // 确保不换行

	// 计数器，用于确定当前绘制的是第几个缩略图
	int count = 0;
	int currentIndex = 0;

	// 遍历所有窗口截图
	{
		for (auto it = m_WindowImages.begin(); it != m_WindowImages.end(); ++it)
		{
			// 跳过前几页的缩略图
			if (currentIndex < startIndex) {
				currentIndex++;
				continue;
			}

			// 如果已经达到当前页的最大显示数量，则退出循环
			if (count >= maxThumbnailsPerPage) {
				break;
			}

			HWND hwnd = it->first;
			Gdiplus::Image* image = it->second;

			if (image == nullptr) {
				currentIndex++;
				continue;
			}

			// 计算当前缩略图的位置
			int row = count / thumbnailsPerRow;
			int col = count % thumbnailsPerRow;

			// 计算位置
			int x = displayArea.X + horizontalSpacing + col * (thumbnailWidth + horizontalSpacing);
			int y = displayArea.Y + verticalSpacing + row * (thumbnailHeight + verticalSpacing);


			Rect thumbnailRect(x, y, thumbnailWidth, thumbnailHeight);// 创建缩略图区域
			m_thumbnailRects.push_back({ thumbnailRect, hwnd });// 保存缩略图位置信息

			// 分离图像区域和标题区域
			int titleHeight = 10 * m_Scale;
			Rect imageRect(
				thumbnailRect.X,
				thumbnailRect.Y,
				thumbnailRect.Width,
				thumbnailRect.Height - titleHeight
			);

			Rect titleRect(
				thumbnailRect.X,
				thumbnailRect.Y + thumbnailRect.Height - titleHeight,
				thumbnailRect.Width,
				titleHeight
			);

			// 使用固定大小显示所有缩略图
			const int imagePadding = 5;
			Rect fixedImageRect(
				imageRect.X + imagePadding,
				imageRect.Y + imagePadding,
				imageRect.Width - (imagePadding * 2),
				imageRect.Height - (imagePadding * 4)
			);

			// 绘制缩略图（使用高质量插值）
			graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
			graphics->DrawImage(image, fixedImageRect);
			graphics->SetInterpolationMode(InterpolationModeDefault);

			// 获取窗口标题
			std::wstring windowTitle = WindowHandleManager::GetWindowTitle(hwnd);

			// 绘制窗口标题
			graphics->DrawString(
				windowTitle.c_str(),
				-1,
				&titleFont,
				RectF(static_cast<REAL>(titleRect.X), static_cast<REAL>(titleRect.Y),
					static_cast<REAL>(titleRect.Width), static_cast<REAL>(titleRect.Height)),
				&format,
				&textBrush
			);
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"绘制窗口截图:%s", windowTitle.c_str());

			// 更新计数器
			count++;
			currentIndex++;
		}

		// 添加居中显示的页码导航
		int totalPages = (m_WindowImages.size() + maxThumbnailsPerPage - 1) / maxThumbnailsPerPage;
		if (totalPages > 1) {
			// 设置页码导航文本样式 - 统一使用微软雅黑字体
			Gdiplus::Font pageFont(L"微软雅黑", 9);
			Gdiplus::Font arrowFont(L"微软雅黑", 9, FontStyleBold);
			SolidBrush normalPageBrush(Color(160, 160, 160));    // 普通页码颜色
			SolidBrush currentPageBrush(Color(255, 255, 255));   // 当前页码颜色 - 白色高亮
			SolidBrush arrowEnabledBrush(Color(210, 210, 210));  // 可用箭头颜色
			SolidBrush arrowDisabledBrush(Color(90, 90, 90));    // 禁用箭头颜色

			StringFormat navFormat;
			navFormat.SetAlignment(StringAlignmentCenter);
			navFormat.SetLineAlignment(StringAlignmentCenter);

			// 计算导航元素的宽度
			int elementWidth = 15*m_Scale;    // 页码和箭头宽度
			int textWidth = 50*m_Scale;       // "上一页"/"下一页"文本宽度

			// 决定要显示哪些页码
			int startPage = 0;
			int endPage = totalPages - 1;
			bool showLeftEllipsis = false;
			bool showRightEllipsis = false;

			if (totalPages > 5) {
				if (m_currentPage < 2) {
					// 当前页接近开始
					startPage = 0;
					endPage = 2;
					showRightEllipsis = true;
				}
				else if (m_currentPage > totalPages - 3) {
					// 当前页接近结束
					startPage = totalPages - 3;
					endPage = totalPages - 1;
					showLeftEllipsis = true;
				}
				else {
					// 当前页在中间
					startPage = m_currentPage - 1;
					endPage = m_currentPage + 1;
					showLeftEllipsis = true;
					showRightEllipsis = true;
				}
			}

			// 计算导航区域总宽度
			int totalNavWidth = textWidth * 2 + elementWidth * 2;  // 上一页+◀+▶+下一页
			totalNavWidth += ((endPage - startPage) + 1) * elementWidth;  // 页码
			if (showLeftEllipsis) totalNavWidth += elementWidth;
			if (showRightEllipsis) totalNavWidth += elementWidth;

			// 计算导航区域的位置（水平居中）
			int navAreaHeight = 30;
			int navAreaWidth = totalNavWidth;

			int navAreaX = displayArea.X + (displayArea.Width - navAreaWidth) / 2;  // 水平居中
			int navAreaY = displayArea.Y + displayArea.Height - navAreaHeight - 15;  // 底部边距15像素

			Rect navArea(navAreaX, navAreaY, navAreaWidth, navAreaHeight);

			// 从左边开始计算各个元素位置
			int currentX = navArea.X;

			// 绘制"上一页"文本
			bool prevEnabled = m_currentPage > 0;
			RectF prevTextRect(currentX, navArea.Y, textWidth, navArea.Height);
			graphics->DrawString(L"上一页", -1, &pageFont, prevTextRect, &navFormat,
				prevEnabled ? &arrowEnabledBrush : &arrowDisabledBrush);

			// 在添加到数组前打印调试信息
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"添加上一页按钮: X=%.1f, Y=%.1f, W=%.1f, H=%.1f, 启用=%d\n",
				prevTextRect.X, prevTextRect.Y, prevTextRect.Width, prevTextRect.Height, prevEnabled);

			// 保存按钮区域
			m_NavButtons.push_back({ prevTextRect, NAV_PREV_TEXT, prevEnabled });
			currentX += textWidth;

			// 绘制左箭头
			RectF leftArrowRect(currentX, navArea.Y, elementWidth, navArea.Height);
			graphics->DrawString(L"◀", -1, &arrowFont, leftArrowRect, &navFormat,
				prevEnabled ? &arrowEnabledBrush : &arrowDisabledBrush);
			// 保存按钮区域
			m_NavButtons.push_back({ leftArrowRect, NAV_PREV_ARROW, prevEnabled });
			currentX += elementWidth;

			// 显示左侧省略号
			if (showLeftEllipsis) {
				RectF ellipsisRect(currentX, navArea.Y, elementWidth, navArea.Height);
				graphics->DrawString(L"...", -1, &pageFont, ellipsisRect, &navFormat, &normalPageBrush);
				// 省略号不需要点击功能
				currentX += elementWidth;
			}

			// 绘制页码
			for (int page = startPage; page <= endPage; page++) {
				RectF pageRect(currentX, navArea.Y, elementWidth, navArea.Height);
				wchar_t pageText[8];
				swprintf(pageText, 8, L"%d", page + 1);

				bool isCurrentPage = (page == m_currentPage);
				graphics->DrawString(pageText, -1, &pageFont, pageRect, &navFormat,
					isCurrentPage ? &arrowDisabledBrush : &arrowEnabledBrush);

				// 保存页码按钮区域 (当前页不可点击)
				m_NavButtons.push_back({ pageRect, page, !isCurrentPage });
				currentX += elementWidth;
			}

			// 显示右侧省略号
			if (showRightEllipsis) {
				RectF ellipsisRect(currentX, navArea.Y, elementWidth, navArea.Height);
				graphics->DrawString(L"...", -1, &pageFont, ellipsisRect, &navFormat, &normalPageBrush);
				// 省略号不需要点击功能
				currentX += elementWidth;
			}

			// 绘制右箭头
			bool nextEnabled = m_currentPage < totalPages - 1;
			RectF rightArrowRect(currentX, navArea.Y, elementWidth, navArea.Height);
			graphics->DrawString(L"▶", -1, &arrowFont, rightArrowRect, &navFormat,
				nextEnabled ? &arrowEnabledBrush : &arrowDisabledBrush);
			// 保存按钮区域
			m_NavButtons.push_back({ rightArrowRect, NAV_NEXT_ARROW, nextEnabled });
			currentX += elementWidth;

			// 绘制"下一页"文本
			RectF nextTextRect(currentX, navArea.Y, textWidth, navArea.Height);
			graphics->DrawString(L"下一页", -1, &pageFont, nextTextRect, &navFormat,
				nextEnabled ? &arrowEnabledBrush : &arrowDisabledBrush);
			// 保存按钮区域
			m_NavButtons.push_back({ nextTextRect, NAV_NEXT_TEXT, nextEnabled });
		}
	}

	// 恢复图形状态
	graphics->Restore(state);
	m_Bool_IsWindowDisplayNeedRedraw = false;
	DB(ConsoleHandle, L"绘画缩略图完毕");
	return true;
}

BOOL Ui_WindowRecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	LoadRes();
	ModifyStyle(0, SS_NOTIFY); // 确保对话框接收鼠标消息
	GetUserDPI();
	UpdateScale();
	InitCtrl();
	m_Thread_CaptureWindowImage = std::thread(&Ui_WindowRecordDlg::CaptureWindowImageThread, this);
	//下拉框初始化
	m_PopupListBox.SetMaxWidth(320 * m_Scale);
	InitDropListData();
	InitDroplistCtrl();

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

void Ui_WindowRecordDlg::SetWindowToAppWindow()
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

void Ui_WindowRecordDlg::InitWindowRecordRuntimeRect()
{
	if (m_windowRecordUI.bkBrush) return; 

	float scale = m_Scale <= 0 ? 1.0f : m_Scale;

	// 计时背景矩形放在原开始录制按钮左侧
	int timeBkW = (int)(135 * scale);
	int timeBkH = (int)(49 * scale);
	int timeBkX = 648 * m_Scale - timeBkW - 10 * m_Scale;
	int timeBkY = 520 * m_Scale + (61 * m_Scale - timeBkH) / 2;
	m_windowRecordUI.rect_timeBk = Gdiplus::Rect(timeBkX, timeBkY, timeBkW, timeBkH);

	// 计时文本区域（内部留一点边距）
	m_windowRecordUI.rect_timerText = Gdiplus::RectF(
		(REAL)(timeBkX + 6 * scale),
		(REAL)(timeBkY + 16 * scale),
		(REAL)(timeBkW - 12 * scale),
		(REAL)(timeBkH / 2.2f)
	);

	// 大小文本区域放在计时框下方
	m_windowRecordUI.rect_sizeText = Gdiplus::RectF(
		(REAL)(timeBkX + 2 * scale),
		(REAL)(timeBkY + timeBkH + 6 * scale),
		(REAL)(timeBkW),
		(REAL)(30 * scale)
	);

	// 创建画刷 / 画笔 / 字体
	m_windowRecordUI.bkBrush = new Gdiplus::SolidBrush(Gdiplus::Color(16, 16, 16));
	m_windowRecordUI.borderPen = new Gdiplus::Pen(Gdiplus::Color(80, 80, 80), 1.0f);
	m_windowRecordUI.timeTextBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255));
	m_windowRecordUI.sizeTextBrush = new Gdiplus::SolidBrush(Gdiplus::Color(210, 210, 210));
	m_windowRecordUI.timerFont = new Gdiplus::Font(L"微软雅黑", 12 * scale);
	m_windowRecordUI.sizeFont = new Gdiplus::Font(L"微软雅黑", 6 * scale);

	m_windowRecordUI.timerStr = L"00:00:00";
	m_windowRecordUI.sizeStr = L"0.00 KB/--GB";
}

void Ui_WindowRecordDlg::SetWindowRecordCountTimer()
{
	m_windowRecordElapsedSec = 0;
	m_windowRecordUI.timerStr = L"00:00:00";
	SetTimer(WINDOWRECDLG_TIMER_TIMERECORDED, 1000, NULL);
}

void Ui_WindowRecordDlg::StopWindowRecordCountTimer()
{
	KillTimer(WINDOWRECDLG_TIMER_TIMERECORDED);
}

bool Ui_WindowRecordDlg::InitWindowRecordFreeSpaceFromPath(const std::wstring& path)
{
	if (path.size() < 2 || path[1] != L':')
		return false;
	wchar_t rootPath[4] = { path[0], L':', L'\\', 0 };
	ULARGE_INTEGER freeAvail{}, totalBytes{}, totalFree{};
	if (GetDiskFreeSpaceExW(rootPath, &freeAvail, &totalBytes, &totalFree))
	{
		m_windowFreeSpaceGB = (double)totalFree.QuadPart / (1024.0 * 1024.0 * 1024.0);
		m_windowHasFreeSpace = true;
		return true;
	}
	return false;
}

void Ui_WindowRecordDlg::UpdateWindowRecordSizeUI(double sizeMB)
{
	double value = 0.0;
	const wchar_t* unit = L"MB";
	if (sizeMB < 1.0)
	{
		value = sizeMB * 1024.0;
		unit = L"KB";
	}
	else if (sizeMB < 1024.0)
	{
		value = sizeMB;
		unit = L"MB";
	}
	else
	{
		value = sizeMB / 1024.0;
		unit = L"GB";
	}

	wchar_t leftBuf[64] = { 0 };
	swprintf_s(leftBuf, L"%.2f %s", value, unit);

	wchar_t rightBuf[64] = { 0 };
	if (m_windowHasFreeSpace && m_windowFreeSpaceGB > 0.0)
	{
		swprintf_s(rightBuf, L"/%.2fGB", m_windowFreeSpaceGB);
	}
	else
	{
		wcscpy_s(rightBuf, L"/--GB");
	}

	m_windowRecordUI.sizeStr = std::wstring(leftBuf) + rightBuf;

	// 计时框 + 大小文本
	CRect inv(
		m_windowRecordUI.rect_timeBk.X,
		m_windowRecordUI.rect_timeBk.Y,
		m_windowRecordUI.rect_timeBk.X + m_windowRecordUI.rect_timeBk.Width,
		m_windowRecordUI.rect_timeBk.Y + m_windowRecordUI.rect_timeBk.Height + (int)(m_windowRecordUI.rect_sizeText.Height) + 8
	);
	InvalidateRect(&inv, FALSE);
}

void Ui_WindowRecordDlg::StartNonVipAutoStopTimerIfNeeded()
{
	// 条件：非VIP或绑定超限（与原逻辑等价）
	bool need = false;
	if (!App.m_IsVip && !App.m_IsNonUserPaid) need = true;
	if (App.m_IsOverBinds && !App.m_IsNonUserPaid) need = true;
	if (!need) return;

	m_nonVipRemainingMs = NONEVIP_RECORDTIME * 1000; 
	m_nonVipTimerStartTick = GetTickCount64();
	m_nonVipTimerActive = true;
	SetTimer(TIMER_NONEVIP_RECORDTIME, m_nonVipRemainingMs, NULL);
	DEBUG_CONSOLE_STR(ConsoleHandle, L"[自动停止] 已启动非VIP自动停止计时器");
}

void Ui_WindowRecordDlg::PauseNonVipAutoStopTimer()
{
	if (!m_nonVipTimerActive) return;
	// 计算已消耗
	ULONGLONG now = GetTickCount64();
	ULONGLONG elapsed = now - m_nonVipTimerStartTick;
	if (elapsed >= (ULONGLONG)m_nonVipRemainingMs)
	{
		m_nonVipRemainingMs = 0;// 已到期或误差超限
	}
	else
	{
		m_nonVipRemainingMs -= (int)elapsed;
	}
	KillTimer(TIMER_NONEVIP_RECORDTIME);
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"[自动停止] 暂停计时, 剩余: %d ms", m_nonVipRemainingMs);
}

void Ui_WindowRecordDlg::ResumeNonVipAutoStopTimer()
{
	if (!m_nonVipTimerActive) return;
	if (m_nonVipRemainingMs <= 0)
	{
		// 没有剩余时间，不再重启
		DEBUG_CONSOLE_STR(ConsoleHandle, L"[自动停止] 剩余时间<=0，不再恢复");
		return;
	}
	m_nonVipTimerStartTick = GetTickCount64();
	SetTimer(TIMER_NONEVIP_RECORDTIME, m_nonVipRemainingMs, NULL);
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"[自动停止] 恢复计时, 继续倒计: %d ms", m_nonVipRemainingMs);
}

void Ui_WindowRecordDlg::ResetNonVipAutoStopTimerState()
{
	KillTimer(TIMER_NONEVIP_RECORDTIME);
	m_nonVipRemainingMs = 0;
	m_nonVipTimerStartTick = 0;
	m_nonVipTimerActive = false;
	DEBUG_CONSOLE_STR(ConsoleHandle, L"[自动停止] 状态已重置");
}

void Ui_WindowRecordDlg::NotifyRecTop_Start()
{
	if (App.m_Dlg_Main.m_Dlg_RecTopDlg &&
		::IsWindow(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd()))
	{
		App.m_Dlg_Main.m_Dlg_RecTopDlg->SetRecordContext_Window();
		::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(),
			MSG_CHILDDLG_TIMERCOUNTSTART, 0, 0); // 顶栏进入计时 UI
	}
}

void Ui_WindowRecordDlg::NotifyRecTop_Stop()
{
	if (App.m_Dlg_Main.m_Dlg_RecTopDlg &&
		::IsWindow(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd()))
	{
		::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(),
			MSG_CHILDDLG_STOPTIMERCOUNTANDSHOWNORMALUI, 0, 0);
		App.m_Dlg_Main.m_Dlg_RecTopDlg->SetRecordContext_None();
	}
}

LRESULT Ui_WindowRecordDlg::On_WindowRecFileSizeUpdate(WPARAM wParam, LPARAM lParam)
{
	double sizeMB = (double)wParam / 100.0;
	UpdateWindowRecordSizeUI(sizeMB);
	return 1;
}

LRESULT Ui_WindowRecordDlg::On_TopRec_StartRecord(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedBtnStartrecord();
	return LRESULT();
}

LRESULT Ui_WindowRecordDlg::On_TopRec_StopRecord(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedBtnStopWindowRecord();
	return LRESULT();
}

LRESULT Ui_WindowRecordDlg::On_TopRec_PauseRecord(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedBtnPauseWindowRecord();
	return LRESULT();
}

LRESULT Ui_WindowRecordDlg::On_TopRec_ResumeRecord(WPARAM wParam, LPARAM lParam)
{
	OnBnClickedBtnResumeWindowRecord();
	return LRESULT();
}

void Ui_WindowRecordDlg::Ui_SetWindowRect(const CRect& rect)
{
	m_CRect_WindowRect.left = rect.left;
	m_CRect_WindowRect.right = rect.right;
	m_CRect_WindowRect.top = rect.top;
	m_CRect_WindowRect.bottom = rect.bottom;
	m_WindowWidth = m_CRect_WindowRect.Width();
	m_WindowHeight = m_CRect_WindowRect.Height();
}

void Ui_WindowRecordDlg::Ui_UpdateWindowPos(const CRect& rect)
{
	m_CRect_WindowRect.left = rect.left;
	m_CRect_WindowRect.right = rect.right;
	m_CRect_WindowRect.top = rect.top;
	m_CRect_WindowRect.bottom = rect.bottom;
	MoveWindow(m_CRect_WindowRect);
}

void Ui_WindowRecordDlg::CleanUpGdiPngRes()
{
	m_Btn_AppIcon.ClearImages();
	m_Btn_PrifileIcon.ClearImages();
	m_Btn_OpenVip.ClearImages();
	m_Btn_Minimal.ClearImages();
	m_Btn_Close.ClearImages();
	m_Btn_Return.ClearImages();
	m_Btn_Refresh.ClearImages();
	m_Btn_StartRecord.ClearImages();
	m_Btn_TipsOfRecordMode.ClearImages();
}

void Ui_WindowRecordDlg::CleanUpWindowImage()
{
	for (auto& pair : m_WindowImages)
	{
		delete pair.second;
	}
	m_WindowImages.clear();
}

void Ui_WindowRecordDlg::ResetWindowRecordUIState(bool clearTimerAndSize)
{
	// 停止录制计时器
	StopWindowRecordCountTimer();
	m_IsRecording = false;

	// 显示/隐藏相关按钮文本
	m_Btn_StartRecord.ShowWindow(SW_SHOW);
	m_Btn_WindowRecPause.ShowWindow(SW_HIDE);
	m_Btn_WindowRecStop.ShowWindow(SW_HIDE);
	m_Btn_WindowRecResume.ShowWindow(SW_HIDE);
	m_stat_hotKeyStartRecord.ShowWindow(SW_SHOW);

	// 复位开始按钮图标
	m_Btn_StartRecord.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_StartRecord.LoadPNG(CHILDDLG_PNG_STARTRECORD);

	// 重置计时与大小字符串
	if (clearTimerAndSize && m_windowRecordUI.bkBrush)
	{
		m_windowRecordElapsedSec = 0;
		m_windowRecordUI.timerStr = L"00:00:00";
		m_windowRecordUI.sizeStr = L"0.00 KB/--GB";
	}

	// 局部刷新（计时框所在区域 + 按钮区域），简单起见整体刷新
	Invalidate(FALSE);
}

void Ui_WindowRecordDlg::EnableShadow()
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

void Ui_WindowRecordDlg::CaptureAllWindowImages()
{
	for (auto& pair : m_WindowImages)
	{    // 清空现有的图像容器
		delete pair.second;
	}
	m_WindowImages.clear();

	// 获取所有窗口句柄并打印所有获取到的窗口名称
	WindowHandleManager::GetInstance().RefreshWindowHandles();
	const std::vector<HWND>& windowHandles = WindowHandleManager::GetInstance().GetWindowHandles();
	DEBUG_CONSOLE_STR(ConsoleHandle, L"获取到的所有窗口名称:");
	for (size_t i = 0; i < windowHandles.size(); i++)
	{
		std::wstring windowTitle = WindowHandleManager::GetWindowTitle(windowHandles[i]);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"[%d] %s", i + 1, windowTitle.c_str());
	}
	DEBUG_CONSOLE_STR(ConsoleHandle, L"--------------------------");

	// 遍历所有窗口句柄
	std::map<HWND, Gdiplus::Image*> newImages;
	for (const HWND& hwnd : windowHandles)
	{
		// 获取窗口位置和大小
		std::wstring windowTitle = WindowHandleManager::GetWindowTitle(hwnd);// 获取窗口标题
		RECT windowRect;
		bool isMinimized = ::IsIconic(hwnd);
		if (isMinimized)
		{
			// 对于最小化窗口，获取原始窗口尺寸（恢复前的尺寸）
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			if (!::GetWindowPlacement(hwnd, &wp))
			{
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"GetWindowPlacement失败: %s (错误码: %d)",
					windowTitle.c_str(), GetLastError());
				continue;
			}
			windowRect = wp.rcNormalPosition;
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"窗口已最小化，使用原始尺寸: %s", windowTitle.c_str());
		}
		else
		{
			if (!::GetWindowRect(hwnd, &windowRect))
			{
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"GetWindowRect失败: %s (错误码: %d)",
					windowTitle.c_str(), GetLastError());
				continue;
			}
		}

		//判断窗口大小是否合理
		int width = windowRect.right - windowRect.left;
		int height = windowRect.bottom - windowRect.top;
		if (width <= 0 || height <= 0)
		{
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"跳过窗口: %s (无效窗口尺寸 %dx%d)",
				windowTitle.c_str(), width, height);
			continue;  // 跳过无效尺寸的窗口
		}
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"正在捕获窗口: %s (尺寸: %dx%d)",
			windowTitle.c_str(), width, height);

		// 创建设备上下文和位图
		HDC hdcScreen = ::GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
		HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
		HGDIOBJ hOldBitmap = SelectObject(hdcMem, hbmScreen);

		// 清空位图背景（透明处理）
		RECT rect = { 0, 0, width, height };
		FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

		// 对最小化窗口的特殊处理
		BOOL result = FALSE;
		if (isMinimized)
		{
			// 先让窗口显示到前台，然后抓取窗口截图，之后在恢复窗口原本的状态
			WINDOWPLACEMENT wpOld;
			wpOld.length = sizeof(WINDOWPLACEMENT);
			::GetWindowPlacement(hwnd, &wpOld);
			WINDOWPLACEMENT wpNew = wpOld;
			wpNew.showCmd = SW_SHOWNOACTIVATE;
			::SetWindowPlacement(hwnd, &wpNew);
			Sleep(100);// 等待窗口绘制
			result = ::PrintWindow(hwnd, hdcMem, m_captureflag);
			::SetWindowPlacement(hwnd, &wpOld);
		}
		else
		{
			result = ::PrintWindow(hwnd, hdcMem, m_captureflag);
		}
		Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(hbmScreen, NULL);

		// 如果是有效内容并且不是自己，则添加到容器
		if (windowTitle != L"极速录屏大师")
		{
			newImages[hwnd] = static_cast<Gdiplus::Image*>(pBitmap);
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"成功捕获并存储窗口: %s,窗口hwnd:%p", windowTitle.c_str(), (void*)hwnd);
		}

		// 清理GDI资源
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hbmScreen);
		DeleteDC(hdcMem);
		::ReleaseDC(NULL, hdcScreen);
	}

	//获取互斥锁，更新当前存储的窗口截图容器成员
	{
		std::lock_guard<std::mutex> lk(m_imagesMutex);
		for (auto& kv : m_WindowImages)
			delete kv.second;
		m_WindowImages.swap(newImages);
		m_Bool_IsMouseInThumbnail = false;
		m_Rect_CurrnetHovertTumbnailRect = Gdiplus::Rect();
		m_Rect_LastHovertTumbnailRect = Gdiplus::Rect();
	}
	m_Bool_IsWindowImageReady.store(true);
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"共捕获 %d 个窗口截图", m_WindowImages.size());
}

void Ui_WindowRecordDlg::OnBnClickedBtnReturn()
{
	m_PopupListBox.HideListBox();
	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			KillTimer(TIMER_NONEVIP_RECORDTIME);
			if (ScreenRecorder::GetInstance()->isPausing())
			{
				ScreenRecorder::GetInstance()->ResumeRecording();
			}
			ScreenRecorder::GetInstance()->ReleaseInstance();
			ResetNonVipAutoStopTimerState();
			ResetWindowRecordUIState(true);
			ModalDlg_MFC::ShowModal_Priview(nullptr);
		}
		else
		{
			return;
		}
	}

	GetParent()->ShowWindow(SW_SHOW);
	this->ShowWindow(SW_HIDE);
	NotifyRecTop_Stop();
	App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_HIDE);

	m_Bool_IsWindowDisplayNeedRedraw = true;
	App.m_Dlg_Main.SetTrayClickCallback([=]()
		{
			App.m_Dlg_Main.ShowWindow(SW_SHOW);
			App.m_Dlg_Main.SetForegroundWindow();
		});
	m_Shadow.Show(m_hWnd);
}

void Ui_WindowRecordDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	CDialogEx::OnOK();
}

void Ui_WindowRecordDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnCancel();
}

LRESULT Ui_WindowRecordDlg::OnNavButtonAction(WPARAM wParam, LPARAM lParam)
{
	int action = (int)wParam;
	int totalPages = (m_WindowImages.size() + 8 - 1) / 8; // 每页最多8个(4x2)
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"Handle nav button click, action: %d, current page: %d, total pages: %d\n",
		action, m_currentPage, totalPages);

	int newPage = m_currentPage;

	// 处理特殊动作
	if (action == NAV_PREV_TEXT || action == NAV_PREV_ARROW) {
		if (m_currentPage > 0) {
			newPage = m_currentPage - 1;
		}
	}
	else if (action == NAV_NEXT_TEXT || action == NAV_NEXT_ARROW) {
		if (m_currentPage < totalPages - 1) {
			newPage = m_currentPage + 1;
		}
	}
	else if (action >= 0 && action < totalPages) {
		// 直接跳转到指定页码
		newPage = action;
	}

	// 如果页码发生变化，更新并repaint
	if (newPage != m_currentPage) {
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"Page changed from %d to %d\n", m_currentPage, newPage);
		m_currentPage = newPage;

		// 仅repaint窗口显示区域，避免整个窗口闪烁
		CRect invalidRect(
			m_Rect_WindowsDiplayArea.X,
			m_Rect_WindowsDiplayArea.Y,
			m_Rect_WindowsDiplayArea.X + m_Rect_WindowsDiplayArea.Width,
			m_Rect_WindowsDiplayArea.Y + m_Rect_WindowsDiplayArea.Height
		);

		m_Bool_IsWindowDisplayNeedRedraw = true;
		InvalidateRect(invalidRect);
	}

	return 0;
}

LRESULT Ui_WindowRecordDlg::OnCaptureCompelete(WPARAM wParam, LPARAM lParam)
{
	DEBUG_CONSOLE_STR(ConsoleHandle, L"更新录制窗口下拉框");
	m_Array_RecordWindowList.RemoveAll();
	m_PopupListBox.DeleteAllRow(L"录制窗口下拉框");
	// 初始化录制窗口列表 - 从当前获取的窗口截图生成
	for (const auto& pair : m_WindowImages)
	{
		HWND hwnd = pair.first;
		std::wstring windowTitle = WindowHandleManager::GetWindowTitle(hwnd);
		if (!windowTitle.empty()) {
			m_Array_RecordWindowList.Add(windowTitle.c_str());
		}
	}

	// 如果没有窗口，添加默认项
	if (m_Array_RecordWindowList.GetSize() == 0) {
		m_Array_RecordWindowList.Add(L"未检测到可用窗口");
	}

	CRect RecordWindowRect;
	m_Btn_RecordWindowOption.GetWindowRect(&RecordWindowRect);
	m_PopupListBox.addListBox(RecordWindowRect.Width(), RecordWindowRect.Height(),
		this, m_Array_RecordWindowList, L"录制窗口下拉框");
	m_PopupListBox.SetTextSize(14, L"");
	m_Bool_IsWindowDisplayNeedRedraw = true;
	Invalidate();
	return 1;
}

LRESULT Ui_WindowRecordDlg::OnRecordWindowMinimal(WPARAM wParam, LPARAM lParam)
{
	ResetNonVipAutoStopTimerState();
	ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
	if (pSRIns->isPausing())
		pSRIns->ResumeRecording();
	pSRIns->ReleaseInstance();

	ResetWindowRecordUIState(true);

	App.m_Dlg_Main.UpdateRecentRecordFile(m_outputfileName);
	App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(m_outputfileName);
	m_IsRecording = false;
	NotifyRecTop_Stop();
	this->ShowWindow(SW_RESTORE);
	ModalDlg_MFC::ShowModal_RecordWindowMinimal();

	if (m_IsRecording)
	{
		m_Btn_StartRecord.LoadClickPNG(CHILDDLG_PNG_ISRECORDING);
		m_Btn_StartRecord.LoadPNG(CHILDDLG_PNG_ISRECORDING);
		Invalidate();
	}
	else
	{
		m_Btn_StartRecord.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
		m_Btn_StartRecord.LoadPNG(CHILDDLG_PNG_STARTRECORD);
		Invalidate();
	}
	return 1;
}

LRESULT Ui_WindowRecordDlg::On_BroadCast_UserLogin(WPARAM wParam, LPARAM lParam)
{
	//updatelogin
	return 1;
}

LRESULT Ui_WindowRecordDlg::OnNcHitTest(CPoint point)
{
	// 转到客户区坐标
	CPoint clientPoint = point;
	ScreenToClient(&clientPoint);

	//  如果当前有 hover 且鼠标已移出整个缩略图显示区域，repaint整个窗口，清除高亮
	if (m_Bool_IsMouseInThumbnail &&
		!m_Rect_WindowsDiplayArea.Contains(clientPoint.x, clientPoint.y))
	{
		m_Bool_IsWindowDisplayNeedRedraw = true;
		Invalidate();

		// 清除 hover 状态
		m_Bool_IsMouseInThumbnail = FALSE;
		m_Rect_CurrnetHovertTumbnailRect = Gdiplus::Rect();
		m_Rect_LastHovertTumbnailRect = Gdiplus::Rect();
	}

	if (m_Bool_IsWindowImageReady &&
		m_Rect_WindowsDiplayArea.Contains(clientPoint.x, clientPoint.y))
	{
		for (auto& thumbnailRect : m_thumbnailRects)
		{
			if (thumbnailRect.rect.Contains(clientPoint.x, clientPoint.y))
			{
				// 只有当“新”cell 与当前 cell 不同时才处理
				if (!m_Rect_CurrnetHovertTumbnailRect.Equals(thumbnailRect.rect))
				{
					//  先用 上一次的 current 计算并失效“旧”的图片区域
					if (!m_Rect_CurrnetHovertTumbnailRect.IsEmptyArea())
					{
						CRect oldImgArea(
							m_Rect_CurrnetHovertTumbnailRect.X + kImagePadding,
							m_Rect_CurrnetHovertTumbnailRect.Y + kImagePadding,
							m_Rect_CurrnetHovertTumbnailRect.X + m_Rect_CurrnetHovertTumbnailRect.Width - kImagePadding,
							m_Rect_CurrnetHovertTumbnailRect.Y + m_Rect_CurrnetHovertTumbnailRect.Height - kThumbnailTitleHeight - kImagePadding
						);
						InvalidateRect(&oldImgArea, TRUE);
					}

					// 记录上一张并更新 current
					m_Rect_LastHovertTumbnailRect = m_Rect_CurrnetHovertTumbnailRect;
					m_Rect_CurrnetHovertTumbnailRect = thumbnailRect.rect;
					m_Bool_IsMouseInThumbnail = true;

					// 再失效“新”的图片区域
					CRect newImgArea(
						thumbnailRect.rect.X + kImagePadding,
						thumbnailRect.rect.Y + kImagePadding,
						thumbnailRect.rect.X + thumbnailRect.rect.Width - kImagePadding,
						thumbnailRect.rect.Y + thumbnailRect.rect.Height - kThumbnailTitleHeight - kImagePadding
					);
					DB(ConsoleHandle, L"repaint缩略图区域");
					InvalidateRect(&newImgArea);
				}
				break;
			}
		}
	}

	// 获取当前鼠标状态，判断是否为点击事件
	static bool isMouseDown = false;
	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) {
		// 鼠标左键按下状态
		if (!isMouseDown) {
			// 鼠标刚刚按下 - 只在这种情况下处理导航按钮点击
			isMouseDown = true;
			m_PopupListBox.HideListBox();
			// 检查是否点击了缩略图
			for (const auto& thumb : m_thumbnailRects)
			{
				if (clientPoint.x >= thumb.rect.X &&
					clientPoint.x <= thumb.rect.X + thumb.rect.Width &&
					clientPoint.y >= thumb.rect.Y &&
					clientPoint.y <= thumb.rect.Y + thumb.rect.Height)
				{
					// 处理缩略图点击
					HandleThumbnailClick(thumb.hwnd);
					return HTCLIENT; // 避免拖动窗口
				}
			}

			// 检查是否点击了导航按钮
			for (const auto& btn : m_NavButtons)
			{
				if (clientPoint.x >= btn.rect.X &&
					clientPoint.x <= btn.rect.X + btn.rect.Width &&
					clientPoint.y >= btn.rect.Y &&
					clientPoint.y <= btn.rect.Y + btn.rect.Height)
				{
					if (btn.enabled)
					{
						DEBUG_CONSOLE_FMT(ConsoleHandle, L"OnNcHitTest: Clicked on navigation button with action: %d\n", btn.action);
						PostMessage(WM_USER + 100, btn.action);
						break;
					}
				}
			}
		}
	}
	else {
		// 鼠标左键松开状态
		isMouseDown = false;
	}

	// 先调用基类获得原始命中区域
	LRESULT hit = CDialog::OnNcHitTest(point);
	if (hit == HTCLIENT)
	{
		// 如果点击在客户区，则返回 HTCAPTION 使得窗口可拖动
		return HTCAPTION;
	}
	return hit;
}

void Ui_WindowRecordDlg::HandleThumbnailClick(HWND hwnd)
{
	if (!::IsWindow(hwnd))
	{
		ModalDlg_MFC::ShowModal_WindowHwndInvalid();
		OnBnClickedBtnRefresh();
		return;
	}

	// 获取窗口标题
	std::wstring windowTitle = WindowHandleManager::GetWindowTitle(hwnd);

	// 设置选中的窗口句柄和标题
	m_selectedWindowHandle = hwnd;

	//更新当前选中窗口句柄指针的指向
	m_Hwnd_CurSelect = hwnd;

	ShowHWndOnTop(hwnd);

	// 更新UI显示选中的窗口
	m_Btn_RecordWindowOption.SetWindowText(windowTitle.c_str());
	m_Btn_NoSelect.ShowWindow(SW_HIDE);
	m_Btn_RecordWindowReselect.ShowWindow(SW_SHOW);
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"选择了窗口: %s\n", windowTitle.c_str());
	Invalidate();

}

BOOL Ui_WindowRecordDlg::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//EnableShadow();
	return CDialogEx::OnNcActivate(bActive);
}

HBRUSH Ui_WindowRecordDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	//int id = pWnd->GetDlgCtrlID();
	//if (id == WWDLG_BTN_COMBOX)
	//{//设置指定滑块的背景色
	//	pDC->SetBkColor(RGB(26, 31, 37));
	//	static CBrush s_brushBackground(RGB(26, 31, 37));
	//	return (HBRUSH)s_brushBackground;
	//}

	return hbr;
}

BOOL Ui_WindowRecordDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// 获取当前鼠标位置
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	// 检查是否悬停在缩略图上
	for (const auto& thumb : m_thumbnailRects)
	{
		if (point.x >= thumb.rect.X &&
			point.x <= thumb.rect.X + thumb.rect.Width &&
			point.y >= thumb.rect.Y &&
			point.y <= thumb.rect.Y + thumb.rect.Height)
		{
			// 设置鼠标光标为手型
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return TRUE; // 表示已处理光标设置
		}
	}

	// 检查是否悬停在导航按钮上 (现有代码)
	for (const auto& btn : m_NavButtons)
	{
		if (point.x >= btn.rect.X &&
			point.x <= btn.rect.X + btn.rect.Width &&
			point.y >= btn.rect.Y &&
			point.y <= btn.rect.Y + btn.rect.Height)
		{
			if (btn.enabled)
			{
				// 设置鼠标光标为手型
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return TRUE; // 表示已处理光标设置
			}
		}
	}

	// 对于其他区域，使用默认光标
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void Ui_WindowRecordDlg::OnBnClickedBtnClose()
{
	m_PopupListBox.HideListBox();
	if (!ScreenRecorder::GetInstance()->IsRecording())
	{
		ResetWindowRecordUIState(true);
	}
	ResetNonVipAutoStopTimerState();
	if (ModalDlg_MFC::ShowModal_IsCloseToBar() == IDCANCEL)
		return;

	ShowWindow(SW_HIDE);
	App.m_Dlg_Main.UpdateQuitWay();
	App.m_Dlg_Main.HandleClose();
}

void Ui_WindowRecordDlg::OnBnClickedBtnRecordwindowoption()
{
	// 更新下拉框的显示位置
	if (m_PopupListBox.IsListBoxVisiable(L"录制窗口下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	CRect Rect;
	m_Btn_RecordWindowOption.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"录制窗口下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_WindowRecordDlg::OnBnClickedBtnRecordmode()
{
	// 更新下拉框的显示位置
	if (m_PopupListBox.IsListBoxVisiable(L"录制模式下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	CRect Rect;
	m_Btn_RecordMode.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"录制模式下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_WindowRecordDlg::OnBnClickedBtnSystemaudiooption()
{
	// 更新下拉框的显示位置
	if (m_PopupListBox.IsListBoxVisiable(L"系统声音下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	CRect Rect;
	m_Btn_SystemAudioOption.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"系统声音下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_WindowRecordDlg::OnBnClickedBtnMicroaudiooption()
{
	// 更新下拉框的显示位置
	if (m_PopupListBox.IsListBoxVisiable(L"麦克风声音下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	CRect Rect;
	m_Btn_MicroAudioOption.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"麦克风声音下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_WindowRecordDlg::OnBnClickedBtnCombox()
{
	// 更新下拉框的显示位置
	if (m_PopupListBox.IsListBoxVisiable(L"录制选项下拉框"))
	{
		m_PopupListBox.HideListBox();
		return;
	}
	CRect Rect;
	m_Btn_ComBox.GetWindowRect(&Rect);
	m_PopupListBox.UpdateDroplistXY(L"录制选项下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_WindowRecordDlg::OnBnClickedBtnReselectrecordwindow()
{
	m_PopupListBox.HideListBox();
	if (m_Hwnd_CurSelect)
	{
		m_Hwnd_CurSelect = nullptr;
		m_Btn_RecordWindowOption.SetWindowTextW(L"未选择");
		m_Btn_RecordWindowReselect.ShowWindow(SW_HIDE);
		m_Btn_NoSelect.ShowWindow(SW_SHOW);
		Invalidate();
	}
}

LRESULT Ui_WindowRecordDlg::OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam)
{
	CRect rect, parRect;
	auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	MsgParam::LISTBOX_SELECT_INFO* pInfo = (MsgParam::LISTBOX_SELECT_INFO*)wParam;
	if (pInfo)
	{
		int nIndex = pInfo->nIndex;
		CString strText = pInfo->strText;
		std::wstring strBoxName = pInfo->strBoxName;
		// 根据不同的下拉框名称进行不同处理
		if (strBoxName == L"录制窗口下拉框")
		{
			m_Btn_RecordWindowOption.SetWindowText(strText);
			// 遍历窗口找到匹配标题的窗口句柄
			for (const auto& pair : m_WindowImages)
			{
				HWND hwnd = pair.first;
				std::wstring windowTitle = WindowHandleManager::GetWindowTitle(hwnd);
				if (windowTitle == strText.GetString())
				{
					// 记录选中的窗口句柄
					m_selectedWindowHandle = hwnd;
					m_Hwnd_CurSelect = hwnd;
					m_Btn_RecordWindowReselect.SetWindowTextW(L"重新选择");
					m_Btn_NoSelect.ShowWindow(SW_HIDE);
					m_Btn_RecordWindowReselect.ShowWindow(SW_SHOW);
					Invalidate();
					break;
				}
			}
		}
		else if (strBoxName == L"录制模式下拉框")
		{
			m_Btn_RecordMode.SetWindowText(strText);
		}
		else if (strBoxName == L"系统声音下拉框")
		{
			m_Btn_SystemAudioOption.SetWindowText(strText);
			pDlg->m_Dlg_Config->m_Btn_AudioDevice.SetWindowTextW(strText);
		}
		else if (strBoxName == L"麦克风声音下拉框")
		{
			m_Btn_MicroAudioOption.SetWindowText(strText);
			pDlg->m_Dlg_Config->m_Btn_MicroDevice.SetWindowTextW(strText);
		}
		else if (strBoxName == L"录制选项下拉框")
		{
			m_Btn_ComBox.SetWindowText(strText);
		}
	}
	ReturnToPage();
	return 1;
}

void Ui_WindowRecordDlg::ReturnToPage()
{
	if (App.m_Dlg_Main.m_Dlg_RecTopDlg->IsWindowVisible())
		App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_HIDE);

	CString mode;
	m_Btn_ComBox.GetWindowTextW(mode);
	m_PopupListBox.HideListBox();
	if (mode == L"录窗口")
	{//不需要转到其他窗口
		return;
	}


	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			this->OnBnClickedBtnStartrecord();	//停止录制
			NotifyRecTop_Stop();				//同步顶栏退出“录制中”状态
			App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_HIDE);
		}
		else
		{
			return;
		}
	}

	//需要转到其他窗口
	m_Btn_ComBox.SetWindowTextW(L"录窗口");
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
		pDlg->OnBnClickedBtnScreenRecord();
		m_Bool_IsWindowDisplayNeedRedraw = true;
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录区域")
	{
		::PostMessage(this->GetSafeHwnd(), MSG_WINDOWRECORDPAGE_CLICKAREARECORD, NULL, NULL);
		m_Bool_IsWindowDisplayNeedRedraw = true;
	}
	else if (mode == L"录游戏")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		pDlg->ShowWindow(SW_SHOW);
		pDlg->OnBnClickedBtnGamingrecording();
		m_Bool_IsWindowDisplayNeedRedraw = true;
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录摄像头")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		pDlg->ShowWindow(SW_SHOW);
		pDlg->On_UiDropDowmMeau_CameraRecord(NULL, NULL);
		m_Bool_IsWindowDisplayNeedRedraw = true;
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"录声音")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		this->GetParent()->ShowWindow(SW_SHOW);
		::PostMessage(this->GetParent()->GetSafeHwnd(), MSG_UIDROPDOWNMENU_SYSTEMAUDIOMICRO, NULL, NULL);
		m_Bool_IsWindowDisplayNeedRedraw = true;
		m_Shadow.Show(m_hWnd);
	}
	else if (mode == L"跟随鼠标")
	{
		m_Shadow.Show(m_hWnd);
		this->ShowWindow(SW_HIDE);
		this->GetParent()->ShowWindow(SW_SHOW);
		::PostMessage(this->GetParent()->GetSafeHwnd(), MSG_UIDROPDOWNMENU_MOUSERECORD, NULL, NULL);
		m_Bool_IsWindowDisplayNeedRedraw = true;
		m_Shadow.Show(m_hWnd);
	}
}

void Ui_WindowRecordDlg::OnBnClickedBtnStartrecord()
{
	m_PopupListBox.HideListBox();
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
			if (!ModalDlg_SDL::ShowModal_Rai(this, m_Scale))
			{
				//点击了开通会员
				App.m_Dlg_Main.OnBnClickedBtnOpenvip();
				return;
			}
		}
		App.UserWindowRecord(false);

		// 获取所有窗口句柄
		auto& manager = WindowHandleManager::GetInstance();
		const auto& handles = manager.GetWindowHandles();

		DBFMT(ConsoleHandle, L"目标录制的Hwnd:%p", (void*)m_Hwnd_CurSelect);
		if (!::IsWindow(m_Hwnd_CurSelect))
		{
			ModalDlg_MFC::ShowModal_WindowHwndInvalid();
			OnBnClickedBtnRefresh();
			m_Hwnd_CurSelect = nullptr;
			return;
		}

		// 打印所有窗口标题
		DEBUG_CONSOLE_STR(ConsoleHandle, L"----窗口标题----");
		int index = 0;
		HWND RecordinghWnd = nullptr;
		for (HWND hwnd : handles)
		{
			DBFMT(ConsoleHandle, L"当前遍历的hwnd:%p", (void*)hwnd);
			if (hwnd == m_Hwnd_CurSelect)
			{
				RecordinghWnd = hwnd;
				DBFMT(ConsoleHandle, L"当前遍历的获取的hwnd:%p成功赋值给RecordinghWnd:%p，录制的窗口标题为:%s", 
					(void*)hwnd, (void*)RecordinghWnd, manager.GetWindowTitle(RecordinghWnd).c_str());
				break;
			}
		}
		if (!RecordinghWnd)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"录制了无效的窗口句柄，无法进行录制");
			ModalDlg_MFC::ShowModal_RecordHwndInvalid();
			return;
		}

		// 如果被录制的窗口最小化,先提示用户是否解除最小化
		if (::IsIconic(RecordinghWnd))
		{
			if (ModalDlg_MFC::ShowModal_WindowMinimalRecordTips() == IDCANCEL)
			{
				return;
			}
			else
			{
				//等待被录制窗口完全显示
				::ShowWindow(RecordinghWnd, SW_RESTORE);
				::UpdateWindow(RecordinghWnd);
				int maxWaitTime = 50;
				int waitCount = 0;
				while (::IsIconic(RecordinghWnd) && waitCount < maxWaitTime)
				{
					Sleep(100); // 每次等待100毫秒
					waitCount++;
				}
				Sleep(1000);//等待窗口完全绘制
				if (!::IsWindowVisible(RecordinghWnd))
				{ // 确保窗口可见
					::ShowWindow(RecordinghWnd, SW_SHOW);
				}
				::SetForegroundWindow(RecordinghWnd);
				::BringWindowToTop(RecordinghWnd);
			}
		}

		//录制指定窗口
		std::wstring title = WindowHandleManager::GetWindowTitle(RecordinghWnd);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"录制的窗口:%s", title.c_str());

		RecordingParams recp = App.m_Dlg_Main.CollectRecordingParams();
		CString recordModeStr;
		m_Btn_RecordMode.GetWindowTextW(recordModeStr);
		if (recordModeStr == L"录制系统音频")
			recp.recordMode = ScreenRecorder::RecordMode::SystemSound;
		else if (recordModeStr == L"录制麦克风")
			recp.recordMode = ScreenRecorder::RecordMode::Microphone;
		else if (recordModeStr == L"录制系统音频和麦克风")
			recp.recordMode = ScreenRecorder::RecordMode::Both;
		else
		{
			DB(ConsoleHandle, L"录制模式获取错误");
			throw std::runtime_error("error while select record mode during window recode start");
		}
			

		//应用窗口录制
		ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
		pSRIns->SetVideoEncoder(GlobalFunc::MapUICodecToEncoderName(recp.codecText.c_str()));
		pSRIns->SetAudioCaptureDevice(recp.audioDevice.c_str());
		pSRIns->SetMicroDeviceName(recp.microDevice.c_str());
		pSRIns->SetRecordMouse(recp.RecordMouse);
		pSRIns->SetRecordCallBack(ScreenRecorder::WindowRecord_WindowMinimalAndClose, [this]()
			{//设置窗口最小化时，进行的回调函数
				::PostMessage(this->GetSafeHwnd(), MSG_WINDOWRECORDPAGE_RECORDWINDOWMINIMAL, NULL, NULL);
			});
		pSRIns->SetWindowRecordParam(
			RecordinghWnd,
			recp.videoResolution,
			recp.videoQuality,
			recp.videoFormat,
			recp.encodePreset,
			recp.recordMode,
			recp.audioSampleRate,
			recp.audioBitRate,
			recp.fps);
		this->ShowWindow(SW_MINIMIZE);
		pSRIns->SetRecordSizeCallback([=](double mb)
			{
				::PostMessage(this->GetSafeHwnd(),
					MSG_SCRENC_FILESIZEUPDATE,
					(WPARAM)(mb * 100.0),
					(LPARAM)0);
			});
		pSRIns->SetRecordSizeCallbackInterval(1000);

		ModalDlg_SDL::ShowModal_CountDown(3, "即将开始录制", [=]()
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"倒计时结束");
			}, true);

		//开始录制 
		std::thread([this, recp, pSRIns]()
			{
				CT2A outputfileName(recp.outputFilePath.c_str());
				m_IsRecording = pSRIns->startRecording(outputfileName);
			}).detach();

		CT2A outputfileName(recp.outputFilePath.c_str());
		m_IsRecording = true;
		m_outputfileName = outputfileName;
		StartNonVipAutoStopTimerIfNeeded();

		// 初始化剩余空间并立刻更新显示
		m_windowLastOutputPath = recp.outputFilePath;
		if (!InitWindowRecordFreeSpaceFromPath(m_windowLastOutputPath))
		{
			m_windowHasFreeSpace = false;
			m_windowFreeSpaceGB = 0.0;
		}
		UpdateWindowRecordSizeUI(0.0);

		// 切换按钮显示和文本显示
		m_Btn_StartRecord.ShowWindow(SW_HIDE);
		m_Btn_WindowRecPause.ShowWindow(SW_SHOW);
		m_Btn_WindowRecStop.ShowWindow(SW_SHOW);
		m_Btn_WindowRecResume.ShowWindow(SW_HIDE);
		m_stat_hotKeyStartRecord.ShowWindow(SW_HIDE);
		SetWindowRecordCountTimer();// 启动计时
		NotifyRecTop_Start();
		
	}
	else
	{//停止录制
		App.UserWindowRecord(true);
		ResetNonVipAutoStopTimerState();
		ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
		if (pSRIns->isPausing())
			pSRIns->ResumeRecording();
		pSRIns->ReleaseInstance();
		
		ResetWindowRecordUIState(true);

		App.m_Dlg_Main.UpdateRecentRecordFile(m_outputfileName);
		App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(m_outputfileName);
		m_IsRecording = false;
		NotifyRecTop_Stop();
		this->ShowWindow(SW_RESTORE);
		ModalDlg_MFC::ShowModal_Priview(this);
	}

	if (m_IsRecording)
	{
		m_Btn_StartRecord.LoadClickPNG(CHILDDLG_PNG_ISRECORDING);
		m_Btn_StartRecord.LoadPNG(CHILDDLG_PNG_ISRECORDING);
		Invalidate();
	}
	else
	{
		m_Btn_StartRecord.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
		m_Btn_StartRecord.LoadPNG(CHILDDLG_PNG_STARTRECORD);
		Invalidate();
	}
}

void Ui_WindowRecordDlg::OnBnClickedBtnRefresh()
{
	m_PopupListBox.HideListBox();
	m_Bool_IsWindowImageReady = false;
	OnBnClickedBtnReselectrecordwindow();
	DB(ConsoleHandle, L"点击刷新按钮，并设置m_Bool_IsWindowImageReady为falses");
	Invalidate(false);
	UpdateWindow();

	if (m_Thread_CaptureWindowImage.joinable())
	{
		m_Thread_CaptureWindowImage.join();
	}
	m_Thread_CaptureWindowImage = std::thread(&Ui_WindowRecordDlg::CaptureWindowImageThread, this);
}

void Ui_WindowRecordDlg::OnBnClickedBtnMinimal()
{
	m_PopupListBox.HideListBox();
	m_Shadow.Show(m_hWnd);
	this->ShowWindow(SW_MINIMIZE);
	App.m_Dlg_Main.SetTrayClickCallback([this]()
		{
			this->ShowWindow(SW_SHOW);
			this->ShowWindow(SW_RESTORE);
			this->SetForegroundWindow();
		});
}

void Ui_WindowRecordDlg::OnBnClickedBtnAdvanceoptionsystemaudio()
{
	m_PopupListBox.HideListBox();
	auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
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
	if (!pDlg->m_Dlg_Config) {
		pDlg->m_Dlg_Config = new Ui_ConfigDlg;
		pDlg->m_Dlg_Config->Ui_SetWindowRect(WindowRect);
		if (!pDlg->m_Dlg_Config->Create(IDD_DIALOG_CONFIGDLG, this)) {
			delete pDlg->m_Dlg_Config;
			pDlg->m_Dlg_Config = nullptr;
			return;
		}
	}
	::SetWindowPos(pDlg->m_Dlg_Config->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	this->EnableWindow(FALSE);// 禁用主窗口以模拟模态行为
	// 更新位置并显示配置窗口
	pDlg->m_Dlg_Config->Ui_UpdateWindowPos(WindowRect);
	pDlg->m_Dlg_Config->SetForegroundWindow();
	pDlg->m_Dlg_Config->ShowWindow(SW_SHOW);
	pDlg->m_Dlg_Config->m_bool_IsEraseTopMost = false;
	pDlg->m_Dlg_Config->SetTimer(Ui_ConfigDlg::TIMER_DELAYED_REDRAW, 50, NULL);
}

void Ui_WindowRecordDlg::OnBnClickedBtnAdvancemicrooption()
{
	m_PopupListBox.HideListBox();
	OnBnClickedBtnAdvanceoptionsystemaudio();
}

void Ui_WindowRecordDlg::OnBnClickedBtnConfig()
{
	m_PopupListBox.HideListBox();
	OnBnClickedBtnAdvancemicrooption();
}

LRESULT Ui_WindowRecordDlg::OnBnClickBtnAreaRecord(WPARAM wParam, LPARAM lParam)
{
	auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	SendMessage(
		WM_COMMAND,
		MAKEWPARAM(WWDLG_BTN_RETURN, BN_CLICKED)
	);
	std::thread([pDlg]()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			pDlg->PostMessage(
				WM_COMMAND,
				MAKEWPARAM(MAINDLG_BTN_AREARECORDING, BN_CLICKED)
			);
		}).detach();
	//pDlg->OnBnClickedBtnArearecording();
	return 1;
}

void Ui_WindowRecordDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	//m_Shadow.Show(m_hWnd);
	// TODO: 在此处添加消息处理程序代码
}

void Ui_WindowRecordDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// 如果尚未追踪，则同时请求 WM_MOUSELEAVE 和 WM_NCMOUSELEAVE
	if (!m_bMouseLeaveTrack)
	{
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.dwFlags = TME_LEAVE | TME_NONCLIENT;  // 同时监听客户区和非客户区离开
		tme.hwndTrack = m_hWnd;
		::TrackMouseEvent(&tme);
		m_bMouseLeaveTrack = TRUE;
	}

	CDialogEx::OnMouseMove(nFlags, point);  // 之后走 OnNcHitTest 处理 hover 切换
}

void Ui_WindowRecordDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_PopupListBox.HideListBox();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_WindowRecordDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);
	if (nState != WA_INACTIVE) {
		m_Bool_IsWindowDisplayNeedRedraw = true;
		Invalidate();
	}
}

void Ui_WindowRecordDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialogEx::OnWindowPosChanged(lpwndpos);

	// 当窗口位置或大小变化时，可能需要repaint
	if (!(lpwndpos->flags & SWP_NOMOVE) || !(lpwndpos->flags & SWP_NOSIZE)) {
		m_Bool_IsWindowDisplayNeedRedraw = false;
	}
}

void Ui_WindowRecordDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (bShow) {
		// 窗口显示时强制repaint缩略图
		m_Bool_IsWindowDisplayNeedRedraw = true;
		Invalidate();
	}
}

void Ui_WindowRecordDlg::OnExitSizeMove()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Bool_IsWindowDisplayNeedRedraw = true;
	Invalidate();
	CDialogEx::OnExitSizeMove();
}

void Ui_WindowRecordDlg::OnMouseLeave()
{
	m_bMouseLeaveTrack = FALSE;
	ClearHoverState();
	return;
}

void Ui_WindowRecordDlg::ClearHoverState()
{
	if (!m_Bool_IsMouseInThumbnail)
		return;

	// 整个 cell 区域（含标题）
	CRect fullCell(
		m_Rect_CurrnetHovertTumbnailRect.X,
		m_Rect_CurrnetHovertTumbnailRect.Y,
		m_Rect_CurrnetHovertTumbnailRect.X + m_Rect_CurrnetHovertTumbnailRect.Width,
		m_Rect_CurrnetHovertTumbnailRect.Y + m_Rect_CurrnetHovertTumbnailRect.Height
	);

	// 标记下次要repaint所有缩略图
	m_Bool_IsWindowDisplayNeedRedraw = true;

	// 擦除旧蓝框
	InvalidateRect(&fullCell);

	// 清除悬停状态
	m_Bool_IsMouseInThumbnail = FALSE;
	m_Rect_CurrnetHovertTumbnailRect = Rect();
	m_Rect_LastHovertTumbnailRect = Rect();
}

void Ui_WindowRecordDlg::DrawThumbnailsUiMode(Gdiplus::Graphics* memGraphics)
{
	std::lock_guard<std::mutex> lk(m_imagesMutex);
	//窗口句柄截图显示区域的绘图是否需要
	if (m_Bool_IsWindowDisplayNeedRedraw)
	{
		DB(ConsoleHandle, L"开始repaint缩略窗口显示区域");
		if (!DrawWindowThumbnails(memGraphics))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"绘画“正在加载中”");
			// 创建字体和画刷
			Gdiplus::Font font(L"微软雅黑", 12, Gdiplus::FontStyleRegular);
			Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255, 255)); // 黑色画刷

			// 定义要显示的文字
			const WCHAR* text = L"正在加载中";

			// 测量文字区域大小
			Gdiplus::RectF boundRect;
			memGraphics->MeasureString(text, -1, &font, Gdiplus::PointF(0, 0), &boundRect);

			// 计算文字在矩形中的居中位置
			float x = m_Rect_WindowsDiplayArea.X + (m_Rect_WindowsDiplayArea.Width - boundRect.Width) / 2;
			float y = m_Rect_WindowsDiplayArea.Y + (m_Rect_WindowsDiplayArea.Height - boundRect.Height) / 2;

			// 在计算出的位置绘制文字
			memGraphics->DrawString(text, -1, &font, Gdiplus::PointF(x, y), &brush);
		}
	}

	if (m_Bool_IsMouseInThumbnail)
	{
		memGraphics->SetSmoothingMode(SmoothingModeHighQuality);
		memGraphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
		memGraphics->SetTextContrast(255);

		// --- 当前 hover 缩略图 ---
		// 找到索引
		int idx = 0;
		for (auto& tr : m_thumbnailRects)
		{
			if (tr.rect.Equals(m_Rect_CurrnetHovertTumbnailRect))
				break;
			++idx;
		}
		idx = m_currentPage * 8 + idx;
		if (idx < 0 || idx >= (int)m_WindowImages.size())
		{// 这轮 hover 已经无效了，清掉状态，下次画原图或 loading
			m_Bool_IsMouseInThumbnail = false;
			DB(ConsoleHandle, L"hover 已经无效,去掉状态返回");
			return;
		}
		auto itCur = m_WindowImages.begin();
		std::advance(itCur, idx);
		if (itCur == m_WindowImages.end() || itCur->second == nullptr)
		{
			DB(ConsoleHandle, L"(itCur == m_WindowImages.end() || itCur->second == nullptr)为真，返回");
			m_Bool_IsMouseInThumbnail = false;
			return;
		}

		// 计算内部图片区域
		CRect curImgArea(
			m_Rect_CurrnetHovertTumbnailRect.X + kImagePadding,
			m_Rect_CurrnetHovertTumbnailRect.Y + kImagePadding,
			m_Rect_CurrnetHovertTumbnailRect.X + m_Rect_CurrnetHovertTumbnailRect.Width - kImagePadding,
			m_Rect_CurrnetHovertTumbnailRect.Y + m_Rect_CurrnetHovertTumbnailRect.Height - kThumbnailTitleHeight - kImagePadding
		);
		Rect curImgArea_Rect;
		curImgArea_Rect.X = curImgArea.left;
		curImgArea_Rect.Y = curImgArea.top;
		curImgArea_Rect.Width = curImgArea.Width();
		curImgArea_Rect.Height = curImgArea.Height();

		// 利用 ColorMatrix 调暗当前缩略图
		{
			const float brightness = 0.6f;
			Gdiplus::ColorMatrix cm = {
				brightness, 0,          0,          0, 0,
				0,          brightness, 0,          0, 0,
				0,          0,          brightness, 0, 0,
				0,          0,          0,          1, 0,
				0,          0,          0,          0, 1
			};
			Gdiplus::ImageAttributes imgAttr;
			imgAttr.SetColorMatrix(&cm, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

			// itCur 是 std::map<HWND,Image*>::iterator
			Gdiplus::Image* srcImg = itCur->second;
			UINT srcW = srcImg->GetWidth();
			UINT srcH = srcImg->GetHeight();

			Gdiplus::RectF dstRect(
				(REAL)curImgArea_Rect.X,
				(REAL)curImgArea_Rect.Y,
				(REAL)curImgArea_Rect.Width,
				(REAL)curImgArea_Rect.Height
			);
			memGraphics->DrawImage(
				srcImg,
				dstRect,
				0, 0,
				(REAL)srcW,
				(REAL)srcH,
				Gdiplus::UnitPixel,
				&imgAttr
			);
		}

		if (m_Bitmap_AddWindow)// 绘制鼠标悬停图标到缩略图内部正中偏上
		{
			INT iconW = m_Bitmap_AddWindow->GetWidth();
			INT iconH = m_Bitmap_AddWindow->GetHeight();
			// 目标绘制矩形
			float iconX = curImgArea_Rect.X + (curImgArea_Rect.Width - iconW) / 2.0f;
			// 往上偏移 half icon 高度
			float iconY = curImgArea_Rect.Y + (curImgArea_Rect.Height - iconH) / 2.0f - 10.0f;
			memGraphics->DrawImage(
				m_Bitmap_AddWindow,
				iconX, iconY,
				static_cast<REAL>(iconW),
				static_cast<REAL>(iconH)
			);
		}

		// 在图标下方绘制白色提示文字 “点击选择该窗口”
		{
			Gdiplus::Font hintFont(L"微软雅黑", 10, Gdiplus::FontStyleRegular);
			Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
			Gdiplus::StringFormat sf;
			sf.SetAlignment(Gdiplus::StringAlignmentCenter);
			// 文字中心 X 坐标
			float textX = curImgArea_Rect.X + curImgArea_Rect.Width / 2.0f;
			// 文字 Y 坐标：图标底部 + 4px 间距
			float textY = (m_Bitmap_AddWindow
				? (curImgArea_Rect.Y + (curImgArea_Rect.Height - m_Bitmap_AddWindow->GetHeight()) / 2.0f
					- 10.0f + m_Bitmap_AddWindow->GetHeight() + 4.0f)
				: (curImgArea_Rect.Y + curImgArea_Rect.Height / 2.0f));
			Gdiplus::PointF origin(textX, textY);
			memGraphics->DrawString(
				L"点击选择该窗口",
				-1,
				&hintFont,
				origin,
				&sf,
				&textBrush
			);
		}

		// 画图蓝色边框
		memGraphics->SetPixelOffsetMode(PixelOffsetModeHalf);
		Pen pen(Color(0, 139, 255), 1.0f);
		RectF r(
			curImgArea_Rect.X + 0.5f,
			curImgArea_Rect.Y + 0.5f,
			curImgArea_Rect.Width - 1.0f,
			curImgArea_Rect.Height - 1.0f
		);
		pen.SetAlignment(PenAlignmentInset);
		memGraphics->DrawRectangle(&pen, r);

		// 上一次 hover 的缩略图，仅repaint图片
		if (!m_Rect_LastHovertTumbnailRect.IsEmptyArea())
		{
			int idxLast = 0;
			for (auto& tr : m_thumbnailRects)
			{
				if (tr.rect.Equals(m_Rect_LastHovertTumbnailRect))
					break;
				++idxLast;
			}
			idxLast = m_currentPage * 8 + idxLast;
			size_t count = m_WindowImages.size();
			if (idxLast >= count) {
				DBFMT(ConsoleHandle, L"idxLast[%u] 超出范围，映射大小=%zu，跳过repaint", idxLast, count);
				return;
			}
			auto itLast = m_WindowImages.begin();
			std::advance(itLast, idxLast);
			CRect lastImgArea(
				m_Rect_LastHovertTumbnailRect.X + kImagePadding,
				m_Rect_LastHovertTumbnailRect.Y + kImagePadding,
				m_Rect_LastHovertTumbnailRect.X + m_Rect_LastHovertTumbnailRect.Width - kImagePadding,
				m_Rect_LastHovertTumbnailRect.Y + m_Rect_LastHovertTumbnailRect.Height - kThumbnailTitleHeight - kImagePadding
			);
			Rect lastImgArea_Rect;
			lastImgArea_Rect.X = lastImgArea.left;
			lastImgArea_Rect.Y = lastImgArea.top;
			lastImgArea_Rect.Width = lastImgArea.Width();
			lastImgArea_Rect.Height = lastImgArea.Height();
			memGraphics->DrawImage((*itLast).second, lastImgArea_Rect);
		}
	}
}

void Ui_WindowRecordDlg::DrawWindowImageUiMode(Gdiplus::Graphics* graphics)
{
	DB(ConsoleHandle, L"获取m_imagesMutex锁");
	std::lock_guard<std::mutex> lk(m_imagesMutex);
	DB(ConsoleHandle, L"获取到m_imagesMutex锁");
	// 查找选中窗口的 Image*
	auto it = m_WindowImages.find(m_Hwnd_CurSelect);
	if (it == m_WindowImages.end() || it->second == nullptr)
		return;


	Gdiplus::Image* srcImg = it->second;
	UINT srcW = srcImg->GetWidth();
	UINT srcH = srcImg->GetHeight();

	// 构建与 DrawAngleRect 一致的圆角路径，用于裁剪
	Gdiplus::GraphicsPath path;
	Gdiplus::RectF fullRectF(
		(REAL)m_Rect_WindowsDiplayArea.X,
		(REAL)m_Rect_WindowsDiplayArea.Y,
		(REAL)m_Rect_WindowsDiplayArea.Width,
		(REAL)m_Rect_WindowsDiplayArea.Height
	);
	const REAL r = 30.0f; // 圆角半径
	path.AddArc(fullRectF.X, fullRectF.Y, r * 2, r * 2, 180, 90);
	path.AddArc(fullRectF.X + fullRectF.Width - r * 2, fullRectF.Y, r * 2, r * 2, 270, 90);
	path.AddArc(fullRectF.X + fullRectF.Width - r * 2, fullRectF.Y + fullRectF.Height - r * 2, r * 2, r * 2, 0, 90);
	path.AddArc(fullRectF.X, fullRectF.Y + fullRectF.Height - r * 2, r * 2, r * 2, 90, 90);
	path.CloseFigure();

	// 裁剪到圆角区域
	graphics->SetClip(&path, Gdiplus::CombineModeReplace);

	// 填充整个圆角区域背景为黑色
	{
		Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
		graphics->FillPath(&blackBrush, &path);
	}

	// 计算左右黑边宽度 = 0.072 * 矩形宽度
	REAL margin = fullRectF.Width * 0.072f;

	// 缩略图高度填满区域，高度 = fullRectF.Height
	REAL targetH = fullRectF.Height;
	// 按原图纵横比缩放宽度
	REAL scaleH = targetH / (REAL)srcH;
	REAL targetW = srcW * scaleH;

	// 在左右 margin 内水平居中
	REAL innerWidth = fullRectF.Width - 2 * margin;
	REAL drawX = fullRectF.X + margin + (innerWidth - targetW) * 0.5f;
	REAL drawY = fullRectF.Y;

	// 绘制等比缩放后的图像
	DB(ConsoleHandle, L"绘制缩略图图像");
	graphics->DrawImage(
		srcImg,
		Gdiplus::RectF(drawX, drawY, targetW, targetH)
	);

	// 重置裁剪
	graphics->ResetClip();

	// 重画圆角边框
	{
		Gdiplus::Pen borderPen(Gdiplus::Color(60, 62, 66), 2.0f);
		graphics->DrawPath(&borderPen, &path);
	}
}

void Ui_WindowRecordDlg::ShowHWndOnTop(HWND hwnd)
{
	if (!::IsWindow(hwnd))
		return;
	if (::IsIconic(hwnd))
		::ShowWindow(hwnd, SW_RESTORE);
	::ShowWindow(hwnd, SW_SHOWNORMAL);
	::SetWindowPos(
		hwnd,
		HWND_TOP,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW
	);
}

void Ui_WindowRecordDlg::OnNcMouseLeave()
{
	m_bMouseLeaveTrack = FALSE;
	ClearHoverState();
}

void Ui_WindowRecordDlg::OnPaint()
{
	CPaintDC dc(this);
	//预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
	Gdiplus::Graphics memGraphics(&memBitmap);
	memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	memGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
	//memGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
	memGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

	//绘画标题栏背景和绘画客户区背景
	SolidBrush CaptionBrush(Color(37, 39, 46));
	SolidBrush ClientBrush(Color(26, 27, 32));
	memGraphics.FillRectangle(&CaptionBrush, m_Rect_Top);//绘画标题栏背景
	memGraphics.FillRectangle(//绘画客户区背景
		&ClientBrush,
		m_Rect_Top.GetLeft(),
		m_Rect_Top.GetBottom(),
		static_cast<INT>(m_WindowWidth),
		static_cast<INT>(m_WindowHeight - m_Rect_Top.GetBottom())
	);

	////绘画自定义控件的按钮边框
	//录制选项按钮边框
	SolidBrush BorderColor(Color(38, 45, 53));
	Pen BorderPen(&BorderColor, 2);
	CRect BtnComBoxRect;
	m_Btn_ComBox.GetWindowRect(BtnComBoxRect);
	ScreenToClient(BtnComBoxRect);
	memGraphics.DrawRectangle(&BorderPen, BtnComBoxRect.left, BtnComBoxRect.top, BtnComBoxRect.Width(),
		BtnComBoxRect.Height());

	//录制窗口选项按钮边框
	CRect RecordWindowOptionRect;
	m_Btn_RecordWindowOption.GetWindowRect(RecordWindowOptionRect);
	ScreenToClient(RecordWindowOptionRect);
	DrawBorder(&BorderPen, RecordWindowOptionRect, &memGraphics);

	//录制模式选项按钮边框
	CRect RecordModeBtnRect;
	m_Btn_RecordMode.GetWindowRect(RecordModeBtnRect);
	ScreenToClient(RecordModeBtnRect);
	DrawBorder(&BorderPen, RecordModeBtnRect, &memGraphics);

	//系统声音选项按钮边框
	CRect SystemAudioBtnrect;
	m_Btn_SystemAudioOption.GetWindowRect(SystemAudioBtnrect);
	ScreenToClient(SystemAudioBtnrect);
	DrawBorder(&BorderPen, SystemAudioBtnrect, &memGraphics);

	//麦克风选项按钮边框
	CRect MicroSystemBtnRect;
	m_Btn_MicroAudioOption.GetWindowRect(MicroSystemBtnRect);
	ScreenToClient(MicroSystemBtnRect);
	DrawBorder(&BorderPen, MicroSystemBtnRect, &memGraphics);

	//绘画开始录制旁边的竖线
	if (!m_IsRecording)
	{
		SolidBrush lineBrush(Color(6, 7, 8));
		Pen linePen(&lineBrush, 2);
		CRect StartRecordRect;
		m_Btn_StartRecord.GetWindowRect(StartRecordRect);
		ScreenToClient(StartRecordRect);
		Point p1, p2;
		p1.X = StartRecordRect.left - 35 * m_Scale;
		p1.Y = StartRecordRect.top;
		p2.X = p1.X;
		p2.Y = StartRecordRect.bottom;
		memGraphics.DrawLine(&linePen, p1, p2);
	}

	//绘画窗口显示区域矩形(圆角)
	DrawAngleRect(
		m_Rect_WindowsDiplayArea,
		&memGraphics,
		30.0f,                                // 圆角半径
		2.0f,                                 // 边框宽度
		Gdiplus::Color(60, 62, 66),           // 边框颜色
		Gdiplus::Color(36, 37, 40)            // 填充颜色
	);

	if (m_Bool_IsWindowImageReady)
	{
		if (m_Hwnd_CurSelect)
		{//缩略图绘画模式
			DB(ConsoleHandle, L"进入OnPaint执行缩略图绘画模式");
			DrawWindowImageUiMode(&memGraphics);
		}
		else
		{//选中缩略图后绘画模式
			DB(ConsoleHandle, L"进入OnPaint执行选中缩略图后绘画模式");
			DrawThumbnailsUiMode(&memGraphics);
		}
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"在窗口缩略图未准备好的情况下,绘画“正在加载中”");
		// 创建字体和画刷
		Gdiplus::Font font(L"微软雅黑", 12, Gdiplus::FontStyleRegular);
		Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255, 255)); // 黑色画刷

		// 定义要显示的文字
		const WCHAR* text = L"正在加载中";

		// 测量文字区域大小
		Gdiplus::RectF boundRect;
		memGraphics.MeasureString(text, -1, &font, Gdiplus::PointF(0, 0), &boundRect);

		// 计算文字在矩形中的居中位置
		float x = m_Rect_WindowsDiplayArea.X + (m_Rect_WindowsDiplayArea.Width - boundRect.Width) / 2;
		float y = m_Rect_WindowsDiplayArea.Y + (m_Rect_WindowsDiplayArea.Height - boundRect.Height) / 2;

		// 在计算出的位置绘制文字
		memGraphics.DrawString(text, -1, &font, Gdiplus::PointF(x, y), &brush);
	}

	// 录制时绘制录制时长及录制大小
	if (m_IsRecording && m_windowRecordUI.bkBrush)
	{
		Gdiplus::StringFormat fmtCenter;
		fmtCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
		fmtCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);

		// 计时框背景与边框
		memGraphics.FillRectangle(m_windowRecordUI.bkBrush, m_windowRecordUI.rect_timeBk);
		memGraphics.DrawRectangle(m_windowRecordUI.borderPen, m_windowRecordUI.rect_timeBk);

		// 计时文本
		memGraphics.DrawString(
			m_windowRecordUI.timerStr.c_str(), -1,
			m_windowRecordUI.timerFont,
			m_windowRecordUI.rect_timerText,
			&fmtCenter,
			m_windowRecordUI.timeTextBrush
		);

		// 大小文本
		Gdiplus::StringFormat fmtSize;
		fmtSize.SetAlignment(Gdiplus::StringAlignmentCenter);
		fmtSize.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		memGraphics.DrawString(
			m_windowRecordUI.sizeStr.c_str(), -1,
			m_windowRecordUI.sizeFont,
			m_windowRecordUI.rect_sizeText,
			&fmtSize,
			m_windowRecordUI.sizeTextBrush
		);
	}

	//一次性绘画到窗口上
	Gdiplus::Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));

	///设置按钮背景色
	m_Btn_Username.LarSetParentBk();
	m_Btn_Config.LarSetParentBk();
	m_Btn_FeedBack.LarSetParentBk();
	//m_Btn_ComBox.LarSetParentBk();

	m_Shadow.Show(m_hWnd);

	DB(ConsoleHandle, L"Ui_WindowRecordDlg:repaint..");
}

void Ui_WindowRecordDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == WINDOWRECDLG_TIMER_TIMERECORDED)
	{
		auto pSR = ScreenRecorder::GetInstance();
		if (pSR->isRecording() && !pSR->isPausing())
		{
			++m_windowRecordElapsedSec;
			int h = m_windowRecordElapsedSec / 3600;
			int m = (m_windowRecordElapsedSec % 3600) / 60;
			int s = m_windowRecordElapsedSec % 60;
			wchar_t buf[16] = { 0 };
			swprintf_s(buf, L"%02d:%02d:%02d", h, m, s);
			m_windowRecordUI.timerStr = buf;

			CRect inv(
				m_windowRecordUI.rect_timeBk.X,
				m_windowRecordUI.rect_timeBk.Y,
				m_windowRecordUI.rect_timeBk.X + m_windowRecordUI.rect_timeBk.Width,
				m_windowRecordUI.rect_timeBk.Y + m_windowRecordUI.rect_timeBk.Height
			);
			InvalidateRect(&inv, FALSE);
		}
	}
	else if (nIDEvent == TIMER_NONEVIP_RECORDTIME)
	{
		ResetNonVipAutoStopTimerState();
		ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
		pSRIns->ReleaseInstance();
		ResetWindowRecordUIState(true);	//超时自动停止后复位 UI
		NotifyRecTop_Stop();			//同步顶栏退出“录制中”状态
		App.m_Dlg_Main.UpdateRecentRecordFile(m_outputfileName);
		App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(m_outputfileName);
		Invalidate();
		this->ShowWindow(SW_RESTORE);
		App.UserNoneVipRecordSuccess();
		//弹框提示用户无法继续录制的原因
		if (!App.m_IsVip && !App.m_IsNonUserPaid)//不是vip
			ModalDlg_MFC::ShowModal_TrialOver(nullptr);
		else if (App.m_IsOverBinds)//设备绑定超限
			ModalDlg_MFC::ShowModal_OverBindsTips();
	}
	CDialogEx::OnTimer(nIDEvent);
}

void Ui_WindowRecordDlg::OnBnClickedBtnMenu()
{
	m_PopupListBox.HideListBox();
	App.OpenFeedBackLink(
		this->GetSafeHwnd(),
		L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
	);
}

void Ui_WindowRecordDlg::OnBnClickedBtnPauseWindowRecord()
{
	auto pSR = ScreenRecorder::GetInstance();
	if (pSR->isRecording() && !pSR->isPausing())
	{
		pSR->PauseRecording();
		PauseNonVipAutoStopTimer();
		m_Btn_WindowRecPause.ShowWindow(SW_HIDE);
		m_Btn_WindowRecResume.ShowWindow(SW_SHOW);
	}
	if (App.m_Dlg_Main.m_Dlg_RecTopDlg &&
		App.m_Dlg_Main.m_Dlg_RecTopDlg->GetRecordContext() == Ui_RecTopDlg::RecContextType::Window)
	{
		App.m_Dlg_Main.m_Dlg_RecTopDlg->SetPauseUIMode();
	}
}

void Ui_WindowRecordDlg::OnBnClickedBtnResumeWindowRecord()
{
	auto pSR = ScreenRecorder::GetInstance();
	if (pSR->isRecording() && pSR->isPausing())
	{
		pSR->ResumeRecording();
		ResumeNonVipAutoStopTimer();
		m_Btn_WindowRecResume.ShowWindow(SW_HIDE);
		m_Btn_WindowRecPause.ShowWindow(SW_SHOW);

		//同步顶栏从“暂停”恢复为正常录制态
		if (App.m_Dlg_Main.m_Dlg_RecTopDlg &&
			App.m_Dlg_Main.m_Dlg_RecTopDlg->GetRecordContext() == Ui_RecTopDlg::RecContextType::Window)
		{
			App.m_Dlg_Main.m_Dlg_RecTopDlg->ResumeFromPauseUiMode();
		}
	}
}

void Ui_WindowRecordDlg::OnBnClickedBtnStopWindowRecord()
{
	if (m_IsRecording)
	{
		OnBnClickedBtnStartrecord(); 
	}
}

void Ui_WindowRecordDlg::OnBnClickedBtnProfileicon()
{
	m_PopupListBox.HideListBox();
}

void Ui_WindowRecordDlg::OnBnClickedBtnUsername()
{
	m_PopupListBox.HideListBox();
}

void Ui_WindowRecordDlg::OnBnClickedBtnOpenvip()
{
	m_PopupListBox.HideListBox();
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
			//更新界面
			App.m_IsVip = true;
			App.m_Dlg_Main.Invalidate(false);
			if (App.m_Dlg_Main.m_Dlg_Gaming)
				App.m_Dlg_Main.m_Dlg_Gaming->Invalidate(false);
			if (App.m_Dlg_Main.m_Dlg_Carmera)
				App.m_Dlg_Main.m_Dlg_Carmera->Invalidate(false);

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

void Ui_WindowRecordDlg::OnBnClickedBtnTipsofrecordmode()
{
	m_PopupListBox.HideListBox();
}

void Ui_WindowRecordDlg::OnBnClickedBtnNoselect()
{
	m_PopupListBox.HideListBox();
}

void Ui_WindowRecordDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialogEx::OnKillFocus(pNewWnd);
}
