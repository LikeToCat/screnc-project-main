// Ui_ConfigDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_ConfigDlg.h"
#include "afxdialogex.h"
#include "CDebug.h"
#include "DeviceManager.h"
#include "CMessage.h"
#include "GlobalFunc.h"
#include "WasapiCapture.h"
#include "Ui_MainDlg.h"
#include "CMessage.h"
#include "AppCloseManager.h"
#include "theApp.h"
#include "Ui_NoneVipDlg.h"
#include "LarStringConversion.h"
#include "ConfigFileHandler.h"
extern HANDLE ConsoleHandle;
const CString DefautFpsConfig = L"24fps(默认最佳设置)";
int Ui_ConfigDlg::diffY = 0;
// Ui_ConfigDlg 对话框

IMPLEMENT_DYNAMIC(Ui_ConfigDlg, CDialogEx)

Ui_ConfigDlg::Ui_ConfigDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CONFIGDLG, pParent)
{
	m_NavClickType = FolderConfig;
}

Ui_ConfigDlg::~Ui_ConfigDlg()
{
	CleanupResources();                // 释放资源
}

void Ui_ConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, CONFIGDLG_BTN_TITLETEXT, m_Btn_TitleText);
	DDX_Control(pDX, CONFIGDLG_BTN_FOLDERCONFIG, m_Btn_FolderConfig);
	DDX_Control(pDX, CONFIGDLG_BTN_VIDEOCONFIG, m_Btn_VideoConfig);
	DDX_Control(pDX, CONFIGDLG_BTN_AUDIOCONFIG, m_Btn_AudioConfig);
	DDX_Control(pDX, CONFIGDLG_BTN_MOUSECONFIG, m_Btn_MouseConfig);
	DDX_Control(pDX, CONFIGDLG_STAT_FOLDERCONFIG, m_Stat_FolderConfig);
	DDX_Control(pDX, CONFIGDLG_STAT_SAVEFOLDER, m_Stat_SavePath);
	DDX_Control(pDX, CONFIGDLG_BTN_PATH, m_Btn_Path);
	DDX_Control(pDX, CONFIGDLG_BTN_FOLDERSELECT, m_Btn_FolderSelect);
	DDX_Control(pDX, CONFIGDLG_BTN_CLOSE, m_Btn_Close);
	DDX_Control(pDX, CONFIGDLG_STAT_VIDEOCONFIG, m_Stat_VideoConfig);
	DDX_Control(pDX, CONFIGDLG_STAT_FOLDERFORMAT, m_Stat_VideoFormat);
	DDX_Control(pDX, CONFIGDLG_BTN_VIDEOFORMAT, m_Btn_VideoFormat);
	DDX_Control(pDX, CONFIGDLG_BTN_GENERALCONFIG, m_Btn_GeneralConfig);
	DDX_Control(pDX, CONFIGDLG_BTN_VIDEOQUILATY, m_Btn_VideoQuality);
	DDX_Control(pDX, CONFIGDLG_STAT_MEASURE, m_Stat_Measure);
	DDX_Control(pDX, CONFIGDLG_BTN_FPS, m_Btn_Fps);
	DDX_Control(pDX, CONFIGDLG_STAT_FPS, m_Stat_Fps);
	DDX_Control(pDX, CONFIGDLG_BTN_VIDEOCODEC, m_Btn_VideoCodec);
	DDX_Control(pDX, CONFIGDLG_STAT_VIDEOCODEC, m_Stat_VideoCodec);
	DDX_Control(pDX, CONFIGDLG_BTN_VIDEOBITRATEMODE, m_Btn_VideoBitrateMode);
	DDX_Control(pDX, CONFIGDLG_STAT_VIDEOQUILATY, m_Stat_VideoQualityMode);
	DDX_Control(pDX, CONFIGDLG_BTN_VIDEOBITRATEPERCENT, m_Btn_VideoBitRatePercent);
	DDX_Control(pDX, CONFIGDLG_STAT_AUDIOPARAM, m_Stat_AudioConfig);
	DDX_Control(pDX, CONFIGDLG_BTN_AUDIOBITRATE, m_Btn_AudioSampleRate);
	DDX_Control(pDX, CONFIGDLG_STAT_SAMPLERATE, m_Stat_SampleRate);
	DDX_Control(pDX, CONFIGDLG_STAT_BITRATE, m_Stat_BitRate);
	DDX_Control(pDX, CONFIGDLG_BTN_AUDIOKBPS, m_Btn_BitRate);
	DDX_Control(pDX, CONFIGDLG_STAT_MOUSECONFIG, m_Stat_MouseConfig);
	DDX_Control(pDX, CONFIGDLG_STAT_GERNERALCONFIG, m_Stat_GeneralConfig);
	DDX_Control(pDX, CONFIGDLG_STAT_EXESTARTFIRST, m_Stat_StartupFirst);
	DDX_Control(pDX, CONFIGDLG_STAT_CLOSEEXE, m_Stat_CloseExe);
	DDX_Control(pDX, CONFIGDLG_STAT_MOUSEDISPLAY, m_Stat_MouseDisplay);
	DDX_Control(pDX, CONFIGDLG_BTN_AUDIOCAPTUREDEVICE, m_Btn_AudioDevice);
	DDX_Control(pDX, CONFIGDLG_STAT_AUDIODEVICE, m_Stat_AudioDevice);
	DDX_Control(pDX, CONFIGDLG_BTN_MICRODEVICE, m_Btn_MicroDevice);
	DDX_Control(pDX, CONFIGDLG_STAT_MICRODEVICE, m_Stat_MicroDevice);
	DDX_Control(pDX, CONFIGDLG_BTN_DISPLAYMOUSE, m_Btn_DisplayMouse);
	DDX_Control(pDX, CONFIGDLG_BTN_STARTUPFIRST, m_Btn_OpenStart);
	DDX_Control(pDX, CONFIGDLG_BTN_MINIALTOBAR, m_Btn_MInimalTobar);
	DDX_Control(pDX, CONFIGDLG_BTN_EXITEXE, m_Btn_QuitExe);
	DDX_Control(pDX, CONFIGDLG_BTN_WATER, m_Btn_Water);
}

BEGIN_MESSAGE_MAP(Ui_ConfigDlg, CDialogEx)
	ON_WM_NCACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(CONFIGDLG_BTN_VIDEOFORMAT, &Ui_ConfigDlg::OnBnClickedBtnVideoformat)
	ON_BN_CLICKED(CONFIGDLG_BTN_VIDEOQUILATY, &Ui_ConfigDlg::OnBnClickedBtnVideoquilaty)
	ON_BN_CLICKED(CONFIGDLG_BTN_FPS, &Ui_ConfigDlg::OnBnClickedBtnFps)
	ON_BN_CLICKED(CONFIGDLG_BTN_VIDEOCODEC, &Ui_ConfigDlg::OnBnClickedBtnVideocodec)
	ON_BN_CLICKED(CONFIGDLG_BTN_VIDEOBITRATEMODE, &Ui_ConfigDlg::OnBnClickedBtnVideobitratemode)
	ON_BN_CLICKED(CONFIGDLG_BTN_VIDEOBITRATEPERCENT, &Ui_ConfigDlg::OnBnClickedBtnVideobitratepercent)
	ON_BN_CLICKED(CONFIGDLG_BTN_AUDIOBITRATE, &Ui_ConfigDlg::OnBnClickedBtnAudiobitrate)
	ON_BN_CLICKED(CONFIGDLG_BTN_AUDIOKBPS, &Ui_ConfigDlg::OnBnClickedBtnAudiokbps)

	ON_MESSAGE(MSG_CLARLISTBOX_SELECTED, &Ui_ConfigDlg::OnBnClickedBtnListBoxSelected)
	ON_BN_CLICKED(CONFIGDLG_BTN_FOLDERCONFIG, &Ui_ConfigDlg::OnBnClickedBtnFolderconfig)
	ON_BN_CLICKED(CONFIGDLG_BTN_VIDEOCONFIG, &Ui_ConfigDlg::OnBnClickedBtnVideoconfig)
	ON_BN_CLICKED(CONFIGDLG_BTN_AUDIOCONFIG, &Ui_ConfigDlg::OnBnClickedBtnAudioconfig)
	ON_BN_CLICKED(CONFIGDLG_BTN_MOUSECONFIG, &Ui_ConfigDlg::OnBnClickedBtnMouseconfig)
	ON_BN_CLICKED(CONFIGDLG_BTN_GENERALCONFIG, &Ui_ConfigDlg::OnBnClickedBtnGeneralconfig)
	ON_BN_CLICKED(CONFIGDLG_BTN_CLOSE, &Ui_ConfigDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(CONFIGDLG_BTN_AUDIOCAPTUREDEVICE, &Ui_ConfigDlg::OnBnClickedBtnAudiocapturedevice)
	ON_BN_CLICKED(CONFIGDLG_BTN_MICRODEVICE, &Ui_ConfigDlg::OnBnClickedBtnMicrodevice)
	ON_BN_CLICKED(CONFIGDLG_BTN_FOLDERSELECT, &Ui_ConfigDlg::OnBnClickedBtnFolderselect)
	ON_WM_MOUSEMOVE()
	ON_WM_MOVE()
	ON_WM_EXITSIZEMOVE()
	ON_WM_ENTERSIZEMOVE()
	ON_BN_CLICKED(CONFIGDLG_BTN_DISPLAYMOUSE, &Ui_ConfigDlg::OnBnClickedBtnDisplaymouse)
	ON_BN_CLICKED(CONFIGDLG_BTN_STARTUPFIRST, &Ui_ConfigDlg::OnBnClickedBtnStartupfirst)
	ON_BN_CLICKED(CONFIGDLG_BTN_MINIALTOBAR, &Ui_ConfigDlg::OnBnClickedBtnMinialtobar)
	ON_BN_CLICKED(CONFIGDLG_BTN_EXITEXE, &Ui_ConfigDlg::OnBnClickedBtnExitexe)
	ON_BN_CLICKED(CONFIGDLG_BTN_WATER, &Ui_ConfigDlg::OnBnClickedBtnWater)
END_MESSAGE_MAP()

// Ui_ConfigDlg 消息处理程序

BOOL Ui_ConfigDlg::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//EnableShadow();
	return CDialogEx::OnNcActivate(bActive);
}

LRESULT Ui_ConfigDlg::OnNcHitTest(CPoint point)
{
	// 将屏幕坐标转换为客户区坐标
	CPoint clientPoint = point;
	ScreenToClient(&clientPoint);

	// 检查点是否在标题栏区域内
	if (m_Rect_TitleBar.Contains(Gdiplus::Point(clientPoint.x, clientPoint.y)))
	{
		// 点击在标题栏区域，返回HTCAPTION使窗口可拖动
		return HTCAPTION;
	}
	return CDialogEx::OnNcHitTest(point);
}

void Ui_ConfigDlg::OnPaint()
{
	CPaintDC dc(this);
	m_Shadow.Show(m_hWnd);
	//预缓冲Gdiplus对象
	using namespace Gdiplus;
	Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
	Graphics memGraphics(&memBitmap);

	//绘画标题栏背景和绘画客户区背景
	SolidBrush CaptionBrush(Color(26, 27, 32));
	SolidBrush ConfigAreaBrush(Color(26, 31, 37));
	SolidBrush NavBtnAreaBrush(Color(31, 39, 47));
	memGraphics.FillRectangle(&CaptionBrush, m_Rect_TitleBar);//绘画标题栏背景
	memGraphics.FillRectangle(&ConfigAreaBrush, m_Rect_ConfigArea);//绘画设置区域背景
	memGraphics.FillRectangle(&NavBtnAreaBrush, m_Rect_ConfigNavBtn);//绘画导航栏背景

	//绘画导航栏左侧蓝色竖杠
	switch (m_NavClickType)
	{
	case Ui_ConfigDlg::FolderConfig:
	{
		CRect rect;
		m_Btn_FolderConfig.GetWindowRect(rect);
		ScreenToClient(rect);
		DrawRoundedRectangle(&memGraphics, rect.left, rect.top, 7 * m_Scale, rect.Height(), 3);
	}
	break;
	case Ui_ConfigDlg::VideoConfig:
	{
		CRect rect;
		m_Btn_VideoConfig.GetWindowRect(rect);
		ScreenToClient(rect);
		DrawRoundedRectangle(&memGraphics, rect.left, rect.top, 7 * m_Scale, rect.Height(), 3);
	}
	break;
	case Ui_ConfigDlg::AudioConfig:
	{
		CRect rect;
		m_Btn_AudioConfig.GetWindowRect(rect);
		ScreenToClient(rect);
		DrawRoundedRectangle(&memGraphics, rect.left, rect.top, 7 * m_Scale, rect.Height(), 3);
	}
	break;
	case Ui_ConfigDlg::MouseConfig:
	{
		CRect rect;
		m_Btn_MouseConfig.GetWindowRect(rect);
		ScreenToClient(rect);
		DrawRoundedRectangle(&memGraphics, rect.left, rect.top, 7 * m_Scale, rect.Height(), 3);
	}
	break;
	case Ui_ConfigDlg::GeneralConfig:
	{
		CRect rect;
		m_Btn_GeneralConfig.GetWindowRect(rect);
		ScreenToClient(rect);
		DrawRoundedRectangle(&memGraphics, rect.left, rect.top, 7 * m_Scale, rect.Height(), 3);
	}
	break;
	default:
		break;
	}

	//一次性绘画到窗口上
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));

	DB(ConsoleHandle, L"Ui_ConfigDlg:repaint..");
}

HBRUSH Ui_ConfigDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

void Ui_ConfigDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}

BOOL Ui_ConfigDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	GetUserDPI();                      // 获取用户DPI
	LoadResources();                   // 加载资源
	UpdateScale();                     // 更新缩放
	InitializeUI();                    // 初始化界面	

	m_ListBoxs.SetListBoxHideWhenMouseLeave(false);// 设置当鼠标离开下拉框时，下拉框不隐藏
	m_ListBoxs.SetTextSize(14, L"");
	m_ListBoxs.SetScrollbarWidth(6 * m_Scale);

	//设置窗口双缓冲
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	LONG lStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	SetWindowLong(m_hWnd, GWL_EXSTYLE, lStyle | WS_EX_LAYERED);

	//设置窗口阴影效果
	LONG nStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	LONG newStyle = nStyle & (~(WS_CAPTION | WS_BORDER | WS_THICKFRAME));
	SetWindowLong(m_hWnd, GWL_STYLE, newStyle);
	m_Shadow.Create(m_hWnd);
	m_Shadow.SetShadowTopmost();
	return TRUE;
}

