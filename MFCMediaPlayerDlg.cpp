
// MFCMediaPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "theApp.h"
#include "MFCMediaPlayerDlg.h"
#include "afxdialogex.h"
#include "CDebug.h"
#include "DeviceManager.h"
#include "WindowHandleManager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//导入外部变量
extern HANDLE ConsoleHandle;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCMediaPlayerDlg 对话框

CMFCMediaPlayerDlg::CMFCMediaPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCMediaPlayerDlg::IDD, pParent)
	, m_ScreenRecorder(nullptr)
	, m_CameraCapture(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCMediaPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, MainDlg_Btn_ScreenRecording, m_ScreenRecordingBtn);
	DDX_Control(pDX, MainDlg_Pic_DisplayArea, m_DisplayArea);
}

BEGIN_MESSAGE_MAP(CMFCMediaPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(MainDlg_Btn_ScreenRecording, &CMFCMediaPlayerDlg::OnBnClickedBtnScreenrecording)
	ON_BN_CLICKED(MainDlg_Btn_ScreenRecording2, &CMFCMediaPlayerDlg::OnBnClickedBtnStopScreenrecording)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCMediaPlayerDlg::OnOpenCameraClickedButton)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCMediaPlayerDlg::OnCloseCameraDisplayClickedButton)
	ON_BN_CLICKED(MainDlg_Btn_ScreenRecording3, &CMFCMediaPlayerDlg::OnAreaRecordingClickedBtnScreenrecording)
	ON_BN_CLICKED(MainDlg_Btn_ScreenRecording4, &CMFCMediaPlayerDlg::OnHandleRecordingClickedBtn)
END_MESSAGE_MAP()


// CMFCMediaPlayerDlg 消息处理程序

BOOL CMFCMediaPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCMediaPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCMediaPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCMediaPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//录屏按钮按下
void CMFCMediaPlayerDlg::OnBnClickedBtnScreenrecording()
{
	////构造保存的文件路径及其文件名
	//CString frameRateStr, formatStr, outputNameStr;
	//outputNameStr = L"output" + frameRateStr + L"." + formatStr.MakeLower();
	//
	////录制对象创建
	//if (!m_ScreenRecorder) {
	//	m_ScreenRecorder = new ScreenRecorder(
	//		ScreenRecorder::ResolutionRatio::Rs_1080P,
	//		ScreenRecorder::VideoQuality::Origin,
	//		ScreenRecorder::VideoFormat::MP4,
	//		ScreenRecorder::EncodingPreset::Fast,
	//		ScreenRecorder::RecordMode::Both,
	//		AudioSampleRate::Hz_44100,
	//		AudioBitRate::Kbps_128,
	//		180
	//		);
	//}
	//
	////开始录制 
	//CT2A outputfileName(outputNameStr);
	//m_ScreenRecorder->startRecording(outputfileName);
}

//停止按钮按下
void CMFCMediaPlayerDlg::OnBnClickedBtnStopScreenrecording()
{
	// TODO:  在此添加控件通知处理程序代码
	m_ScreenRecorder->stopRecording();
}

BOOL CMFCMediaPlayerDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	//if (m_ScreenRecorder) {
	//	m_ScreenRecorder->stopRecording();
	//	delete m_ScreenRecorder;
	//}
	return CDialogEx::DestroyWindow();
}

//打开摄像头
void CMFCMediaPlayerDlg::OnOpenCameraClickedButton()
{
	////获取摄像头设备信息和录制参数
	//std::vector<DeviceInfo> Devices = DeviceManager::GetInstance().GetCameraDevices();
	//DeviceInfo deviceInfo = Devices.front();
	//std::vector<CameraCapability> Options = DeviceManager::GetInstance().GetCameraCapabilities(deviceInfo.nameA);
	//CameraCapability option = Options.at(14);
	//
	////构造摄像头录制所需参数结构体
	//CameraOptions camerOption;
	//camerOption.deviceDesc = deviceInfo.alternateName;
	//camerOption.deviceName = deviceInfo.nameA;
	//camerOption.fps = option.fps;
	//camerOption.vcodec = option.vcodec;
	//camerOption.pixelX = option.width;
	//camerOption.pixelY = option.height;
	//
	////创建摄像头捕获器
	//if (!m_CameraCapture) {
	//	m_CameraCapture = new CameraCapture(camerOption);
	//}
	//if (!m_CameraCapture || !m_CameraCapture->Init()) {
	//	DEBUG_CONSOLE_STR(ConsoleHandle, L"摄像头捕获器初始化失败");
	//	return;
	//}
	//
	////创建摄像头画面显示器
	//CRect DispAreaRect;
	//m_DisplayArea.GetClientRect(DispAreaRect);
	//DEBUG_CONSOLE_FMT(ConsoleHandle, L"摄像头显示区域:%d,%d,%d,%d",
	//	DispAreaRect.left, DispAreaRect.top, DispAreaRect.right, DispAreaRect.bottom);
	//if (!m_CameraDisplayer) {
	//	m_CameraDisplayer = new CameraDisplayer(m_CameraCapture);
	//}
	//if (!m_CameraDisplayer || !m_CameraDisplayer->Init(this->GetDlgItem(MainDlg_Pic_DisplayArea))) {
	//	DEBUG_CONSOLE_STR(ConsoleHandle, L"摄像头画面显示器初始化失败");
	//	return;
	//}
	//
	////开始捕获和显示
	//m_CameraCapture->StartCapture();
	//m_CameraDisplayer->StartDisplay();
}

