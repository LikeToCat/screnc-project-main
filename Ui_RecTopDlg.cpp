// Ui_RecTopDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_RecTopDlg.h"
#include "afxdialogex.h"
#include "PngLoader.h"
#include "Ui_PopupMenuDlg.h"
#include "Ui_ScreenCaptureDlg.h"
#include "CDebug.h"
#include "ModalDialogFunc.h"
#include "CMessage.h"
#include "theApp.h"
extern HANDLE ConsoleHandle;
// Ui_RecTopDlg 对话框

IMPLEMENT_DYNAMIC(Ui_RecTopDlg, CDialogEx)

Ui_RecTopDlg::Ui_RecTopDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_RECTOPWINDOW, pParent)
{
	m_Rect_WindowRect.SetRectEmpty();
	m_IsRecording = false;	
	m_Menu_Transp = nullptr;
	m_IsHideDuringRecord = false;
	m_ctxType = RecContextType::None;
}

Ui_RecTopDlg::~Ui_RecTopDlg()
{
}

void Ui_RecTopDlg::SetRecordContext_None()
{
	// 复位按钮显示
	m_ctxType = RecContextType::None;
	m_btn_Recording.ShowWindow(SW_HIDE);
	m_btn_StartRecord.ShowWindow(SW_SHOW);
	m_btn_StartRecordText.ShowWindow(SW_SHOW);
	m_btn_pause.ShowWindow(SW_HIDE);
	m_btn_resumeRecord.ShowWindow(SW_HIDE);
}

void Ui_RecTopDlg::SetRecordContext_Child()
{
	m_ctxType = RecContextType::Child;
}

void Ui_RecTopDlg::SetRecordContext_Window()
{
	m_ctxType = RecContextType::Window;
}

void Ui_RecTopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, RECTOPDLG_BTN_NAIL, m_Btn_Nail);
	DDX_Control(pDX, RECTOPDLG_BTN_TRANSPARENT, m_Btn_Transparent);
	DDX_Control(pDX, RECTOPDLG_BTN_PIXEL, m_Stat_Pixel);
	DDX_Control(pDX, RECTOPDLG_STAT_TIMECOUNT, m_Stat_TimeCount);
	DDX_Control(pDX, RECTOPDLG_BTN_SCREENPHOTO, m_Btn_ScreenShot);
	DDX_Control(pDX, RECTOPDLG_BTN_CLOSE, m_btn_Close);
	DDX_Control(pDX, TOPREC_BTN_RECORDING, m_btn_Recording);
	DDX_Control(pDX, TOPRECDLG_BTN_STARTRECORDPNG, m_btn_StartRecord);
	DDX_Control(pDX, TOPRECDLG_BTN_STARTRECORD, m_btn_StartRecordText);
	DDX_Control(pDX, TOPREC_BTN_PAUSE, m_btn_pause);
	DDX_Control(pDX, TOPREC_BTN_RESUMERECORD, m_btn_resumeRecord);
}

void Ui_RecTopDlg::SetWindowRect(CRect Rect)
{
	m_Rect_WindowRect = Rect;
}

void Ui_RecTopDlg::HideAllTooltips()
{
	m_toolTips.ShowWindow(SW_HIDE);
}

void Ui_RecTopDlg::CleanUpGdiPngRes()
{
	m_Btn_Nail.ClearImages();
	m_Btn_Transparent.ClearImages();
	m_Btn_ScreenShot.ClearImages();
	m_btn_Close.ClearImages();
	m_btn_StartRecord.ClearImages();
	m_btn_Recording.ClearImages();
}

void Ui_RecTopDlg::SetPauseUIMode()
{
	// 顶部窗口自身状态与按钮切换
	auto now = std::chrono::steady_clock::now();
	m_IsPaused = true;
	m_PauseStart = now; 

	m_btn_pause.ShowWindow(SW_HIDE);
	m_btn_resumeRecord.ShowWindow(SW_SHOW);
	{
		// 计算截至暂停瞬间的有效录制时长
		auto elapsed = now - m_Time_Start - m_PausedAccum;
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
		if (seconds < 0) seconds = 0;

		int hours = static_cast<int>(seconds / 3600);
		int minutes = static_cast<int>((seconds % 3600) / 60);
		int secs = static_cast<int>(seconds % 60);

		wchar_t buf[32] = { 0 };
		swprintf_s(buf, L"暂停中%02d:%02d:%02d", hours, minutes, secs);
		m_Stat_TimeCount.LarSetText(buf);
	}

	Invalidate(false);
}