void Ui_ConfigDlg::OnCancel()
{
	//CDialogEx::OnCancel();
}

BOOL Ui_ConfigDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void Ui_ConfigDlg::GetUserDPI()
{
	// 获取显示器DPI
	HDC hdc = ::GetDC(NULL);
	int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
	::ReleaseDC(NULL, hdc);

	// 计算缩放系数（相对于96DPI）
	m_Scale = dpiX / 96.0f;
}

void Ui_ConfigDlg::UpdateScale()
{
	// 设定窗口大小为 696x686 像素
	int windowWidth = 696 * m_Scale;
	int windowHeight = 686 * m_Scale;
	// 如果 m_CRect_WindowRect 已定义，则使用其尺寸，否则使用默认尺寸
	if (m_CRect_WindowRect.Width() > 0 && m_CRect_WindowRect.Height() > 0) {
		windowWidth = m_CRect_WindowRect.Width();
		windowHeight = m_CRect_WindowRect.Height();
	}
	else {
		m_CRect_WindowRect.SetRect(0, 0, windowWidth, windowHeight);
	}
	SetWindowPos(NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);

	//窗口标题栏区域
	m_Rect_TitleBar.X = 0;
	m_Rect_TitleBar.Y = 0;
	m_Rect_TitleBar.Width = 696 * m_Scale;
	m_Rect_TitleBar.Height = 35 * m_Scale;

	//窗口左侧导航按钮区域
	m_Rect_ConfigNavBtn.X = 0;
	m_Rect_ConfigNavBtn.Y = m_Rect_TitleBar.GetBottom();
	m_Rect_ConfigNavBtn.Height = m_WindowHeight - m_Rect_TitleBar.Height;
	m_Rect_ConfigNavBtn.Width = 0.126 * m_WindowWidth;

	//录制参数设置区域
	m_Rect_ConfigArea.X = m_Rect_ConfigNavBtn.Width;
	m_Rect_ConfigArea.Y = m_Rect_TitleBar.GetBottom();
	m_Rect_ConfigArea.Width = m_WindowWidth - m_Rect_ConfigNavBtn.Width;
	m_Rect_ConfigArea.Height = m_WindowHeight - m_Rect_TitleBar.Height;

	//设置（标题）
	float TitleTextWidth = 47 * m_Scale;
	float TitleTextHeight = 18 * m_Scale;
	float TitleTextX = 10 * m_Scale;
	float TitleTextY = 10 * m_Scale;
	m_Btn_TitleText.MoveWindow(TitleTextX, TitleTextY, TitleTextWidth, TitleTextHeight);

	//设置关闭按钮
	float CloseBtnWidth = 28 * m_Scale;
	float CloseBtnHeight = 28 * m_Scale;
	float CloseBtnX = m_WindowWidth - CloseBtnWidth - 10 * m_Scale;
	float CloseBtnY = 5 * m_Scale;
	m_Btn_Close.MoveWindow(CloseBtnX, CloseBtnY, CloseBtnWidth, CloseBtnHeight);

	////设置导航栏区域的按钮
	//设置保存目录
	float FolderBtnWidth = m_Rect_ConfigNavBtn.Width;
	float FolderBtnHeight = 17 * m_Scale;
	float FolderBtnX = m_Rect_ConfigNavBtn.X + (m_Rect_ConfigNavBtn.Width - FolderBtnWidth) / 2;
	float FolderBtnY = m_Rect_ConfigNavBtn.Y + 10 * m_Scale;
	m_Btn_FolderConfig.MoveWindow(FolderBtnX, FolderBtnY, FolderBtnWidth, FolderBtnHeight);

	float NavBtnSpacing = 15 * m_Scale;//导航栏按钮上下间距
	m_Btn_VideoConfig.MoveWindow(
		FolderBtnX, FolderBtnY + FolderBtnHeight + NavBtnSpacing,
		FolderBtnWidth, FolderBtnHeight);	//设置视频参数按钮
	m_Btn_AudioConfig.MoveWindow(
		FolderBtnX, FolderBtnY + (FolderBtnHeight + NavBtnSpacing) * 2,
		FolderBtnWidth, FolderBtnHeight);	//设置音频参数按钮
	m_Btn_MouseConfig.MoveWindow(
		FolderBtnX, FolderBtnY + (FolderBtnHeight + NavBtnSpacing) * 3,
		FolderBtnWidth, FolderBtnHeight);	//设置鼠标设置按钮
	m_Btn_GeneralConfig.MoveWindow(
		FolderBtnX, FolderBtnY + (FolderBtnHeight + NavBtnSpacing) * 4,
		FolderBtnWidth, FolderBtnHeight
	);//设置通用设置按钮

	////设置区域控件位置
	//保存目录导航文字
	float FolderConfigTextWidth = 58 * m_Scale;
	float FolderConfigTextHeight = 22 * m_Scale;
	float FolderConfigTextX = m_Rect_ConfigNavBtn.Width + 0.022 * m_Rect_ConfigArea.Width;
	float FolderConfigTextY = 46 * m_Scale;
	m_Stat_FolderConfig.MoveWindow(FolderConfigTextX, FolderConfigTextY + 12 * m_Scale, FolderConfigTextWidth + 10 * m_Scale, FolderConfigTextHeight);

	float TextSpacing = 20 * m_Scale;//设置区域中每个按钮上下间距

	float SavePathY = FolderConfigTextY + FolderConfigTextHeight + TextSpacing;
	m_Stat_SavePath.MoveWindow(FolderConfigTextX, SavePathY,
		FolderConfigTextWidth, FolderConfigTextHeight);	//存储目录文字

	//路径框按钮
	float PathBtnY = SavePathY;
	float PathBtnX = FolderConfigTextX + FolderConfigTextWidth + 10 * m_Scale;
	float PathBtnWidth = 286 * m_Scale;
	float PathBtnHeight = 25 * m_Scale;
	m_Btn_Path.MoveWindow(PathBtnX, PathBtnY, PathBtnWidth, PathBtnHeight);


	//存储目录选择按钮 
	float PathSelectWidth = 20 * m_Scale;
	float PathSelectHeight = 20 * m_Scale;
	float PathSelectX = PathBtnX + PathBtnWidth + 10 * m_Scale;
	float PathSelectY = PathBtnY + (PathBtnHeight - PathSelectHeight) / 2;
	m_Btn_FolderSelect.MoveWindow(PathSelectX, PathSelectY, PathSelectWidth, PathSelectHeight);

	//视频参数导航文字Y坐标
	float VideoConfigY = SavePathY + FolderConfigTextHeight + 60 * m_Scale;
	//保存目录和视频参数设置区域的分界线
	m_Boundary1.p1.X = FolderConfigTextX;
	m_Boundary1.p1.Y = PathSelectY + PathSelectHeight + (VideoConfigY - PathSelectY) / 2;
	m_Boundary1.p2.X = FolderConfigTextX + m_WindowWidth - 30 * m_Scale;
	m_Boundary1.p2.Y = m_Boundary1.p1.Y;

	//视频参数导航文字
	m_Stat_VideoConfig.MoveWindow(FolderConfigTextX, VideoConfigY + 12 * m_Scale,
		FolderConfigTextWidth + 10 * m_Scale, FolderConfigTextHeight);

	//文件格式
	float VideoFormatY = VideoConfigY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_VideoFormat.MoveWindow(FolderConfigTextX, VideoFormatY,
		FolderConfigTextWidth, FolderConfigTextHeight);

	//视频格式
	float VideoFormatBtnWidth = PathBtnWidth;
	float VideoFormatBtnHeight = PathBtnHeight;
	float VideoFormatBtnX = PathBtnX;
	float VideoFormatBtnY = VideoFormatY;
	m_Btn_VideoFormat.MoveWindow(VideoFormatBtnX, VideoFormatBtnY, VideoFormatBtnWidth, VideoFormatBtnHeight);

	//尺寸
	float MeasureWidth = FolderConfigTextWidth;
	float MeasureHeight = FolderConfigTextHeight;
	float MeasureX = FolderConfigTextX;
	float MeasureY = VideoFormatY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_Measure.MoveWindow(MeasureX, MeasureY, MeasureWidth, MeasureHeight);

	//录制分辨率下拉框
	float VideoQualityBtnWidth = PathBtnWidth;
	float VideoQualityBtnHeight = PathBtnHeight;
	float VideoQualityBtnX = PathBtnX;
	float VideoQualityBtnY = MeasureY;
	m_Btn_VideoQuality.MoveWindow(VideoQualityBtnX, VideoQualityBtnY,
		VideoQualityBtnWidth, VideoQualityBtnHeight);

	//帧率
	float FpsTextWidth = FolderConfigTextWidth;
	float FpsTextHeight = FolderConfigTextHeight;
	float FpsTextX = FolderConfigTextX;
	float FpsTextY = MeasureY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_Fps.MoveWindow(FpsTextX, FpsTextY,
		FpsTextWidth, FpsTextHeight
	);

	//帧率下拉框
	float FpsBtnWidth = PathBtnWidth;
	float FpsBtnHeight = PathBtnHeight;
	float FpsBtnX = PathBtnX;
	float FpsBtnY = FpsTextY;
	m_Btn_Fps.MoveWindow(FpsBtnX, FpsBtnY,
		FpsBtnWidth, FpsBtnHeight);

	//视频编码器
	float VideoCodecTextWidth = FolderConfigTextWidth + 7 * m_Scale;
	float VideoCodecTextHeight = FolderConfigTextHeight;
	float VideoCodecTextX = FolderConfigTextX;
	float VideoCodecTextY = FpsTextY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_VideoCodec.MoveWindow(VideoCodecTextX, VideoCodecTextY,
		VideoCodecTextWidth, VideoCodecTextHeight
	);

	//视频编码器下拉框
	float VideoCodecBtnWidth = PathBtnWidth;
	float VideoCodecBtnHeight = PathBtnHeight;
	float VideoCodecBtnX = PathBtnX;
	float VideoCodecBtnY = VideoCodecTextY;
	m_Btn_VideoCodec.MoveWindow(VideoCodecBtnX, VideoCodecBtnY,
		VideoCodecBtnWidth, VideoCodecBtnHeight);

	//画质
	float VideoQualityModeTextWidth = FolderConfigTextWidth;
	float VideoQualityModeTextHeight = FolderConfigTextHeight;
	float VideoQualityModeTextX = FolderConfigTextX;
	float VideoQualityModeTextY = VideoCodecTextY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_VideoQualityMode.MoveWindow(VideoQualityModeTextX, VideoQualityModeTextY,
		VideoQualityModeTextWidth, VideoQualityModeTextHeight);

	//画质下拉框
	float VideoQualityModeBtnWidth = PathBtnWidth;
	float VideoQualityModeBtnHeight = PathBtnHeight;
	float VideoQualityModeBtnX = PathBtnX;
	float VideoQualityModeBtnY = VideoQualityModeTextY;
	m_Btn_VideoBitrateMode.MoveWindow(VideoQualityModeBtnX, VideoQualityModeBtnY,
		VideoQualityModeBtnWidth, VideoQualityModeBtnHeight);

	//画质比特率百分比选择下拉框
	float VideoBitRatePercentBtnWidth = PathBtnWidth / 2;
	float VideoBitRatePercentBtnHeight = PathBtnHeight;
	float VideoBitRatePercentBtnX = PathBtnX + VideoQualityModeBtnWidth + 10 * m_Scale;
	float VideoBitRatePercentBtnY = VideoQualityModeBtnY;
	m_Btn_VideoBitRatePercent.MoveWindow(VideoBitRatePercentBtnX, VideoBitRatePercentBtnY,
		VideoBitRatePercentBtnWidth, VideoBitRatePercentBtnHeight);

	//音频参数Y坐标
	float AudioConfigTextY = VideoQualityModeTextY + FolderConfigTextHeight + 32 * m_Scale;
	//视频参数区域和音频参数区域分界线 
	m_Boundary2.p1.X = m_Boundary1.p1.X;
	m_Boundary2.p1.Y = VideoQualityModeTextY + VideoBitRatePercentBtnHeight + (AudioConfigTextY - VideoQualityModeTextY) / 2;
	m_Boundary2.p2.X = m_Boundary1.p2.X;
	m_Boundary2.p2.Y = m_Boundary2.p1.Y;

	//音频参数
	float AudioConfigTextWidth = FolderConfigTextWidth + 5 * m_Scale;
	float AudioConfigTextHeight = FolderConfigTextHeight;
	float AudioConfigTextX = FolderConfigTextX;
	m_Stat_AudioConfig.MoveWindow(AudioConfigTextX, AudioConfigTextY + 16 * m_Scale,
		AudioConfigTextWidth, AudioConfigTextHeight);

	//采样率
	float SampleRateTextWidth = FolderConfigTextWidth;
	float SampleRateTextHeight = FolderConfigTextHeight;
	float SampleRateTextX = FolderConfigTextX;
	float SampleRateTextY = AudioConfigTextY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_SampleRate.MoveWindow(SampleRateTextX, SampleRateTextY,
		SampleRateTextWidth, SampleRateTextHeight);

	//采样率下拉框
	float AudioSampleRateBtnWidth = PathBtnWidth;
	float AudioSampleRateBtnHeight = PathBtnHeight;
	float AudioSampleRateBtnX = SampleRateTextX + SampleRateTextWidth + 10 * m_Scale;
	float AudioSampleRateBtnY = SampleRateTextY;
	m_Btn_AudioSampleRate.MoveWindow(AudioSampleRateBtnX, AudioSampleRateBtnY,
		AudioSampleRateBtnWidth, AudioSampleRateBtnHeight);

	//比特率
	float BitRateTextWidth = FolderConfigTextWidth;
	float BitRateTextHeight = FolderConfigTextHeight;
	float BitRateTextX = FolderConfigTextX;
	float BitRateTextY = SampleRateTextY + FolderConfigTextHeight + 22 * m_Scale;
	m_Stat_BitRate.MoveWindow(BitRateTextX, BitRateTextY,
		BitRateTextWidth, BitRateTextHeight);

	// 比特率下拉框
	float BitRateBtnWidth = PathBtnWidth;
	float BitRateBtnHeight = PathBtnHeight;
	float BitRateBtnX = BitRateTextX + SampleRateTextWidth + 10 * m_Scale;
	float BitRateBtnY = BitRateTextY;
	m_Btn_BitRate.MoveWindow(BitRateBtnX, BitRateBtnY,
		BitRateBtnWidth, BitRateBtnHeight);

	//音频设备
	float AudioDeviceStatWidth = FolderConfigTextWidth;
	float AudioDeviceStatHeight = FolderConfigTextHeight;
	float AudioDeviceStatX = FolderConfigTextX;
	float AudioDeviceStatY = BitRateBtnY + BitRateBtnHeight + 22 * m_Scale;
	m_Stat_AudioDevice.MoveWindow(AudioDeviceStatX, AudioDeviceStatY,
		AudioDeviceStatWidth, AudioDeviceStatHeight);

	// 音频设备下拉框
	float AudioDeviceBtnWidth = PathBtnWidth;
	float AudioDeviceBtnHeight = PathBtnHeight;
	float AudioDeviceX = BitRateBtnX;
	float AudioDeviceY = AudioDeviceStatY;
	m_Btn_AudioDevice.MoveWindow(AudioDeviceX, AudioDeviceY,
		AudioDeviceBtnWidth, AudioDeviceBtnHeight);

	//麦克风设备
	float MicroDeviceStatWidth = FolderConfigTextWidth + 20 * m_Scale;
	float MicroDeviceStatHeight = FolderConfigTextHeight;
	float MicroDeviceStatX = FolderConfigTextX - 10 * m_Scale;
	float MicroDeviceStatY = AudioDeviceY + AudioDeviceBtnHeight + 22 * m_Scale;
	m_Stat_MicroDevice.MoveWindow(MicroDeviceStatX, MicroDeviceStatY,
		MicroDeviceStatWidth, MicroDeviceStatHeight);

	// 麦克风设备下拉框
	float MicroDeviceBtnWidth = PathBtnWidth;
	float MicroDeviceBtnHeight = PathBtnHeight;
	float MicroDeviceX = BitRateBtnX;
	float MicroDeviceY = MicroDeviceStatY;
	m_Btn_MicroDevice.MoveWindow(MicroDeviceX, MicroDeviceY,
		MicroDeviceBtnWidth, MicroDeviceBtnHeight);

	//鼠标设置Y坐标
	float MouseConfigTextY = MicroDeviceY + FolderConfigTextHeight + 22 * m_Scale;
	//音频参数设置区域与鼠标设置区域分界线
	m_Boundary3.p1.X = m_Boundary1.p1.X;
	m_Boundary3.p1.Y = BitRateBtnY + BitRateBtnHeight + (MouseConfigTextY - BitRateBtnY) / 2;
	m_Boundary3.p2.X = m_Boundary1.p2.X;
	m_Boundary3.p2.Y = m_Boundary3.p1.Y;

	//鼠标设置
	float MouseConfigTextWidth = FolderConfigTextWidth + 5 * m_Scale;
	float MouseConfigTextHeight = FolderConfigTextHeight;
	float MouseConfigTextX = FolderConfigTextX;
	m_Stat_MouseConfig.MoveWindow(MouseConfigTextX, MouseConfigTextY + 16 * m_Scale,
		MouseConfigTextWidth, MouseConfigTextHeight);

	//鼠标显示 
	float MouseDisplayTextWidth = FolderConfigTextWidth;
	float MouseDisplayTextHeight = FolderConfigTextHeight;
	float MouseDisplayTextX = FolderConfigTextX;
	float MouseDisplayTextY = MouseConfigTextY + FolderConfigTextHeight + 22 * m_Scale;;
	m_Stat_MouseDisplay.MoveWindow(MouseDisplayTextX, MouseDisplayTextY,
		MouseDisplayTextWidth, MouseDisplayTextHeight);

	//显示鼠标
	float DisplayMouseCkWidth = 75 * m_Scale;
	float DisplayMouseCkHeight = 18 * m_Scale;
	float DisplayMouseCkX = MouseDisplayTextX + MouseDisplayTextWidth + 27 * m_Scale;
	float DisplayMouseCkY = MouseDisplayTextY + (MouseDisplayTextHeight - DisplayMouseCkHeight) / 2;
	m_Btn_DisplayMouse.MoveWindow(DisplayMouseCkX, DisplayMouseCkY,
		DisplayMouseCkWidth, DisplayMouseCkHeight);

	//鼠标设置区域Y坐标
	float GeneralConfigTextY = MouseDisplayTextY + FolderConfigTextHeight + 22 * m_Scale;;
	//通用设置区域和鼠标设置区域的分界线
	m_Boundary4.p1.X = m_Boundary1.p1.X;
	m_Boundary4.p1.Y = DisplayMouseCkY + DisplayMouseCkHeight + (GeneralConfigTextY - DisplayMouseCkY) / 2;
	m_Boundary4.p2.X = m_Boundary1.p2.X;
	m_Boundary4.p2.Y = m_Boundary4.p1.Y;

	//通用设置
	float GeneralConfigTextWidth = FolderConfigTextWidth;
	float GeneralConfigTextHeight = FolderConfigTextHeight;
	float GeneralConfigTextX = FolderConfigTextX;
	m_Stat_GeneralConfig.MoveWindow(GeneralConfigTextX, GeneralConfigTextY + 16 * m_Scale,
		GeneralConfigTextWidth, GeneralConfigTextHeight);

	//开机启动
	float StartupFirstTextWidth = FolderConfigTextWidth;
	float StartupFirstTextHeight = FolderConfigTextHeight;
	float StartupFirstTextX = FolderConfigTextX;
	float StartupFirstTextY = GeneralConfigTextY + FolderConfigTextHeight + 22 * m_Scale;;
	m_Stat_StartupFirst.MoveWindow(StartupFirstTextX, StartupFirstTextY,
		StartupFirstTextWidth, StartupFirstTextHeight);

	//开机启动状态选择框
	float StartupFirstCkWidth = 75 * m_Scale;
	float StartupFirstCkHeight = 18 * m_Scale;
	float StartupFirstCkX = StartupFirstTextX + StartupFirstTextWidth + 27 * m_Scale;
	float StartupFirstCkY = StartupFirstTextY + (StartupFirstTextHeight - StartupFirstCkHeight) / 2;
	m_Btn_OpenStart.MoveWindow(StartupFirstCkX, StartupFirstCkY,
		StartupFirstCkWidth, StartupFirstCkHeight);

	//水印设置
	float WaterWidth = 85 * m_Scale;
	float WaterHeight = StartupFirstCkHeight;
	float WaterX = StartupFirstCkX + StartupFirstCkWidth + 10 * m_Scale;
	float WaterY = StartupFirstCkY;
	m_Btn_Water.MoveWindow(WaterX, WaterY, WaterWidth, WaterHeight);

	//关闭软件
	float CloseExeWidth = FolderConfigTextWidth;
	float CloseExeHeight = FolderConfigTextHeight;
	float CloseExeX = FolderConfigTextX;
	float CloseExeY = StartupFirstTextY + FolderConfigTextHeight + 22 * m_Scale;;
	m_Stat_CloseExe.MoveWindow(CloseExeX, CloseExeY,
		CloseExeWidth, CloseExeHeight);

	//最小化到托盘，不退出程序
	float MinimalToBarCkWidth = 92 * m_Scale;
	float MinimalToBarCkHeight = 18 * m_Scale;
	float MinimalToBarCkX = CloseExeX + CloseExeWidth + 26 * m_Scale;
	float MinimalToBarCkY = CloseExeY + (CloseExeHeight - MinimalToBarCkHeight) / 2;
	m_Btn_MInimalTobar.MoveWindow(MinimalToBarCkX, MinimalToBarCkY,
		MinimalToBarCkWidth, MinimalToBarCkHeight);

	//退出程序
	float CloseExeCkWidth = 80 * m_Scale;
	float CloseExeCkHeight = 18 * m_Scale;
	float CloseExeCkX = MinimalToBarCkX + MinimalToBarCkWidth + 10 * m_Scale;
	float CloseExeCkY = MinimalToBarCkY;
	m_Btn_QuitExe.MoveWindow(CloseExeCkX, CloseExeCkY,
		CloseExeCkWidth, CloseExeCkHeight);
}

