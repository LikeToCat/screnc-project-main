// Ui_GamingDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_GamingDlg.h"
#include "Ui_MainDlg.h"
#include "Ui_UserProfileDlg.h"
#include "GlobalFunc.h"
#include "afxdialogex.h"
#include "PngLoader.h"
#include "CDebug.h"
#include "theApp.h"
#include "Ui_RadarTimerSDL.h"
#include "ConfigFileHandler.h"
#include "ModalDialogFunc.h"
#include "WindowHandleManager.h"
#include "GameFocusDetector.h"
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")
extern HANDLE ConsoleHandle;
void test()
{
	//static bool ckReverse = false;//每次点击都会取反这个变量，用于测试不同的分支
	//
	//if (!ckReverse)
	//{
	//	//定义一个居中与屏幕的640x480大小的矩形区域
	//	CRect rgRect;
	//	int scWidth = GetSystemMetrics(SM_CXSCREEN);
	//	int scHeight = GetSystemMetrics(SM_CYSCREEN);
	//	int rgWidth = 640;
	//	int rgHeight = 480;
	//	rgRect.left = (scWidth - rgWidth) / 2;
	//	rgRect.top = (scHeight - rgHeight) / 2;
	//	rgRect.right = rgRect.left + rgWidth;
	//	rgRect.bottom = rgRect.top + rgHeight;
	//
	//	//创建SDL测试窗口
	//	auto tSDLins = LLarSDL::GetInstance();
	//	tSDLins->Run(rgRect);
	//}
	//else
	//{
	//	auto tSDLins = LLarSDL::GetInstance();
	//	tSDLins->ReleaseInstance();
	//}
	//ckReverse = !ckReverse;
}
// Ui_GamingDlg 对话框
void OnFocusChanged(HWND hwnd, bool isGameWindow);	//用户点击窗口焦点改变
IMPLEMENT_DYNAMIC(Ui_GamingDlg, CDialogEx)

Ui_GamingDlg::Ui_GamingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_GAMINGDLG, pParent)
	, m_bDragging(false)
	, m_bDraggingAudio(false)
	, m_nInitialThumbPos(0)
	, m_RecHwnd(nullptr)
{

}

Ui_GamingDlg::~Ui_GamingDlg()
{

}

void Ui_GamingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, GAMINGDLG_BTN_SCREEN, m_Btn_ScreenRecord);
	DDX_Control(pDX, GAMINGDLG_BTN_AREARECORD, m_Btn_AreaRecord);
	DDX_Control(pDX, GAMINGDLG_BTN_GAIMGRECORD, m_Btn_GamingRecord);
	DDX_Control(pDX, GAMINGDLG_BTN_WINDOWRECORD, m_Btn_WindowRecord);
	DDX_Control(pDX, GAMINGDLG_BTN_RECORDSCREEN, m_Btn_RecordGame);
	DDX_Control(pDX, GAMINGDLG_BTN_STARTRECORD, m_Btn_Rec);
	DDX_Control(pDX, GAMINGDLG_BTN_AUDIOICON, m_Btn_SystemAudioIcon);
	DDX_Control(pDX, GAMINGDLG_BTN_MICROICON, m_Btn_MicroAudioIcon);
	DDX_Control(pDX, GAMINGDLG_BTN_ADVANCEOPT, m_Btn_AdvanceOpt);
	DDX_Control(pDX, GAMINGDLG_STAT_SYSTEMAUDIO, m_Stat_SystemAudio);
	DDX_Control(pDX, GAMINGDLG_STAT_MICROAUDIO, m_Stat_MicroAudio);
	DDX_Control(pDX, GAMINGDLG_BTN_MORE, m_Btn_More);
	DDX_Control(pDX, GAMINGDLG_STAT_SYSTEMAUDIOPERCENT, m_Stat_SystemAudioPercent);
	DDX_Control(pDX, GAMINGDLG_STAT_MICROAUDIOPERCENT, m_Stat_MicroAudioPercent);
	DDX_Control(pDX, GAMINGDLG_BTN_APPICON, m_Btn_TitleIcon);
	DDX_Control(pDX, GAMINGDLG_BTN_VIDEOLIST, m_Btn_VideoList);
	DDX_Control(pDX, GAMINGDLG_BTN_USERICON, m_Btn_UserIcon);
	DDX_Control(pDX, GAMINGDLG_BTN_LOGIN, m_Btn_Login);
	DDX_Control(pDX, GAMINGDLG_BTN_OPENVIP, m_Btn_OpenVip);
	DDX_Control(pDX, GAMINGDLG_BTN_CONFIG, m_Btn_Config);
	DDX_Control(pDX, GAMINGDLG_BTN_MENU, m_Btn_FeedBack);
	DDX_Control(pDX, GAMINGDLG_BTN_MINIMAL, m_Btn_Minimal);
	DDX_Control(pDX, GAMINGDLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, GAMINGDLG_STAT_TITLETEXT, m_Stat_TitleText);
	DDX_Control(pDX, GAMINGDLG_BTN_CKAUDIO, m_Btn_CkAudio);
	DDX_Control(pDX, GAMINGDLG_BTN_CKNOAUDIO, m_Btn_CkNoAudio);
	DDX_Control(pDX, GAMINGDLG_BTN_CKMICRO, m_Btn_CkMicro);
	DDX_Control(pDX, GAMINGDLG_BTN_CKNOMICRO, m_Btn_CkNoMIicro);
	DDX_Control(pDX, GAMINGDLG_STAT_SCREENRECORD, m_Stat_ScreenRecord);
	DDX_Control(pDX, GAMINGDLG_STAT_AREARECRD, m_Stat_AreaRecord);
	DDX_Control(pDX, GAMINGDLG_STAT_GAMINGRECORD, m_Stat_GamingRecord);
	DDX_Control(pDX, GAMINGDLG_STAT_WINDOWRECORD, m_Stat_WindowRecord);
	DDX_Control(pDX, GAMMINGDLG_BTN_PHONE, m_Btn_Phone);
	DDX_Control(pDX, GAMINGDLG_BTN_SELECTGAME, m_btn_selectGame);
	DDX_Control(pDX, GAMEINGDLG_STAT_HOTKEYSTARTRECORD, m_stat_hotKeyStartRecord);
}

BEGIN_MESSAGE_MAP(Ui_GamingDlg, CDialogEx)
	ON_WM_NCACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOVE()
	ON_WM_EXITSIZEMOVE()
	ON_WM_ENTERSIZEMOVE()
	ON_WM_TIMER()
	ON_WM_NCLBUTTONDOWN()

	//按钮控件响应
	ON_BN_CLICKED(GAMINGDLG_BTN_CKAUDIO, &Ui_GamingDlg::OnBnClickedBtnCkaudio)
	ON_BN_CLICKED(GAMINGDLG_BTN_CKNOAUDIO, &Ui_GamingDlg::OnBnClickedBtnCknoaudio)
	ON_BN_CLICKED(GAMINGDLG_BTN_MINIMAL, &Ui_GamingDlg::OnBnClickedBtnMinimal)
	ON_BN_CLICKED(GAMINGDLG_BTN_CLOSE, &Ui_GamingDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(GAMINGDLG_BTN_SCREEN, &Ui_GamingDlg::OnBnClickedBtnScreen)
	ON_BN_CLICKED(GAMINGDLG_BTN_AREARECORD, &Ui_GamingDlg::OnBnClickedBtnArearecord)
	ON_BN_CLICKED(GAMINGDLG_BTN_WINDOWRECORD, &Ui_GamingDlg::OnBnClickedBtnWindowrecord)
	ON_BN_CLICKED(GAMINGDLG_BTN_STARTRECORD, &Ui_GamingDlg::OnBnClickedBtnStartrecord)
	ON_BN_CLICKED(GAMINGDLG_BTN_CKMICRO, &Ui_GamingDlg::OnBnClickedBtnCkmicro)
	ON_BN_CLICKED(GAMINGDLG_BTN_CKNOMICRO, &Ui_GamingDlg::OnBnClickedBtnCknomicro)
	ON_BN_CLICKED(GAMINGDLG_BTN_VIDEOLIST, &Ui_GamingDlg::OnBnClickedBtnVideolist)
	ON_BN_CLICKED(GAMINGDLG_BTN_LOGIN, &Ui_GamingDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(GAMMINGDLG_BTN_PHONE, &Ui_GamingDlg::OnBnClickedBtnPhone)
	ON_BN_CLICKED(GAMINGDLG_BTN_OPENVIP, &Ui_GamingDlg::OnBnClickedBtnOpenvip)
	ON_BN_CLICKED(GAMINGDLG_BTN_APPICON, &Ui_GamingDlg::OnBnClickedBtnAppicon)
	ON_BN_CLICKED(GAMINGDLG_BTN_CONFIG, &Ui_GamingDlg::OnBnClickedBtnConfig)
	ON_BN_CLICKED(GAMINGDLG_BTN_MORE, &Ui_GamingDlg::OnBnClickedBtnMore)
	ON_BN_CLICKED(GAMINGDLG_BTN_ADVANCEOPT, &Ui_GamingDlg::OnBnClickedBtnAdvanceopt)
	ON_BN_CLICKED(GAMINGDLG_BTN_RECORDSCREEN, &Ui_GamingDlg::OnBnClickedBtnRecordedGame)
	ON_BN_CLICKED(GAMINGDLG_BTN_GAIMGRECORD, &Ui_GamingDlg::OnBnClickedBtnGaimgrecord)

	//广播
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGININ, &Ui_GamingDlg::On_BroadCast_UserLogin)
	ON_MESSAGE(BROADCASTMSG_USERLOGIN_ISLOGINOUT, &Ui_GamingDlg::On_BroadCast_UserLogOut)

	//SDL窗口的响应
	ON_MESSAGE(MSG_UIPROFILE_SIGNOUT, &Ui_GamingDlg::On_SDLBnClick_UserLogOut)

	//其他MFC窗口消息的响应
	ON_MESSAGE(MSG_USERPRIFLEDLG_WINDOWHIDDENBYTIMTER, &Ui_GamingDlg::On_UserProfileDlg_WindowHidenByTimer)
	ON_BN_CLICKED(GAMINGDLG_BTN_FEEDBACK, &Ui_GamingDlg::OnBnClickedBtnFeedback)
	ON_BN_CLICKED(GAMINGDLG_BTN_SELECTGAME, &Ui_GamingDlg::OnBnClickedBtnSelectgame)

	//下拉框点击响应
	ON_MESSAGE(MSG_CLARLISTBOX_SELECTED, &Ui_GamingDlg::OnBnClickedBtnListBoxSelected)

	ON_MESSAGE(MSG_GAMEREC_RECORDWINDOWMINIMAL, &Ui_GamingDlg::OnRecordWindowMinimal)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// Ui_GamingDlg 消息处理程序

BOOL Ui_GamingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	LoadRes();
	GetUserDPI();
	UpdateScale();
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

	ModifyStyleEx(WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
	SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
	SetWindowTextW(L"极速录屏大师");
	return TRUE;
}

void Ui_GamingDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnCancel();
}

void Ui_GamingDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialogEx::OnOK();
}

BOOL Ui_GamingDlg::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//EnableShadow();
	return CDialogEx::OnNcActivate(bActive);
}