void Ui_RecTopDlg::ResumeFromPauseUiMode()
{
	// 汇总暂停时长与按钮切换
	if (m_IsPaused)
	{
		auto now = std::chrono::steady_clock::now();
		m_PausedAccum += (now - m_PauseStart);
	}
	m_IsPaused = false;
	m_btn_resumeRecord.ShowWindow(SW_HIDE);
	m_btn_pause.ShowWindow(SW_SHOW);
}

void Ui_RecTopDlg::GetUserDPI()
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

void Ui_RecTopDlg::UpdateScale()
{
	if (m_Rect_WindowRect.IsRectEmpty())
	{
		m_WindowWidth = 590 * m_Scale;
		m_WindowHeight = 34 * m_Scale;
		m_WindowX = (GetSystemMetrics(SM_CXSCREEN) - m_WindowWidth)/2;
		m_WindowY = 0;
	}
	else
	{
		m_WindowWidth = m_Rect_WindowRect.Width();
		m_WindowHeight = m_Rect_WindowRect.Height();
		m_WindowX = m_Rect_WindowRect.left;
		m_WindowY = m_Rect_WindowRect.top;
	}
	MoveWindow(m_WindowX, m_WindowY, m_WindowWidth, m_WindowHeight);

	//顶针
	float NailWidth = 28 * m_Scale;
	float NailHeight = 28 * m_Scale;
	float NailX = 10 * m_Scale;
	float NailY = (m_WindowHeight - NailHeight) / 2;
	m_Btn_Nail.MoveWindow(NailX, NailY, NailWidth, NailHeight);

	//不透明度
	float TranspWidth = 28 * m_Scale;
	float TranspHeight = 28 * m_Scale;
	float TranspX = NailX + NailWidth + 10 * m_Scale;
	float TranspY = NailY;
	m_Btn_Transparent.MoveWindow(TranspX, TranspY, TranspWidth, TranspHeight);

	//分辨率
	float PixelWidth = 110 * m_Scale;
	float PixelHeight = 20 * m_Scale;
	float PixelX = TranspX + 20 * m_Scale;
	float PixelY = (m_WindowHeight - PixelHeight) / 2 - 1 * m_Scale;
	m_Stat_Pixel.MoveWindow(PixelX, PixelY, PixelWidth, PixelHeight);

	//时间计数文本
	float TimeCWidth = 150 * m_Scale;
	float TimeCHeight = 20 * m_Scale;
	float TimeCX = (m_WindowWidth - TimeCWidth) / 2;
	float TimeCY = (m_WindowHeight - TimeCHeight) / 2 - 2 * m_Scale;;
	m_Stat_TimeCount.MoveWindow(TimeCX, TimeCY, TimeCWidth, TimeCHeight);

	//关闭按钮
	float CloseWidth = 28 * m_Scale;
	float CloseHeight = 28 * m_Scale;
	float CloseX = m_WindowWidth - CloseWidth - 10 * m_Scale;
	float CloseY = (m_WindowHeight - CloseHeight) / 2;
	m_btn_Close.MoveWindow(CloseX, CloseY, CloseWidth, CloseHeight);

	//截屏按钮
	float ScrShotWidth = 28 * m_Scale;
	float ScrShotHeight = 28 * m_Scale;
	float ScrShotX = CloseX - 10 * m_Scale - ScrShotWidth;
	float ScrShotY = (m_WindowHeight - ScrShotHeight) / 2 + 1 * m_Scale;
	m_Btn_ScreenShot.MoveWindow(ScrShotX, ScrShotY, ScrShotWidth, ScrShotHeight);

	//开始录制文字按钮
	float StartRecTWidth = 70 * m_Scale;
	float StartRecTHeight = 28 * m_Scale;
	float StartRecX = ScrShotX - StartRecTWidth - 10 * m_Scale;
	float StartRecY = (m_WindowHeight - StartRecTHeight) / 2;
	m_btn_StartRecordText.MoveWindow(StartRecX, StartRecY, StartRecTWidth, StartRecTHeight);

	//开始录制与录制中按钮
	float StartRecordWidth = 28 * m_Scale;
	float StartRecordHeight = 28 * m_Scale;
	float StartRecordX = StartRecX - StartRecordWidth;
	float StartRecordY = (m_WindowHeight - StartRecordWidth) / 2;
	m_btn_StartRecord.MoveWindow(StartRecordX, StartRecordY, StartRecordWidth, StartRecordHeight);
	m_btn_Recording.MoveWindow(StartRecordX + StartRecTWidth, StartRecordY, StartRecordWidth, StartRecordHeight);

	//暂停按钮和恢复播放按钮
	float pauseWH = StartRecordWidth;
	float pauseX = StartRecordX + StartRecTWidth - StartRecordWidth - 5 * m_Scale;
	float pauseY = (m_WindowHeight - pauseWH) / 2;
	m_btn_pause.MoveWindow(pauseX, pauseY, pauseWH, pauseWH);
	m_btn_resumeRecord.MoveWindow(pauseX, pauseY, pauseWH, pauseWH);
	m_btn_resumeRecord.ShowWindow(SW_HIDE);
	m_btn_pause.ShowWindow(SW_HIDE);
}