void Ui_ConfigDlg::EnableShadow()
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

void Ui_ConfigDlg::SetButtonStyle(
	Gdiplus::Color BorderColor,
	Gdiplus::Color TextColor,
	Gdiplus::Color BkColor,
	Gdiplus::Color TextHoverColor,
	Gdiplus::Color TextClickColor,
	CLarBtn& larbtn,
	int mode)
{
	if (mode == 0) {
		Gdiplus::SolidBrush BkBrush(BkColor);
		larbtn.LarSetTextSize(19);
		larbtn.LarSetTextStyle(false, false, false);
		larbtn.LaSetTextColor(TextColor);
		larbtn.LarSetBorderColor(BorderColor);
		larbtn.LarSetEraseBkEnable(false);
		//larbtn.LarSetButtonNoInteraction(true);
		larbtn.LaSetTextHoverColor(TextHoverColor);
		larbtn.LaSetTextClickedColor(TextClickColor);
		larbtn.LarSetNormalFiilBrush(BkBrush);
	}
}

void Ui_ConfigDlg::ConfigAreaMove(int mode)
{
	// 【修改】统一由 deltaY 表示本次滚动要“加到控件Y坐标上的位移”，再做边界“夹紧”(clamp)
	// 原因：仅用“是否超过边界就直接return”的判断，可能因上次定位或像素取整导致轻微误差，从而在顶端仍然允许继续滚动一步。
	// 现在改为：先算出本次理想位移，再按上下边界分别裁剪，顶端只会“补齐到边界”，不会越界。
	const int step = static_cast<int>(65 * m_Scale);
	int deltaY = (mode == 1) ? +step : -step; // mode==1: 鼠标滚轮向上(让内容往下靠近顶部); mode==0: 鼠标滚轮向下(让内容向上)

	// 顶部锚点：以“保存目录”标题 m_Stat_FolderConfig 作为滚动区域的顶部参考
	CRect folderConfigRect;
	m_Stat_FolderConfig.GetWindowRect(folderConfigRect);
	ScreenToClient(folderConfigRect);

	// 顶部允许位置：标题栏底部 + 5*m_Scale（与各导航定位保持一致）
	const int topLimit = m_Rect_TitleBar.GetBottom() + static_cast<int>(5 * m_Scale);

	// 底部锚点：以“最小化到托盘”按钮作为底部参考（保持与原逻辑一致）
	CRect minimalRect;
	m_Btn_MInimalTobar.GetWindowRect(minimalRect);
	ScreenToClient(minimalRect);

	// 底部允许位置（与原逻辑一致，保留0.981窗口高度的阈值）
	const int bottomLimit = static_cast<int>(m_WindowHeight * 0.981);

	// 【修改】顶部边界夹紧：deltaY > 0 表示尝试让内容“向下补”，只允许补到 topLimit，不可越过
	if (deltaY > 0) {
		const int maxDown = topLimit - folderConfigRect.top; // 允许往下补的最大量（>=0 表示还没到顶）
		if (maxDown <= 0) {
			return; // 已经在顶端或轻微越界，禁止继续往下补
		}
		if (deltaY > maxDown) {
			deltaY = maxDown; // 只补齐到刚好贴顶
		}
	}
	// 【修改】底部边界夹紧：deltaY < 0 表示尝试往上滚，保持底部元素不超过 bottomLimit
	else if (deltaY < 0) {
		// 如果已经“很靠下”（底部在阈值之上/等于），禁止继续上滚
		if (minimalRect.bottom <= bottomLimit) {
			return;
		}
		// 计算本次最多允许向上滚动的量，使 bottom 恰好贴到 bottomLimit
		const int maxUp = bottomLimit - minimalRect.bottom; // 注意：这是一个非正数(<=0)
		if (deltaY < maxUp) {
			deltaY = maxUp; // 夹紧到刚好贴底阈值
		}
	}

	// 获取标题栏底部位置（用于隐藏被顶栏遮挡的控件）
	const int titleBarBottom = m_Rect_TitleBar.GetBottom();
	CRect rect;

	// 辅助：移动控件并根据是否进入标题栏区域决定显隐
	auto moveAndCheckVisibility = [&](CWnd& control) {
		control.GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 仅移动配置区域内的控件
		if (rect.left < m_Rect_ConfigArea.X)
			return;

		const int newTop = rect.top + deltaY; // 【修改】使用 deltaY，替代旧的“rect.top - moveDistance”

		control.MoveWindow(rect.left, newTop, rect.Width(), rect.Height());

		if (newTop < titleBarBottom) {
			control.ShowWindow(SW_HIDE);  // 进入标题栏区域则隐藏
		}
		else if (!control.IsWindowVisible()) {
			control.ShowWindow(SW_SHOW);  // 重新进入可视区域则显示
		}
		};

	// 【原逻辑】逐个控件移动（不变）
	moveAndCheckVisibility(m_Btn_Path);
	moveAndCheckVisibility(m_Btn_VideoFormat);
	moveAndCheckVisibility(m_Btn_VideoQuality);
	moveAndCheckVisibility(m_Btn_Fps);
	moveAndCheckVisibility(m_Btn_VideoCodec);
	moveAndCheckVisibility(m_Btn_VideoBitrateMode);
	moveAndCheckVisibility(m_Btn_VideoBitRatePercent);
	moveAndCheckVisibility(m_Btn_AudioSampleRate);
	moveAndCheckVisibility(m_Btn_BitRate);
	moveAndCheckVisibility(m_Btn_AudioDevice);
	moveAndCheckVisibility(m_Btn_MicroDevice);
	moveAndCheckVisibility(m_Btn_FolderSelect);
	moveAndCheckVisibility(m_Btn_DisplayMouse);
	moveAndCheckVisibility(m_Btn_OpenStart);
	moveAndCheckVisibility(m_Btn_MInimalTobar);
	moveAndCheckVisibility(m_Btn_QuitExe);
	moveAndCheckVisibility(m_Btn_Water);

	moveAndCheckVisibility(m_Stat_FolderConfig);
	moveAndCheckVisibility(m_Stat_SavePath);
	moveAndCheckVisibility(m_Stat_VideoFormat);
	moveAndCheckVisibility(m_Stat_VideoConfig);
	moveAndCheckVisibility(m_Stat_Measure);
	moveAndCheckVisibility(m_Stat_Fps);
	moveAndCheckVisibility(m_Stat_VideoCodec);
	moveAndCheckVisibility(m_Stat_VideoQualityMode);
	moveAndCheckVisibility(m_Stat_AudioConfig);
	moveAndCheckVisibility(m_Stat_SampleRate);
	moveAndCheckVisibility(m_Stat_BitRate);
	moveAndCheckVisibility(m_Stat_MouseConfig);
	moveAndCheckVisibility(m_Stat_GeneralConfig);
	moveAndCheckVisibility(m_Stat_StartupFirst);
	moveAndCheckVisibility(m_Stat_CloseExe);
	moveAndCheckVisibility(m_Stat_MouseDisplay);
	moveAndCheckVisibility(m_Stat_AudioDevice);
	moveAndCheckVisibility(m_Stat_MicroDevice);

	// 【保留】只repaint配置区域
	CRect configRect(
		m_Rect_ConfigArea.X,
		m_Rect_ConfigArea.Y,
		m_Rect_ConfigArea.X + m_Rect_ConfigArea.Width,
		m_Rect_ConfigArea.Y + m_Rect_ConfigArea.Height
	);
	InvalidateRect(configRect);
}