LRESULT Ui_GamingDlg::OnNcHitTest(CPoint point)
{
	// 转换屏幕坐标到客户区坐标
	CPoint clientPoint = point;
	ScreenToClient(&clientPoint);

	// 创建CRect对象用于点位检测
	CRect titleBarRect(m_Rect_TitleBar.X, m_Rect_TitleBar.Y,
		m_Rect_TitleBar.X + m_Rect_TitleBar.Width,
		m_Rect_TitleBar.Y + m_Rect_TitleBar.Height);

	CRect mainAreaRect(m_Rect_MainArea.X, m_Rect_MainArea.Y,
		m_Rect_MainArea.X + m_Rect_MainArea.Width,
		m_Rect_MainArea.Y + m_Rect_MainArea.Height);

	CRect navAreaRect(m_Rect_NavArea.X, m_Rect_NavArea.Y,
		m_Rect_NavArea.X + m_Rect_NavArea.Width,
		m_Rect_NavArea.Y + m_Rect_NavArea.Height);

	// 检查点击位置是否在主交互区域或导航区域
	if (mainAreaRect.PtInRect(clientPoint) || navAreaRect.PtInRect(clientPoint))
	{
		// 在主交互区域或导航区域内，保持正常点击行为
		return HTCLIENT;
	}

	// 检查点击位置是否在标题栏区域
	if (titleBarRect.PtInRect(clientPoint))
	{
		// 在标题栏区域内，允许拖动
		m_listBoxs.HideListBox();
		return HTCAPTION;
	}

	// 对于其他区域，由基类决定
	LRESULT hit = CDialog::OnNcHitTest(point);
	if (hit == HTCLIENT)
	{
		// 对于未明确定义的客户区域，也允许拖动
		return HTCAPTION;
	}

	return hit;
}

void Ui_GamingDlg::OnPaint()
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
	SolidBrush CaptionBrush(Color(26, 27, 32));
	SolidBrush ClientBrush(Color(26, 31, 37));
	SolidBrush NavBrush(Color(26, 27, 32));
	memGraphice.FillRectangle(&NavBrush, m_Rect_NavArea);
	memGraphice.FillRectangle(&CaptionBrush, m_Rect_TitleBar);
	memGraphice.FillRectangle(&ClientBrush, m_Rect_MainArea);

	// ===== 系统音量条绘制 =====

	// 1. 计算滑块位置
	float audioThumbRelativePos = m_Rect_AudioThumb.X - m_Rect_AudioBar.X;
	float audioThumbCenterPos = audioThumbRelativePos + m_Rect_AudioThumb.Width / 2;

	if (audioThumbRelativePos <= 0) {
		// 滑块在最左侧，全部使用空载音量条
		memGraphice.DrawImage(m_Bitmap_Bar, m_Rect_AudioBar);
	}
	else if (audioThumbCenterPos >= m_Rect_AudioBar.Width) {
		// 滑块在最右侧，全部使用满载音量条
		memGraphice.DrawImage(m_Bitmap_FullBar, m_Rect_AudioBar);
	}
	else {
		// 滑块在中间位置，需要分成两部分绘制

		// 左侧部分 - 满载音量条
		RectF leftBarRect(m_Rect_AudioBar.X, m_Rect_AudioBar.Y,
			audioThumbCenterPos, m_Rect_AudioBar.Height);

		// 右侧部分 - 空载音量条
		RectF rightBarRect(m_Rect_AudioBar.X + audioThumbCenterPos, m_Rect_AudioBar.Y,
			m_Rect_AudioBar.Width - audioThumbCenterPos, m_Rect_AudioBar.Height);

		// 计算满载音量条的源矩形 (裁剪部分)
		float fullBarWidth = m_Bitmap_FullBar->GetWidth();
		float fullBarRatio = fullBarWidth / m_Rect_AudioBar.Width;
		float leftBarSourceWidth = audioThumbCenterPos * fullBarRatio;

		RectF leftBarSourceRect(0, 0, leftBarSourceWidth, m_Bitmap_FullBar->GetHeight());

		// 计算空载音量条的源矩形 (裁剪部分)
		float emptyBarWidth = m_Bitmap_Bar->GetWidth();
		float emptyBarRatio = emptyBarWidth / m_Rect_AudioBar.Width;
		float rightBarSourceX = audioThumbCenterPos * emptyBarRatio;
		float rightBarSourceWidth = emptyBarWidth - rightBarSourceX;

		RectF rightBarSourceRect(rightBarSourceX, 0, rightBarSourceWidth, m_Bitmap_Bar->GetHeight());

		// 绘制满载音量条(左侧)
		memGraphice.DrawImage(m_Bitmap_FullBar, leftBarRect,
			leftBarSourceRect.X, leftBarSourceRect.Y,
			leftBarSourceRect.Width, leftBarSourceRect.Height, UnitPixel);

		// 绘制空载音量条(右侧)
		memGraphice.DrawImage(m_Bitmap_Bar, rightBarRect,
			rightBarSourceRect.X, rightBarSourceRect.Y,
			rightBarSourceRect.Width, rightBarSourceRect.Height, UnitPixel);
	}

	// ===== 麦克风音量条绘制 =====

	// 同样的逻辑应用于麦克风音量条
	float microThumbRelativePos = m_Rect_MicroThumb.X - m_Rect_MicroBar.X;
	float microThumbCenterPos = microThumbRelativePos + m_Rect_MicroThumb.Width / 2;

	if (microThumbRelativePos <= 0) {
		// 滑块在最左侧，全部使用空载音量条
		memGraphice.DrawImage(m_Bitmap_Bar, m_Rect_MicroBar);
	}
	else if (microThumbCenterPos >= m_Rect_MicroBar.Width) {
		// 滑块在最右侧，全部使用满载音量条
		memGraphice.DrawImage(m_Bitmap_FullBar, m_Rect_MicroBar);
	}
	else {
		// 滑块在中间位置，需要分成两部分绘制

		// 左侧部分 - 满载音量条
		RectF leftBarRect(m_Rect_MicroBar.X, m_Rect_MicroBar.Y,
			microThumbCenterPos, m_Rect_MicroBar.Height);

		// 右侧部分 - 空载音量条
		RectF rightBarRect(m_Rect_MicroBar.X + microThumbCenterPos, m_Rect_MicroBar.Y,
			m_Rect_MicroBar.Width - microThumbCenterPos, m_Rect_MicroBar.Height);

		// 计算满载音量条的源矩形 (裁剪部分)
		float fullBarWidth = m_Bitmap_FullBar->GetWidth();
		float fullBarRatio = fullBarWidth / m_Rect_MicroBar.Width;
		float leftBarSourceWidth = microThumbCenterPos * fullBarRatio;

		RectF leftBarSourceRect(0, 0, leftBarSourceWidth, m_Bitmap_FullBar->GetHeight());

		// 计算空载音量条的源矩形 (裁剪部分)
		float emptyBarWidth = m_Bitmap_Bar->GetWidth();
		float emptyBarRatio = emptyBarWidth / m_Rect_MicroBar.Width;
		float rightBarSourceX = microThumbCenterPos * emptyBarRatio;
		float rightBarSourceWidth = emptyBarWidth - rightBarSourceX;

		RectF rightBarSourceRect(rightBarSourceX, 0, rightBarSourceWidth, m_Bitmap_Bar->GetHeight());

		// 绘制满载音量条(左侧)
		memGraphice.DrawImage(m_Bitmap_FullBar, leftBarRect,
			leftBarSourceRect.X, leftBarSourceRect.Y,
			leftBarSourceRect.Width, leftBarSourceRect.Height, UnitPixel);

		// 绘制空载音量条(右侧)
		memGraphice.DrawImage(m_Bitmap_Bar, rightBarRect,
			rightBarSourceRect.X, rightBarSourceRect.Y,
			rightBarSourceRect.Width, rightBarSourceRect.Height, UnitPixel);
	}

	// 绘制滑块
	m_Rect_AudioThumb.Y = m_Rect_AudioBar.Y + (m_Rect_AudioBar.Height - m_Rect_AudioThumb.Height) / 2;
	m_Rect_MicroThumb.Y = m_Rect_MicroBar.Y + (m_Rect_MicroBar.Height - m_Rect_MicroThumb.Height) / 2;

	//绘画VIPlogo
	if ((App.m_IsVip && App.m_isLoginIn) || (App.m_IsNonUserPaid))
	{
		memGraphice.DrawImage(m_Bitmap_VipLogo, m_Rect_Viplogo);
	}

	memGraphice.DrawImage(m_Bitmap_Thumb, m_Rect_AudioThumb);
	memGraphice.DrawImage(m_Bitmap_Thumb, m_Rect_MicroThumb);

	// 一次性绘画到窗口上
	Gdiplus::Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&membitmap, 0, 0, WindowWidth, WindowHeight);
	
	DB(ConsoleHandle, L"Ui_GamingDlg:repaint..");
}

HBRUSH Ui_GamingDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

LRESULT Ui_GamingDlg::On_UserProfileDlg_WindowHidenByTimer(WPARAM wParam, LPARAM lParam)
{
	HideUserProfile();
	return LRESULT();
}

void Ui_GamingDlg::Ui_SetWindowRect(const CRect& rect)
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

void Ui_GamingDlg::Ui_UpdateWindowPos(const CRect& rect)
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

void Ui_GamingDlg::CleanUpGdiPngRes()
{
	m_Btn_ScreenRecord.ClearImages();// 全屏录制
	m_Btn_AreaRecord.ClearImages();// 区域录制
	m_Btn_GamingRecord.ClearImages();// 游戏录制
	m_Btn_WindowRecord.ClearImages();	// 应用窗口录制
	m_Btn_RecordGame.ClearImages();// 录全屏
	m_Btn_Rec.ClearImages();// Rec
	m_Btn_SystemAudioIcon.ClearImages();	// 扬声器图标
	m_Btn_MicroAudioIcon.ClearImages();// 麦克风图标
	m_Btn_More.ClearImages();// 更多
	m_Btn_TitleIcon.ClearImages();//应用程序图标
	m_Btn_VideoList.ClearImages();	// 视频列表
	m_Btn_UserIcon.ClearImages();	// 图标
	m_Btn_OpenVip.ClearImages();	// 开通会员
	m_Btn_Minimal.ClearImages();	// 最小化
	m_Btn_Close.ClearImages();		// 关闭
	m_Btn_CkAudio.ClearImages();// 录制系统声音
	m_Btn_CkNoAudio.ClearImages();// 不录制系统声音
	m_Btn_CkMicro.ClearImages();// 录制麦克风声音
	m_Btn_CkNoMIicro.ClearImages();// 不录制麦克风声音
}