void Ui_RecTopDlg::InitCtrl()
{
	//创建提示工具控件
	m_toolTips.LarCreate();
	m_toolTips.LarSetBkColor(Color(36, 37, 40));
	m_toolTips.LarSetBorderColor(Color(36, 37, 40));
	m_toolTips.LarSetShadowEnable(true);

	//置顶
	m_Btn_Nail.LoadPNG(TOPREC_PNG_NAIL);
	m_Btn_Nail.SetHoverEffectColor(20, 255, 255, 255);
	m_Btn_Nail.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_Nail.SetStretchMode(0.90f);
	m_Btn_Nail.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_Btn_Nail.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"下次录制时是否隐藏");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_Btn_Nail.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"下次录制时是否隐藏");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	//不透明
	m_Btn_Transparent.LoadPNG(TOPREC_PNG_TRANSP);
	m_Btn_Transparent.SetHoverEffectColor(20, 255, 255, 255);
	m_Btn_Transparent.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_Transparent.SetStretchMode(0.90f);
	m_Btn_Transparent.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_Btn_Transparent.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"窗口透明度");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_Btn_Transparent.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"窗口透明度");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	//分辨率
	m_Stat_Pixel.LarSetTextSize(20);
	CString Pixel;
	Pixel.Format(L"%dX%d", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	m_Stat_Pixel.LarSetText(Pixel);

	//时间计数文本
	m_Stat_TimeCount.LarSetTextSize(20);

	//关闭
	m_btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_btn_Close.SetHoverEffectColor(20, 255, 255, 255);
	m_btn_Close.SetBackgroundColor(RGB(37, 39, 46));
	m_btn_Close.SetStretchMode(0.75f);
	m_btn_Close.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_btn_Close.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"关闭");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_btn_Close.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"关闭");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	//截屏
	m_Btn_ScreenShot.LoadPNG(RECTOP_PNG_SCREENSHOT);
	m_Btn_ScreenShot.SetHoverEffectColor(20, 255, 255, 255);
	m_Btn_ScreenShot.SetBackgroundColor(RGB(37, 39, 46));
	m_Btn_ScreenShot.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_Btn_ScreenShot.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"截屏");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_Btn_ScreenShot.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"截屏");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	//开始录制文字按钮
	m_btn_StartRecordText.LarSetTextSize(20);
	m_btn_StartRecordText.LaSetTextColor(Gdiplus::Color(155, 255, 255, 255));
	m_btn_StartRecordText.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_btn_StartRecordText.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_btn_StartRecordText.LarSetBorderColor(Gdiplus::Color(255, 37, 39, 46));
	m_btn_StartRecordText.LarSetEraseBkEnable(false);
	m_btn_StartRecordText.LarSetNormalFiilBrush(SolidBrush(Color(255, 37, 39, 46)));

	//开始录制与录制中按钮
	m_btn_StartRecord.LoadPNG(RECTOP_PNG_STARTRECORD);
	m_btn_StartRecord.SetHoverEffectColor(20, 255, 255, 255);
	m_btn_StartRecord.SetBackgroundColor(RGB(37, 39, 46));
	m_btn_StartRecord.ShowWindow(SW_SHOW);
	m_btn_StartRecord.SetStretchMode(0.90f);
	m_btn_StartRecord.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_btn_StartRecord.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"录制");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_btn_StartRecord.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"录制");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	m_btn_Recording.LoadPNG(RECTOP_PNG_RECORDING);
	m_btn_Recording.SetHoverEffectColor(20, 255, 255, 255);
	m_btn_Recording.SetBackgroundColor(RGB(37, 39, 46));
	m_btn_Recording.ShowWindow(SW_HIDE);
	m_btn_Recording.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_btn_Recording.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"停止录制");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_btn_Recording.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"停止录制");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	m_btn_pause.LoadPNG(RECTOPDLG_PNG_PAUSE);
	m_btn_pause.SetHoverEffectColor(20, 255, 255, 255);
	m_btn_pause.SetBackgroundColor(RGB(37, 39, 46));
	m_btn_pause.ShowWindow(SW_SHOW);
	m_btn_pause.SetStretchMode(0.90f);
	m_btn_pause.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_btn_pause.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"暂停");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_btn_pause.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"暂停");
			m_toolTips.ShowWindow(SW_HIDE);
		});

	m_btn_resumeRecord.LoadPNG(TOPREC_PNG_RESUMERECORD);
	m_btn_resumeRecord.SetHoverEffectColor(20, 255, 255, 255);
	m_btn_resumeRecord.SetBackgroundColor(RGB(37, 39, 46));
	m_btn_resumeRecord.ShowWindow(SW_HIDE);
	m_btn_resumeRecord.SetStretchMode(0.90f);
	m_btn_resumeRecord.SetBtnHoverCallBack([this]()
		{
			CRect nailRect;
			m_btn_resumeRecord.GetWindowRect(nailRect);
			m_toolTips.LarSetTipText(L"恢复录制");
			m_toolTips.ShowToolTipsWindow(nailRect.left, nailRect.bottom + 8 * m_Scale);
		});
	m_btn_resumeRecord.SetBtnLeaveCallBack([this]()
		{
			m_toolTips.LarSetTipText(L"恢复录制");
			m_toolTips.ShowWindow(SW_HIDE);
		});
}