void Ui_ConfigDlg::InitializeUI()
{
	////-----------图标按钮
	//标题
	m_Btn_TitleText.LoadPNG(CONFIGDLG_PNG_TITLEICON);
	m_Btn_TitleText.SetBackgroundColor(RGB(26, 27, 32));

	//目录选择按钮
	m_Btn_FolderSelect.LoadPNG(CONFIGDLG_PNG_SELECTFOLDER);
	m_Btn_FolderSelect.SetBackgroundColor(RGB(26, 31, 37));

	//关闭
	m_Btn_Close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_Btn_Close.SetBackgroundColor(RGB(26, 27, 32));
	m_Btn_Close.SetHoverEffectColor(15, 255, 255, 255);
	m_Btn_Close.SetStretchMode(0.75f);

	DEBUG_CONSOLE_FMT(ConsoleHandle, L"m_Bool_MInimalTobar:%d,m_Btn_QuitExe:%d",
		m_Bool_MInimalTobar == true ? 1 : 0, m_Bool_QuitExe == true ? 1 : 0);

	//初始化状态控件
	if (!m_Bool_IsDisplayMouse)
	{
		m_Btn_DisplayMouse.LoadPNG(CONFIGDLG_PNG_DISPLAY);
		m_Btn_DisplayMouse.SetBackgroundColor(RGB(26, 31, 37));
		m_Btn_DisplayMouse.SetClickEffectColor(0, 26, 31, 37);
	}
	else
	{
		m_Btn_DisplayMouse.LoadPNG(CONFIGDLG_PNG_DISPLAYMOUSE_HOLD);
		m_Btn_DisplayMouse.SetBackgroundColor(RGB(26, 31, 37));
		m_Btn_DisplayMouse.SetClickEffectColor(0, 26, 31, 37);
	}


	if (!m_Bool_IsOpenStart)
	{
		m_Btn_OpenStart.LoadPNG(CONFIGDLG_PNG_OPENSTART);
		m_Btn_OpenStart.SetBackgroundColor(RGB(26, 31, 37));
		m_Btn_OpenStart.SetClickEffectColor(0, 26, 31, 37);
	}
	else
	{
		m_Btn_OpenStart.LoadPNG(CONFIGDLG_PNG_OPENSTART_HOLD);
		m_Btn_OpenStart.SetBackgroundColor(RGB(26, 31, 37));
		m_Btn_OpenStart.SetClickEffectColor(0, 26, 31, 37);
	}


	if (!m_Bool_MInimalTobar)
	{
		m_Btn_QuitExe.LoadPNG(CONFIGDLG_PNG_QUITEXE_HOLD);
		m_Btn_QuitExe.LoadClickPNG(CONFIGDLG_PNG_QUITEXE_HOLD);
		m_Btn_QuitExe.SetBackgroundColor(RGB(26, 31, 37));

		m_Btn_MInimalTobar.LoadPNG(CONFIGDLG_PNG_MINIMALTOBAR);
		m_Btn_MInimalTobar.LoadClickPNG(CONFIGDLG_PNG_MINIMALTOBAR);
		m_Btn_MInimalTobar.SetBackgroundColor(RGB(26, 31, 37));
	}
	else
	{
		m_Btn_QuitExe.LoadPNG(CONFIGDLG_PNG_QUITEXE);
		m_Btn_QuitExe.LoadClickPNG(CONFIGDLG_PNG_QUITEXE);
		m_Btn_QuitExe.SetBackgroundColor(RGB(26, 31, 37));

		m_Btn_MInimalTobar.LoadPNG(CONFIGDLG_PNG_MINIMALTOBAR_HOLD);
		m_Btn_MInimalTobar.LoadClickPNG(CONFIGDLG_PNG_MINIMALTOBAR_HOLD);
		m_Btn_MInimalTobar.SetBackgroundColor(RGB(26, 31, 37));
	}

	if (!App.m_IsVip)
		m_Bool_IsWaterOn = true;
	if (!m_Bool_IsWaterOn)
	{	//水印
		m_Btn_Water.LoadPNG(CONFIGDLG_PNG_NOWATERON);
		m_Btn_Water.SetBackgroundColor(RGB(26, 27, 32));
	}
	else
	{
		m_Btn_Water.LoadPNG(CONFIGDLG_PNG_NOWATEROFF);
		m_Btn_Water.SetBackgroundColor(RGB(26, 27, 32));
	}

	////---------------#

	////-------------------文本
	//视频格式文本
	m_Stat_VideoFormat.LarSetTextSize(19);
	m_Stat_VideoFormat.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_VideoFormat.LarSetTextStyle(false, false, false);

	//保存目录设置文本
	m_Stat_SavePath.LarSetTextSize(19);
	m_Stat_SavePath.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_SavePath.LarSetTextStyle(false, false, false);

	//提示设置界面为保存目录设置的文本
	m_Stat_FolderConfig.LarSetTextSize(19);
	m_Stat_FolderConfig.LarSetTextStyle(false, false, false);

	//视频参数文本
	m_Stat_VideoConfig.LarSetTextSize(19);
	m_Stat_VideoConfig.LarSetTextStyle(false, false, false);

	// 尺寸
	m_Stat_Measure.LarSetTextSize(19);
	m_Stat_Measure.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_Measure.LarSetTextStyle(false, false, false);

	// 帧率
	m_Stat_Fps.LarSetTextSize(19);
	m_Stat_Fps.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_Fps.LarSetTextStyle(false, false, false);

	// 视频编码器
	m_Stat_VideoCodec.LarSetTextSize(19);
	m_Stat_VideoCodec.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_VideoCodec.LarSetTextStyle(false, false, false);

	// 画质
	m_Stat_VideoQualityMode.LarSetTextSize(19);
	m_Stat_VideoQualityMode.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_VideoQualityMode.LarSetTextStyle(false, false, false);

	// 音频参数
	m_Stat_AudioConfig.LarSetTextSize(19);
	m_Stat_AudioConfig.LarSetTextStyle(false, false, false);

	// 采样率
	m_Stat_SampleRate.LarSetTextSize(19);
	m_Stat_SampleRate.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_SampleRate.LarSetTextStyle(false, false, false);

	// 比特率
	m_Stat_BitRate.LarSetTextSize(19);
	m_Stat_BitRate.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_BitRate.LarSetTextStyle(false, false, false);

	//音频设备文本
	m_Stat_AudioDevice.LarSetTextSize(19);
	m_Stat_AudioDevice.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_AudioDevice.LarSetTextStyle(false, false, false);

	// 鼠标设置
	m_Stat_MouseConfig.LarSetTextSize(19);
	m_Stat_MouseConfig.LarSetTextStyle(false, false, false);

	// 鼠标显示
	m_Stat_MouseDisplay.LarSetTextSize(19);
	m_Stat_MouseDisplay.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_MouseDisplay.LarSetTextStyle(false, false, false);

	m_Stat_MicroDevice.LarSetTextSize(19);
	m_Stat_MicroDevice.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_MicroDevice.LarSetTextStyle(false, false, false);

	// 通用设置
	m_Stat_GeneralConfig.LarSetTextSize(19);
	m_Stat_GeneralConfig.LarSetTextStyle(false, false, false);

	// 开机启动
	m_Stat_StartupFirst.LarSetTextSize(19);
	m_Stat_StartupFirst.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_StartupFirst.LarSetTextStyle(false, false, false);

	// 关闭软件
	m_Stat_CloseExe.LarSetTextSize(19);
	m_Stat_CloseExe.LarSetTextColor(RGB(129, 130, 133));
	m_Stat_CloseExe.LarSetTextStyle(false, false, false);

	////-------------------#

	////-------------------文本按钮
	Color TextColor(155, 209, 209, 209);//文本颜色
	Color TexthoverColor(255, 209, 209, 209);
	Color TextClickColor(255, 209, 209, 209);
	Color BorderColor(30, 39, 47); //边框颜色
	Color BkColor(0, 30, 39, 47);     //按钮背景颜色
	SetButtonStyle(BorderColor, TextColor, BkColor, TexthoverColor, TextClickColor, m_Btn_FolderConfig); // 保存目录
	SetButtonStyle(BorderColor, TextColor, BkColor, TexthoverColor, TextClickColor, m_Btn_VideoConfig);  // 视频参数
	SetButtonStyle(BorderColor, TextColor, BkColor, TexthoverColor, TextClickColor, m_Btn_AudioConfig);  // 音频参数
	SetButtonStyle(BorderColor, TextColor, BkColor, TexthoverColor, TextClickColor, m_Btn_MouseConfig);  // 鼠标设置
	SetButtonStyle(BorderColor, TextColor, BkColor, TexthoverColor, TextClickColor, m_Btn_GeneralConfig);// 通用设置


	 ////-------------------下拉框
	Color ComboBox_TextColor(211, 211, 211);//下拉框文本颜色
	Color ComboBox_BorderColor(73, 73, 73);//下拉框边框颜色
	Color ComboBox_BkColor(36, 37, 40);//下拉框背景颜色
	CString SavePath = GlobalFunc::GetDefaultVideoSavePath();
	m_Btn_Path.SetWindowText(SavePath);
	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_Path);//路径下拉框
	m_Btn_Path.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_VideoFormat);//视频格式下拉框 
	m_Btn_VideoFormat.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_VideoQuality);// 录制分辨率
	m_Btn_VideoQuality.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_VideoBitRatePercent);// 比特率百分比选择下拉框
	m_Btn_VideoBitRatePercent.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_Fps);// 帧率下拉框
	m_Btn_Fps.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_VideoCodec);// 视频编码器下拉框
	m_Btn_VideoCodec.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_VideoBitrateMode);// 画质下拉框
	m_Btn_VideoBitrateMode.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_AudioSampleRate);// 采样率下拉框
	m_Btn_AudioSampleRate.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_BitRate);// 比特率下拉框
	m_Btn_BitRate.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_AudioDevice);// 音频设备下拉框
	m_Btn_AudioDevice.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	SetButtonStyle(ComboBox_BorderColor, ComboBox_TextColor, ComboBox_BkColor, TexthoverColor, TextClickColor,
		m_Btn_MicroDevice);
	m_Btn_MicroDevice.LarSetHoverFillBrush(SolidBrush(Color(43, 44, 49)));

	////-------------------#

	////-----------创建下拉框
	//初始化下拉框选项
	InitDropListData();

	//计算滚动偏差
	CRect ClientBtnRect;
	m_Btn_VideoFormat.GetWindowRect(ClientBtnRect);
	this->ScreenToClient(ClientBtnRect);
	diffY = ClientBtnRect.top - m_Rect_TitleBar.Height;

	//计算每个每个按钮的宽高
	CRect btnRect;
	m_Btn_VideoFormat.GetWindowRect(btnRect);
	int btnHeight = btnRect.Height();
	int btnWidth = btnRect.Width();

	//计算视频下拉框初始位置
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_VideoFormat, L"视频格式下拉框");
	m_Btn_VideoFormat.SetWindowTextW(m_Array_VideoFormat.GetAt(0));

	//视频分辨率下拉框宽高设置
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_VideoPix, L"视频分辨率下拉框");
	m_Btn_VideoQuality.SetWindowTextW(m_Array_VideoPix.GetAt(0));

	//视频FPS下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_VideoFps, L"视频FPS下拉框");
	m_Btn_Fps.SetWindowTextW(m_Array_VideoFps.GetAt(3));

	//视频编码器下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_VideoCodec, L"视频编码器下拉框");
	//如果有硬件编码器，则设置硬件编码器
	if (m_Array_VideoCodec.GetCount() > 1)
	{
		m_Btn_VideoCodec.SetWindowTextW(m_Array_VideoCodec.GetAt(1));
	}
	else
	{
		m_Btn_VideoCodec.SetWindowTextW(m_Array_VideoCodec.GetAt(0));
	}

	//视频画质下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_VideoQuality, L"视频画质下拉框");
	m_Btn_VideoBitrateMode.SetWindowTextW(m_Array_VideoQuality.GetAt(1));

	//音频设备下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_AudioDevice, L"音频设备下拉框");
	m_Btn_AudioDevice.SetWindowTextW(m_Array_AudioDevice.GetAt(0));

	//麦克风设备下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_MicroDevice, L"麦克风设备下拉框");
	m_Btn_MicroDevice.SetWindowTextW(m_Array_MicroDevice.GetAt(0));

	//视频码率下拉框
	CRect VideoBitRatePercentBtnRect;
	m_Btn_VideoBitRatePercent.GetWindowRect(VideoBitRatePercentBtnRect);
	m_ListBoxs.addListBox(VideoBitRatePercentBtnRect.Width(), VideoBitRatePercentBtnRect.Height(), this, m_Array_VideoMbps, L"视频码率下拉框");
	m_Btn_VideoBitRatePercent.SetWindowTextW(m_Array_VideoMbps.GetAt(0));

	//音频采样率下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_AudioSampleRate, L"音频采样率下拉框");
	m_Btn_AudioSampleRate.SetWindowTextW(m_Array_AudioSampleRate.GetAt(3));

	//音频比特率下拉框
	m_ListBoxs.addListBox(btnWidth, btnHeight, this, m_Array_AudioMbps, L"音频比特率下拉框");
	m_Btn_BitRate.SetWindowTextW(m_Array_AudioMbps.GetAt(1));
}