void Ui_GamingDlg::ShowLoginUi()
{
	if (App.m_userInfo.nickname != L"")
	{
		m_Btn_Login.ShowWindow(SW_HIDE);
		m_Btn_Phone.SetWindowTextW(App.m_userInfo.nickname);
		m_Btn_Phone.ShowWindow(SW_SHOW);
	}
	if (!App.m_IsVip || !App.m_isLoginIn)
	{
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	else
	{
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	}
	Invalidate(false);
}

void Ui_GamingDlg::ShowNoneUserPayUi()
{
	if (App.m_IsNonUserPaid)
	{
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	}
	else
	{
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	Invalidate(false);
}

void Ui_GamingDlg::GetUserDPI()
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

void Ui_GamingDlg::UpdateScale()
{
	// 设定默认窗口大小为 789x474 像素
	int windowWidth = 789 * m_Scale;
	int windowHeight = 474 * m_Scale;

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
	m_Rect_TitleBar.Height = windowHeight * 0.106;

	//设置导航栏区域
	m_Rect_NavArea.X = 0;
	m_Rect_NavArea.Y = m_Rect_TitleBar.Height;
	m_Rect_NavArea.Width = 0.155 * windowWidth;
	m_Rect_NavArea.Height = windowHeight - m_Rect_TitleBar.Height;

	//设置主互动区域
	m_Rect_MainArea.X = m_Rect_NavArea.Width;
	m_Rect_MainArea.Y = m_Rect_TitleBar.Height;
	m_Rect_MainArea.Width = windowWidth - m_Rect_NavArea.Width;
	m_Rect_MainArea.Height = windowHeight - m_Rect_TitleBar.Height;

	UpdateTitleBarCtrl();//调整标题栏中的控件
	UpdateNavAreaCtrl();//调整导航栏中的控件
	UpdateMainAreaCtrl();//调整主互动区域中的控件
}

void Ui_GamingDlg::UpdateTitleBarCtrl()
{
	//极速录屏大师图标
	float TitleIconWidth = 35 * m_Scale;
	float TitleIconHeight = 30 * m_Scale;
	float TitleIconX = 15 * m_Scale;
	float TitleIconY = (m_Rect_TitleBar.Height - TitleIconHeight) / 2;
	m_Btn_TitleIcon.MoveWindow(TitleIconX, TitleIconY, TitleIconWidth, TitleIconHeight);

	//极速录屏大师文本
	float TitleTextWidth = 140 * m_Scale;
	float TitleTextHeight = 25 * m_Scale;
	float TitleTextX = TitleIconX + TitleIconWidth - 5 * m_Scale;
	float TitleTextY = TitleIconY + (TitleIconHeight - TitleTextHeight) / 2 - 2 * m_Scale;
	m_Stat_TitleText.MoveWindow(TitleTextX, TitleTextY, TitleTextWidth, TitleTextHeight);

	//视频列表
	float VideoListWidth = 90 * m_Scale;
	float VideoListHeight = 20 * m_Scale;
	float VideoListX = m_Rect_NavArea.GetRight();
	float VideoListY = TitleTextY + (TitleTextHeight - VideoListHeight) / 2;
	m_Btn_VideoList.MoveWindow(VideoListX, VideoListY, VideoListWidth, VideoListHeight);

	//用户图标
	float UserIconWidth = 16 * m_Scale;
	float UserIconHeight = 16 * m_Scale;
	float UserIconX = 0.40 * m_Rect_TitleBar.Width + VideoListWidth + 10 * m_Scale;
	float UserIconY = (m_Rect_TitleBar.Height - UserIconHeight) / 2 + 1 * m_Scale;
	m_Btn_UserIcon.MoveWindow(UserIconX, UserIconY, UserIconWidth, UserIconHeight);

	//viplogo
	m_Rect_Viplogo.Width = 22 * m_Scale;
	m_Rect_Viplogo.Height = 22 * m_Scale;
	m_Rect_Viplogo.X = UserIconX - m_Rect_Viplogo.Width - 5 * m_Scale;
	m_Rect_Viplogo.Y = UserIconY + (UserIconHeight - m_Rect_Viplogo.Height) / 2;

	//登录按钮
	float LoginBtnWidth = 38 * m_Scale;
	float LoginBtnHeight = 17 * m_Scale;
	float LoginBtnX = UserIconX + UserIconWidth + 5 * m_Scale;
	float LoginBtnY = UserIconY + (UserIconHeight - LoginBtnHeight) / 2 + 1 * m_Scale;
	m_Btn_Login.MoveWindow(LoginBtnX, LoginBtnY, LoginBtnWidth, LoginBtnHeight);

	//手机号
	float PhoneBtnWidth = 100 * m_Scale;
	float PhoneBtnHeight = 20 * m_Scale;
	float PhoneBtnX = UserIconX + 19 * m_Scale;
	float PhoneBtnY = UserIconY + (UserIconHeight - PhoneBtnHeight) / 2 + 1 * m_Scale;
	m_Btn_Phone.MoveWindow(PhoneBtnX, PhoneBtnY, PhoneBtnWidth, PhoneBtnHeight);

	//开通会员
	float OpenVipWidth = 94.3 * m_Scale;
	float OpenVipHeight = 48.3 * m_Scale;
	float OpenVipX = 0.682 * m_Rect_TitleBar.Width;
	float OpenVipY = (m_Rect_TitleBar.Height - OpenVipHeight) / 2 ;
	m_Btn_OpenVip.MoveWindow(OpenVipX, OpenVipY - 2 * m_Scale, OpenVipWidth, OpenVipHeight);

	//设置
	float ConfigBtnWidth = LoginBtnWidth;
	float ConfigBtnHeight = LoginBtnHeight;
	float ConfigBtnX = OpenVipX + OpenVipWidth + 4 * m_Scale;
	float ConfigBtnY = OpenVipY + (OpenVipHeight - ConfigBtnHeight) / 2 + 2 * m_Scale;
	m_Btn_Config.MoveWindow(ConfigBtnX, ConfigBtnY, ConfigBtnWidth, ConfigBtnHeight);

	//反馈
	float FeedBackWidth = ConfigBtnWidth;
	float FeedBackHeight = ConfigBtnHeight;
	float FeedBackX = ConfigBtnX + ConfigBtnWidth + 9 * m_Scale;
	float FeedBackY = ConfigBtnY;
	m_Btn_FeedBack.MoveWindow(FeedBackX, FeedBackY, FeedBackWidth, FeedBackHeight);

	//关闭
	float CloseBtnWidth = 24 * m_Scale;
	float CloseBtnHeight = 24 * m_Scale;
	float CloseBtnX = m_Rect_TitleBar.X + m_Rect_TitleBar.Width - 10 * m_Scale - CloseBtnWidth;
	float CloseBtnY = FeedBackY + (FeedBackHeight - CloseBtnHeight) / 2 - 1 * m_Scale;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	//最小化
	float MinimalBtnWidth = 24 * m_Scale;
	float MinimalBtnHeight = 24 * m_Scale;
	float MinimalX = CloseBtnX - MinimalBtnWidth - 3 * m_Scale;
	float MinimalY = CloseBtnY;
	m_Btn_Minimal.MoveWindow(MinimalX, MinimalY, MinimalBtnWidth, MinimalBtnHeight);
}

void Ui_GamingDlg::UpdateNavAreaCtrl()
{
	//定义图标大小
	float PngWidth = 69 * m_Scale;
	float PngHeight = 62 * m_Scale;
	float ScreenRecordX = (m_Rect_NavArea.Width - PngWidth) / 2;
	float ScreenRecordY = (m_Rect_NavArea.Height / 4 - PngHeight) / 2 + PngHeight / 2;

	//定义文字大小
	float TextWidth = 113 * m_Scale;
	float TextHeight = 22 * m_Scale;
	float TextX = ScreenRecordX + (PngWidth - TextWidth) / 2;

	//全屏录制按钮
	m_Btn_ScreenRecord.MoveWindow(ScreenRecordX, ScreenRecordY, PngWidth, PngHeight);
	m_Stat_ScreenRecord.MoveWindow(TextX, ScreenRecordY + PngHeight + 5 * m_Scale, TextWidth, TextHeight);

	//选区录制按钮
	float AreaRecordX = ScreenRecordX;
	float AreaRecordY = ScreenRecordY + PngHeight + (m_Rect_NavArea.Height / 4 - PngHeight) / 2 + 15 * m_Scale;
	m_Btn_AreaRecord.MoveWindow(AreaRecordX, AreaRecordY, PngWidth, PngHeight);
	m_Stat_AreaRecord.MoveWindow(TextX, AreaRecordY + PngHeight + 5 * m_Scale, TextWidth, TextHeight);

	//游戏录制按钮
	float GamingX = ScreenRecordX;
	float GamingY = AreaRecordY + PngHeight + (m_Rect_NavArea.Height / 4 - PngHeight) / 2 + 15 * m_Scale;
	m_Btn_GamingRecord.MoveWindow(GamingX, GamingY, PngWidth, PngHeight);
	m_Stat_GamingRecord.MoveWindow(TextX, GamingY + PngHeight + 5 * m_Scale, TextWidth, TextHeight);

	//应用窗口录制
	float WindowRecordX = GamingX;
	float WindowRecordY = GamingY + PngHeight + (m_Rect_NavArea.Height / 4 - PngHeight) / 2 + 15 * m_Scale;
	m_Btn_WindowRecord.MoveWindow(WindowRecordX, WindowRecordY, PngWidth, PngHeight);
	m_Stat_WindowRecord.MoveWindow(TextX, WindowRecordY + PngHeight + 5 * m_Scale, TextWidth, TextHeight);
}

void Ui_GamingDlg::UpdateMainAreaCtrl()
{
	//定义滑块大小
	m_Rect_AudioThumb.Width = 12 * m_Scale;
	m_Rect_AudioThumb.Height = 20 * m_Scale;
	m_Rect_MicroThumb.Width = m_Rect_AudioThumb.Width;
	m_Rect_MicroThumb.Height = m_Rect_AudioThumb.Height;

	//录制全屏
	float RecordScreenWidth = 38 * m_Scale;
	float RecordScreenHeight = 66 * m_Scale;
	float RecordScreenX = m_Rect_MainArea.X + m_Rect_MainArea.Width * 0.155;
	float RecordScreenY = m_Rect_MainArea.Y + 20 * m_Scale;
	m_Btn_RecordGame.MoveWindow(RecordScreenX, RecordScreenY, RecordScreenWidth, RecordScreenHeight);

	//选择游戏
	float SeGameW = 120 * m_Scale;
	float SeGameH = 25 * m_Scale;
	float SeGameX = RecordScreenX + (RecordScreenWidth - SeGameW)/2;
	float SeGameY = RecordScreenY + RecordScreenHeight;
	m_btn_selectGame.MoveWindow(SeGameX, SeGameY, SeGameW, SeGameH);

	//Rec
	float RecWidth = 100 * m_Scale;
	float RecHeight = 100 * m_Scale;
	float RecX = m_Rect_MainArea.X + m_Rect_MainArea.Width / 2 + m_Rect_MainArea.Width / 4 - RecWidth;
	float RecY = m_Rect_MainArea.Y + 23 * m_Scale;
	m_Btn_Rec.MoveWindow(RecX, RecY, RecWidth, RecHeight);

	//快捷键文本提示（Alt + B）
	float HotKeySRW = RecWidth + 100 * m_Scale;
	float HotKeySRH = 20 * m_Scale;
	float HotKeySRX = RecX + (RecWidth - HotKeySRW) / 2;
	float HotKeySRY = RecY + RecHeight + 5 * m_Scale;
	m_stat_hotKeyStartRecord.MoveWindow(HotKeySRX, HotKeySRY, HotKeySRW, HotKeySRH);

	//系统声音文本m_Rect_MainArea.Y + m_Rect_MainArea.Height * 216 / m_Rect_MainArea.Height
	float SystemAudioTextX = m_Rect_MainArea.X + m_Rect_MainArea.Width * 26 / m_Rect_MainArea.Width;
	float StstemAudioTextY = 0.404 * m_WindowHeight;
	float SystemAudioTextWidth = 68 * m_Scale;
	float SystemAudioTextHeight = 20 * m_Scale;
	m_Stat_SystemAudio.MoveWindow(SystemAudioTextX, StstemAudioTextY, SystemAudioTextWidth, SystemAudioTextHeight);

	//录制系统声音
	float RecordSystemAudioBtnWidth = 101.2 * m_Scale;
	float RecordSystemAudioBtnHeight = 24.15 * m_Scale;
	float RecordSystemAudioBtnX = SystemAudioTextX + 78 * m_Scale;
	float RecordSystemAudioBtnY = StstemAudioTextY + (SystemAudioTextHeight - RecordSystemAudioBtnHeight) / 2;
	m_Btn_CkAudio.MoveWindow(RecordSystemAudioBtnX, RecordSystemAudioBtnY, RecordSystemAudioBtnWidth, RecordSystemAudioBtnHeight);

	//不录制系统声音
	float NoCkAudioBtnWidth = 112.7 * m_Scale;
	float NoCkAudioBtnHeight = 24.15 * m_Scale;
	float NoCkAudioBtnX = RecordSystemAudioBtnX + RecordSystemAudioBtnWidth + 5 * m_Scale;
	float NoCkAudioBtnY = RecordSystemAudioBtnY;
	m_Btn_CkNoAudio.MoveWindow(NoCkAudioBtnX, NoCkAudioBtnY, NoCkAudioBtnWidth, NoCkAudioBtnHeight);

	//高级选项
	float AdvanceOptX = 0.878 * m_WindowWidth;
	float AdvanceOptY = NoCkAudioBtnY;
	float AdvanceOptHeight = NoCkAudioBtnHeight * 1.35f;
	float AdvanceOptWidth = 66 * m_Scale * 1.35f;
	m_Btn_AdvanceOpt.MoveWindow(AdvanceOptX, AdvanceOptY, AdvanceOptWidth, AdvanceOptHeight);

	//系统声音图标 
	float AudioIconWidth = 64 * m_Scale;
	float AudioIconHeight = 60 * m_Scale;
	float AudioIconX = SystemAudioTextX + SystemAudioTextWidth - AudioIconWidth;
	float AudioIconY = StstemAudioTextY + SystemAudioTextHeight + 15 * m_Scale;
	m_Btn_SystemAudioIcon.MoveWindow(AudioIconX, AudioIconY, AudioIconWidth, AudioIconHeight);

	//音量条
	m_Rect_AudioBar.Width = 499 * m_Scale;
	m_Rect_AudioBar.Height = 9;
	m_Rect_AudioBar.X = AudioIconX + AudioIconWidth + 5 * m_Scale;
	m_Rect_AudioBar.Y = AudioIconY + (AudioIconHeight - m_Rect_AudioBar.Height) / 2;

	//音量条数字
	float AudioPercentWidth = 54 * m_Scale;
	float AudioPercentHeight = 22 * m_Scale;
	float AudioPercentX = m_Rect_AudioBar.X + m_Rect_AudioBar.Width + 10 * m_Scale;
	float AudioPercentY = m_Rect_AudioBar.Y + (m_Rect_AudioBar.Height - AudioPercentHeight) / 2;
	m_Stat_SystemAudioPercent.MoveWindow(AudioPercentX, AudioPercentY, AudioPercentWidth, AudioPercentHeight);

	//麦克风声音文本
	float MicroTextWidth = SystemAudioTextWidth + 25 * m_Scale;
	float MicroTextHeight = SystemAudioTextHeight;
	float MicroTextX = m_Rect_MainArea.X + m_Rect_MainArea.Width * 26 / m_Rect_MainArea.Width - 14 * m_Scale;
	float MicroTextY = 0.692 * m_WindowHeight;
	m_Stat_MicroAudio.MoveWindow(MicroTextX, MicroTextY, MicroTextWidth, MicroTextHeight);

	//录制麦克风
	float MicroCkWidth = 89.7 * m_Scale;
	float MicroCkHeight = 24.15 * m_Scale;
	float MicroCkX = RecordSystemAudioBtnX;
	float MicroCkY = MicroTextY + (MicroTextHeight - MicroCkHeight) / 2 + 2 * m_Scale;
	m_Btn_CkMicro.MoveWindow(MicroCkX, MicroCkY, MicroCkWidth, MicroCkHeight);

	//不录制麦克风
	float NoMicroCkWidth = 101.2 * m_Scale;
	float NoMicroCkHeight = 24.15 * m_Scale;
	float NoMicroCkX = MicroCkX + MicroCkWidth + 15 * m_Scale;
	float NoMicroCkY = MicroCkY;
	m_Btn_CkNoMIicro.MoveWindow(NoMicroCkX, NoMicroCkY, NoMicroCkWidth, NoMicroCkHeight);

	//麦克风图标
	float MicroIconWidth = 59 * m_Scale;
	float MicroIconHeight = 58 * m_Scale;
	float MicroIconX = AudioIconX + 3 * m_Scale;
	float MicroIconY = MicroTextY + MicroTextHeight + 15 * m_Scale;
	m_Btn_MicroAudioIcon.MoveWindow(MicroIconX, MicroIconY, MicroIconWidth, MicroIconHeight);

	//麦克风音量条
	m_Rect_MicroBar.Width = 499 * m_Scale;
	m_Rect_MicroBar.Height = 9;
	m_Rect_MicroBar.X = MicroIconX + MicroIconWidth + 5 * m_Scale;
	m_Rect_MicroBar.Y = MicroIconY + (MicroIconHeight - m_Rect_MicroBar.Height) / 2;

	//麦克风音量数字
	float MicroPercentWidth = AudioPercentWidth;
	float MicroPercentHeight = AudioPercentHeight;
	float MicroPercentX = m_Rect_MicroBar.X + m_Rect_MicroBar.Width + 10 * m_Scale;
	float MicroPercentY = m_Rect_MicroBar.Y + (m_Rect_MicroBar.Height - MicroPercentHeight) / 2;
	m_Stat_MicroAudioPercent.MoveWindow(MicroPercentX, MicroPercentY, MicroPercentWidth, MicroPercentHeight);

	//更多
	float MoreBtnWidth = 25 * m_Scale;
	float MoreBtnHeight = 72 * m_Scale;
	float MoreBtnX = m_Rect_MainArea.X + m_Rect_MainArea.Width - MoreBtnWidth - 5 * m_Scale;
	float MoreBtnY = m_Rect_MainArea.Y + m_Rect_MainArea.Height * 50 / m_Rect_MainArea.Height;
	m_Btn_More.MoveWindow(MoreBtnX, MoreBtnY, MoreBtnWidth, MoreBtnHeight);
}

void Ui_GamingDlg::InitTitleBarCtrl()
{
	//应用程序图标
	m_Btn_TitleIcon.LoadPNG(WWDLG_PNG_APPICON);
	m_Btn_TitleIcon.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_TitleIcon.SetUseHandCursor(FALSE);

	//极速录屏大师文本
	m_Stat_TitleText.LarSetTextSize(22);
	m_Stat_TitleText.LarSetTextStyle(false, false, false);

	m_stat_hotKeyStartRecord.LarSetTextSize(20);
	m_stat_hotKeyStartRecord.LarSetTextColor(RGB(255, 255, 255));
	m_stat_hotKeyStartRecord.LarSetTextCenter();

	//视频列表
	m_Btn_VideoList.LoadPNG(MAINDLG_BTN_VIDEOLIST);
	m_Btn_VideoList.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_VideoList.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_VideoList.SetStretchMode(0.75f);
	m_Btn_VideoList.ShowWindow(SW_HIDE);

	//用户头像
	m_Btn_UserIcon.LoadPNG(MAINDLG_PNG_PROFILEICON);
	m_Btn_UserIcon.SetBackgroundColor(RGB(26, 27, 32));

	//登录按钮
	SolidBrush BkBrush(Color(26, 27, 32));
	m_Btn_Login.LarSetTextSize(20);
	m_Btn_Login.LarSetNormalFiilBrush(BkBrush);
	m_Btn_Login.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Login.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_Login.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Login.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_Login.LarSetEraseBkEnable(false);
	m_Btn_Login.LarSetTextStyle(false, false, false);

	//手机号
	m_Btn_Phone.LarSetTextSize(17);
	m_Btn_Phone.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Phone.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_Phone.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Phone.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_Phone.LarSetEraseBkEnable(false);
	m_Btn_Phone.LarSetTextCenter(false);
	m_Btn_Phone.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 27, 32)));
	m_Btn_Phone.ShowWindow(SW_HIDE);
	m_Btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);

	//开通会员
	if (App.m_IsVip && App.m_isLoginIn)
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	else
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_OPENVIP);
	m_Btn_OpenVip.SetBackgroundColor(RGB(26, 27, 32));

	//设置
	m_Btn_Config.LarSetTextSize(20);
	m_Btn_Config.LarSetNormalFiilBrush(BkBrush);
	m_Btn_Config.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_Config.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_Config.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_Config.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_Config.LarSetEraseBkEnable(false);

	//菜单
	m_Btn_FeedBack.LarSetTextSize(20);
	m_Btn_FeedBack.LarSetNormalFiilBrush(BkBrush);
	m_Btn_FeedBack.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_FeedBack.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_FeedBack.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_FeedBack.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_FeedBack.LarSetEraseBkEnable(false);
	m_Btn_FeedBack.ShowWindow(SW_SHOW);

	//最小化 
	m_Btn_Minimal.LoadPNG(MAINDLG_PNG_MINIMAL);
	m_Btn_Minimal.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Minimal.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Minimal.SetStretchMode(0.85f);

	//关闭
	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Close.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Close.SetStretchMode(0.75f);
}