//关闭摄像头预览
void CMFCMediaPlayerDlg::OnCloseCameraDisplayClickedButton()
{
	// 首先停止显示 - 这会停止SDL相关资源
	//if (m_CameraDisplayer) {
	//	m_CameraDisplayer->StopDisplay();
	//	// 等待显示完全停止
	//	Sleep(100);
	//}

	// 然后停止摄像头捕获
	//if (m_CameraCapture) {
	//	m_CameraCapture->StopCapture();
	//	// 等待捕获完全停止
	//	Sleep(50);
	//}
}

//选区录制按钮按下
void CMFCMediaPlayerDlg::OnAreaRecordingClickedBtnScreenrecording()
{
	//AreaSelectDlg m_Dlg_AreaSelect;
	//if (m_Dlg_AreaSelect.DoModal() == IDOK)
	//{
	//	//获取选择的录制区域
	//	const CRect& SelectRect = m_Dlg_AreaSelect.GetSelectRect();
	//
	//	////构造保存的文件路径及其文件名
	//	CString frameRateStr, formatStr, outputNameStr;
	//	outputNameStr = L"AreaRecordingOutput" + frameRateStr + L"." + formatStr.MakeLower();
	//	
	//	//录制对象创建
	//	if (!m_ScreenRecorder) {
	//		m_ScreenRecorder = new ScreenRecorder(
	//			SelectRect.left, SelectRect.top, SelectRect.right, SelectRect.bottom,
	//			ScreenRecorder::ResolutionRatio::Rs_1080P,
	//			ScreenRecorder::VideoQuality::Origin,
	//			ScreenRecorder::VideoFormat::MP4,
	//			ScreenRecorder::EncodingPreset::Fast,
	//			ScreenRecorder::RecordMode::Both,
	//			AudioSampleRate::Hz_44100,
	//			AudioBitRate::Kbps_128,
	//			180
	//		);
	//	}
	//	
	//	//开始录制 
	//	CT2A outputfileName(outputNameStr);
	//	m_ScreenRecorder->startRecording(outputfileName);
	//}
}


void CMFCMediaPlayerDlg::OnHandleRecordingClickedBtn()
{
	//// 获取所有窗口句柄
	//auto& manager = WindowHandleManager::GetInstance();
	//const auto& handles = manager.GetWindowHandles();
	//
	//// 打印所有窗口标题
	//DEBUG_CONSOLE_STR(ConsoleHandle, L"----窗口标题----");
	//int index = 0;
	//HWND RecordinghWnd = handles.at(0);
	//for (HWND hwnd : handles) {
	//	std::wstring title = WindowHandleManager::GetWindowTitle(hwnd);
	//	if (title == L"微信") {
	//		RecordinghWnd = hwnd;
	//	}
	//	DEBUG_CONSOLE_FMT(ConsoleHandle, L"标题:%s", title.c_str());
	//}
	//
	////录制指定窗口
	//std::wstring title = WindowHandleManager::GetWindowTitle(RecordinghWnd);
	//DEBUG_CONSOLE_FMT(ConsoleHandle, L"录制的窗口:%s", title.c_str());
	//
	////构造保存的文件路径及其文件名
	//CString frameRateStr, formatStr, outputNameStr;
	//outputNameStr = L"HwndOutput" + frameRateStr + L"." + formatStr.MakeLower();
	//
	////录制对象创建
	//if (!m_ScreenRecorder) {
	//	m_ScreenRecorder = new ScreenRecorder(
	//		RecordinghWnd,
	//		ScreenRecorder::ResolutionRatio::Rs_1080P,
	//		ScreenRecorder::VideoQuality::Origin,
	//		ScreenRecorder::VideoFormat::MP4,
	//		ScreenRecorder::EncodingPreset::Fast,
	//		ScreenRecorder::RecordMode::Both,
	//		AudioSampleRate::Hz_44100,
	//		AudioBitRate::Kbps_128,
	//		24
	//	);
	//}
	//
	////开始录制 
	//CT2A outputfileName(outputNameStr);
	//m_ScreenRecorder->startRecording(outputfileName);
}