void Ui_ConfigDlg::Ui_SetWindowRect(const CRect& rect)
{
	m_WindowX = rect.left;
	m_WindowY = rect.top;
	m_WindowWidth = rect.Width();
	m_WindowHeight = rect.Height();
	m_CRect_WindowRect.SetRect(m_WindowX, m_WindowY, m_WindowX + m_WindowWidth, m_WindowY + m_WindowHeight);
}

void Ui_ConfigDlg::Ui_UpdateWindowPos(const CRect& rect)
{
	m_WindowX = rect.left;
	m_WindowY = rect.top;
	m_WindowWidth = rect.right - rect.left;
	m_WindowHeight = rect.bottom - rect.top;
	m_CRect_WindowRect.SetRect(m_WindowX, m_WindowY, m_WindowX + m_WindowWidth, m_WindowY + m_WindowHeight);
	MoveWindow(m_WindowX, m_WindowY, m_WindowWidth, m_WindowHeight);
}

void Ui_ConfigDlg::CleanUpGdiPngRes()
{
	m_Btn_TitleText.ClearImages();
	m_Btn_FolderSelect.ClearImages();
	m_Btn_Close.ClearImages();
	m_Btn_DisplayMouse.ClearImages();
	m_Btn_OpenStart.ClearImages();
	m_Btn_MInimalTobar.ClearImages();
	m_Btn_QuitExe.ClearImages();
}

void Ui_ConfigDlg::LoadResources()
{
}

void Ui_ConfigDlg::CleanupResources()
{
}

void Ui_ConfigDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_ListBoxs.HideListBox();
	CDialogEx::OnLButtonDown(nFlags, point);
}