void Ui_GamingDlg::InitNavAreaCtrl()
{
	//全屏录制
	m_Btn_ScreenRecord.LoadPNG(MAINDLG_PNG_SCREENRECORD);
	m_Btn_ScreenRecord.LoadClickPNG(MAINDLG_PNG_SCREENRECORD_CLICK);
	m_Btn_ScreenRecord.SetBackgroundColor(RGB(26, 27, 32));
	m_Stat_ScreenRecord.LarSetTextSize(20);
	m_Stat_ScreenRecord.LarSetTextCenter();
	m_Btn_ScreenRecord.SetUseHoverImage(TRUE);

	//选区录制
	m_Btn_AreaRecord.LoadPNG(MAINDLG_PNG_AREARECORD);
	m_Btn_AreaRecord.LoadClickPNG(MAINDLG_PNG_AREARECORD_CLICK);
	m_Btn_AreaRecord.SetBackgroundColor(RGB(26, 27, 32));
	m_Stat_AreaRecord.LarSetTextSize(20);
	m_Stat_AreaRecord.LarSetTextCenter();
	m_Btn_AreaRecord.SetUseHoverImage(TRUE);

	//游戏录制
	m_Btn_GamingRecord.LoadPNG(MAINDLG_PNG_GAMINGRECORD_CLICK);
	m_Btn_GamingRecord.SetBackgroundColor(RGB(26, 27, 32));
	m_Stat_GamingRecord.LarSetTextSize(20);
	m_Stat_GamingRecord.LarSetTextColor(RGB(0, 198, 123));
	m_Stat_GamingRecord.LarSetTextCenter();
	m_Btn_GamingRecord.SetUseHoverImage(TRUE);

	//应用窗口录制
	m_Btn_WindowRecord.LoadPNG(MAINDLG_PNG_WINDOWRECORD);
	m_Btn_WindowRecord.LoadClickPNG(MAINDLG_PNG_WINDOWRECORD_CLICK);
	m_Btn_WindowRecord.SetBackgroundColor(RGB(26, 27, 32));
	m_Stat_WindowRecord.LarSetTextSize(20);
	m_Stat_WindowRecord.LarSetTextCenter();
	m_Btn_WindowRecord.SetUseHoverImage(TRUE);
}