void Ui_RecTopDlg::InitToolWindow(bool IsMinimalWithMain)
{
	// 设置窗口为始终置顶
	::SetWindowPos(
		this->GetSafeHwnd(),
		HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED
	);

	if (!IsMinimalWithMain)
	{
		::SetParent(m_hWnd, ::GetDesktopWindow());
		::SetWindowLongPtr(m_hWnd, GWLP_HWNDPARENT, (LONG_PTR)::GetDesktopWindow());
	}
}

void Ui_RecTopDlg::LoadRes()
{
}

void Ui_RecTopDlg::CreateMenu()
{
	if (!m_Menu_Transp)
	{
		m_Menu_Transp = new Ui_PopupMenuDlg(31 * m_Scale, 96 * m_Scale,
			Color(36, 37, 40), Color(73, 73, 73), LOGINDLG_PNG_GOU);
		m_Menu_Transp->Create(IDD_DIALOG_MENU, NULL);
		for (size_t i = 0; i < 5; i++)
		{
			CString btnText;
			int percent = 100 - i * 20;
			btnText.Format(L"%d%%", percent);
			m_Menu_Transp->AddBtn(
				btnText,
				Color(255, 255, 255, 255), Color(255, 255, 255, 255), Color(255, 255, 255, 255),
				Color(255, 36, 37, 40), Color(255, 64, 65, 70), Color(255, 64, 65, 70),
				[=]()
				{
					DB(ConsoleHandle, L"点击菜单按钮");
					SetOpacity(percent);
				},
				true
			);
		}
	}
}

void Ui_RecTopDlg::SetOpacity(int percent)
{
	BYTE alpha = static_cast<BYTE>(percent * 255 / 100);
	LONG ex = ::GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE);
	if (!(ex & WS_EX_LAYERED))
		::SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, ex | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(this->GetSafeHwnd(), 0, alpha, LWA_ALPHA);
}