BOOL Ui_ConfigDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 处理滚动
	m_ListBoxs.HideListBox();
	if (zDelta > 0)
	{
		ConfigAreaMove(1);
	}
	else
	{
		ConfigAreaMove(0);
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void Ui_ConfigDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_DELAYED_REDRAW)
	{
		if (m_redrawTimerCount <= 10)
			KillTimer(TIMER_DELAYED_REDRAW);
		// 执行repaint
		Invalidate();
		UpdateWindow();
		m_redrawTimerCount++;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void Ui_ConfigDlg::InitDropListData()
{
	//视频格式选项
	m_Array_VideoFormat.Add(L"MP4");
	m_Array_VideoFormat.Add(L"AVI");
	m_Array_VideoFormat.Add(L"FLV");

	//视频分辨率选项
	m_Array_VideoPix.Add(L"当前屏幕分辨率");
	m_Array_VideoPix.Add(L"360P");
	m_Array_VideoPix.Add(L"480P");
	m_Array_VideoPix.Add(L"720P");
	m_Array_VideoPix.Add(L"1080P");
	m_Array_VideoPix.Add(L"2K");
	m_Array_VideoPix.Add(L"4K");

	//视频帧率选项
	m_Array_VideoFps.Add(L"5fps(最低性能占用)");
	m_Array_VideoFps.Add(L"10fps");
	m_Array_VideoFps.Add(L"20fps");
	m_Array_VideoFps.Add(L"24fps(默认最佳设置)");
	m_Array_VideoFps.Add(L"30fps");
	m_Array_VideoFps.Add(L"45fps");
	m_Array_VideoFps.Add(L"60fps(高性能电脑录制，高占用率)");
	m_Array_VideoFps.Add(L"90fps");
	m_Array_VideoFps.Add(L"120fps(超高性能电脑录制，超高占用率)");
	m_Array_VideoFps.Add(L"180fps(慎选！顶配电脑录制)");

	// 编码器选项
	VideoEncoderType Codec = DeviceManager::GetInstance().GetSupportedHWEncoder();
	m_Array_VideoCodec.Add(L"H264软件编码器(高CPU占用率,但最兼容)");

	switch (Codec)
	{
	case VideoEncoderType::NVIDIA_NVENC:
		m_Array_VideoCodec.Add(L"硬件加速NVIDIA NVENC(N卡用户首选)");
		break;
	case VideoEncoderType::AMD_AMF:
		m_Array_VideoCodec.Add(L"硬件加速AMD AMF(A卡用户首选)");
		break;
	case VideoEncoderType::INTEL_QSV:
		m_Array_VideoCodec.Add(L"硬件加速Intel QSV(I卡用户首选)");
		break;
	case VideoEncoderType::NVIDIA_CUDA:
		m_Array_VideoCodec.Add(L"硬件加速NVIDIA CUDA(特定场景适用)");
		break;
	default:
		break;
	}

	//画质选项
	m_Array_VideoQuality.Add(L"视频品质优先");
	m_Array_VideoQuality.Add(L"平衡");
	m_Array_VideoQuality.Add(L"视频帧率优先");

	//码率百分比
	m_Array_VideoMbps.Add(L"原画");
	m_Array_VideoMbps.Add(L"超清");
	m_Array_VideoMbps.Add(L"高清");
	m_Array_VideoMbps.Add(L"标清");

	// 音频采样率
	m_Array_AudioSampleRate.Add(L"8000HZ");
	m_Array_AudioSampleRate.Add(L"11025HZ");
	m_Array_AudioSampleRate.Add(L"22050HZ");
	m_Array_AudioSampleRate.Add(L"44100HZ");
	m_Array_AudioSampleRate.Add(L"48000HZ");

	//音频比特率
	m_Array_AudioMbps.Add(L"64Kbps");
	m_Array_AudioMbps.Add(L"128Kbps");
	m_Array_AudioMbps.Add(L"192Kbps");
	m_Array_AudioMbps.Add(L"256Kbps");
	m_Array_AudioMbps.Add(L"320Kbps");

	//音频捕获设备
	std::vector<CaptureDeviceInfo> AudioCaptures;
	if (WasapiCapture::GetInstance()->getCaptureDevicesInfo(&AudioCaptures) && AudioCaptures.size() > 0)
	{
		for (auto& AudioCapture : AudioCaptures)
		{
			const CString& AudioCaptureName = AudioCapture.deviceName.c_str();
			if (AudioCapture.isDefault)
			{
				CString defaultDeviceName;
				defaultDeviceName.Format(L"%s(默认)", AudioCaptureName);
				m_Array_AudioDevice.Add(defaultDeviceName);
			}
			else
			{
				m_Array_AudioDevice.Add(AudioCaptureName);
			}
		}
	}
	else
	{
		m_Array_AudioDevice.Add(L"无设备可用");
	}

	//麦克风设备
	auto MicroDevices = DeviceManager::GetInstance().GetMicrophoneDevices();
	if (MicroDevices.size() > 0)
	{//如果有麦克风设备
		for (auto& MicroDevice : MicroDevices)
		{
			m_Array_MicroDevice.Add(MicroDevice.nameW.c_str());
		}
	}
	else
	{
		m_Array_MicroDevice.Add(L"无设备可用");
	}
}

void Ui_ConfigDlg::TipOfOpenVip()
{
	CRect Rect, NoneVipRect;
	GetWindowRect(Rect);
	int NoneVipDlgWidth = 420 * m_Scale;
	int NoneVipDlgHeight = 220 * m_Scale;
	int NoneVipX = Rect.left + (Rect.Width() - NoneVipDlgWidth) / 2;
	int NoneVipY = Rect.top + (Rect.Height() - NoneVipDlgHeight) / 2;
	NoneVipRect.left = NoneVipX;
	NoneVipRect.top = NoneVipY;
	NoneVipRect.right = NoneVipX + NoneVipDlgWidth;
	NoneVipRect.bottom = NoneVipY + NoneVipDlgHeight;
	if (!m_Dlg_NoneVip)//630 330
	{
		m_Dlg_NoneVip = new Ui_MessageModalDlg;
		m_Dlg_NoneVip->Ui_SetWindowRect(NoneVipRect);
		if (!App.m_IsVip && !App.m_IsNonUserPaid)
		{
			m_Dlg_NoneVip->SetDefaultBtnGradientColor(
				Gdiplus::Color(255, 255, 232, 213), Gdiplus::Color(255, 224, 184, 140),
				Gdiplus::Color(255, 224, 184, 140), Gdiplus::Color(255, 255, 232, 213),
				Gdiplus::Color(255, 224, 184, 140), Gdiplus::Color(255, 255, 232, 213)
			);
		}
		m_Dlg_NoneVip->Create(IDD_DIALOG_NONEVIP, this);
	}
	m_Dlg_NoneVip->MoveWindow(NoneVipX, NoneVipY, NoneVipDlgWidth, NoneVipDlgHeight);
	m_Dlg_NoneVip->m_Stat_MessageType.SetWindowTextW(L"温馨提示");
	m_Dlg_NoneVip->m_Stat_AppText.SetWindowTextW(L"极速录屏大师");
	if (App.m_isLoginIn || true)//为了去掉这里的判断条件，这里加true（调整为未登录时也可看到支付页面并支付）
	{
		if (App.m_userInfo.isPaid)
		{
			if (App.m_userInfo.currentBindings > App.m_userInfo.maxBindings)
			{
				m_Dlg_NoneVip->m_Stat_MessageInfo.SetWindowTextW(L"您的设备绑定数量超出限制，请解绑多余设备");
				m_Dlg_NoneVip->m_Btn_OpenVip.SetWindowTextW(L"解绑设备");
			}
		}
		else
		{
			m_Dlg_NoneVip->m_Stat_MessageInfo.SetWindowTextW(L"您还不是VIP用户，请开通VIP后使用此功能");
			m_Dlg_NoneVip->m_Btn_OpenVip.SetWindowTextW(L"开通VIP");
		}
	}
	else
	{
		m_Dlg_NoneVip->m_Stat_MessageInfo.SetWindowTextW(L"您还还没有登录，需要登录后开通VIP才能使用");
		m_Dlg_NoneVip->m_Btn_OpenVip.SetWindowTextW(L"登录");
	}
	m_Dlg_NoneVip->SetTimer(1001, 500, NULL);
	m_Dlg_NoneVip->ShowWindow(SW_SHOW);
	this->EnableWindow(false);
}

void Ui_ConfigDlg::OnBnClickedBtnVideoformat()
{
	if (m_ListBoxs.IsListBoxVisiable(L"视频格式下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	else if(!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect VideoPixbtnRect;
	m_Btn_VideoFormat.GetWindowRect(&VideoPixbtnRect);
	m_ListBoxs.UpdateDroplistXY(L"视频格式下拉框", VideoPixbtnRect.left, VideoPixbtnRect.top + VideoPixbtnRect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnVideoquilaty()
{
	if (m_ListBoxs.IsListBoxVisiable(L"视频分辨率下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect VideoPixbtnRect;
	m_Btn_VideoQuality.GetWindowRect(&VideoPixbtnRect);
	m_ListBoxs.UpdateDroplistXY(L"视频分辨率下拉框", VideoPixbtnRect.left, VideoPixbtnRect.top + VideoPixbtnRect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnFps()
{
	if (m_ListBoxs.IsListBoxVisiable(L"视频FPS下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_Fps.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"视频FPS下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnVideocodec()
{
	if (m_ListBoxs.IsListBoxVisiable(L"视频编码器下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_VideoCodec.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"视频编码器下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnVideobitratemode()
{
	if (m_ListBoxs.IsListBoxVisiable(L"视频画质下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_VideoBitrateMode.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"视频画质下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnVideobitratepercent()
{
	if (m_ListBoxs.IsListBoxVisiable(L"视频码率下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_VideoBitRatePercent.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"视频码率下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnAudiobitrate()
{
	if (m_ListBoxs.IsListBoxVisiable(L"音频采样率下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_AudioSampleRate.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"音频采样率下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnAudiokbps()
{
	if (m_ListBoxs.IsListBoxVisiable(L"音频比特率下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	//更新下拉框的显示位置
	CRect Rect;
	m_Btn_BitRate.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"音频比特率下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnAudiocapturedevice()
{
	if (m_ListBoxs.IsListBoxVisiable(L"音频设备下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	CRect Rect;
	m_Btn_AudioDevice.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"音频设备下拉框", Rect.left, Rect.top + Rect.Height());
}

void Ui_ConfigDlg::OnBnClickedBtnMicrodevice()
{
	if (m_ListBoxs.IsListBoxVisiable(L"麦克风设备下拉框"))
	{
		m_ListBoxs.HideListBox();
		return;
	}
	if (!m_bool_IsEraseTopMost)
	{
		::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		m_bool_IsEraseTopMost = true;
	}
	CRect Rect;
	m_Btn_MicroDevice.GetWindowRect(&Rect);
	m_ListBoxs.UpdateDroplistXY(L"麦克风设备下拉框", Rect.left, Rect.top + Rect.Height());
}

LRESULT Ui_ConfigDlg::OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam)
{
	MsgParam::LISTBOX_SELECT_INFO* pInfo = (MsgParam::LISTBOX_SELECT_INFO*)wParam;
	if (pInfo)
	{
		int nIndex = pInfo->nIndex;
		CString strText = pInfo->strText;
		std::wstring strBoxName = pInfo->strBoxName;

		// 根据不同的下拉框名称进行不同处理
		if (strBoxName == L"视频格式下拉框")
		{
			m_Btn_VideoFormat.SetWindowText(strText);
		}
		else if (strBoxName == L"视频分辨率下拉框")
		{
			m_Btn_VideoQuality.SetWindowText(strText);
		}
		if (strBoxName == L"视频FPS下拉框")
		{
			m_Btn_Fps.SetWindowText(strText);
		}
		if (strBoxName == L"视频画质下拉框")
		{
			m_Btn_VideoBitrateMode.SetWindowText(strText);
		}
		else if (strBoxName == L"视频编码器下拉框")
		{
			m_Btn_VideoCodec.SetWindowText(strText);
		}
		if (strBoxName == L"视频码率下拉框")
		{
			m_Btn_VideoBitRatePercent.SetWindowText(strText);
		}
		else if (strBoxName == L"音频采样率下拉框")
		{
			m_Btn_AudioSampleRate.SetWindowText(strText);
		}
		if (strBoxName == L"音频比特率下拉框")
		{
			m_Btn_BitRate.SetWindowText(strText);
		}
		else if (strBoxName == L"音频设备下拉框")
		{
			m_Btn_AudioDevice.SetWindowText(strText);
		}
		if (strBoxName == L"麦克风设备下拉框")
		{
			m_Btn_MicroDevice.SetWindowText(strText);
		}
	}

	//判断是否点击了VIP功能设置
	if (AdjustVipMode())
	{
		TipOfOpenVip();
	}
	return 1;
}

bool Ui_ConfigDlg::AdjustVipMode()
{
	CString FpsStr;
	m_Btn_Fps.GetWindowTextW(FpsStr);
	int fps = GlobalFunc::ExtractFpsValue(FpsStr);
	if (fps <= 24)
		return false;

	bool IsUnLoginOrNoneVip = false;
	bool IsDeviceOverBind = false;
	if (!App.m_isLoginIn || !App.m_IsVip)
	{
		IsUnLoginOrNoneVip = true;
	}
	if (App.m_userInfo.currentBindings > App.m_userInfo.maxBindings)
	{
		IsDeviceOverBind = true;
	}

	if (IsUnLoginOrNoneVip || IsDeviceOverBind)
	{
		// 恢复fps设置
		CString FpsStr;
		m_Btn_Fps.GetWindowTextW(FpsStr);
		int fps = GlobalFunc::ExtractFpsValue(FpsStr);
		if (fps > 24)
		{
			m_Btn_Fps.SetWindowTextW(DefautFpsConfig);
		}
	}
	return (!IsUnLoginOrNoneVip && !IsDeviceOverBind) ? false : true;
}

bool Ui_ConfigDlg::JudgePathValid(std::string Path)
{
	// 如果路径为空，则需要管理员权限（默认为安全策略）
	if (Path.empty())
		return false;

	// 将UTF-8路径转换为宽字符串
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, nullptr, 0);
	std::wstring widePath(wideLen, 0);
	MultiByteToWideChar(CP_UTF8, 0, Path.c_str(), -1, &widePath[0], wideLen);

	// 标准化路径
	wchar_t fullPath[MAX_PATH] = { 0 };
	if (!GetFullPathNameW(widePath.c_str(), MAX_PATH, fullPath, nullptr)) {
		DWORD error = GetLastError();
		OutputDebugStringW(L"GetFullPathName failed with error: ");
		OutputDebugStringW(std::to_wstring(error).c_str());
		return false;
	}

	// 检查路径是否为空盘符根目录（如 "C:\"）
	if (wcslen(fullPath) == 3 && fullPath[1] == L':' && (fullPath[2] == L'\\' || fullPath[2] == L'/')) {
		OutputDebugStringW(L"Path is a root drive, likely needs admin: ");
		OutputDebugStringW(fullPath);
		return false;
	}

	std::wstring normalizedPath = fullPath;

	// 检查目录是否存在，不存在则检查父目录
	DWORD attrs = GetFileAttributesW(normalizedPath.c_str());
	std::wstring testDirPath;

	if (attrs != INVALID_FILE_ATTRIBUTES)
	{
		if (attrs & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 是目录，直接使用
			testDirPath = normalizedPath;
		}
		else
		{
			// 是文件，获取父目录
			size_t lastSlash = normalizedPath.find_last_of(L"\\/");
			if (lastSlash != std::wstring::npos) {
				testDirPath = normalizedPath.substr(0, lastSlash);
			}
			else {
				// 没有路径分隔符，使用当前目录
				testDirPath = L".";
			}
		}
	}
	else
	{
		// 路径不存在，尝试获取父目录
		size_t lastSlash = normalizedPath.find_last_of(L"\\/");
		if (lastSlash != std::wstring::npos) {
			testDirPath = normalizedPath.substr(0, lastSlash);
		}
		else {
			// 没有路径分隔符，使用当前目录
			testDirPath = L".";
		}
	}

	// 确认测试目录存在
	attrs = GetFileAttributesW(testDirPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
	{
		OutputDebugStringW(L"Test directory doesn't exist: ");
		OutputDebugStringW(testDirPath.c_str());
		return false;
	}

	OutputDebugStringW(L"Testing write permission on directory: ");
	OutputDebugStringW(testDirPath.c_str());

	// 创建唯一的临时文件名
	std::wstring testFilePath = testDirPath;
	if (testFilePath.back() != L'\\' && testFilePath.back() != L'/') {
		testFilePath += L"\\";
	}

	wchar_t tempFileName[MAX_PATH] = { 0 };
	UINT result = GetTempFileNameW(testDirPath.c_str(), L"perm", 0, tempFileName);
	if (result == 0)
	{
		DWORD error = GetLastError();
		OutputDebugStringW(L"GetTempFileName failed with error: ");
		OutputDebugStringW(std::to_wstring(error).c_str());

		// 如果是权限问题，则需要管理员权限
		return !(error == ERROR_ACCESS_DENIED ||
			error == ERROR_PATH_NOT_FOUND ||
			error == ERROR_DIRECTORY);
	}

	// GetTempFileName实际上已创建了文件，尝试写入
	HANDLE hFile = CreateFileW(
		tempFileName,
		GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,  // 文件应该已存在
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		OutputDebugStringW(L"CreateFile failed with error: ");
		OutputDebugStringW(std::to_wstring(error).c_str());
		DeleteFileW(tempFileName);// 删除GetTempFileName创建的空文件

		// 如果是权限问题，则需要管理员权限
		return !(error == ERROR_ACCESS_DENIED ||
			error == ERROR_SHARING_VIOLATION);
	}

	// 写入一些数据
	const char testData[] = "Testing write permissions";
	DWORD bytesWritten = 0;
	BOOL writeResult = WriteFile(
		hFile,
		testData,
		sizeof(testData) - 1,
		&bytesWritten,
		nullptr
	);


	CloseHandle(hFile);	// 关闭文件句柄
	DeleteFileW(tempFileName);// 删除测试文件
	if (!writeResult)
	{
		DWORD error = GetLastError();
		OutputDebugStringW(L"WriteFile failed with error: ");
		OutputDebugStringW(std::to_wstring(error).c_str());
		return false;
	}
	OutputDebugStringW(L"Path permission test succeeded");
	return true;
}

void Ui_ConfigDlg::OnBnClickedBtnFolderconfig()
{
	m_ListBoxs.HideListBox();
	// 获取文件夹配置标题控件的当前位置
	CRect folderConfigRect;
	m_Stat_FolderConfig.GetWindowRect(folderConfigRect);
	ScreenToClient(folderConfigRect);

	// 计算目标位置
	int targetTop = m_Rect_ConfigArea.Y + 5 * m_Scale;

	// 计算需要的垂直偏移量
	int offsetY = targetTop - folderConfigRect.top;

	// 移动配置区域中的所有控件
	CRect rect;
	auto moveControl = [&](CWnd& control) {
		control.GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 如果控件在配置区域内，则进行移动
		if (rect.left >= m_Rect_ConfigArea.X) {
			// 计算移动后的新位置
			int newTop = rect.top + offsetY;

			// 移动控件
			control.MoveWindow(rect.left, newTop, rect.Width(), rect.Height());

			// 检查移动后的控件是否与标题栏重叠
			if (newTop < m_Rect_TitleBar.GetBottom()) {
				control.ShowWindow(SW_HIDE); // 隐藏与标题栏重叠的控件
			}
			else {
				control.ShowWindow(SW_SHOW); // 显示不与标题栏重叠的控件
			}
		}
	};

	// 应用移动到所有相关控件
	moveControl(m_Stat_FolderConfig);
	moveControl(m_Stat_SavePath);
	moveControl(m_Btn_Path);
	moveControl(m_Btn_FolderSelect);
	moveControl(m_Stat_VideoConfig);
	moveControl(m_Stat_VideoFormat);
	moveControl(m_Btn_VideoFormat);
	moveControl(m_Stat_Measure);
	moveControl(m_Btn_VideoQuality);
	moveControl(m_Stat_Fps);
	moveControl(m_Btn_Fps);
	moveControl(m_Stat_VideoCodec);
	moveControl(m_Btn_VideoCodec);
	moveControl(m_Stat_VideoQualityMode);
	moveControl(m_Btn_VideoBitrateMode);
	moveControl(m_Btn_VideoBitRatePercent);
	moveControl(m_Stat_AudioConfig);
	moveControl(m_Stat_SampleRate);
	moveControl(m_Btn_AudioSampleRate);
	moveControl(m_Stat_BitRate);
	moveControl(m_Btn_BitRate);
	moveControl(m_Stat_MouseConfig);
	moveControl(m_Stat_MouseDisplay);
	moveControl(m_Btn_DisplayMouse);
	moveControl(m_Stat_GeneralConfig);
	moveControl(m_Stat_StartupFirst);
	moveControl(m_Btn_OpenStart);
	moveControl(m_Stat_CloseExe);
	moveControl(m_Btn_MInimalTobar);
	moveControl(m_Btn_QuitExe);
	moveControl(m_Btn_AudioDevice);
	moveControl(m_Btn_MicroDevice);
	moveControl(m_Stat_MicroDevice);
	moveControl(m_Stat_AudioDevice);
	moveControl(m_Btn_Water);

	// 更新分界线位置
	m_Boundary1.p1.Y += offsetY;
	m_Boundary1.p2.Y += offsetY;
	m_Boundary2.p1.Y += offsetY;
	m_Boundary2.p2.Y += offsetY;
	m_Boundary3.p1.Y += offsetY;
	m_Boundary3.p2.Y += offsetY;
	m_Boundary4.p1.Y += offsetY;
	m_Boundary4.p2.Y += offsetY;


	m_NavClickType = FolderConfig;
	// repaint整个窗口
	Invalidate(false);
}

void Ui_ConfigDlg::OnBnClickedBtnVideoconfig()
{
	m_ListBoxs.HideListBox();
	// 获取视频参数标题控件的当前位置
	CRect videoConfigRect;
	m_Stat_VideoConfig.GetWindowRect(videoConfigRect);
	ScreenToClient(videoConfigRect);

	// 计算目标位置：配置区域顶部加上5*m_Scale的偏移
	int targetTop = m_Rect_ConfigArea.Y + 5 * m_Scale;

	// 计算需要的垂直偏移量
	int offsetY = targetTop - videoConfigRect.top;

	// 移动配置区域中的所有控件
	CRect rect;
	auto moveControl = [&](CWnd& control) {
		control.GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 如果控件在配置区域内，则进行移动
		if (rect.left >= m_Rect_ConfigArea.X) {
			// 计算移动后的新位置
			int newTop = rect.top + offsetY;

			// 移动控件
			control.MoveWindow(rect.left, newTop, rect.Width(), rect.Height());

			// 检查移动后的控件是否与标题栏重叠
			if (newTop < m_Rect_TitleBar.GetBottom()) {
				control.ShowWindow(SW_HIDE); // 隐藏与标题栏重叠的控件
			}
			else {
				control.ShowWindow(SW_SHOW); // 显示不与标题栏重叠的控件
			}
		}
	};

	// 应用移动到所有相关控件
	moveControl(m_Stat_FolderConfig);
	moveControl(m_Stat_SavePath);
	moveControl(m_Btn_Path);
	moveControl(m_Btn_FolderSelect);
	moveControl(m_Stat_VideoConfig);
	moveControl(m_Stat_VideoFormat);
	moveControl(m_Btn_VideoFormat);
	moveControl(m_Stat_Measure);
	moveControl(m_Btn_VideoQuality);
	moveControl(m_Stat_Fps);
	moveControl(m_Btn_Fps);
	moveControl(m_Stat_VideoCodec);
	moveControl(m_Btn_VideoCodec);
	moveControl(m_Stat_VideoQualityMode);
	moveControl(m_Btn_VideoBitrateMode);
	moveControl(m_Btn_VideoBitRatePercent);
	moveControl(m_Stat_AudioConfig);
	moveControl(m_Stat_SampleRate);
	moveControl(m_Btn_AudioSampleRate);
	moveControl(m_Stat_BitRate);
	moveControl(m_Btn_BitRate);
	moveControl(m_Stat_MouseConfig);
	moveControl(m_Stat_MouseDisplay);
	moveControl(m_Btn_DisplayMouse);
	moveControl(m_Stat_GeneralConfig);
	moveControl(m_Stat_StartupFirst);
	moveControl(m_Btn_OpenStart);
	moveControl(m_Stat_CloseExe);
	moveControl(m_Btn_MInimalTobar);
	moveControl(m_Btn_QuitExe);
	moveControl(m_Btn_AudioDevice);
	moveControl(m_Btn_MicroDevice);
	moveControl(m_Stat_MicroDevice);
	moveControl(m_Stat_AudioDevice);
	moveControl(m_Btn_Water);

	// 更新分界线位置
	m_Boundary1.p1.Y += offsetY;
	m_Boundary1.p2.Y += offsetY;
	m_Boundary2.p1.Y += offsetY;
	m_Boundary2.p2.Y += offsetY;
	m_Boundary3.p1.Y += offsetY;
	m_Boundary3.p2.Y += offsetY;
	m_Boundary4.p1.Y += offsetY;
	m_Boundary4.p2.Y += offsetY;

	// 高亮显示当前选中的导航按钮
	Color ActiveBtnBorderColor(64, 153, 255);
	Color NormalBtnBorderColor(31, 39, 47);
	Color TextColor(209, 209, 209);
	Color BkColor(31, 39, 47);
	Color TextHoverColor(255, 255, 255, 255);
	Color TextClickColor(255, 255, 255, 255);

	m_NavClickType = VideoConfig;
	// repaint整个窗口
	Invalidate(false);
}

void Ui_ConfigDlg::OnBnClickedBtnAudioconfig()
{
	m_ListBoxs.HideListBox();
	// 获取音频参数标题控件的当前位置
	CRect audioConfigRect;
	m_Stat_AudioConfig.GetWindowRect(audioConfigRect);
	ScreenToClient(audioConfigRect);

	// 计算目标位置：配置区域顶部加上5*m_Scale的偏移
	int targetTop = m_Rect_ConfigArea.Y + 5 * m_Scale;

	// 计算需要的垂直偏移量
	int offsetY = targetTop - audioConfigRect.top;

	// 移动配置区域中的所有控件
	CRect rect;
	auto moveControl = [&](CWnd& control) {
		control.GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 如果控件在配置区域内，则进行移动
		if (rect.left >= m_Rect_ConfigArea.X) {
			// 计算移动后的新位置
			int newTop = rect.top + offsetY;

			// 移动控件
			control.MoveWindow(rect.left, newTop, rect.Width(), rect.Height());

			// 检查移动后的控件是否与标题栏重叠
			if (newTop < m_Rect_TitleBar.GetBottom()) {
				control.ShowWindow(SW_HIDE); // 隐藏与标题栏重叠的控件
			}
			else {
				control.ShowWindow(SW_SHOW); // 显示不与标题栏重叠的控件
			}
		}
	};

	// 应用移动到所有相关控件
	moveControl(m_Stat_FolderConfig);
	moveControl(m_Stat_SavePath);
	moveControl(m_Btn_Path);
	moveControl(m_Btn_FolderSelect);
	moveControl(m_Stat_VideoConfig);
	moveControl(m_Stat_VideoFormat);
	moveControl(m_Btn_VideoFormat);
	moveControl(m_Stat_Measure);
	moveControl(m_Btn_VideoQuality);
	moveControl(m_Stat_Fps);
	moveControl(m_Btn_Fps);
	moveControl(m_Stat_VideoCodec);
	moveControl(m_Btn_VideoCodec);
	moveControl(m_Stat_VideoQualityMode);
	moveControl(m_Btn_VideoBitrateMode);
	moveControl(m_Btn_VideoBitRatePercent);
	moveControl(m_Stat_AudioConfig);
	moveControl(m_Stat_SampleRate);
	moveControl(m_Btn_AudioSampleRate);
	moveControl(m_Stat_BitRate);
	moveControl(m_Btn_BitRate);
	moveControl(m_Stat_MouseConfig);
	moveControl(m_Stat_MouseDisplay);
	moveControl(m_Btn_DisplayMouse);
	moveControl(m_Stat_GeneralConfig);
	moveControl(m_Stat_StartupFirst);
	moveControl(m_Btn_OpenStart);
	moveControl(m_Stat_CloseExe);
	moveControl(m_Btn_MInimalTobar);
	moveControl(m_Btn_QuitExe);
	moveControl(m_Btn_AudioDevice);
	moveControl(m_Btn_MicroDevice);
	moveControl(m_Stat_MicroDevice);
	moveControl(m_Stat_AudioDevice);
	moveControl(m_Btn_Water);

	// 更新分界线位置
	m_Boundary1.p1.Y += offsetY;
	m_Boundary1.p2.Y += offsetY;
	m_Boundary2.p1.Y += offsetY;
	m_Boundary2.p2.Y += offsetY;
	m_Boundary3.p1.Y += offsetY;
	m_Boundary3.p2.Y += offsetY;
	m_Boundary4.p1.Y += offsetY;
	m_Boundary4.p2.Y += offsetY;

	m_NavClickType = AudioConfig;
	// repaint整个窗口
	Invalidate(false);
}

void Ui_ConfigDlg::OnBnClickedBtnMouseconfig()
{
	m_ListBoxs.HideListBox();
	// 获取鼠标设置区域的位置
	CRect mouseConfigRect;
	m_Stat_MouseConfig.GetWindowRect(mouseConfigRect);
	ScreenToClient(mouseConfigRect);

	// 计算鼠标设置区域的总高度（包括其下方的所有控件）
	CRect mouseDisplayRect, displayMouseRect;
	m_Stat_MouseDisplay.GetWindowRect(mouseDisplayRect);
	m_Btn_DisplayMouse.GetWindowRect(displayMouseRect);
	ScreenToClient(mouseDisplayRect);
	ScreenToClient(displayMouseRect);

	// 计算鼠标设置区域的底部位置
	int sectionBottom = displayMouseRect.bottom;

	// 计算可视区域的高度
	int visibleHeight = m_Rect_ConfigArea.Height;

	// 计算鼠标设置区域的高度
	int sectionHeight = sectionBottom - mouseConfigRect.top;

	// 计算理想的顶部位置
	int idealTop;

	if (sectionHeight < visibleHeight * 0.7) {
		// 区域较小，可以居中显示并留有适当上边距
		idealTop = m_Rect_ConfigArea.Y + (visibleHeight - sectionHeight) / 3;
	}
	else {
		// 区域较大，放在顶部以确保尽可能多的内容可见
		idealTop = m_Rect_ConfigArea.Y + 5 * m_Scale;
	}

	// 计算需要的垂直偏移量
	int offsetY = idealTop - mouseConfigRect.top;

	{
		CRect closeExeRect, minimalToBarRect, closeExeCkRect;
		m_Stat_CloseExe.GetWindowRect(closeExeRect);
		m_Btn_MInimalTobar.GetWindowRect(minimalToBarRect);
		m_Btn_QuitExe.GetWindowRect(closeExeCkRect);
		ScreenToClient(closeExeRect);
		ScreenToClient(minimalToBarRect);
		ScreenToClient(closeExeCkRect);

		int currentBottomMost = max(closeExeRect.bottom, max(minimalToBarRect.bottom, closeExeCkRect.bottom));
		int bottomLimit = static_cast<int>(m_WindowHeight * 0.981);
		int projectedBottom = currentBottomMost + offsetY;

		if (projectedBottom < bottomLimit)
		{
			// 【增添】将偏移量夹紧到“刚好贴到底部阈值”
			offsetY = bottomLimit - currentBottomMost;
		}
	}

	// 移动配置区域中的所有控件
	CRect rect;
	auto moveControl = [&](CWnd& control) {
		control.GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 如果控件在配置区域内，则进行移动
		if (rect.left >= m_Rect_ConfigArea.X) {
			// 计算移动后的新位置
			int newTop = rect.top + offsetY;

			// 移动控件
			control.MoveWindow(rect.left, newTop, rect.Width(), rect.Height());

			// 检查移动后的控件是否与标题栏重叠
			if (newTop < m_Rect_TitleBar.GetBottom()) {
				control.ShowWindow(SW_HIDE); // 隐藏与标题栏重叠的控件
			}
			else if (!control.IsWindowVisible()) {
				control.ShowWindow(SW_SHOW); // 如果不重叠且当前是隐藏的，则显示
			}
		}
	};

	// 应用移动到所有相关控件
	moveControl(m_Stat_FolderConfig);
	moveControl(m_Stat_SavePath);
	moveControl(m_Btn_Path);
	moveControl(m_Btn_FolderSelect);
	moveControl(m_Stat_VideoConfig);
	moveControl(m_Stat_VideoFormat);
	moveControl(m_Btn_VideoFormat);
	moveControl(m_Stat_Measure);
	moveControl(m_Btn_VideoQuality);
	moveControl(m_Stat_Fps);
	moveControl(m_Btn_Fps);
	moveControl(m_Stat_VideoCodec);
	moveControl(m_Btn_VideoCodec);
	moveControl(m_Stat_VideoQualityMode);
	moveControl(m_Btn_VideoBitrateMode);
	moveControl(m_Btn_VideoBitRatePercent);
	moveControl(m_Stat_AudioConfig);
	moveControl(m_Stat_SampleRate);
	moveControl(m_Btn_AudioSampleRate);
	moveControl(m_Stat_BitRate);
	moveControl(m_Btn_BitRate);
	moveControl(m_Stat_MouseConfig);
	moveControl(m_Stat_MouseDisplay);
	moveControl(m_Btn_DisplayMouse);
	moveControl(m_Stat_GeneralConfig);
	moveControl(m_Stat_StartupFirst);
	moveControl(m_Btn_OpenStart);
	moveControl(m_Stat_CloseExe);
	moveControl(m_Btn_MInimalTobar);
	moveControl(m_Btn_QuitExe);
	moveControl(m_Btn_AudioDevice);
	moveControl(m_Btn_MicroDevice);
	moveControl(m_Stat_MicroDevice);
	moveControl(m_Stat_AudioDevice);
	moveControl(m_Btn_Water);

	// 更新分界线位置
	m_Boundary1.p1.Y += offsetY;
	m_Boundary1.p2.Y += offsetY;
	m_Boundary2.p1.Y += offsetY;
	m_Boundary2.p2.Y += offsetY;
	m_Boundary3.p1.Y += offsetY;
	m_Boundary3.p2.Y += offsetY;
	m_Boundary4.p1.Y += offsetY;
	m_Boundary4.p2.Y += offsetY;

	m_NavClickType = MouseConfig;
	Invalidate(false);
}

void Ui_ConfigDlg::OnBnClickedBtnGeneralconfig()
{
	m_ListBoxs.HideListBox();
	// 获取通用设置区域的位置
	CRect generalConfigRect;
	m_Stat_GeneralConfig.GetWindowRect(generalConfigRect);
	ScreenToClient(generalConfigRect);

	// 计算通用设置区域的总高度（包括其所有子控件）
	CRect closeExeRect, minimalToBarRect, closeExeCkRect;
	m_Stat_CloseExe.GetWindowRect(closeExeRect);
	m_Btn_MInimalTobar.GetWindowRect(minimalToBarRect);
	m_Btn_QuitExe.GetWindowRect(closeExeCkRect);
	ScreenToClient(closeExeRect);
	ScreenToClient(minimalToBarRect);
	ScreenToClient(closeExeCkRect);

	// 获取区域的最底部位置
	int sectionBottom = max(closeExeRect.bottom, max(minimalToBarRect.bottom, closeExeCkRect.bottom));

	// 计算可视区域的高度
	int visibleHeight = m_Rect_ConfigArea.Height;

	// 计算通用设置区域的高度
	int sectionHeight = sectionBottom - generalConfigRect.top;

	// 检查区域是否需要特殊处理
	int configAreaBottom = m_Rect_ConfigArea.Y + visibleHeight;

	// 计算最佳的顶部位置
	int idealTop = configAreaBottom - sectionHeight - 10 * m_Scale; // 留出底部边距

	// 确保顶部位置不会太高（至少保留一定的上边距）
	idealTop = max(idealTop, m_Rect_ConfigArea.Y + 5 * m_Scale);

	// 计算需要的垂直偏移量
	int offsetY = idealTop - generalConfigRect.top;

	{
		CRect closeExeRect, minimalToBarRect, closeExeCkRect;
		m_Stat_CloseExe.GetWindowRect(closeExeRect);
		m_Btn_MInimalTobar.GetWindowRect(minimalToBarRect);
		m_Btn_QuitExe.GetWindowRect(closeExeCkRect);
		ScreenToClient(closeExeRect);
		ScreenToClient(minimalToBarRect);
		ScreenToClient(closeExeCkRect);

		int currentBottomMost = max(closeExeRect.bottom, max(minimalToBarRect.bottom, closeExeCkRect.bottom));
		int bottomLimit = static_cast<int>(m_WindowHeight * 0.981);
		int projectedBottom = currentBottomMost + offsetY;

		if (projectedBottom < bottomLimit)
		{
			// 【增添】将偏移量夹紧到“刚好贴到底部阈值”
			offsetY = bottomLimit - currentBottomMost;
		}
	}

	// 移动配置区域中的所有控件
	CRect rect;
	auto moveControl = [&](CWnd& control) {
		control.GetWindowRect(&rect);
		ScreenToClient(&rect);

		// 如果控件在配置区域内，则进行移动
		if (rect.left >= m_Rect_ConfigArea.X) {
			// 计算移动后的新位置
			int newTop = rect.top + offsetY;

			// 移动控件
			control.MoveWindow(rect.left, newTop, rect.Width(), rect.Height());

			// 检查移动后的控件是否与标题栏重叠
			if (newTop < m_Rect_TitleBar.GetBottom()) {
				control.ShowWindow(SW_HIDE); // 隐藏与标题栏重叠的控件
			}
			else if (!control.IsWindowVisible()) {
				control.ShowWindow(SW_SHOW); // 如果不重叠且当前是隐藏的，则显示
			}
		}
	};

	// 应用移动到所有相关控件
	moveControl(m_Stat_FolderConfig);
	moveControl(m_Stat_SavePath);
	moveControl(m_Btn_Path);
	moveControl(m_Btn_FolderSelect);
	moveControl(m_Stat_VideoConfig);
	moveControl(m_Stat_VideoFormat);
	moveControl(m_Btn_VideoFormat);
	moveControl(m_Stat_Measure);
	moveControl(m_Btn_VideoQuality);
	moveControl(m_Stat_Fps);
	moveControl(m_Btn_Fps);
	moveControl(m_Stat_VideoCodec);
	moveControl(m_Btn_VideoCodec);
	moveControl(m_Stat_VideoQualityMode);
	moveControl(m_Btn_VideoBitrateMode);
	moveControl(m_Btn_VideoBitRatePercent);
	moveControl(m_Stat_AudioConfig);
	moveControl(m_Stat_SampleRate);
	moveControl(m_Btn_AudioSampleRate);
	moveControl(m_Stat_BitRate);
	moveControl(m_Btn_BitRate);
	moveControl(m_Stat_MouseConfig);
	moveControl(m_Stat_MouseDisplay);
	moveControl(m_Btn_DisplayMouse);
	moveControl(m_Stat_GeneralConfig);
	moveControl(m_Stat_StartupFirst);
	moveControl(m_Btn_OpenStart);
	moveControl(m_Stat_CloseExe);
	moveControl(m_Btn_MInimalTobar);
	moveControl(m_Btn_QuitExe);
	moveControl(m_Btn_AudioDevice);
	moveControl(m_Btn_MicroDevice);
	moveControl(m_Stat_MicroDevice);
	moveControl(m_Stat_AudioDevice);
	moveControl(m_Btn_Water);

	// 更新分界线位置
	m_Boundary1.p1.Y += offsetY;
	m_Boundary1.p2.Y += offsetY;
	m_Boundary2.p1.Y += offsetY;
	m_Boundary2.p2.Y += offsetY;
	m_Boundary3.p1.Y += offsetY;
	m_Boundary3.p2.Y += offsetY;
	m_Boundary4.p1.Y += offsetY;
	m_Boundary4.p2.Y += offsetY;
	m_NavClickType = GeneralConfig;
	Invalidate(false);
}

void Ui_ConfigDlg::OnBnClickedBtnClose()
{
	m_ListBoxs.HideListBox();
	// 隐藏配置对话框
	ShowWindow(SW_HIDE);

	// 获取主窗口并重新启用
	CWnd* pMainWnd = GetParent();
	if (pMainWnd)
	{
		if (pMainWnd->GetSafeHwnd() == App.m_Dlg_Main)
		{
			if (pMainWnd)
			{
				pMainWnd->EnableWindow(TRUE);
				pMainWnd->SetActiveWindow();
				pMainWnd->SetForegroundWindow();
				App.m_Dlg_Main.m_Shadow.RestoreFromHide();
				if (App.m_Dlg_Main.m_Dlg_Videolist)
					App.m_Dlg_Main.m_Dlg_Videolist->RestoreShadow();
				auto pDlg = (Ui_MainDlg*)pMainWnd;
				if (pDlg->m_Dlg_Gaming)
					pDlg->m_Dlg_Gaming->EnableWindow(TRUE);
				if (pDlg->m_Dlg_WindowRecord)
					pDlg->m_Dlg_WindowRecord->EnableWindow(TRUE);
				if (pDlg->m_Dlg_Carmera)
					pDlg->m_Dlg_Carmera->EnableWindow(TRUE);
			}
		}
		else
		{
			pMainWnd->EnableWindow(TRUE);
		}
	}
	m_Shadow.Show(m_hWnd);
}

void Ui_ConfigDlg::OnBnClickedBtnFolderselect()
{
	m_ListBoxs.HideListBox();
	BROWSEINFO bi = { 0 };
	bi.hwndOwner = this->GetSafeHwnd();
	bi.lpszTitle = L"请选择文件夹";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;  // 新对话框样式，仅显示文件系统目录

	// 显示文件夹选择对话框
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != NULL)
	{
		// 获取所选文件夹的路径
		WCHAR szPath[MAX_PATH] = { 0 };
		if (SHGetPathFromIDList(pidl, szPath))
		{
			// 设置路径到控件
			CString PathAnsi = szPath;
			std::string PathUtf8 = GlobalFunc::ConvertPathToUtf8(PathAnsi);
			if (JudgePathValid(PathUtf8))
			{
				m_Btn_Path.SetWindowTextW(LARSC::Utf8ToWideString(PathUtf8).c_str());
			}
			else
			{
				Ui_MessageModalDlg MessageDlg;
				MessageDlg.SetModal(L"极速录屏大师", L"很抱歉，温馨提示",
					L"软件没有权限在您选择的目录下保存文件，请选择其他目录", L"确认");
				MessageDlg.DoModal();
			}
		}
		CoTaskMemFree(pidl);// 释放PIDL
	}
}

void Ui_ConfigDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialogEx::OnMouseMove(nFlags, point);
}

void Ui_ConfigDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
	//m_Shadow.Show(m_hWnd);
	// TODO: 在此处添加消息处理程序代码
}

void Ui_ConfigDlg::OnExitSizeMove()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Shadow.Show(m_hWnd);
	CDialogEx::OnExitSizeMove();
}

void Ui_ConfigDlg::OnEnterSizeMove()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_Shadow.Show(m_hWnd);
	CDialogEx::OnEnterSizeMove();
}

void Ui_ConfigDlg::OnBnClickedBtnDisplaymouse()
{
	m_ListBoxs.HideListBox();
	if (m_Bool_IsDisplayMouse)
	{
		m_Btn_DisplayMouse.LoadPNG(CONFIGDLG_PNG_DISPLAY);
		m_Bool_IsDisplayMouse = false;
	}
	else
	{
		m_Btn_DisplayMouse.LoadPNG(CONFIGDLG_PNG_DISPLAYMOUSE_HOLD);
		m_Bool_IsDisplayMouse = true;
	}
}

void Ui_ConfigDlg::OnBnClickedBtnStartupfirst()
{
	m_ListBoxs.HideListBox();
	if (m_Bool_IsOpenStart)
	{
		m_Btn_OpenStart.LoadPNG(CONFIGDLG_PNG_OPENSTART);
		m_Bool_IsOpenStart = false;
	}
	else
	{
		m_Btn_OpenStart.LoadPNG(CONFIGDLG_PNG_OPENSTART_HOLD);
		m_Bool_IsOpenStart = true;
	}
}

void Ui_ConfigDlg::OnBnClickedBtnMinialtobar()
{
	m_ListBoxs.HideListBox();
	m_Btn_MInimalTobar.LoadPNG(CONFIGDLG_PNG_MINIMALTOBAR_HOLD);
	m_Btn_MInimalTobar.LoadClickPNG(CONFIGDLG_PNG_MINIMALTOBAR_HOLD);
	m_Btn_QuitExe.LoadPNG(CONFIGDLG_PNG_QUITEXE);
	m_Btn_QuitExe.LoadClickPNG(CONFIGDLG_PNG_QUITEXE);

	m_Bool_MInimalTobar = true;
	m_Bool_QuitExe = false;
	// 更新应用程序关闭行为
	auto pMainDlg = (Ui_MainDlg*)GetParent();
	auto closeManager = pMainDlg->m_closeManager.get();
	if (pMainDlg && closeManager) {
		closeManager->SetCloseMode(
			m_Bool_MInimalTobar ?
			AppCloseManager::CloseMode::MinimizeToTray :
			AppCloseManager::CloseMode::ExitApplication
		);
	}
	Invalidate(false);
}

void Ui_ConfigDlg::OnBnClickedBtnExitexe()
{
	m_ListBoxs.HideListBox();
	m_Btn_MInimalTobar.LoadPNG(CONFIGDLG_PNG_MINIMALTOBAR);
	m_Btn_MInimalTobar.LoadClickPNG(CONFIGDLG_PNG_MINIMALTOBAR);
	m_Btn_QuitExe.LoadPNG(CONFIGDLG_PNG_QUITEXE_HOLD);
	m_Btn_QuitExe.LoadClickPNG(CONFIGDLG_PNG_QUITEXE_HOLD);

	m_Bool_QuitExe = true;
	m_Bool_MInimalTobar = false;

	// 更新应用程序关闭行为
	auto pMainDlg = (Ui_MainDlg*)GetParent();
	auto closeManager = pMainDlg->m_closeManager.get();
	if (pMainDlg && closeManager) {
		closeManager->SetCloseMode(
			m_Bool_MInimalTobar ?
			AppCloseManager::CloseMode::MinimizeToTray :
			AppCloseManager::CloseMode::ExitApplication
		);
	}
	Invalidate(false);
}

BOOL Ui_ConfigDlg::SaveConfigToFile()
{
	ConfigFileHandler ConfigHandler(App.m_ConfigPath);
	ConfigHandler.WriteConfigFile("Config", "IsDisplayMouse", m_Bool_IsDisplayMouse ? "1" : "0");
	ConfigHandler.WriteConfigFile("Config", "IsOpenStart", m_Bool_IsOpenStart ? "1" : "0");
	ConfigHandler.WriteConfigFile("Config", "MInimalTobar", m_Bool_MInimalTobar ? "1" : "0");
	ConfigHandler.WriteConfigFile("Config", "QuitExe", m_Bool_QuitExe ? "1" : "0");
	ConfigHandler.WriteConfigFile("Config", "IsAutoLogin", App.m_IsAutoLogin ? "1" : "0");
	ConfigHandler.WriteConfigFile("Config", "IsWTOn", m_Bool_IsWaterOn ? "1" : "0");
	return TRUE;
}

void Ui_ConfigDlg::SetMinimalToBar(bool isEnable)
{
	if (isEnable)
	{
		OnBnClickedBtnMinialtobar();
	}
	else
	{
		OnBnClickedBtnExitexe();
	}
}

void Ui_ConfigDlg::SetQuitExe(bool isQuitExe)
{
	if (isQuitExe)
	{
		OnBnClickedBtnExitexe();
	}
	else
	{
		OnBnClickedBtnMinialtobar();
	}
}

void Ui_ConfigDlg::GetExeQuitWay(bool* IsQuitExe, bool* IsMinimalTobar)
{
	*IsQuitExe = m_Bool_QuitExe;
	*IsMinimalTobar = m_Bool_MInimalTobar;
}

const CString Ui_ConfigDlg::getRecSavePath()
{
	CString str;
	m_Btn_Path.GetWindowTextW(str);
	return str;
}

BOOL Ui_ConfigDlg::LoadConfigFromFile()
{
	// 获取应用程序路径
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 获取应用程序目录
	CString strAppPath = szPath;
	int nPos = strAppPath.ReverseFind('\\');
	if (nPos > 0)
		strAppPath = strAppPath.Left(nPos + 1);

	// 构建配置文件完整路径
	CString strConfigFile = strAppPath + _T("onnx\\config.ini");

	// 检查文件是否存在
	if (GetFileAttributes(strConfigFile) == INVALID_FILE_ATTRIBUTES)
	{
		// 文件不存在，返回FALSE但不报错，因为可能是首次运行
		return FALSE;
	}

	// 打开配置文件
	FILE* fp = NULL;
	errno_t err = _tfopen_s(&fp, strConfigFile, _T("r"));
	if (err != 0 || fp == NULL)
	{
		// 无法打开文件但不弹出错误，使用默认值继续
		return FALSE;
	}

	// 读取配置信息
	TCHAR szLine[256];
	TCHAR szKey[64];
	int nValue;

	while (_fgetts(szLine, 256, fp) != NULL)
	{
		// 忽略注释行和节名行
		if (szLine[0] == _T('#') || szLine[0] == _T('[') || szLine[0] == _T('\n'))
			continue;

		// 解析键值对
		if (_stscanf_s(szLine, _T("%[^=]=%d"), szKey, _countof(szKey), &nValue) == 2)
		{
			// 删除键名中可能存在的空格
			CString strKey = szKey;
			strKey.TrimRight();

			// 设置相应的成员变量
			if (strKey == _T("IsDisplayMouse"))
				m_Bool_IsDisplayMouse = (nValue != 0);
			else if (strKey == _T("IsOpenStart"))
				m_Bool_IsOpenStart = (nValue != 0);
			else if (strKey == _T("MInimalTobar"))
				m_Bool_MInimalTobar = (nValue != 0);
			else if (strKey == _T("QuitExe"))
				m_Bool_QuitExe = (nValue != 0);
			else if (strKey == _T("IsWTOn"))
				m_Bool_IsWaterOn = (nValue != 0);
		}
	}

	// 关闭文件
	fclose(fp);

	return TRUE;
}

void Ui_ConfigDlg::OnBnClickedBtnWater()
{
	DBFMT(ConsoleHandle, L"m_Bool_IsWaterOn:%s", m_Bool_IsWaterOn ?  L"true" : L"false");
	m_ListBoxs.HideListBox();
	if (m_Bool_IsWaterOn)
	{
		DBFMT(ConsoleHandle, L"App.m_IsVip:%s", App.m_IsVip ?  L"true" : L"false");
		if (App.m_IsVip && App.m_isLoginIn)
		{
			m_Btn_Water.LoadPNG(CONFIGDLG_PNG_NOWATERON);
			m_Bool_IsWaterOn = false;
			Invalidate(false);
		}
		else
		{
			TipOfOpenVip();
		}
	}
	else
	{
		m_Btn_Water.LoadPNG(CONFIGDLG_PNG_NOWATEROFF);
		Invalidate(false);
		m_Bool_IsWaterOn = true;
	}
}

void Ui_ConfigDlg::DrawRoundedRectangle(Gdiplus::Graphics* graphics,
	int x, int y, int width, int height, int cornerRadius)
{
	// 检查参数有效性
	if (!graphics || width <= 0 || height <= 0) return;

	// 如果cornerRadius为0或负值，则绘制普通矩形
	if (cornerRadius <= 0)
	{
		Gdiplus::SolidBrush blueBrush(Gdiplus::Color(255, 0, 139, 255));
		graphics->FillRectangle(&blueBrush, x, y, width, height);
		return;
	}

	// 确保圆角半径不超过宽度和高度的一半
	cornerRadius = min(cornerRadius, min(width / 2, height / 2));

	// 创建蓝色画刷 - RGB(0,139,255)
	Gdiplus::SolidBrush blueBrush(Gdiplus::Color(255, 0, 139, 255));

	// 创建GraphicsPath来绘制圆角矩形
	Gdiplus::GraphicsPath path;

	// 路径点坐标
	Gdiplus::REAL left = static_cast<Gdiplus::REAL>(x);
	Gdiplus::REAL top = static_cast<Gdiplus::REAL>(y);
	Gdiplus::REAL right = static_cast<Gdiplus::REAL>(x + width);
	Gdiplus::REAL bottom = static_cast<Gdiplus::REAL>(y + height);
	Gdiplus::REAL radius = static_cast<Gdiplus::REAL>(cornerRadius);
	Gdiplus::REAL diameter = radius * 2.0f;

	// 创建圆角矩形路径
	// 左上角圆弧
	path.AddArc(left, top, diameter, diameter, 180, 90);

	// 顶边线
	path.AddLine(left + radius, top, right - radius, top);

	// 右上角圆弧
	path.AddArc(right - diameter, top, diameter, diameter, 270, 90);

	// 右边线
	path.AddLine(right, top + radius, right, bottom - radius);

	// 右下角圆弧
	path.AddArc(right - diameter, bottom - diameter, diameter, diameter, 0, 90);

	// 底边线
	path.AddLine(right - radius, bottom, left + radius, bottom);

	// 左下角圆弧
	path.AddArc(left, bottom - diameter, diameter, diameter, 90, 90);

	// 左边线，闭合路径
	path.AddLine(left, bottom - radius, left, top + radius);

	// 确保路径闭合
	path.CloseFigure();

	// 设置抗锯齿模式以获得平滑的边缘
	Gdiplus::SmoothingMode originalMode = graphics->GetSmoothingMode();
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	// 填充路径
	graphics->FillPath(&blueBrush, &path);

	// 恢复原来的抗锯齿模式
	graphics->SetSmoothingMode(originalMode);
}