void Ui_GamingDlg::InitMainAreaCtrl()
{
	//游戏手柄
	m_Btn_RecordGame.LoadPNG(GAMINGDLG_PNG_GAMEHANDLER);
	m_Btn_RecordGame.LoadClickPNG(GAMINGDLG_PNG_GAMEHANDLER);
	m_Btn_RecordGame.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_RecordGame.SetUseHandCursor(true);

	//选择游戏
	m_btn_selectGame.LarSetTextSize(20);
	m_btn_selectGame.LarSetNormalFiilBrush(SolidBrush(Color(26, 31, 37)));
	m_btn_selectGame.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_btn_selectGame.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_btn_selectGame.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_btn_selectGame.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_btn_selectGame.LarSetEraseBkEnable(false);
	m_btn_selectGame.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 
		12 * m_Scale, 10 * m_Scale);
	m_btn_selectGame.LarSetTextMaxWidth((120 - 12) * m_Scale);
	m_btn_selectGame.ShowWindow(SW_SHOW);

	//Rec
	m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_Rec.SetBackgroundColor(RGB(26, 31, 37));

	//系统声音
	m_Stat_SystemAudio.LarSetTextSize(20);
	m_Stat_SystemAudio.LarSetTextStyle(false, false, false);

	//系统声音图标
	m_Btn_SystemAudioIcon.LoadPNG(CHILDDLG_PNG_SYSTEMAUDIO);
	m_Btn_SystemAudioIcon.LoadClickPNG(CHILDDLG_PNG_SYSTEMAUDIO);
	m_Btn_SystemAudioIcon.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_SystemAudioIcon.SetClickEffectColor(255, 26, 31, 37);
	m_Btn_SystemAudioIcon.SetUseHandCursor(false);

	//录制系统声音
	m_Btn_CkAudio.LoadPNG(GAMINGDLG_PNG_CKAUDIO_HOLD);
	m_Btn_CkAudio.LoadClickPNG(GAMINGDLG_PNG_CKAUDIO_HOLD);
	m_Btn_CkAudio.SetBackgroundColor(RGB(26, 31, 37));

	//不录制系统声音
	m_Btn_CkNoAudio.LoadPNG(GAMINGDLG_PNG_NOCKAUDIO);
	m_Btn_CkNoAudio.LoadClickPNG(GAMINGDLG_PNG_NOCKAUDIO);
	m_Btn_CkNoAudio.SetBackgroundColor(RGB(26, 31, 37));

	//系统声音文本
	m_Stat_SystemAudioPercent.LarSetTextSize(22);
	m_Stat_SystemAudioPercent.LarSetTextStyle(false, false, false);
	m_Stat_SystemAudioPercent.LarSetEraseColor(RGB(26, 31, 37));
	m_Stat_SystemAudioPercent.LarSetIsEraseBk(true);

	//麦克风声音
	m_Stat_MicroAudio.LarSetTextSize(20);
	m_Stat_MicroAudio.LarSetTextStyle(false, false, false);

	//麦克风图标
	m_Btn_MicroAudioIcon.LoadPNG(GAMINGDLG_PNG_MICROUNABLE);
	m_Btn_MicroAudioIcon.LoadClickPNG(GAMINGDLG_PNG_MICROUNABLE);
	m_Btn_MicroAudioIcon.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_MicroAudioIcon.SetClickEffectColor(255, 26, 31, 37);
	m_Btn_MicroAudioIcon.SetUseHandCursor(false);

	//录制麦克风
	m_Btn_CkMicro.LoadPNG(GAMINGDLG_PNG_CKMICRO_HOLD);
	m_Btn_CkMicro.LoadClickPNG(GAMINGDLG_PNG_CKMICRO_HOLD);
	m_Btn_CkMicro.SetBackgroundColor(RGB(26, 31, 37));

	//不录制麦克风
	m_Btn_CkNoMIicro.LoadPNG(GAMINGDLG_PNG_NOCKMICRO);
	m_Btn_CkNoMIicro.LoadClickPNG(GAMINGDLG_PNG_NOCKMICRO);
	m_Btn_CkNoMIicro.SetBackgroundColor(RGB(26, 31, 37));

	//麦克风声音文本
	m_Stat_MicroAudioPercent.LarSetTextSize(22);
	m_Stat_MicroAudioPercent.LarSetTextStyle(false, false, false);
	m_Stat_MicroAudioPercent.LarSetEraseColor(RGB(26, 31, 37));
	m_Stat_MicroAudioPercent.LarSetIsEraseBk(true);

	//更多
	//m_Btn_More.LoadPNG(MAINDLG_PNG_MORE);
	//m_Btn_More.LoadClickPNG(MAINDLG_PNG_MORE_CLICK);
	//m_Btn_More.SetBackgroundColor(RGB(26, 31, 37));
	m_Btn_More.ShowWindow(SW_HIDE);

	//高级选项
	m_Btn_AdvanceOpt.LarSetTextSize(20);
	m_Btn_AdvanceOpt.LarSetNormalFiilBrush(SolidBrush(Color(26, 31, 37)));
	m_Btn_AdvanceOpt.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_Btn_AdvanceOpt.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_Btn_AdvanceOpt.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_Btn_AdvanceOpt.LarSetBorderColor(Gdiplus::Color(255, 26, 27, 32));
	m_Btn_AdvanceOpt.LarSetEraseBkEnable(false);//GAMINGDLG_PNG_RIGHTARROW
	m_Btn_AdvanceOpt.LarSetBtnNailImage(
		GAMINGDLG_PNG_CONFIGADVANCEOPT, CLarBtn::NailImageLayout::Left,
		15 * m_Scale, 15 * m_Scale
	);
	m_Btn_AdvanceOpt.LarAdjustTextDisplayPos(3 * m_Scale, 0);
	m_Btn_AdvanceOpt.ShowWindow(SW_SHOW);

	// 设置初始滑块位置
	m_Rect_AudioThumb.X = m_Rect_AudioBar.X + m_Rect_AudioBar.Width - m_Rect_AudioThumb.Width;
	m_Rect_MicroThumb.X = m_Rect_AudioBar.X + m_Rect_MicroBar.Width - m_Rect_MicroThumb.Width;

	// 设置初始百分比显示
	m_Stat_SystemAudioPercent.SetWindowText(_T("100%"));
	m_Stat_MicroAudioPercent.SetWindowText(_T("100%"));
}

void Ui_GamingDlg::InitCtrl()
{
	InitTitleBarCtrl();
	InitNavAreaCtrl();
	InitMainAreaCtrl();
}

void Ui_GamingDlg::LoadRes()
{
	m_Bitmap_Bar = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(GAMINGDLG_PNG_BAR),
		L"PNG");

	m_Bitmap_FullBar = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(GAMINGDLG_PNG_FULLBAR),
		L"PNG");

	m_Bitmap_Thumb = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(GAMINGDLG_PNG_THUMB),
		L"PNG"
	);
	m_Bitmap_VipLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(MAINDLG_PNG_VIP),
		L"PNG"
	);
}

void Ui_GamingDlg::MinimizeToTaskbar()
{
	//将当前子窗口最小化到任务栏时执行代码

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
	// 最小化窗口
	ShowWindow(SW_MINIMIZE);
}

void Ui_GamingDlg::HideUserProfile()
{
	auto pDlg = App.m_Dlg_Main.getProfileDlg();
	pDlg->ShowWindow(SW_HIDE);
	m_Btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_DOWN, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);
	Invalidate();
}

BOOL Ui_GamingDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void Ui_GamingDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	const float hitMargin = 13.3f * m_Scale;  // 点击扩大范围
	HideUserProfile();
	m_listBoxs.HideListBox();
	// 点击在滑块上，则开始拖拽
	CRect audioThumbHit(
		int(m_Rect_AudioThumb.X - hitMargin),
		int(m_Rect_AudioThumb.Y - hitMargin),
		int(m_Rect_AudioThumb.X + m_Rect_AudioThumb.Width + hitMargin),
		int(m_Rect_AudioThumb.Y + m_Rect_AudioThumb.Height + hitMargin)
	);
	if (audioThumbHit.PtInRect(point))
	{
		m_bDragging = true;
		m_bDraggingAudio = true;
		m_ptDragStart = point;
		m_ptDragCurrent = point;
		m_nInitialThumbPos = int(m_Rect_AudioThumb.X);
		SetCapture();
		return;
	}
	CRect microThumbHit(
		int(m_Rect_MicroThumb.X - hitMargin),
		int(m_Rect_MicroThumb.Y - hitMargin),
		int(m_Rect_MicroThumb.X + m_Rect_MicroThumb.Width + hitMargin),
		int(m_Rect_MicroThumb.Y + m_Rect_MicroThumb.Height + hitMargin)
	);
	if (microThumbHit.PtInRect(point))
	{
		m_bDragging = true;
		m_bDraggingAudio = false;
		m_ptDragStart = point;
		m_ptDragCurrent = point;
		m_nInitialThumbPos = int(m_Rect_MicroThumb.X);
		SetCapture();
		return;
	}

	 // 点击在“音量条”区域（不含滑块）
	Gdiplus::RectF audioBarHit(
		m_Rect_AudioBar.X - hitMargin,
		m_Rect_AudioBar.Y - hitMargin,
		m_Rect_AudioBar.Width + hitMargin * 2,
		m_Rect_AudioBar.Height + hitMargin * 2
	);
	if (audioBarHit.Contains((float)point.x, (float)point.y))
	{
		// 计算相对位置
		float rel = point.x - m_Rect_AudioBar.X;
		rel = max(0.0f, min(rel, m_Rect_AudioBar.Width));

		// 更新滑块 X 坐标
		float newX = m_Rect_AudioBar.X + rel - m_Rect_AudioThumb.Width / 2;
		newX = max(
			(float)m_Rect_AudioBar.X,
			min(newX,
				(float)(m_Rect_AudioBar.X + m_Rect_AudioBar.Width - m_Rect_AudioThumb.Width))
		);
		m_Rect_AudioThumb.X = newX;

		// 计算百分比并更新静态文本
		{
			int percent = int((rel / m_Rect_AudioBar.Width) * 100.0f + 0.5f);
			CString s;
			s.Format(_T("%d%%"), percent);
			m_Stat_SystemAudioPercent.LarSetText(s);
		}

		Invalidate(FALSE);
		return;
	}

	// 点击在“麦克风条”区域（不含滑块）
	Gdiplus::RectF microBarHit(
		m_Rect_MicroBar.X - hitMargin,
		m_Rect_MicroBar.Y - hitMargin,
		m_Rect_MicroBar.Width + hitMargin * 2,
		m_Rect_MicroBar.Height + hitMargin * 2
	);
	if (microBarHit.Contains((float)point.x, (float)point.y))
	{
		float rel = point.x - m_Rect_MicroBar.X;
		rel = max(0.0f, min(rel, m_Rect_MicroBar.Width));

		float newX = m_Rect_MicroBar.X + rel - m_Rect_MicroThumb.Width / 2;
		newX = max(
			(float)m_Rect_MicroBar.X,
			min(newX,
				(float)(m_Rect_MicroBar.X + m_Rect_MicroBar.Width - m_Rect_MicroThumb.Width))
		);
		m_Rect_MicroThumb.X = newX;

		// 计算百分比并更新静态文本
		{
			int percent = int((rel / m_Rect_MicroBar.Width) * 100.0f + 0.5f);
			CString s;
			s.Format(_T("%d%%"), percent);
			m_Stat_MicroAudioPercent.LarSetText(s);
		}

		Invalidate(FALSE);
		return;
	}

	// 其余区域交由基类处理
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_GamingDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		// 保存旧位置以便计算repaint区域
		Gdiplus::RectF oldThumbRect;
		if (m_bDraggingAudio)
			oldThumbRect = m_Rect_AudioThumb;
		else
			oldThumbRect = m_Rect_MicroThumb;

		// 更新当前鼠标位置
		m_ptDragCurrent = point;

		// 计算新位置
		int newPos = m_nInitialThumbPos + (m_ptDragCurrent.x - m_ptDragStart.x);

		if (m_bDraggingAudio)
		{
			// 限制在音频条边界内
			int minX = m_Rect_AudioBar.X;
			int maxX = m_Rect_AudioBar.X + m_Rect_AudioBar.Width - m_Rect_AudioThumb.Width;
			newPos = max(minX, min(newPos, maxX));

			// 更新滑块位置
			m_Rect_AudioThumb.X = newPos;

			// 更新百分比文本
			float relativePos = (newPos - minX) / (float)(maxX - minX);
			int percent = (int)(relativePos * 100);
			CString percentStr;
			percentStr.Format(_T("%d%%"), percent);
			m_Stat_SystemAudioPercent.LarSetText(percentStr);

			// 计算需要repaint的区域 - 确保包含整个音量条和滑块移动区域
			CRect invalidRect;
			invalidRect.SetRect(
				(int)m_Rect_AudioBar.X - 5,  // 扩大左侧边界
				(int)m_Rect_AudioBar.Y - (int)m_Rect_AudioThumb.Height, // 确保包含滑块上方区域
				(int)(m_Rect_AudioBar.X + m_Rect_AudioBar.Width + 5), // 扩大右侧边界
				(int)(m_Rect_AudioBar.Y + m_Rect_AudioBar.Height + m_Rect_AudioThumb.Height + 5) // 确保包含滑块下方区域
			);

			// repaint区域
			InvalidateRect(invalidRect, FALSE);
		}
		else
		{
			// 限制在麦克风条边界内
			int minX = m_Rect_MicroBar.X;
			int maxX = m_Rect_MicroBar.X + m_Rect_MicroBar.Width - m_Rect_MicroThumb.Width;
			newPos = max(minX, min(newPos, maxX));

			// 更新滑块位置
			m_Rect_MicroThumb.X = newPos;

			// 更新百分比文本
			float relativePos = (newPos - minX) / (float)(maxX - minX);
			int percent = (int)(relativePos * 100);
			CString percentStr;
			percentStr.Format(_T("%d%%"), percent);
			m_Stat_MicroAudioPercent.LarSetText(percentStr);

			// 计算需要repaint的区域 - 确保包含整个音量条和滑块移动区域
			CRect invalidRect;
			invalidRect.SetRect(
				(int)m_Rect_MicroBar.X - 5, // 扩大左侧边界
				(int)m_Rect_MicroBar.Y - (int)m_Rect_MicroThumb.Height, // 确保包含滑块上方区域
				(int)(m_Rect_MicroBar.X + m_Rect_MicroBar.Width + 5), // 扩大右侧边界
				(int)(m_Rect_MicroBar.Y + m_Rect_MicroBar.Height + m_Rect_MicroThumb.Height + 5) // 确保包含滑块下方区域
			);

			// repaint区域
			InvalidateRect(invalidRect, FALSE);
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void Ui_GamingDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		m_bDragging = false;
		ReleaseCapture();
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

void Ui_GamingDlg::OnBnClickedBtnCkaudio()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	m_Btn_CkAudio.LoadPNG(GAMINGDLG_PNG_CKAUDIO_HOLD);
	m_Btn_CkAudio.LoadClickPNG(GAMINGDLG_PNG_CKAUDIO_HOLD);
	m_Btn_CkAudio.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_CkNoAudio.LoadPNG(GAMINGDLG_PNG_NOCKAUDIO);
	m_Btn_CkNoAudio.LoadClickPNG(GAMINGDLG_PNG_NOCKAUDIO);
	m_Btn_CkNoAudio.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_SystemAudioIcon.LoadPNG(CHILDDLG_PNG_SYSTEMAUDIO);
	m_Btn_SystemAudioIcon.LoadClickPNG(CHILDDLG_PNG_SYSTEMAUDIO);
	m_Btn_SystemAudioIcon.SetBackgroundColor(RGB(26, 31, 37));

	m_IsRecordSystemAudio = true;

}