BEGIN_MESSAGE_MAP(Ui_RecTopDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()

	ON_BN_CLICKED(RECTOPDLG_BTN_TRANSPARENT, &Ui_RecTopDlg::OnBnClickedBtnTransparent)
	ON_BN_CLICKED(RECTOPDLG_BTN_NAIL, &Ui_RecTopDlg::OnBnClickedBtnNail)
	ON_BN_CLICKED(TOPRECDLG_BTN_STARTRECORDPNG, &Ui_RecTopDlg::OnBnClickedBtnStartrecordpng)
	ON_BN_CLICKED(TOPRECDLG_BTN_STARTRECORD, &Ui_RecTopDlg::OnBnClickedBtnStartrecord)
	ON_BN_CLICKED(TOPREC_BTN_RECORDING, &Ui_RecTopDlg::OnBnClickedBtnRecording)
	ON_BN_CLICKED(RECTOPDLG_BTN_SCREENPHOTO, &Ui_RecTopDlg::OnBnClickedBtnScreenphoto)
	ON_BN_CLICKED(RECTOPDLG_BTN_CLOSE, &Ui_RecTopDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(TOPREC_BTN_PAUSE, &Ui_RecTopDlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(TOPREC_BTN_RESUMERECORD,&Ui_RecTopDlg::OnBnClickedBtnResumerecord)
	ON_BN_CLICKED(RECTOPDLG_BTN_CLOSE, &Ui_RecTopDlg::OnBnClickedBtnClose)

	ON_MESSAGE(MSG_SCREENSHOTDLG_SHOTCOMPELETE, &Ui_RecTopDlg::OnBnScreenShotCompelete)
	ON_MESSAGE(MSG_CHILDDLG_TIMERCOUNTSTART, &Ui_RecTopDlg::OnTimerCountStart)
	ON_MESSAGE(MSG_CHILDDLG_STOPTIMERCOUNTANDSHOWNORMALUI, &Ui_RecTopDlg::OnStopTimeCountAndShowNormalUi)
END_MESSAGE_MAP()

// Ui_RecTopDlg 消息处理程序

BOOL Ui_RecTopDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

BOOL Ui_RecTopDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	LoadRes();
	GetUserDPI();
	UpdateScale();
	InitCtrl();
	CreateMenu();
	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口为工具窗口，并且不随着主窗口最小化而最小化
	InitToolWindow(false);
	m_Shadow.Create(m_hWnd);//创建阴影框架
	return TRUE; 
}

void Ui_RecTopDlg::OnPaint()
{
	CPaintDC dc(this); 
	m_Shadow.Show(m_hWnd);
	//预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
	Graphics memGraphics(&memBitmap);

	//绘画标题栏背景和绘画客户区背景
	SolidBrush BKBrush(Color(37, 39, 46));
	memGraphics.FillRectangle(//绘画客户区背景
		&BKBrush,
		0,
		0,
		m_WindowWidth,
		m_WindowHeight
	);

	//一次性绘画到窗口上
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));

	DB(ConsoleHandle, L"Ui_RecTopDlg:repaint..");
}

void Ui_RecTopDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Menu_Transp->ShowWindow(SW_HIDE);
	CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_RecTopDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	CDialogEx::OnMouseMove(nFlags, point);
}

void Ui_RecTopDlg::OnBnClickedBtnTransparent()
{
	CRect TranspRect;
	m_Btn_Transparent.GetWindowRect(TranspRect);
	m_Menu_Transp->ShowMenu(TranspRect.left, TranspRect.top + TranspRect.Height());
}

void Ui_RecTopDlg::OnBnClickedBtnNail()
{
	m_Menu_Transp->ShowWindow(SW_HIDE);
	m_IsHideDuringRecord = !m_IsHideDuringRecord;
	if (m_IsHideDuringRecord)//TOPREC_PNG_NONAIL
	{
		m_Btn_Nail.LoadPNG(TOPREC_PNG_NONAIL);
	}
	else
	{
		m_Btn_Nail.LoadPNG(TOPREC_PNG_NAIL);
	}
	Invalidate(false);
}

void Ui_RecTopDlg::OnBnClickedBtnStartrecordpng()
{
	if (Ui_RadarTimerSDL::IsInsExist() && Ui_RadarTimerSDL::IsWindowRunning())
		return;
	m_Menu_Transp->ShowWindow(SW_HIDE);
	DB(ConsoleHandle, L"点击了顶部窗口的开始录制按钮");
	HideAllTooltips();

	//补丁..
	if (m_ctxType == RecContextType::None)
	{
		if (App.m_Dlg_Main.m_Dlg_WindowRecord->IsWindowVisible())
		{
			m_ctxType = RecContextType::Window;
		}
		else if(App.m_Dlg_Main.m_Dlg_Child->IsWindowVisible())
		{
			m_ctxType = RecContextType::Child;
		}
	}

	if (m_ctxType == RecContextType::Window)
	{
		if (App.m_Dlg_Main.m_Dlg_WindowRecord &&
			::IsWindow(App.m_Dlg_Main.m_Dlg_WindowRecord->GetSafeHwnd()))
		{
			App.m_Dlg_Main.m_Dlg_WindowRecord->OnBnClickedBtnStartrecord();
		}
		return;
	}
	else if (m_ctxType == RecContextType::Child)
	{
		::PostMessage(App.m_Dlg_Main.m_Dlg_Child->GetSafeHwnd(), MSG_TOPRECDLG_STARTRECORD, NULL, NULL);
		DB(ConsoleHandle, L"向主窗口发送录制消息");
		if (m_IsHideDuringRecord)
		{
			DB(ConsoleHandle, L"顶部窗口隐藏");
			this->ShowWindow(SW_HIDE);
			return;
		}
	}
	else
	{
		// 无上下文时不做事
	}
}