void Ui_GamingDlg::OnBnClickedBtnCknoaudio()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	m_Btn_CkAudio.LoadPNG(GAMINGDLG_PNG_CKAUDIO);
	m_Btn_CkAudio.LoadClickPNG(GAMINGDLG_PNG_CKAUDIO);
	m_Btn_CkAudio.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_CkNoAudio.LoadPNG(GAMINGDLG_PNG_NOCKAUDIO_HOLDE);
	m_Btn_CkNoAudio.LoadClickPNG(GAMINGDLG_PNG_NOCKAUDIO_HOLDE);
	m_Btn_CkNoAudio.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_SystemAudioIcon.LoadPNG(GAMINGDLG_PNG_AUDIOUNABLE);
	m_Btn_SystemAudioIcon.LoadClickPNG(GAMINGDLG_PNG_AUDIOUNABLE);
	m_Btn_SystemAudioIcon.SetBackgroundColor(RGB(26, 31, 37));

	m_IsRecordSystemAudio = false;
}

void Ui_GamingDlg::OnBnClickedBtnCkmicro()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	m_Btn_CkMicro.LoadPNG(GAMINGDLG_PNG_CKMICRO_HOLD);
	m_Btn_CkMicro.LoadClickPNG(GAMINGDLG_PNG_CKMICRO_HOLD);
	m_Btn_CkMicro.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_CkNoMIicro.LoadPNG(GAMINGDLG_PNG_CKAUDIO);
	m_Btn_CkNoMIicro.LoadClickPNG(GAMINGDLG_PNG_CKAUDIO);
	m_Btn_CkNoMIicro.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_MicroAudioIcon.LoadPNG(GAMINGDLG_PNG_MICROUNABLE);
	m_Btn_MicroAudioIcon.LoadClickPNG(GAMINGDLG_PNG_MICROUNABLE);
	m_Btn_MicroAudioIcon.SetBackgroundColor(RGB(26, 31, 37));

	m_IsRecordMicroAudio = true;
}

void Ui_GamingDlg::OnBnClickedBtnCknomicro()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	m_Btn_CkMicro.LoadPNG(GAMINGDLG_PNG_CKMICRO);
	m_Btn_CkMicro.LoadClickPNG(GAMINGDLG_PNG_CKMICRO);
	m_Btn_CkMicro.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_CkNoMIicro.LoadPNG(GAMINGDLG_PNG_CKAUDIO_HOLD);
	m_Btn_CkNoMIicro.LoadClickPNG(GAMINGDLG_PNG_CKAUDIO_HOLD);
	m_Btn_CkNoMIicro.SetBackgroundColor(RGB(26, 31, 37));

	m_Btn_MicroAudioIcon.LoadPNG(CHILDDLG_PNG_MICRO);
	m_Btn_MicroAudioIcon.LoadClickPNG(CHILDDLG_PNG_MICRO);
	m_Btn_MicroAudioIcon.SetBackgroundColor(RGB(26, 31, 37));

	m_IsRecordMicroAudio = false;
}

BOOL Ui_GamingDlg::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CDialog::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

void Ui_GamingDlg::OnBnClickedBtnMinimal()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	m_Shadow.Show(m_hWnd);
	ShowWindow(SW_MINIMIZE);
}

void Ui_GamingDlg::OnBnClickedBtnClose()
{
	m_listBoxs.HideListBox();
	unRegGameDetector();
	HideUserProfile();
	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			ScreenRecorder::GetInstance()->ReleaseInstance();
		}
		else
		{
			return;
		}
	}

	if (ModalDlg_MFC::ShowModal_IsCloseToBar() == IDCANCEL)
		return;

	ShowWindow(SW_HIDE);
	App.m_Dlg_Main.UpdateQuitWay();
	App.m_Dlg_Main.HandleClose();
}

void Ui_GamingDlg::OnBnClickedBtnScreen()
{
	m_listBoxs.HideListBox();
	unRegGameDetector();
	HideUserProfile();
	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			KillTimer(TIMER_NONEVIP_RECORDTIME);
			ScreenRecorder::GetInstance()->ReleaseInstance();

		}
		else
		{
			return;
		}
	}
	m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
	m_IsRecording = false;
	ShowWindow(SW_HIDE);
	auto pMainDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	pMainDlg->ShowWindow(SW_SHOW);
	pMainDlg->OnBnClickedBtnScreenRecord();
	m_Shadow.Show(m_hWnd);
}

void Ui_GamingDlg::OnBnClickedBtnArearecord()
{
	m_listBoxs.HideListBox();
	unRegGameDetector();
	HideUserProfile();
	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			KillTimer(TIMER_NONEVIP_RECORDTIME);
			ScreenRecorder::GetInstance()->ReleaseInstance();
		}
		else
		{
			return;
		}
	}
	m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
	m_IsRecording = false;
	m_Shadow.Show(m_hWnd);
	auto pMainDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	pMainDlg->OnBnClickedBtnArearecording();
	ShowWindow(SW_HIDE);
	pMainDlg->ShowWindow(SW_SHOW);
	m_Shadow.Show(m_hWnd);
}

void Ui_GamingDlg::OnBnClickedBtnWindowrecord()
{
	m_listBoxs.HideListBox();
	unRegGameDetector();
	HideUserProfile();
	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			KillTimer(TIMER_NONEVIP_RECORDTIME);
			ScreenRecorder::GetInstance()->ReleaseInstance();
		}
		else
		{
			return;
		}
	}
	m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
	m_IsRecording = false;
	ShowWindow(SW_HIDE);
	auto pMainDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	pMainDlg->OnBnClickedBtnAppwindowrecording();
	m_Shadow.Show(m_hWnd);
}

void Ui_GamingDlg::OnBnClickedBtnStartrecord()
{
	m_listBoxs.HideListBox();
	HWND recHwnd = m_RecHwnd;
	if (!recHwnd)
	{//如果没有选中任何可录制的窗口
		if (IDCANCEL == ModalDlg_MFC::ShowModal_NoGameWindowSelect())
		{
			return;
		}
		else
		{
			OnBnClickedBtnScreen();
			return;
		}
	}
	HideUserProfile();
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

		App.UserGamingRecord(false);
		unRegGameDetector();
		RecordingParams recp = App.m_Dlg_Main.CollectRecordingParams();
		// 从文本控件直接获取百分比值（格式为"XX%"）
		CString sysVolumeText, microVolumeText;
		m_Stat_SystemAudioPercent.GetWindowText(sysVolumeText);
		m_Stat_MicroAudioPercent.GetWindowText(microVolumeText);

		// 计算系统音量百分比 - 直接从滑块位置计算
		float systemAudioVolume = 0.0f;
		{
			float minX = m_Rect_AudioBar.X;
			float maxX = m_Rect_AudioBar.X + m_Rect_AudioBar.Width - m_Rect_AudioThumb.Width;
			if (maxX > minX)  // 防止除零错误
			{
				systemAudioVolume = (m_Rect_AudioThumb.X - minX) / (maxX - minX);
				systemAudioVolume = max(0.0f, min(1.0f, systemAudioVolume));  // 确保在0-1范围内
			}
		}

		// 计算麦克风音量百分比 - 直接从滑块位置计算
		float microAudioVolume = 0.0f;
		{
			float minX = m_Rect_MicroBar.X;
			float maxX = m_Rect_MicroBar.X + m_Rect_MicroBar.Width - m_Rect_MicroThumb.Width;
			if (maxX > minX)  // 防止除零错误
			{
				microAudioVolume = (m_Rect_MicroThumb.X - minX) / (maxX - minX);
				microAudioVolume = max(0.0f, min(1.0f, microAudioVolume));  // 确保在0-1范围内
			}
		}

		if (!m_IsRecordMicroAudio)
			microAudioVolume = 0;
		if (!m_IsRecordSystemAudio)
			systemAudioVolume = 0;
		ScreenRecorder::RecordMode recordMode = (systemAudioVolume > 0.0f) ?
			((microAudioVolume > 0.0f) ?
				ScreenRecorder::RecordMode::Both : ScreenRecorder::RecordMode::SystemSound) :
			((microAudioVolume > 0.0f) ?
				ScreenRecorder::RecordMode::Microphone : ScreenRecorder::RecordMode::None);

		//判断是选区录制还是全屏录制
		ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
		pSRIns->SetSystemAudioVolume(systemAudioVolume);
		pSRIns->SetMicroVolume(microAudioVolume);
		pSRIns->SetVideoEncoder(GlobalFunc::MapUICodecToEncoderName(recp.codecText.c_str()));
		pSRIns->SetAudioCaptureDevice(recp.audioDevice.c_str());
		pSRIns->SetMicroDeviceName(recp.microDevice.c_str());
		pSRIns->SetRecordMouse(recp.RecordMouse);
		pSRIns->SetRecordCallBack(ScreenRecorder::WindowRecord_WindowMinimalAndClose, [this]()
			{//设置窗口最小化时，进行的回调函数
				::PostMessage(this->GetSafeHwnd(), MSG_GAMEREC_RECORDWINDOWMINIMAL, NULL, NULL);
			});
		if (App.m_Dlg_Main.m_Dlg_Config->m_Bool_IsWaterOn)
			pSRIns->SetVideoTextFilter(L"极速录屏大师", "40", "FFFFFF", recp.logoPath);//设置水印

		pSRIns->SetWindowRecordParam(
			recHwnd,
			recp.videoResolution,
			recp.videoQuality,
			recp.videoFormat,
			recp.encodePreset,
			recordMode,
			recp.audioSampleRate,
			recp.audioBitRate,
			recp.fps
		);
		MinimizeToTaskbar();

		ModalDlg_SDL::ShowModal_CountDown(3, "即将开始录制", [=]()
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"倒计时结束");
			}, true);

		// 开始录制
		CT2A outputfileName(recp.outputFilePath.c_str());
		if (pSRIns->startRecording(outputfileName))
		{
			m_IsRecording = true;
			if (!App.m_IsVip && !App.m_IsNonUserPaid)
				SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, NULL);
			if (App.m_IsOverBinds && !App.m_IsNonUserPaid)//如果超限，1分钟后触发
				SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, NULL);
			m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_ISRECORDING);
			m_Btn_Rec.LoadPNG(CHILDDLG_PNG_ISRECORDING);
			App.m_Dlg_Main.m_closeManager.get()->SetTrayDoubleClickCallback([this]()
				{
					this->ShowWindow(SW_RESTORE);
					this->ShowWindow(SW_SHOW);
				});
		}
		else
		{
			pSRIns->ReleaseInstance();
			ModalDlg_MFC::ShowModal_RecordFalse();
			m_IsRecording = false;
			m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
			m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
			RegGameDetector();
			this->ShowWindow(SW_SHOW);
		}
	}
	else
	{
		App.UserGamingRecord(true);
		RegGameDetector();
		KillTimer(TIMER_NONEVIP_RECORDTIME);
		ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
		pSRIns->ReleaseInstance();
		m_IsRecording = false;
		auto RecordP = App.m_Dlg_Main.GetLastRecordParam();
		App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(RecordP.outputFilePath.c_str());
		m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
		m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
		Invalidate(false);
		ModalDlg_MFC::ShowModal_Priview(this);
	}
}

LRESULT Ui_GamingDlg::OnRecordWindowMinimal(WPARAM wParam, LPARAM lParam)
{
	KillTimer(TIMER_NONEVIP_RECORDTIME);
	ScreenRecorder::GetInstance()->ReleaseInstance();
	m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
	m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
	m_IsRecording = false;
	ModalDlg_MFC::ShowModal_RecordWindowMinimal();
	RegGameDetector();
	return 1;
}

void Ui_GamingDlg::OnBnClickedBtnVideolist()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	if (ScreenRecorder::GetInstance()->IsRecording())
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
		if (MessageDlg.DoModal() == IDOK)
		{
			this->OnBnClickedBtnStartrecord();
		}
		else
		{
			return;
		}
	}
	m_Shadow.Show(m_hWnd);
	auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());

	//视频列表窗口出现
	CRect WindowRect, videoListWindowRect;
	GetWindowRect(WindowRect);
	pDlg->m_Dlg_Videolist->GetWindowRect(videoListWindowRect);

	float Width = 788 * m_Scale;
	float Height = 510 * m_Scale;
	videoListWindowRect.left = WindowRect.left + (WindowRect.Width() - Width) / 2;
	videoListWindowRect.top = WindowRect.top + (WindowRect.Height() - Height) / 2;
	videoListWindowRect.right = videoListWindowRect.left + Width;
	videoListWindowRect.bottom = videoListWindowRect.top + Height;

	pDlg->m_Dlg_Videolist->Ui_UpdateWindowPos(videoListWindowRect);
	pDlg->m_Dlg_Videolist->SetReturnDlg(this);
	pDlg->m_Dlg_Videolist->ShowWindow(SW_SHOW);
	pDlg->m_Dlg_Videolist->SetTimer(Ui_ConfigDlg::TIMER_DELAYED_REDRAW, 50, NULL);
	this->ShowWindow(SW_HIDE);
}

void Ui_GamingDlg::OnBnClickedBtnLogin()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	m_Shadow.Show(m_hWnd);
	if (!App.m_isLoginIn)
	{
		// 创建新的对话框实例
		Ui_LoginDlg* Dlg_Login = new Ui_LoginDlg(this);
		int loginWindowWidth = 350 * m_Scale;
		int loginWindowHeight =390 * m_Scale;

		// 设置位置
		CRect WindowRect, loginWindowRect;
		GetWindowRect(WindowRect);
		loginWindowRect.left = WindowRect.left + (WindowRect.Width() - loginWindowWidth) / 2;
		loginWindowRect.top = WindowRect.top + (WindowRect.Height() - loginWindowHeight) / 2;
		loginWindowRect.right = loginWindowRect.left + loginWindowWidth;
		loginWindowRect.bottom = loginWindowRect.top + loginWindowHeight;
		Dlg_Login->Ui_SetWindowRect(loginWindowRect);

		// 以模态方式显示对话框
		DEBUG_CONSOLE_STR(ConsoleHandle, L"显示登录对话框");
		INT_PTR result = Dlg_Login->DoModal();
		// 处理结果
		if (result == IDOK)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"登录成功，开始获取用户信息");
			if (App.RequestDeviceInfo())
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取失败");
			}
			ShowLoginUi();
			ModalDlg_MFC::ShowModal_LoginSuccess();
		}
		else if (result == IDCANCEL)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"用户取消了登录");
		}
		Dlg_Login->DestroyWindow();
	}
	else
	{
		OnBnClickedBtnPhone();
	}
}

void Ui_GamingDlg::OnBnClickedBtnPhone()
{
	m_listBoxs.HideListBox();
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

		//更新控件状态 
		m_Btn_Phone.LarSetBtnNailImage(MAINDLG_PNG_UP, CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale);
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

void Ui_GamingDlg::ShowUserProfileSDL()
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
			m_Btn_Login.GetWindowRect(BtnRect);
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

Gdiplus::Bitmap* Ui_GamingDlg::GetIconOfHwnd(HWND hWnd)
{
	if (!hWnd || !IsWindow(hWnd)) {
		DB(ConsoleHandle, L"无效的窗口句柄");
		return nullptr;
	}

	HICON hIcon = NULL;

	// 方法1: 尝试使用WM_GETICON获取窗口图标
	// 先尝试获取大图标(ICON_BIG)
	hIcon = (HICON)::SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
	if (!hIcon) {
		// 尝试获取小图标(ICON_SMALL)
		hIcon = (HICON)::SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
	}
	if (!hIcon) {
		// 尝试获取小图标2(ICON_SMALL2) - 未公开的消息值
		hIcon = (HICON)::SendMessage(hWnd, WM_GETICON, 2, 0);
	}

	// 方法2: 尝试从窗口类中获取图标
	if (!hIcon) {
		hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
	}
	if (!hIcon) {
		hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM);
	}

	// 方法3: 尝试从进程可执行文件中提取图标
	if (!hIcon) {
		DWORD processId;
		GetWindowThreadProcessId(hWnd, &processId);

		if (processId != 0) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
			if (hProcess) {
				wchar_t exePath[MAX_PATH];
				if (GetModuleFileNameEx(hProcess, NULL, exePath, MAX_PATH)) {
					// 尝试提取可执行文件的图标
					ExtractIconEx(exePath, 0, &hIcon, NULL, 1);
					if (!hIcon) {
						// 如果获取大图标失败，尝试获取小图标
						ExtractIconEx(exePath, 0, NULL, &hIcon, 1);
					}
				}
				CloseHandle(hProcess);
			}
		}
	}

	// 方法4: 使用默认应用程序图标
	if (!hIcon) {
		hIcon = LoadIcon(NULL, IDI_APPLICATION);
		DB(ConsoleHandle, L"使用默认应用程序图标");
	}

	// 如果获取到了图标，转换为Gdiplus::Bitmap
	if (hIcon) {
		ICONINFO iconInfo;
		if (GetIconInfo(hIcon, &iconInfo)) {
			// 获取图标尺寸
			BITMAP bm;
			if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm)) {
				// 创建Gdiplus::Bitmap对象
				Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromHICON(hIcon);

				// 清理资源
				DeleteObject(iconInfo.hbmColor);
				DeleteObject(iconInfo.hbmMask);

				// 如果这不是从系统资源加载的图标，销毁它
				if (hIcon != LoadIcon(NULL, IDI_APPLICATION)) {
					DestroyIcon(hIcon);
				}

				if (pBitmap) {
					DBFMT(ConsoleHandle, L"成功获取窗口图标，尺寸: %dx%d",
						pBitmap->GetWidth(), pBitmap->GetHeight());
					return pBitmap;
				}
				else {
					DB(ConsoleHandle, L"从HICON创建Bitmap失败");
				}
			}
			else {
				DeleteObject(iconInfo.hbmColor);
				DeleteObject(iconInfo.hbmMask);
			}
		}

		// 如果这不是从系统资源加载的图标，销毁它
		if (hIcon != LoadIcon(NULL, IDI_APPLICATION)) {
			DestroyIcon(hIcon);
		}
	}

	DB(ConsoleHandle, L"无法获取窗口图标");
	return nullptr;
}

void Ui_GamingDlg::SetGameBtnIcon(Gdiplus::Bitmap* bitmap)
{
	m_Btn_RecordGame.LoadPNG(bitmap);
	m_Btn_RecordGame.LoadClickPNG(bitmap);
	m_Btn_RecordGame.SetCircularImage(TRUE, 16 * m_Scale);
	m_Btn_RecordGame.SetCircularBorder(TRUE, 3, 255, 73, 73, 73);
	Invalidate();
	DB(ConsoleHandle, L"更新游戏图标");
}