void Ui_RecTopDlg::OnBnClickedBtnStartrecord()
{
	OnBnClickedBtnStartrecordpng();
}

LRESULT Ui_RecTopDlg::OnBnScreenShotCompelete(WPARAM wParam, LPARAM lParam)
{
	ModalDlg_MFC::ShowModal_ScreenShotCompelete();
	return 1;
}

LRESULT Ui_RecTopDlg::OnTimerCountStart(WPARAM wParam, LPARAM lParam)
{
	DB(ConsoleHandle, L"顶部窗口开始更新录制计时");
	if (m_IsHideDuringRecord)
	{
		ShowWindow(SW_HIDE);
		return 1;
	}
	m_btn_Recording.ShowWindow(SW_SHOW);
	m_btn_StartRecord.ShowWindow(SW_HIDE);
	m_btn_StartRecordText.ShowWindow(SW_HIDE);
	m_IsRecording = true;

	//重置暂停相关状态
	m_IsPaused = false;                                       
	m_PausedAccum = std::chrono::seconds::zero();                    
	m_Time_Start = std::chrono::steady_clock::now();
	m_Stat_TimeCount.LarSetText(L"录制中00:00:00");

	// 按钮可见性
	m_btn_pause.ShowWindow(SW_SHOW);                         
	m_btn_resumeRecord.ShowWindow(SW_HIDE);                

	if (!SetTimer(TIMER_TOPRECDLG_TIMECOUNT, 200, NULL))
		DB(ConsoleHandle, L"顶部窗口录制定时器开启失败！");
	DB(ConsoleHandle, L"顶部窗口更新录制计时器完成");
	return 1;
}

LRESULT Ui_RecTopDlg::OnStopTimeCountAndShowNormalUi(WPARAM wParam, LPARAM lParam)
{
	m_btn_Recording.ShowWindow(SW_HIDE);
	m_btn_StartRecord.ShowWindow(SW_SHOW);
	m_btn_StartRecordText.ShowWindow(SW_SHOW);
	m_IsRecording = false;
	KillTimer(TIMER_TOPRECDLG_TIMECOUNT);
	m_Stat_TimeCount.LarSetText(L"全屏录制");

	// 停止录制后，复位暂停状态与按钮
	m_IsPaused = false;                            
	m_PausedAccum = std::chrono::seconds::zero();  
	m_btn_pause.ShowWindow(SW_SHOW);               
	m_btn_resumeRecord.ShowWindow(SW_HIDE);        
	DB(ConsoleHandle, L"顶部窗口开始停止更新录制计时器，显示正常UI");
	return 1;
}

void Ui_RecTopDlg::OnBnClickedBtnRecording()
{
	DB(ConsoleHandle, L"顶部窗口停止录制按钮被点击");
	HideAllTooltips();
	m_btn_Recording.ShowWindow(SW_HIDE); 
	m_btn_StartRecord.ShowWindow(SW_SHOW);
	m_btn_StartRecordText.ShowWindow(SW_SHOW);
	UpdateWindow();
	m_IsRecording = false;
	KillTimer(TIMER_TOPRECDLG_TIMECOUNT);
	if (m_ctxType == RecContextType::Window)
	{
		if (App.m_Dlg_Main.m_Dlg_WindowRecord &&
			::IsWindow(App.m_Dlg_Main.m_Dlg_WindowRecord->GetSafeHwnd()))
		{
			App.m_Dlg_Main.m_Dlg_WindowRecord->OnBnClickedBtnStopWindowRecord(); 
		}
		return;
	}
	else if (m_ctxType == RecContextType::Child)
	{
		::PostMessage(App.m_Dlg_Main.m_Dlg_Child->GetSafeHwnd(), MSG_TOPRECDLG_STOPRECORD, NULL, NULL);
	}
	m_Stat_TimeCount.LarSetText(L"全屏录制");
	DB(ConsoleHandle, L"顶部窗口停止录制按钮点击按钮处理逻辑完成");
}