void Ui_GamingDlg::AddRecGameItem(const CString& item)
{
	CRect sgRect;
	m_btn_selectGame.GetWindowRect(sgRect);

	m_Csa_Game.Add(item);
	m_listBoxs.DeleteListBox(L"游戏窗口下拉框");
	m_listBoxs.addListBox(
		sgRect.Width(), sgRect.Height(),
		this, m_Csa_Game, L"游戏窗口下拉框");
	m_listBoxs.SetListBoxHideWhenMouseLeave(false);// 设置当鼠标离开下拉框时，下拉框不隐藏
	m_listBoxs.SetTextSize(14, L"");
	m_listBoxs.SetScrollbarWidth(6 * m_Scale);
}

void Ui_GamingDlg::AddGameIcon(std::wstring name, Gdiplus::Bitmap* bitmap)
{
	if (bitmap == nullptr || bitmap->GetLastStatus() != Gdiplus::Ok)
		return;
	// 创建位图的克隆
	Gdiplus::Bitmap* bitmapClone = bitmap->Clone(0, 0, bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetPixelFormat());
	if (bitmapClone == nullptr || bitmapClone->GetLastStatus() != Gdiplus::Ok)
	{
		delete bitmapClone; // 以防万一部分创建成功但状态错误
		return;
	}

	// 检查是否已存在同名的图标
	auto it = m_map_captureGames.find(name);
	if (it != m_map_captureGames.end())
	{
		delete it->second;
	}
	m_map_captureGames[name] = bitmapClone;
	DBFMT(ConsoleHandle, L"成功添加游戏窗口:%s 的位图", name.c_str());
}

bool Ui_GamingDlg::JudegeIsWindowCaptured(const CString& WindowName)
{
	int size = m_Csa_Game.GetSize();
	for (size_t i = 0; i < size; i++)
	{
		CString str = m_Csa_Game.GetAt(i);
		if (WindowName == str)
		{
			return true;
		}
	}
	return false;
}

void Ui_GamingDlg::SetCurrentRecGameWindow(HWND hWnd, std::wstring windowName)
{
	m_RecHwnd = hWnd;
	for (const auto& gi : m_map_captureGames)
	{
		if (gi.first == windowName)
		{
			m_btn_selectGame.SetWindowTextW(windowName.c_str());
			m_Btn_RecordGame.LoadPNG(gi.second);
			m_Btn_RecordGame.LoadClickPNG(gi.second);
			m_Btn_RecordGame.SetCircularImage(TRUE, 16 * m_Scale);
		}
	}
	Invalidate();
}

void Ui_GamingDlg::RegGameDetector()
{
	GameFocusDetector& detector = GameFocusDetector::GetInstance();
	detector.SetFocusChangedCallback(OnFocusChanged);
	detector.ExcludeCurrentProcess();
	if (detector.StartMonitoring())
	{
		DB(ConsoleHandle, L"焦点窗口变化监控已启动");
	}
}

void Ui_GamingDlg::unRegGameDetector()
{
	GameFocusDetector& detector = GameFocusDetector::GetInstance();
	detector.StopMonitoring();
}

void Ui_GamingDlg::UpdateLogOutUi()
{
	m_Btn_Login.ShowWindow(SW_SHOW);
	m_Btn_Phone.ShowWindow(SW_HIDE);
	m_Btn_Phone.SetWindowTextW(L"登录");
	if (!App.m_IsVip || !App.m_isLoginIn)
	{
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_OPENVIP);
	}
	else
	{
		m_Btn_OpenVip.LoadPNG(MAINDLG_PNG_PAYFORLONG);
	}
	Invalidate(false);
}

void Ui_GamingDlg::OnBnClickedBtnOpenvip()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	if (App.m_isLoginIn)
	{
		m_Shadow.Show(m_hWnd);
		auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());

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
		if (pDlg->m_Dlg_VipPay != nullptr) {
			delete pDlg->m_Dlg_VipPay;  // 确保删除旧对象，避免内存泄漏
			pDlg->m_Dlg_VipPay = nullptr;
		}

		pDlg->m_Dlg_VipPay = new Ui_VipPayDlg(this);
		pDlg->m_Dlg_VipPay->Ui_SetWindowPos(WindowRect);  // 设置窗口大小和位置

		INT_PTR result = pDlg->m_Dlg_VipPay->DoModal();
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
		pDlg->m_Dlg_VipPay = nullptr;
	}
	else
	{
		Ui_MessageModalDlg MessageDlg;
		MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"抱歉，请登录后进行此操作", L"确认");
		MessageDlg.DoModal();
	}
}

void Ui_GamingDlg::OnBnClickedBtnAppicon()
{ 
	m_listBoxs.HideListBox();
	HideUserProfile();
}

void Ui_GamingDlg::OnBnClickedBtnConfig()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
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

void Ui_GamingDlg::OnBnClickedBtnMore()
{
	m_listBoxs.HideListBox();
	//auto pDlg = reinterpret_cast<Ui_MainDlg*>(GetParent());
	HideUserProfile();
}

void Ui_GamingDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	//m_Shadow.Show(m_hWnd);
	// TODO: 在此处添加消息处理程序代码
}

void Ui_GamingDlg::OnExitSizeMove()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Shadow.Show(m_hWnd);
	CDialogEx::OnExitSizeMove();
}

void Ui_GamingDlg::OnEnterSizeMove()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Shadow.Show(m_hWnd);
	CDialogEx::OnEnterSizeMove();
}

void Ui_GamingDlg::OnBnClickedBtnRecordedGame()
{
	m_listBoxs.HideListBox();
	if (m_map_captureGames.empty())
	{
		if (IDCANCEL == ModalDlg_MFC::ShowModal_NoGameWindowSelect())
		{
			return;
		}
		else
		{
			OnBnClickedBtnScreen();
			return;
		}
	}
}

LRESULT Ui_GamingDlg::On_BroadCast_UserLogin(WPARAM wParam, LPARAM lParam)
{
	ShowLoginUi();
	return 1;
}

LRESULT Ui_GamingDlg::On_BroadCast_UserLogOut(WPARAM wParam, LPARAM lParam)
{
	UpdateLogOutUi();
	return 1;
}

LRESULT Ui_GamingDlg::On_SDLBnClick_UserLogOut(WPARAM wParam, LPARAM lParam)
{
	bool logoutSuccess = App.RequestSignOut();
	if (logoutSuccess)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"退出登录请求成功，更新UI状态");
		UpdateLogOutUi();
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

void Ui_GamingDlg::OnBnClickedBtnAdvanceopt()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
	this->OnBnClickedBtnConfig();
}

void Ui_GamingDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_NONEVIP_RECORDTIME)
	{
		KillTimer(TIMER_NONEVIP_RECORDTIME);
		if (ScreenRecorder::IsRecording())
		{
			ScreenRecorder::GetInstance()->ReleaseInstance();
			this->ShowWindow(SW_SHOW);
			this->ShowWindow(SW_RESTORE);
			m_Btn_Rec.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
			m_Btn_Rec.LoadPNG(CHILDDLG_PNG_STARTRECORD);
			auto RecordP = App.m_Dlg_Main.GetLastRecordParam();
			App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(RecordP.outputFilePath.c_str());
			Invalidate(false);
			App.UserNoneVipRecordSuccess();
			DB(ConsoleHandle, L"开始弹框提示用户无法录制的原因");
			//弹框提示用户无法继续录制的原因
			if (!App.m_isLoginIn)//未登录
				ModalDlg_MFC::ShowModal_NeedLogin();
			else if (!App.m_IsVip)//不是vip
				ModalDlg_MFC::ShowModal_TrialOver(this);
			else if (App.m_IsOverBinds)//设备绑定超限
				ModalDlg_MFC::ShowModal_OverBindsTips();
			DB(ConsoleHandle, L"弹框无法录制的原因完成");

			m_IsRecording = false;
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void Ui_GamingDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	HideUserProfile();
	CDialogEx::OnNcLButtonDown(nHitTest, point);
}

void Ui_GamingDlg::OnBnClickedBtnGaimgrecord()
{
	m_listBoxs.HideListBox();
	HideUserProfile();
}

void Ui_GamingDlg::OnBnClickedBtnFeedback()
{
	m_listBoxs.HideListBox();
	App.OpenFeedBackLink(
		this->GetSafeHwnd(),
		L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
	);
}

void Ui_GamingDlg::OnBnClickedBtnSelectgame()
{
	if (m_map_captureGames.empty())
	{
		if (IDCANCEL == ModalDlg_MFC::ShowModal_NoGameWindowSelect())
		{
			return;
		}
		else
		{
			OnBnClickedBtnScreen();
			return;
		}
	}

	if (m_listBoxs.IsListBoxVisiable(L"游戏窗口下拉框"))
	{
		m_listBoxs.HideListBox();
		Invalidate();
		return;
	}
	//更新下拉框的显示位置
	CRect sgRect;
	m_btn_selectGame.GetWindowRect(sgRect);
	m_listBoxs.UpdateDroplistXY(L"游戏窗口下拉框", sgRect.left - 5 * m_Scale, sgRect.top + sgRect.Height());
	Invalidate();
}

LRESULT Ui_GamingDlg::OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam)
{
	MsgParam::LISTBOX_SELECT_INFO* pInfo = (MsgParam::LISTBOX_SELECT_INFO*)wParam;
	if (pInfo)
	{
		int nIndex = pInfo->nIndex;
		CString strText = pInfo->strText;
		std::wstring strBoxName = pInfo->strBoxName;
		if (strBoxName == L"游戏窗口下拉框")
		{
			m_btn_selectGame.SetWindowText(strText);
			for (const auto& gi : m_map_captureGames)
			{
				if (gi.first.c_str() == strText)
				{
					m_Btn_RecordGame.LoadPNG(gi.second);
					m_Btn_RecordGame.LoadClickPNG(gi.second);
					m_Btn_RecordGame.SetCircularImage(TRUE, 16 * m_Scale);
					Invalidate();
				}
			}
		}
	}
	return 1;
}

void OnFocusChanged(HWND hwnd, bool isGameWindow)
{
	wchar_t windowTitle[256] = { 0 };
	::GetWindowTextW(hwnd, windowTitle, 256);
	DBFMT(ConsoleHandle, L"焦点窗口变化:%s", windowTitle);
	DBFMT(ConsoleHandle, L"是否是游戏窗口:%s", (isGameWindow ? L"是" : L"否"));
	if (isGameWindow && !App.m_Dlg_Main.m_Dlg_Gaming->JudegeIsWindowCaptured(windowTitle))
	{
		DB(ConsoleHandle, L"--------当前焦点窗口为游戏窗口并且第一次获取到，开始执行添加逻辑--------------");
		auto bitmap = App.m_Dlg_Main.m_Dlg_Gaming->GetIconOfHwnd(hwnd);
		App.m_Dlg_Main.m_Dlg_Gaming->AddGameIcon(windowTitle, bitmap);				//更新当前存储获取到的窗口图标
		App.m_Dlg_Main.m_Dlg_Gaming->SetCurrentRecGameWindow(hwnd, windowTitle);	//更新控件显示
		App.m_Dlg_Main.m_Dlg_Gaming->AddRecGameItem(windowTitle);					//加入选项到下拉框
		DB(ConsoleHandle, L"执行添加完成");
	}
	else if(App.m_Dlg_Main.m_Dlg_Gaming->JudegeIsWindowCaptured(windowTitle))
	{
		App.m_Dlg_Main.m_Dlg_Gaming->SetCurrentRecGameWindow(hwnd, windowTitle);	//更新控件显示
	}
}

void Ui_GamingDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
}