void Ui_RecTopDlg::OnBnClickedBtnScreenphoto()
{
	m_Menu_Transp->ShowWindow(SW_HIDE);

	// 全屏截取当前屏幕
	CRect rcScreen;
	::GetWindowRect(::GetDesktopWindow(), &rcScreen);
	HDC hScreenDC = ::GetDC(NULL);
	CDC screenDC;
	screenDC.Attach(hScreenDC);
	CDC memDC;
	memDC.CreateCompatibleDC(&screenDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&screenDC, rcScreen.Width(), rcScreen.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);
	memDC.BitBlt(0, 0, rcScreen.Width(), rcScreen.Height(),
		&screenDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
	memDC.DeleteDC();
	screenDC.Detach();
	::ReleaseDC(NULL, hScreenDC);
	HBITMAP hBmp = (HBITMAP)bmp.Detach();

	// 创建截屏效果窗口
	Ui_ScreenCaptureDlg* pDlg = new Ui_ScreenCaptureDlg(hBmp, this);
	pDlg->Create(IDD_DIALOG_SCREENCAPTURE, this);
	pDlg->ShowWindow(SW_SHOW);
}

void Ui_RecTopDlg::OnBnClickedBtnClose()
{
	m_Menu_Transp->ShowWindow(SW_HIDE);
	this->ShowWindow(SW_HIDE);
}

void Ui_RecTopDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_TOPRECDLG_TIMECOUNT)
	{
		DB(ConsoleHandle, L"顶部窗口的录制计时器触发响应，更新一次计时");

		//暂停时不刷新计时文本
		if (m_IsPaused) { CDialogEx::OnTimer(nIDEvent); return; } 

		//正向累计
		auto now = std::chrono::steady_clock::now();                    
		auto elapsed = now - m_Time_Start - m_PausedAccum;              
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
		if (seconds < 0) seconds = 0;                                   

		int hours = static_cast<int>(seconds / 3600);                    
		int minutes = static_cast<int>((seconds % 3600) / 60);           
		int secs = static_cast<int>(seconds % 60);                       

		m_stream.str(L"");
		m_stream.clear();
		m_stream << L"录制中"
			<< std::setfill(L'0') << std::setw(2) << hours << L":"
			<< std::setfill(L'0') << std::setw(2) << minutes << L":"
			<< std::setfill(L'0') << std::setw(2) << secs;
		CString BtnText = m_stream.str().c_str();
		m_Stat_TimeCount.LarSetText(BtnText);
	}
	CDialogEx::OnTimer(nIDEvent);
}

void Ui_RecTopDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	if (!bShow)
	{
		m_Menu_Transp->ShowWindow(SW_HIDE);
	}
}

void Ui_RecTopDlg::OnBnClickedBtnPause()
{
	if (m_ctxType == RecContextType::Window)
	{
		if (App.m_Dlg_Main.m_Dlg_WindowRecord &&
			::IsWindow(App.m_Dlg_Main.m_Dlg_WindowRecord->GetSafeHwnd()))
		{
			App.m_Dlg_Main.m_Dlg_WindowRecord->OnBnClickedBtnPauseWindowRecord(); 
		}
	}
	else if (m_ctxType == RecContextType::Child)
	{
		App.m_Dlg_Main.m_Dlg_Child->OnBnClickedBtnPause(); 
	}
	SetPauseUIMode();
}

void Ui_RecTopDlg::OnBnClickedBtnResumerecord()
{
	if (m_ctxType == RecContextType::Window)
	{
		if (App.m_Dlg_Main.m_Dlg_WindowRecord &&
			::IsWindow(App.m_Dlg_Main.m_Dlg_WindowRecord->GetSafeHwnd()))
		{
			App.m_Dlg_Main.m_Dlg_WindowRecord->OnBnClickedBtnResumeWindowRecord(); // 【新增调用】
		}
	}
	else if (m_ctxType == RecContextType::Child)
	{
		App.m_Dlg_Main.m_Dlg_Child->OnBnClickedBtnResume(); 
	}
	ResumeFromPauseUiMode();
}
