
// MFCMediaPlayer.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "UserActionLogger.h"
#include "theApp.h"
#include "MFCMediaPlayerDlg.h"
#include "CameraCapture.h"
#include "CDebug.h"
#include "DeviceManager.h"
#include "Ui_LoadingSDL.h"
#include "WndShadow.h"
#include "LarStringConversion.h"
#include "AutoStartManager.h"
#include "GlobalFunc.h"
#include "Ui_MessageBoxSDL.h"
#include "ConfigFileHandler.h"
#include "HttpRequestHandler.h"
#include "ModalDialogFunc.h"
#include "Ui_UserProfileDlg.h"
HANDLE theApp::m_hInstanceMutex = NULL;
HANDLE ConsoleHandle;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// 
#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12  // ARM64架构的值为12
#endif

std::string LoadSDLWindow_ResPath;

// CInstallerScreenRecordApp
typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);
/// 获取当前系统的版本号
std::string GetOSVersion()
{
	// 动态加载 ntdll.dll 中的 RtlGetVersion 函数来获取版本信息
	HMODULE hMod = ::GetModuleHandleW(L"ntdll.dll");
	if (hMod)
	{
		RtlGetVersionPtr fxPtr = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hMod, "RtlGetVersion"));
		if (fxPtr != nullptr)
		{
			RTL_OSVERSIONINFOEXW osInfo = { 0 };
			osInfo.dwOSVersionInfoSize = sizeof(osInfo);
			if (fxPtr(&osInfo) == 0) // STATUS_SUCCESS: 0
			{
				std::ostringstream oss;
				oss << osInfo.dwMajorVersion << "." << osInfo.dwMinorVersion << " (Build " << osInfo.dwBuildNumber << ")";
				return oss.str();
			}
		}
	}
	return "Unknown";
}
/// 获取当前系统的处理器架构
std::string GetSystemArchitecture()
{
	SYSTEM_INFO sysInfo;
	// 尝试调用 GetNativeSystemInfo 以获得真实的系统信息
	typedef void (WINAPI* LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO);
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo =
		reinterpret_cast<LPFN_GetNativeSystemInfo>(GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo"));
	if (fnGetNativeSystemInfo)
	{
		fnGetNativeSystemInfo(&sysInfo);
	}
	else
	{
		GetSystemInfo(&sysInfo);
	}

	switch (sysInfo.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
		return "64-bit";
	case PROCESSOR_ARCHITECTURE_INTEL:
		return "32-bit";
	case PROCESSOR_ARCHITECTURE_ARM:
		return "ARM";
	case PROCESSOR_ARCHITECTURE_ARM64:
		return "ARM64";
	default:
		return "Unknown Architecture";
	}
}
/// 获取当前执行程序所在目录(包含程序名)
CString GetExecutablePath()
{
	CString strExePath;
	TCHAR szFilePath[MAX_PATH] = { 0 };

	// 获取模块文件名
	if (GetModuleFileName(NULL, szFilePath, MAX_PATH) != 0)
	{
		strExePath = szFilePath;
	}
	else
	{
		// 如果失败，可以记录错误日志或抛出异常
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取可执行程序目录失败");
	}

	return strExePath;
}
// 获取当前可执行程序所在目录
CString GetExecutablePathFolder()
{
	TCHAR szPath[MAX_PATH] = { 0 };

	// 获取可执行文件的完整路径
	if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0)
	{
		return _T("");
	}

	CString strPath(szPath);

	// 移除文件名，只保留目录部分
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos != -1)
	{
		strPath = strPath.Left(nPos); // 不包含最后的反斜杠
	}

	return strPath;
}
//curl回调函数（应用程序启动时）
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}
//curl回调函数（刷新设备信息(支付后)
size_t UserInfoWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	std::string* mem = (std::string*)userp;
	mem->append((char*)contents, realsize);
	return realsize;
}
// 添加到适当的头文件中
std::string GenerateGpuSerialFromDeviceID(const std::string& deviceId) {
	// 创建一个类似G1234567890ABCDEF的格式化序列号
	std::string serialNumber = "G";

	// 提取VEN、DEV和SUBSYS信息
	size_t venPos = deviceId.find("VEN_");
	size_t devPos = deviceId.find("DEV_");
	size_t subsysPos = deviceId.find("SUBSYS_");

	if (venPos != std::string::npos && devPos != std::string::npos && subsysPos != std::string::npos) {
		// 获取VEN部分（4个字符）
		std::string venPart = deviceId.substr(venPos + 4, 4);
		// 获取DEV部分（4个字符）
		std::string devPart = deviceId.substr(devPos + 4, 4);
		// 获取SUBSYS部分（8个字符）
		std::string subsysPart = deviceId.substr(subsysPos + 7, 8);

		// 组合成类似于G1234567890ABCDEF的格式
		serialNumber += venPart + devPart + subsysPart;
	}
	else {
		// 如果不能提取这些信息，生成一个基于时间的序列号
		auto now = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss << std::hex << std::uppercase << time;
		serialNumber += ss.str();

		// 确保总长度为16个字符
		while (serialNumber.length() < 16) {
			serialNumber += "0";
		}
		serialNumber = serialNumber.substr(0, 16);
	}

	return serialNumber;
}

std::string FindInstallTokenFromFile() 
{
	ConfigFileHandler ConfigHandler(App.m_ConfigPath);
	if (ConfigHandler.ParseData())
	{
		return ConfigHandler.asString("InstallerTK", "INTK");
	}
	else
	{
		DB(ConsoleHandle, L"配置文件解析失败！安装Token解析失败");
		return "";
	}
}
//读取配置文件中的应用程序Token
std::string FindAppTokenFromFile(const std::string& targetFileName)
{
	CString Path = GlobalFunc::GetExecutablePathFolder() + "\\onnx\\config.ini";
	std::wstring PathW = Path;
	ConfigFileHandler ConfigHandler(LARSC::ws2s(PathW));
	if (ConfigHandler.ParseData())
	{
		return ConfigHandler.asString("AppConfig", "AppToken");
	}
	return "NULL";
}


// theApp

BEGIN_MESSAGE_MAP(theApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// theApp 构造

theApp::theApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	m_isLoginIn = false;
	m_IsOverBinds = false;
	m_IsVip = false;
	m_IsAutoLogin = false;
	m_IsInitialSuccess = false;
	m_IsWin7Over = false;
	m_IsNonUserPaid = false;
	m_IsHasOpenVip = false;
	m_IsNeedShowMouthBill = false;
	m_IsHasMouthBillWindowShowed = false;
	m_CanGuestBuy = false;

	m_loadingWindow = nullptr;
	m_userActionLogger = nullptr;
	m_InstallerToken = "NULL";
	m_appToken = "NULL";
	m_machine_guid = "NULL";
	m_ReadPacketAmount = 0;
	m_MouthBillPrice = -1;

	CString Path = GlobalFunc::GetExecutablePathFolder() + "\\onnx\\config.ini";
	std::wstring PathW = Path;
	m_ConfigPath = LARSC::ws2s(PathW);

	CString userLogPath = GlobalFunc::GetExecutablePathFolder() + "\\onnx\\userlog.log";
	m_machine_guid = GetMachineGuid();
	m_userActionLogger = UserActionLogger::GetInstance();
	m_userActionLogger->Init(m_machine_guid, std::wstring(userLogPath), 2048);
}

void theApp::OpenFeedBackLink(HWND hWnd, LPCTSTR linkUrl)
{
	LPCTSTR verb = _T("open");
	HINSTANCE result = ::ShellExecute(
		hWnd,
		verb,
		linkUrl,
		nullptr,
		nullptr,
		SW_SHOWNORMAL
	);
	if ((INT_PTR)result <= 32)
	{
		AfxMessageBox(_T("无法启动浏览器打开反馈链接，请检查系统设置。"));
	}
}

bool theApp::IsWin7Over()
{
	m_IsWin7Over = IsWindows7OrHigher();
	DBFMT(ConsoleHandle, L"当前系统是否高于Win7？ : %s", m_IsWin7Over ? L"否" : L"是");
	return m_IsWin7Over;
}

void theApp::initGdiPlus()
{
	Gdiplus::GdiplusStartupInput gdiInput;
	ULONG_PTR gdiToken = 0;
	Gdiplus::Status gdiStatus = Gdiplus::GdiplusStartup(&gdiToken, &gdiInput, NULL);
	if (gdiStatus != Gdiplus::Ok) {
		AfxMessageBox(_T("GDI+ 初始化失败"));
		return;
	}

	// 存储 token
	m_gdiToken = gdiToken;
}

void theApp::CleanUpWindows()
{
	// 销毁所有的窗口
	if (m_pMainWnd != NULL) {
		m_pMainWnd->DestroyWindow();
		m_pMainWnd = NULL;
	}
	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void theApp::CleanUpGDIRes()
{
	//关闭所有窗口的GDI资源
	m_Dlg_Main.CleanUpGdiPngRes();
}

void theApp::CleanUpGDI()
{
	// 关闭 GDI+
	if (m_gdiToken != 0) 
	{
		Gdiplus::GdiplusShutdown(m_gdiToken);
		m_gdiToken = 0;
	}
}

// 唯一的一个 theApp 对象

theApp App;

// theApp 初始化

BOOL theApp::InitInstance()
{
	UserOpenMain(false);
	//setlocale(LC_ALL, "");
	ALLOC_CONSOLE_AND_GET_HANDLE(ConsoleHandle);
	DB(ConsoleHandle, L"判断打开程序是否启动");
	if (!JudgeSingleAppIns())
		return FALSE;
	DB(ConsoleHandle, L"程序未启动，开始初始化");

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	//SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
	//AddDllDirectory(TEXT("."));

	DB(ConsoleHandle, L"初始化INITCOMMONCONTROLSEX组件");
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	DB(ConsoleHandle, L"CWinApp::InitInstance()主程序默认初始化调用");
	CWinApp::InitInstance();

	DB(ConsoleHandle, L"Socket套接字初始化调用");
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	DB(ConsoleHandle, L"控件容器启用");
	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	DB(ConsoleHandle, L"创建 shell 管理器");
	CShellManager* pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	DB(ConsoleHandle, L"激活“Windows Native”视觉管理器");
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	//Test();

	DB(ConsoleHandle, L"MFC预设初始化完毕");

	//初始化窗口阴影效果
	CWndShadow::Initialize(AfxGetInstanceHandle());//初始化窗口阴影效果

	//Bug In
	//先弹出正在加载的窗口
	DB(ConsoleHandle, L"开始获取DPI");
	double dpiScale = 1.0;
	{
		HDC screen = ::GetDC(NULL);
		if (screen != NULL)
		{
			int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
			dpiScale = static_cast<double>(dpiY) / 96.0;
			::ReleaseDC(NULL, screen);
		}
	}
	m_Scale = dpiScale;
	DB(ConsoleHandle, L"获取DPI成功");
	m_loadingWindow = Ui_LoadingSDL::GetInstance();
	DB(ConsoleHandle, L"创建按获取Ui_LoadingSDL单例成功");
	if (!m_loadingWindow->Start(nullptr,
		static_cast<float>(dpiScale)))
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"正在加载SDL窗口创建失败");
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"正在加载SDL窗口创建成功");
	}
	m_loadingWindow->SetWindowTopMost();
	

	if (!InitApp())
	{
		return FALSE;
	}
	m_Dlg_Main.ShowWindow(SW_SHOW);
	m_Dlg_Main.SetForegroundWindow();
	m_Dlg_Main.SetWindowTextW(L"极速录屏大师-main");
	m_Dlg_Main.JudgeDeviceBindBtnShowable();//判断设备列表按钮是显示还是隐藏
	if (m_loadingWindow)
		m_loadingWindow->Destroy();//销毁"正在加载"窗口
	m_IsInitialSuccess = true;

	RegisterUninstallEntry(
		L"极速录屏大师"	  // 卸载程序显示名
	);

	//注册快捷键
	m_pMainWnd->m_hWnd = m_Dlg_Main.m_hWnd;
	BOOL isRegKeyTrue = RegisterHotKey(m_pMainWnd->m_hWnd, ALT_B_STARTORSTOPRECORD, MOD_ALT, 'B');
	if (!isRegKeyTrue)
		DB(ConsoleHandle, L"注册快捷键失败");

	UserOpenMain(true);
	return TRUE;
}

bool theApp::InitApp()//耗时操作
{
	//初始化GDI资源
	m_Thread_RecordPreHot = std::thread(&theApp::RecordPreHotThread, this);
	m_Thread_RecordPreHot.detach();
	initGdiPlus();
	DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化GDI资源成功");
	if (!RequestToken())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"启动失败，未获取到token");
		return false;
	}

	//读取用户配置文件并尝试进行自动登录和第一次弹框
	if (ReadConfig())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"读取到有效的用户配置并已刷新用户信息");
		m_isLoginIn = true;
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"未读取到有效的用户配置，将以游客模式启动");
	}

	//套餐信息获取
	if (!App.RequestPrice())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"获取价格时出现异常");
	}

	//初始化对话框资源
	m_Dlg_Main.Create(IDD_DIALOG_MAINDLG);
	m_pMainWnd = &m_Dlg_Main;    //设置主窗口
	m_Dlg_Main.PreCreateAllChildDialogs();
	bool isAutoStartEnabled = AutoStartManager::IsAutoStartEnabled();//获取用户是否选择了开机启动 
	if (isAutoStartEnabled)
		m_Dlg_Main.m_Dlg_Config->m_Btn_OpenStart.LoadPNG(CONFIGDLG_PNG_OPENSTART_HOLD);
	else
		m_Dlg_Main.m_Dlg_Config->m_Btn_OpenStart.LoadPNG(CONFIGDLG_PNG_OPENSTART);

	m_Dlg_Main.m_Dlg_Config->m_Bool_IsOpenStart = isAutoStartEnabled;

	if (m_isLoginIn)
	{
		m_Dlg_Main.ShowLoginInUi();
		m_Dlg_Main.m_Dlg_Gaming->ShowLoginUi();
		m_Dlg_Main.m_Dlg_Carmera->UpdateLoginUi();
	}

	//判断绑定设备是否超过套餐设定
	if (m_isLoginIn)
	{
		if (m_userInfo.maxBindings < m_userInfo.currentBindings)
		{
			//设置主对话框状态，这个标志将会让主对话框在第一次显示后一段时间内提示用户解绑多于设备，否则退出程序
			m_Dlg_Main.SetNeedReduceBinds(true);
			m_IsOverBinds = true;
		}
	}
	//获取用户电脑所有的多媒体设备
	DeviceManager::GetInstance();
	return true;
}

bool theApp::ReadConfig()
{
	//读取Config.ini文件，是否自动登录
	ConfigFileHandler configHandler(m_ConfigPath);
	if (configHandler.ParseData())
	{
		m_IsAutoLogin = configHandler.asBool("AppConfig", "AutoLogin");
		m_IsHasOpenVip = configHandler.asBool("Config", "IsHasOpenVip");
		m_IsNeedShowMouthBill = configHandler.asBool("Config", "IsNeedShowMouthBill");
		//m_IsHasMouthBillWindowShowed = configHandler.asBool("Config", "IsHasMouthBillWindowShowed");
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"解析配置文件失败!");
	}
	DB(ConsoleHandle, L"自动登录:%s，第一次打开程序:%s，是否需要显示月度套餐:%s，是否弹过月度会员窗口:%s",
		m_IsAutoLogin ? L"true" : L"false",
		m_IsHasOpenVip ? L"true" : L"false",
		m_IsNeedShowMouthBill ? L"true" : L"false",
		IsHasMouthBillWindowShowed ? L"true" : L"false"
	);

	//如果不需要自动登录，返回false
	if (!m_IsAutoLogin)
		return false;

	// 调用接口刷新用户信息
	DEBUG_CONSOLE_STR(ConsoleHandle, L"开始刷新用户信息...");
	bool fetchSuccess = RequestDeviceInfo();
	if (fetchSuccess)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息刷新成功");
		if (m_IsVip)DEBUG_CONSOLE_STR(ConsoleHandle, L"用户是VIP");
		return true;
	}
	else
	{
		return false;
	}
}

void theApp::SaveUserAddedVideos()
{
	// 获取主对话框
	Ui_MainDlg* pMainDlg = dynamic_cast<Ui_MainDlg*>(m_pMainWnd);
	if (pMainDlg && pMainDlg->m_Dlg_Videolist)
	{
		// 获取用户添加的视频列表
		std::vector<VideoInfo> userVideos = pMainDlg->m_Dlg_Videolist->GetUserAddedVideos();

		if (!userVideos.empty())
		{
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"正在保存 %d 个用户添加的视频到配置文件", userVideos.size());

			// 获取应用程序路径
			TCHAR szPath[MAX_PATH];
			GetModuleFileName(NULL, szPath, MAX_PATH);

			// 去掉文件名，只保留路径
			PathRemoveFileSpec(szPath);

			// 构建配置文件路径
			CString configPath = szPath;
			configPath += _T("\\UserVideos.ini");

			// 使用Unicode方式写入配置文件
			HANDLE hFile = CreateFile(configPath, GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				// 写入UTF-16LE的BOM
				WORD bom = 0xFEFF;
				DWORD bytesWritten = 0;
				WriteFile(hFile, &bom, sizeof(WORD), &bytesWritten, NULL);

				// 构建配置文件内容
				std::wstring content;

				// 添加标题行
				content += L"[UserAddedVideos]\r\n";

				// 添加数量
				wchar_t countLine[50];
				swprintf_s(countLine, L"Count=%d\r\n", userVideos.size());
				content += countLine;

				// 写入每个视频的信息
				for (size_t i = 0; i < userVideos.size(); i++)
				{
					const VideoInfo& video = userVideos[i];

					wchar_t indexPrefix[20];
					swprintf_s(indexPrefix, L"Video%d_", i + 1);

					// 写入文件路径
					content += indexPrefix;
					content += L"Path=";
					content += video.filePath;
					content += L"\r\n";

					// 写入文件名
					content += indexPrefix;
					content += L"Name=";
					content += video.fileName;
					content += L"\r\n";

					// 写入时长
					content += indexPrefix;
					content += L"Duration=";
					content += video.duration;
					content += L"\r\n";
				}

				// 写入文件
				WriteFile(hFile, content.c_str(), (DWORD)(content.length() * sizeof(wchar_t)),
					&bytesWritten, NULL);

				// 关闭文件
				CloseHandle(hFile);

				DEBUG_CONSOLE_FMT(ConsoleHandle, L"已保存 %d 个用户添加的视频到配置文件: %s",
					userVideos.size(), (LPCTSTR)configPath);
			}
			else
			{
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"无法创建配置文件: %s, 错误码: %d",
					(LPCTSTR)configPath, GetLastError());
			}
		}
	}
}

void theApp::SaveAppTokenToConfigFile()
{
	ConfigFileHandler ConfigHandler(m_ConfigPath);
	if (ConfigHandler.WriteConfigFile("AppConfig", "AppToken", m_appToken))
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"写入AppToken到配置文件失败");
	}
}

bool theApp::JudgeSingleAppIns()
{
	m_hInstanceMutex = ::CreateMutex(
		NULL,
		TRUE,
		_T("Local\\MyApp_Unique_SingleInstance_Mutex")
	);
	if (!m_hInstanceMutex)
	{
		return false;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// 已经有一个实例在运行，尝试把它激活到前台
		HWND hOldWnd = ::FindWindow(
			NULL,
			_T("极速录屏大师-main")
		);
		if (hOldWnd)
		{
			::ShowWindow(hOldWnd, SW_SHOW);
			::SetForegroundWindow(hOldWnd);
			::SetWindowPos(hOldWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		return false;
	}
	return true;
}

bool theApp::RegisterUninstallEntry(
	LPCWSTR displayName     // 控制面板里显示的名字
)
{
	// 1. 获取当前可执行文件的完整路径
	wchar_t szExePath[MAX_PATH] = { 0 };
	::GetModuleFileNameW(NULL, szExePath, _countof(szExePath));

	// 2. 取可执行文件所在目录
	CString strDir = szExePath;
	::PathRemoveFileSpecW(strDir.GetBuffer());
	strDir.ReleaseBuffer();

	// 3. 构造卸载程序的完整路径（同级目录下 uninstall.exe）
	CString uninstallExePath = strDir + L"\\uninstall.exe";

	// 如果卸载程序不存在，则不注册，返回 false
	if (!PathFileExistsW(uninstallExePath))
	{
		DB(ConsoleHandle, L"卸载程序不存在，注入卸载程序到控制面板失败！");
		return false;
	}

	// 把根从 HKEY_LOCAL_MACHINE 改成 HKEY_CURRENT_USER
	CString appKey = L"Uninstall_" + CString(displayName);
	CString subKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + appKey;
	HKEY   hRootKey = HKEY_CURRENT_USER;

	HKEY hKey = nullptr;
	LONG l = RegCreateKeyExW(
		hRootKey,
		subKey,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_WOW64_64KEY,
		nullptr,
		&hKey,
		nullptr);
	if (l != ERROR_SUCCESS)
	{
		DB(ConsoleHandle, L"注入卸载程序到控制面板失败！");
		return false;
	}

	// 必写值：在控制面板中显示的程序名称
	RegSetValueExW(
		hKey,
		L"DisplayName",
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(displayName),
		static_cast<DWORD>((wcslen(displayName) + 1) * sizeof(wchar_t)));

	// 必写值：卸载命令行，引用卸载程序
	CString cmd = L"\"" + uninstallExePath + L"\"";
	RegSetValueExW(
		hKey,
		L"UninstallString",
		0,
		REG_SZ,
		reinterpret_cast<const BYTE*>(static_cast<LPCWSTR>(cmd)),
		static_cast<DWORD>((wcslen(cmd) + 1) * sizeof(wchar_t)));

	//加载卸载程序图标到控制面板
	CString iconValue = uninstallExePath + L",0";
	RegSetValueExW(hKey, L"DisplayIcon", 0, REG_SZ,
		reinterpret_cast<const BYTE*>((LPCWSTR)iconValue),
		DWORD((wcslen(iconValue) + 1) * sizeof(wchar_t)));

	// 版本号、发布商、安装目录等
	// RegSetValueExW(hKey, L"DisplayVersion", 0, REG_SZ, (BYTE*)L"1.0.0", sizeof(L"1.0.0"));
	// RegSetValueExW(hKey, L"Publisher",      0, REG_SZ, (BYTE*)L"MyCompany", sizeof(L"MyCompany"));
	// RegSetValueExW(hKey, L"InstallLocation",0, REG_SZ, (BYTE*)strDir,    (DWORD)((strDir.GetLength()+1)*sizeof(wchar_t)));

	RegCloseKey(hKey);
	return true;
}

void theApp::CleanUpInterface()
{
	CameraCapture::ReleaseInstance();
	ScreenRecorder::ReleaseInstance();
}

int theApp::ExitInstance()
{
	UserShutDownExe(false);

	//设置是否开机启动
	if (m_IsInitialSuccess)
	{
		bool enableAutoStart = m_Dlg_Main.m_Dlg_Config->m_Bool_IsOpenStart;
		bool success = AutoStartManager::SetAutoStart(enableAutoStart);
		if (!success)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"设置开机启动失败");
			Sleep(2000);
		}
		m_Dlg_Main.m_Dlg_Config->SaveConfigToFile();	//写入用户设置的录制选项配置文件信息
		m_Dlg_Main.m_Dlg_Videolist->WriteToConfigFile();//写入视频列表数据到配置文件
		SaveUserAddedVideos();     //保存用户添加的视频列表
		SaveAppTokenToConfigFile();//保存用户的AppToken到配置文件
		CleanUpGDIRes();           //释放所有GDI资源
		CleanUpWindows();          //销毁所有窗口
		CleanUpInterface();        //释放所有的接口

		//记录用户的应用程序级配置信息
		ConfigFileHandler ConfigHandler(App.m_ConfigPath);
		ConfigHandler.WriteConfigFile("Config", "IsHasOpenVip", m_IsHasOpenVip ? "1" : "0");
		//ConfigHandler.WriteConfigFile("Config", "IsHasMouthBillWindowShowed", m_IsHasMouthBillWindowShowed ? "1" : "0");

		//写入用户userinfo信息（如果登录了的话）
		if (m_isLoginIn && m_userInfo.nickname != "null")
		{
			ConfigFileHandler configHandler(m_ConfigPath);
			configHandler.WriteConfigFile("Userinfo", "phone_number", LARSC::CStringToStdString(m_userInfo.nickname));
		}
		CleanUpGDI();              //关闭所有GDI资源
	}

	UserShutDownExe(true);
	return CWinApp::ExitInstance();
}

bool theApp::Test()
{
	return true;
}

bool theApp::IsValidPaidSubscription(const UserInfo& userInfo)
{
	if (!userInfo.isPaid)// 检查用户是否已付费
	{
		return false;
	}
	if (userInfo.expiresAt.IsEmpty())// 检查是否有过期时间
	{
		return true; // 已付费且无过期时间，视为永久会员
	}

	// 获取当前系统时间
	CTime currentTime = CTime::GetCurrentTime();
	// 解析过期时间
	CTime expiryTime;
	int year, month, day, hour, minute, second;
	if (_stscanf_s(userInfo.expiresAt, _T("%d-%d-%d %d:%d:%d"),
		&year, &month, &day, &hour, &minute, &second) != 6)
	{
		return false;
	}
	expiryTime = CTime(year, month, day, hour, minute, second);
	return (expiryTime > currentTime);
}

bool theApp::IsNotOverBinds(const UserInfo& userInfo)
{
	DBFMT(ConsoleHandle, L"判断当前绑定数量是否超限:%s",
		userInfo.currentBindings <= userInfo.maxBindings ? L"false" : L"true");
	bool IsOverBind = (userInfo.currentBindings > userInfo.maxBindings);

	return IsOverBind;
}

bool theApp::RequestPrice()
{
	std::string Url = "http://localhost:5000/api/pricing";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::GET,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	requestHandler.AddHeader("x-machine-guid", m_machine_guid);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ValueEncodeType::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			// 先解析额外字段(红包 + 低价格)
			if (Root["data"].isMember("coupon_price") && !Root["data"]["coupon_price"].isNull())
			{
				m_CouponPrice.id =
					Root["data"]["coupon_price"]["id"].isNull() ?
					-1 : Root["data"]["coupon_price"]["id"].asUInt();
				m_CouponPrice.name =
					Root["data"]["coupon_price"]["name"].isNull() ?
					L"" : LARSC::s2ws(Root["data"]["coupon_price"]["name"].asString());
				m_CouponPrice.badge =
					Root["data"]["coupon_price"]["badge"].isNull() ?
					L"" : LARSC::s2ws(Root["data"]["coupon_price"]["badge"].asString());
				m_CouponPrice.amount =
					Root["data"]["coupon_price"]["amount"].isNull() ?
					0 : Root["data"]["coupon_price"]["amount"].asUInt();
				m_CouponPrice.desc =
					Root["data"]["coupon_price"]["desc"].isNull() ?
					0 : LARSC::s2ws(Root["data"]["coupon_price"]["desc"].asString());
				m_MouthBillPrice = m_CouponPrice.amount;
			}
			if (Root["data"].isMember("coupon_amount") && !Root["data"]["coupon_amount"].isNull())
			{
				m_ReadPacketAmount = Root["data"]["coupon_amount"].asInt();
			}

			// 解析预订单号
			if (Root["data"].isMember("pre_order_no") && !Root["data"]["pre_order_no"].isNull())
			{
				m_preOrderNo = Root["data"]["pre_order_no"].asString();
			}

			// 解析价格信息
			if (Root["data"].isMember("prices") && Root["data"]["prices"].isArray())
			{
				Json::Value prices = Root["data"]["prices"];
				for (Json::Value::ArrayIndex i = 0; i < prices.size(); i++)
				{
					PriceInfo info;

					// 获取ID
					info.id = prices[i].get("id", 0).asInt();

					// 获取名称 
					if (prices[i].isMember("name") && !prices[i]["name"].isNull())
					{
						std::wstring wideName = LARSC::s2ws(prices[i]["name"].asString());
						info.name = wideName.c_str();
					}

					// 获取标签
					if (prices[i].isMember("badge") && !prices[i]["badge"].isNull())
					{
						std::wstring wideBadge = LARSC::s2ws(prices[i]["badge"].asString());
						info.badge = wideBadge.c_str();
					}

					// 获取价格
					info.amount = prices[i].get("amount", 0.0).asDouble();

					// 获取描述
					if (prices[i].isMember("desc") && !prices[i]["desc"].isNull())
					{
						std::wstring wideDesc = LARSC::s2ws(prices[i]["desc"].asString());
						info.desc = wideDesc.c_str();
					}

					// 添加到数组
					m_PriceInfos.push_back(info);
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"解析到价格信息: ID=%d, 名称=%s, 价格=%.2f, 描述=%s",
						info.id, (LPCTSTR)info.name, info.amount, (LPCTSTR)info.GetFormattedDesc());
				}
				return true;
			}
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
			return false;
		}
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return false;
	}
}

bool theApp::RequestToken()
{
	std::string Url = "http://localhost:5000/api/launch";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;

#define APP_CODE "SCRNREC-1"
#define APP_KEY "ABC123DEF456"
#define TOKEN_FILE_NAME "token.txt"
#define APPTOKEN_FILE_NAME "config.ini"
	CString exePath = GetExecutablePath();
	CW2A asciiPath(exePath, CP_UTF8);
	std::string savePath(asciiPath);
	std::string osVersion = GetDetailedOSVersion();
	std::string osName = MapWindowsVersion();
	std::string architecture = MapArchitecture();

	Root["os"]["name"] = osName;
	Root["os"]["version"] = osVersion;
	Root["os"]["architecture"] = architecture;
	GetCpuInfo(Root["hardware"]["cpu"]);
	GetGpuInfo(Root["hardware"]["gpu"]);
	GetMemoryInfo(Root["hardware"]["memory"]);
	GetDiskInfo(Root["hardware"]["disk"]);
	GetDisplayInfo(Root["hardware"]["display"]);
	GetMachineGuid(Root["hardware"]["machine_guid"]);
	Root["app"]["key"] = APP_KEY;
	Root["app"]["code"] = APP_CODE;
	Root["app"]["path"] = savePath;
	Root["app"]["version"] = "1.0.0";

	//token
	//m_appToken = FindAppTokenFromFile(APPTOKEN_FILE_NAME);
	//DBFMT(ConsoleHandle, L"从文件中找到的AppToken:%s", LARSC::s2ws(m_appToken).c_str());
	//m_InstallerToken = FindInstallTokenFromFile();
	//DBFMT(ConsoleHandle, L"从文件中找到的安装包Token:%s", LARSC::s2ws(m_InstallerToken).c_str());
	//if (m_InstallerToken == "NULL")
	//{
	//	DEBUG_CONSOLE_STR(ConsoleHandle, L"找不到Token，正式版在这里退出程序");
	//	Ui_LoadingSDL::GetInstance()->Destroy();
	//	Ui_MessageModalDlg MessageModal;
	//	MessageModal.SetModal(L"极速录屏大师", L"Ops!出错了", L"无法打开应用程序，缺少关键文件，重新安装解决此问题", L"确认");
	//	MessageModal.DoModal();
	//	return false;
	//}
	//Root["install_token"] = GlobalFunc::AnsiToUtf8(m_InstallerToken).c_str();

	requestHandler.AddJsonBody(Root);
	//if (m_appToken != "NULL")
	//	requestHandler.AddHeader("x-api-token", m_appToken);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ValueEncodeType::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			//m_appToken = Root["data"]["token"].asString();
			m_IsNonUserPaid = Root["data"]["is_non_user_paid"].asBool();
			m_CanGuestBuy = Root["data"]["can_guest_buy"].asBool();
			//m_CanGuestBuy = true
			if (!Root["data"]["user"].isNull())
			{
				DB(ConsoleHandle, L"user信息不为null");
				return true;
			}
			else
			{
				DB(ConsoleHandle, L"程序启动接口，回传user字段为空");
				return true;
			}
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
			return false;
		}
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return true;
	}
	return false;
}

bool theApp::RequestToken(std::string appToken)
{
	std::string Url = "http://localhost:5000/api/launch";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;

#define APP_CODE "SCRNREC-1"
#define APP_KEY "ABC123DEF456"
#define TOKEN_FILE_NAME "token.txt"
#define APPTOKEN_FILE_NAME "config.ini"
	CString exePath = GetExecutablePath();
	CW2A asciiPath(exePath, CP_UTF8);
	std::string savePath(asciiPath);
	std::string osVersion = GetDetailedOSVersion();
	std::string osName = MapWindowsVersion();
	std::string architecture = MapArchitecture();

	Root["os"]["name"] = osName;
	Root["os"]["version"] = osVersion;
	Root["os"]["architecture"] = architecture;
	GetCpuInfo(Root["hardware"]["cpu"]);
	GetGpuInfo(Root["hardware"]["gpu"]);
	GetMemoryInfo(Root["hardware"]["memory"]);
	GetDiskInfo(Root["hardware"]["disk"]);
	GetDisplayInfo(Root["hardware"]["display"]);
	Root["app"]["key"] = APP_KEY;
	Root["app"]["code"] = APP_CODE;
	Root["app"]["path"] = savePath;
	Root["app"]["version"] = "1.0.0";

	//token
	m_appToken = appToken;
	DBFMT(ConsoleHandle, L"从文件中找到的AppToken:%s", LARSC::s2ws(m_appToken).c_str());
	m_InstallerToken = FindInstallTokenFromFile();
	DBFMT(ConsoleHandle, L"从文件中找到的安装包Token:%s", LARSC::s2ws(m_InstallerToken).c_str());
	if (m_InstallerToken == "NULL")
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"找不到Token，正式版在这里退出程序");
		Ui_LoadingSDL::GetInstance()->Destroy();
		Ui_MessageModalDlg MessageModal;
		MessageModal.SetModal(L"极速录屏大师", L"Ops!出错了", L"无法打开应用程序，缺少关键文件，重新安装解决此问题", L"确认");
		MessageModal.DoModal();
		return false;
	}
	Root["install_token"] = GlobalFunc::AnsiToUtf8(m_InstallerToken).c_str();

	requestHandler.AddJsonBody(Root);
	if (m_appToken != "NULL")
		requestHandler.AddHeader("x-api-token", m_appToken);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ValueEncodeType::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			m_appToken = Root["data"]["token"].asString();
			m_IsNonUserPaid = Root["data"]["is_non_user_paid"].asBool();
			m_CanGuestBuy = Root["data"]["can_guest_buy"].asBool();
			if (!Root["data"]["user"].isNull())
			{
				DB(ConsoleHandle, L"user信息不为null");
				return true;
			}
			else
			{
				DB(ConsoleHandle, L"程序启动接口，回传user字段为空");
				return true;
			}
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
			return false;
		}
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return true;
	}
	return false;
}

bool theApp::RequestSeedCode()
{
	std::string Url = "http://localhost:5000/api/send-code";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;
	CString PhoneNumber;
	App.m_Dlg_Main.m_Dlg_Login->m_Edit_PhoneNum.GetWindowTextW(PhoneNumber);
	Root["phone_number"] = LARSC::CStringToStdString(PhoneNumber);

	requestHandler.AddHeader("x-machine-guid", m_machine_guid);
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

bool theApp::RequestSignIn()
{
	std::string Url = "http://localhost:5000/api/login-by-phone";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;
	CString PhoneNumber, VerityCode;
	App.m_Dlg_Main.m_Dlg_Login->m_Edit_PhoneNum.GetWindowTextW(PhoneNumber);
	App.m_Dlg_Main.m_Dlg_Login->m_Edit_VerityCode.GetWindowTextW(VerityCode);
	Root["phone_number"] = LARSC::CStringToStdString(PhoneNumber);
	Root["code"] = LARSC::CStringToStdString(VerityCode);

	//requestHandler.AddHeader("x-api-token", App.m_appToken);
	requestHandler.AddHeader("x-machine-guid", m_machine_guid);
	requestHandler.AddJsonBody(Root);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			if (!Root["data"].isNull() && !Root["data"]["user"].isNull())
			{
				DB(ConsoleHandle, L"解析用户信息");
				ExtractUserDeviceInfo(Root);
				DB(ConsoleHandle, L"判断用户是否为VIP");
				m_IsVip = IsValidPaidSubscription(m_userInfo);
				if (m_IsVip)
				{
					DB(ConsoleHandle, L"用户是VIP，开始判断是否是设备超限");
					m_IsOverBinds = IsNotOverBinds(m_userInfo);
					DBFMT(ConsoleHandle, L"设备超限:%s", m_IsOverBinds ? L"false" : L"true");
				}
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
	return false;
}

bool theApp::RequestDeviceInfo()
{
	std::string Url = "http://localhost:5000/api/info";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::GET,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	//requestHandler.AddHeader("x-api-token", m_appToken);
	requestHandler.AddHeader("x-machine-guid", m_machine_guid);
	if (m_isLoginIn && !m_userInfo.nickname.IsEmpty())
	{
		requestHandler.AddHeader("x-phone-number", LARSC::CStringToStdString(m_userInfo.nickname));
	}
	else
	{
		//读取配置文件中的config.ini的phone_number(本设备最近一次登录所使用的手机号)
		ConfigFileHandler configHandler(m_ConfigPath);
		if (configHandler.ParseData())
		{
			auto phone_number_str = configHandler.asString("Userinfo", "phone_number");
			if (phone_number_str == "null")
			{
				DenyMessageDlg("发生错误，错误代码:246");
				throw std::runtime_error("246");
			}
			requestHandler.AddHeader("x-phone-number", phone_number_str);
		}
		else
		{
			DenyMessageDlg("发生错误，错误代码:245");
			throw std::runtime_error("245");
		}
	}

	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			if (!Root["data"].isNull() && !Root["data"]["user"].isNull())
			{
				bool isOk = false;
				isOk = ExtractUserDeviceInfo(Root);
				if (isOk)
				{
					DB(ConsoleHandle, L"解析成功，开始判断是否是vip");
					m_IsVip = IsValidPaidSubscription(m_userInfo);
					DBFMT(ConsoleHandle, L"解析后，判断是vip:%s", m_IsVip ? L"true" : L"false");
					if (m_IsVip)
					{
						DB(ConsoleHandle, L"开始判断是否是设备超限");
						m_IsOverBinds = IsNotOverBinds(m_userInfo);
						DBFMT(ConsoleHandle, L"设备超限:%s", m_IsOverBinds ? L"false" : L"true");
					}
				}
			}
			else
			{
				DB(ConsoleHandle, L"data字段为空，或者data的user字段为空,未成功完整获取user信息");
				m_userInfo = UserInfo{};
				m_IsVip = false;
				m_IsOverBinds = false;
			}
			m_IsNonUserPaid = Root["data"]["is_non_user_paid"].asBool();
			m_IsNeedShowMouthBill = 
				((!App.m_IsNonUserPaid && !App.m_IsVip) && m_IsHasOpenVip && m_IsNeedShowMouthBill) ? 
				true : false;
			return true;
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
		}

		return false;
	}
	else
	{
		DB(ConsoleHandle, LARSC::s2ws(requestHandler.GetErrorMessage()).c_str());
		return false;
	}
}

bool theApp::RequestDeviceList()
{
	//std::string Url = "http://scrnrec.appapr.com/api/devices";
	std::string Url = "http://localhost:5000/api/devices";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);

	struct DeviceBindingInfo
	{
		//std::string code;
		std::string os_name;
		std::string created_at;
		bool is_current;
		DeviceBindingInfo() {}
	};
	std::vector<DeviceBindingInfo>* Vec_DeviceBindingList = new std::vector<DeviceBindingInfo>;

	Json::Value Root;
	Root["phone_number"] = LARSC::CStringToStdString(App.m_userInfo.nickname);
	requestHandler.AddHeader("x-machine-guid", m_machine_guid);
	requestHandler.AddJsonBody(Root);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (!Root["success"].isNull() && Root["success"].asBool())
		{
			if (!Root["data"].isNull() && Root["data"].isArray())
			{
				Json::Value& dataArray = Root["data"];
				for (size_t i = 0; i < dataArray.size(); i++)
				{
					DeviceBindingInfo deviceBindInfo;
					//deviceBindInfo.code = dataArray[i]["code"].asString();
					deviceBindInfo.os_name = dataArray[i]["os_name"].asString();
					deviceBindInfo.created_at = dataArray[i]["created_at"].asString();
					deviceBindInfo.is_current = dataArray[i]["is_current"].asBool();
					Vec_DeviceBindingList->push_back(deviceBindInfo);
				}
				if (m_Dlg_Main)
				{
					::PostMessage(
						m_Dlg_Main.GetSafeHwnd(),
						HTTPREQUESTCOMPELETE_UPDATE_DEVICEBINGSLIST,
						NULL,
						(LPARAM)Vec_DeviceBindingList);
				}
				return true;
			}
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
			return false;
		}
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return false;
	}
}

bool theApp::RequestDeviceUnbind(std::string DeviceCode)
{
	std::string Url = "http://localhost:5000/api/unbind";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::POST,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);
	Json::Value Root;
	Root["phone_number"] = LARSC::CStringToStdString(m_userInfo.nickname);
	requestHandler.AddHeader("x-machine-guid", m_machine_guid);
	requestHandler.AddJsonBody(Root);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (Root["success"].isNull() || !Root["success"].asBool())
		{
			DenyMessageDlg(Root["message"].asString());
			return false;
		}
		else
		{
			m_IsOverBinds = IsNotOverBinds(m_userInfo);
			DB(ConsoleHandle, L"解绑后，用户超限状态:%s", m_IsOverBinds ? L"超限" : L"未超限");
			return true;
		}
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return false;
	}
}

bool theApp::RequestSignOut()
{
	/*
	* 我接手数据库后端后，如果执行当前RequestSignOut的逻辑，做法与RequestDeviceUnbind无异，但是考虑到
	* 如果RequestSignOut也是RequestDeviceUnbind的逻辑（直接删除数据库当前设备和手机号的项信息）会可能导致
	* 丢失注册过的用户所有信息，所以我打算这里简单的把自动登录标志设为false，然后直接返回成功即可
	*/
	App.m_IsAutoLogin = false;
	return true;

	std::string Url = "http://localhost:5000/api/logout";
	HttpRequestHandler requestHandler(
		HttpRequestHandler::RequestType::GET,
		HttpRequestHandler::ValueEncodeType::UTF8,
		Url
	);

	requestHandler.AddHeader("x-api-token", m_appToken);
	if (requestHandler.PerformRequest())
	{
		Json::Value Root = requestHandler.GetResponeData(HttpRequestHandler::ANSI);
		if (!Root["success"].isNull() || Root["success"].asBool())
		{
			if (m_Dlg_Main && m_Dlg_Main.m_Dlg_UserPrifile && App.m_isLoginIn)
				::PostMessage(m_Dlg_Main.m_Dlg_UserPrifile->GetSafeHwnd(), MSG_APPMSG_LOGOUTSUCCESS, NULL, NULL);
			App.m_isLoginIn = false;
			auto pIns = DialogManager::GetInstance();
			pIns->BroadcastMessage(BROADCASTMSG_USERLOGIN_ISLOGINOUT, NULL, NULL);
			return true;
		}
		else
		{
			DenyMessageDlg(Root["message"].asString());
			return false;
		}
	}
	else
	{
		ErrorMessageDlg(requestHandler);
		return false;
	}
}

bool theApp::UserOpenMain(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if(!isSuccess)
		m_userActionLogger->Log(L"打开了主程序");
	else
		m_userActionLogger->Log(L"打开主程序成功");
	return true;
}

bool theApp::UserScreenRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始全屏录制");
	else
		m_userActionLogger->Log(L"全屏录制成功");
	return true;
}

bool theApp::UserAreaRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始区域录制");
	else
		m_userActionLogger->Log(L"区域录制成功");
	return true;
}

bool theApp::UserGamingRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始游戏录制");
	else
		m_userActionLogger->Log(L"游戏录制成功");
	return true;
}

bool theApp::UserWindowRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始窗口录制");
	else
		m_userActionLogger->Log(L"窗口录制成功");
	return true;
}

bool theApp::UserOnlyAudioRecord(bool isSuccss)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccss)
		m_userActionLogger->Log(L"开始纯音频录制");
	else
		m_userActionLogger->Log(L"纯音频录制成功");
	return true;
}

bool theApp::UserCarmeraRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始摄像头录制");
	else
		m_userActionLogger->Log(L"摄像头录制成功");
	return true;
}

bool theApp::UserFollowMouseRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始跟随鼠标录制");
	else
		m_userActionLogger->Log(L"跟随鼠标录制成功");
	return true;
}

bool theApp::UserOpenVip(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"打开VIP界面");
	else
		m_userActionLogger->Log(L"打开VIP界面成功");
	return true;
}

bool theApp::UserPay(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"支付开始轮询");
	else
		m_userActionLogger->Log(L"支付成功");
	return true;
}

bool theApp::UserShutDownExe(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"关闭程序");
	else
		m_userActionLogger->Log(L"关闭程序成功");
	return true;
}

bool theApp::UserTriggerRecord()
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	m_userActionLogger->Log(L"触发了录制行为");
	return true;
}

bool theApp::UserRecordWithCarmera(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"打开了摄像头预览");
	else
		m_userActionLogger->Log(L"打开摄像头预览成功");
	return true;
}

bool theApp::UserCloseCarmeraPreviewWithRecord(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"关闭了摄像头预览");
	else
		m_userActionLogger->Log(L"关闭摄像头预览成功");
	return true;
}

bool theApp::UserOpenCarmerPreview_InCameraDlg(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开启了摄像头预览（摄像头录制UI）");
	else
		m_userActionLogger->Log(L"开启摄像头预览成功（摄像头录制UI）");
	return true;
}

bool theApp::UserCloseCarmerPreview_InCameraDlg(bool isSuccess)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	if (!isSuccess)
		m_userActionLogger->Log(L"开始关闭摄像头预览（摄像头录制UI）");
	else
		m_userActionLogger->Log(L"关闭摄像头预览成功（摄像头录制UI）");
	return true;
}

bool theApp::UserNoneVipRecordSuccess()
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	m_userActionLogger->Log(L"非VIP自动停止录制，录制成功");
	return true;
}

bool theApp::UserRecordSuccess()
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	m_userActionLogger->Log(L"录制完成且成功");
	return true;
}

bool theApp::UserCloseOpenVip()
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	m_userActionLogger->Log(L"关闭VIP界面");
	return true;
}

bool theApp::UserOpenMouthBiil()
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	m_userActionLogger->Log(L"打开低价套餐界面");
	return true;
}

bool theApp::UserCloseMouthBill()
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	m_userActionLogger->Log(L"关闭打开低价套餐界面");
	return true;
}

bool theApp::UserClickMeal(int id, CString name, int price)
{
	if (!m_userActionLogger)
	{
		DB(ConsoleHandle, L"尝试写入日志文件失败，日志记录器为NULL");
		return false;
	}
	std::ostringstream ostrsream;
	ostrsream << "[id]" << id << " [price]" << price;
	auto log = ostrsream.str();
	auto wlog = L"选择套餐信息：" + LARSC::s2ws(log) + L" [name]:" + std::wstring(name);
	m_userActionLogger->Log(wlog);
	return true;
}

void theApp::RecordPreHotThread()
{
	ScreenRecorder::Preheat();
}

void theApp::UpdateUserInfo(UserInfo userInfo)
{
	m_userInfo = userInfo;
	m_IsVip = IsValidPaidSubscription(m_userInfo);
	m_IsOverBinds = IsNotOverBinds(m_userInfo);
	m_isLoginIn = true;
}

UserInfo theApp::getUserInfo()
{
	if (m_isLoginIn)
	{
		return m_userInfo;
	}
	return UserInfo();
}

bool theApp::IsDeviceOverBind()
{
	return m_userInfo.currentBindings > m_userInfo.maxBindings;
}

bool theApp::ExtractUserDeviceInfo(Json::Value& data_user_Root)
{
	Json::Value& userRoot = data_user_Root["data"]["user"];
	if (userRoot.isNull())
	{
		DB(ConsoleHandle, L"用户信息为空，解析用户信息json失败");
		return false;
	}
	DB(ConsoleHandle, L"用户信息不为空，开始解析json");
	DB(ConsoleHandle, L"用户信息currentBindings解析");
	m_userInfo.currentBindings =
		!userRoot["current_bindings"].isNull() ? userRoot["current_bindings"].asInt() : -1;
	DB(ConsoleHandle, L"用户信息is_paid解析");
	m_userInfo.isPaid =
		!userRoot["is_paid"].isNull() ? userRoot["is_paid"].asBool() : false;
	DB(ConsoleHandle, L"用户信息nickname解析");
	m_userInfo.nickname =
		!userRoot["nickname"].isNull() ? CString(userRoot["nickname"].asCString()) : _T("");
	DB(ConsoleHandle, L"用户信息level解析");
	m_userInfo.level =
		!userRoot["level"].isNull() ? CString(GlobalFunc::AnsiToCString(userRoot["level"].asCString())) : _T("");
	DB(ConsoleHandle, L"用户信息expiresAt解析");
	m_userInfo.expiresAt =
		!userRoot["expires_at"].isNull() ? CString(userRoot["expires_at"].asCString()) : _T("");
	DB(ConsoleHandle, L"用户信息maxBindings解析");
	m_userInfo.maxBindings =
		!userRoot["max_bindings"].isNull() ? userRoot["max_bindings"].asInt() : 0;
	return true;
}

bool theApp::IsWindows7OrHigher()
{
	std::string windowsVersion = MapWindowsVersion();

	// 检查是否是 Windows 7 或更高版本
	if (windowsVersion.find("Windows 7") == 0 ||
		windowsVersion.find("Windows 8") == 0 ||
		windowsVersion.find("Windows 8.1") == 0 ||
		windowsVersion.find("Windows 10") == 0 ||
		windowsVersion.find("Windows 11") == 0)
	{
		return true;
	}

	return false;
}

std::string theApp::MapArchitecture()
{
	return GetSystemArchitecture();
}

std::string theApp::MapWindowsVersion() 
{
	try 
	{
		// 获取详细的Windows版本信息
		OSVERSIONINFOEX osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		// 由于GetVersionEx在较新的Windows上不可靠，我们使用自定义函数
		std::string detailedVersion = GetDetailedOSVersion();

		// 获取Windows版本名称
		std::string windowsName = "NULL";
		if (detailedVersion.find("10.0") != std::string::npos)
		{
			size_t pos = detailedVersion.find("Build ");
			if (pos != std::string::npos) {
				int buildNumber = std::stoi(detailedVersion.substr(pos + 6));
				if (buildNumber >= 22000)
					windowsName = "Windows 11";
				else
					windowsName = "Windows 10";
			}
			else
			{
				windowsName = "Windows 10";
			}
		}
		else if (detailedVersion.find("6.3") != std::string::npos)
			windowsName = "Windows 8.1";
		else if (detailedVersion.find("6.2") != std::string::npos)
			windowsName = "Windows 8";
		else if (detailedVersion.find("6.1") != std::string::npos)
			windowsName = "Windows 7";
		else if (detailedVersion.find("6.0") != std::string::npos)
			windowsName = "Windows Vista";
		else if (detailedVersion.find("5.1") != std::string::npos)
			windowsName = "Windows XP";

		// 检查是否是专业版、家庭版等
		DWORD bufferSize = MAX_PATH;
		WCHAR edition[MAX_PATH] = { 0 };
		HKEY hKey;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			DWORD dataSize = bufferSize * sizeof(WCHAR);
			RegQueryValueExW(hKey, L"EditionID", NULL, NULL, (LPBYTE)edition, &dataSize);
			RegCloseKey(hKey);

			if (wcslen(edition) > 0) {
				std::wstring wEdition(edition);
				std::string editionStr(wEdition.begin(), wEdition.end());

				if (editionStr == "Professional")
					windowsName += " Pro";
				else if (editionStr == "Enterprise")
					windowsName += " Enterprise";
				else if (editionStr == "Education")
					windowsName += " Education";
				else if (editionStr == "Home")
					windowsName += " Home";
			}
		}

		return windowsName;
	}
	catch (...) 
	{
		return "NULL";
	}
}

std::string theApp::GetDetailedOSVersion() {
	try {
		std::string version = "NULL";

		HKEY hKey;
		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			WCHAR releaseId[256] = { 0 };
			WCHAR currentBuild[256] = { 0 };
			WCHAR productName[256] = { 0 };

			DWORD bufferSize = sizeof(releaseId);
			RegQueryValueExW(hKey, L"ReleaseId", NULL, NULL, (LPBYTE)releaseId, &bufferSize);

			bufferSize = sizeof(currentBuild);
			RegQueryValueExW(hKey, L"CurrentBuild", NULL, NULL, (LPBYTE)currentBuild, &bufferSize);

			bufferSize = sizeof(productName);
			RegQueryValueExW(hKey, L"ProductName", NULL, NULL, (LPBYTE)productName, &bufferSize);

			RegCloseKey(hKey);

			// 构建详细版本信息
			std::wstring wProductName(productName);
			std::wstring wCurrentBuild(currentBuild);
			std::wstring wReleaseId(releaseId);

			// 转换为std::string
			std::string productNameStr(wProductName.begin(), wProductName.end());
			std::string currentBuildStr(wCurrentBuild.begin(), wCurrentBuild.end());
			std::string releaseIdStr(wReleaseId.begin(), wReleaseId.end());

			// 从产品名称中提取版本号（10.0或6.3等）
			size_t versionPos = productNameStr.find("Windows ");
			std::string versionNumber = "10.0"; // 默认为Windows 10/11

			if (versionPos != std::string::npos) {
				if (productNameStr.find("10") != std::string::npos)
					versionNumber = "10.0";
				else if (productNameStr.find("8.1") != std::string::npos)
					versionNumber = "6.3";
				else if (productNameStr.find("8") != std::string::npos)
					versionNumber = "6.2";
				else if (productNameStr.find("7") != std::string::npos)
					versionNumber = "6.1";
				else if (productNameStr.find("Vista") != std::string::npos)
					versionNumber = "6.0";
				else if (productNameStr.find("XP") != std::string::npos)
					versionNumber = "5.1";
			}
			version = versionNumber + " " + releaseIdStr + " N/A Build " + currentBuildStr;
		}

		return version;
	}
	catch (...) {
		return "NULL";
	}
}

void theApp::GetCpuInfo(Json::Value& cpuNode)
{
	try
	{
		DB(ConsoleHandle, L"开始使用COM接口获取CPU信息...");
		int  cpuInfo[4] = { -1 };
		char cpuBrandString[0x40] = { 0 };

		// 获取CPU名称（品牌串）
		__cpuid(cpuInfo, 0x80000000);
		unsigned int nExIds = cpuInfo[0];
		for (unsigned int i = 0x80000002; i <= nExIds && i <= 0x80000004; ++i)
		{
			__cpuid(cpuInfo, i);
			memcpy(cpuBrandString + (i - 0x80000002) * 16, cpuInfo, sizeof(cpuInfo));
		}

		DWORD logicalProcessorCount = 0;
		DWORD processorCoreCount = 0;
		std::string processorId = "NULL";

		bool needUninit = false;	// 是否需要执行CoUninitialize
		bool wmiQueryTried = false;	// 标记 WMI 是否整体成功
		HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (SUCCEEDED(hres))
		{
			needUninit = true;
		}
		else if (hres == RPC_E_CHANGED_MODE)
		{
			DB(ConsoleHandle, L"CoInitializeEx 返回 RPC_E_CHANGED_MODE，沿用已有COM初始化。");
		}
		else
		{
			DBFMT(ConsoleHandle, L"CoInitializeEx 失败 result=0x%08X (%d)", (unsigned)hres, (int)hres);
		}

		if (SUCCEEDED(hres) || hres == RPC_E_CHANGED_MODE)
		{
			// 初始化安全
			HRESULT secHr = CoInitializeSecurity(
				NULL, -1, NULL, NULL,
				RPC_C_AUTHN_LEVEL_DEFAULT,
				RPC_C_IMP_LEVEL_IMPERSONATE,
				NULL, EOAC_NONE, NULL
			);
			if (!(SUCCEEDED(secHr) || secHr == RPC_E_TOO_LATE))
			{
				DBFMT(ConsoleHandle, L"CoInitializeSecurity 失败 result=0x%08X (%d)", (unsigned)secHr, (int)secHr);
			}
			else
			{
				IWbemLocator* pLoc = nullptr;
				IWbemServices* pSvc = nullptr;

				HRESULT hr = CoCreateInstance(
					CLSID_WbemLocator, 0,
					CLSCTX_INPROC_SERVER,
					IID_IWbemLocator, (LPVOID*)&pLoc
				);
				if (SUCCEEDED(hr) && pLoc)
				{
					hr = pLoc->ConnectServer(
						_bstr_t(L"ROOT\\CIMV2"),
						NULL, NULL, 0, NULL, 0, 0, &pSvc
					);
					if (SUCCEEDED(hr) && pSvc)
					{
						hr = CoSetProxyBlanket(
							pSvc,
							RPC_C_AUTHN_WINNT,
							RPC_C_AUTHZ_NONE,
							NULL,
							RPC_C_AUTHN_LEVEL_CALL,
							RPC_C_IMP_LEVEL_IMPERSONATE,
							NULL,
							EOAC_NONE
						);
						if (SUCCEEDED(hr))
						{
							IEnumWbemClassObject* pEnumerator = nullptr;
							hr = pSvc->ExecQuery(
								bstr_t("WQL"),
								bstr_t("SELECT NumberOfCores, NumberOfLogicalProcessors, ProcessorId FROM Win32_Processor"),
								WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
								NULL, &pEnumerator
							);
							if (SUCCEEDED(hr) && pEnumerator)
							{
								wmiQueryTried = true;
								IWbemClassObject* pclsObj = nullptr;
								ULONG uReturn = 0;
								hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
								if (SUCCEEDED(hr) && uReturn == 1 && pclsObj)
								{
									VARIANT vtCores, vtThreads, vtPid;
									VariantInit(&vtCores);
									VariantInit(&vtThreads);
									VariantInit(&vtPid);

									if (SUCCEEDED(pclsObj->Get(L"NumberOfCores", 0, &vtCores, 0, 0)) &&
										(vtCores.vt == VT_I4 || vtCores.vt == VT_I2))
									{
										processorCoreCount = (vtCores.vt == VT_I4) ? (DWORD)vtCores.intVal : (DWORD)vtCores.iVal;
									}
									else
									{
										DBFMT(ConsoleHandle, L"读取 NumberOfCores 失败 hr=0x%08X", (unsigned)hr);
									}

									if (SUCCEEDED(pclsObj->Get(L"NumberOfLogicalProcessors", 0, &vtThreads, 0, 0)) &&
										(vtThreads.vt == VT_I4 || vtThreads.vt == VT_I2))
									{
										logicalProcessorCount = (vtThreads.vt == VT_I4) ? (DWORD)vtThreads.intVal : (DWORD)vtThreads.iVal;
									}
									else
									{
										DBFMT(ConsoleHandle, L"读取 NumberOfLogicalProcessors 失败 hr=0x%08X", (unsigned)hr);
									}

									if (SUCCEEDED(pclsObj->Get(L"ProcessorId", 0, &vtPid, 0, 0)) &&
										vtPid.vt == VT_BSTR)
									{
										processorId = _bstr_t(vtPid.bstrVal);
									}
									else
									{
										DBFMT(ConsoleHandle, L"读取 ProcessorId 失败 hr=0x%08X", (unsigned)hr);
									}

									VariantClear(&vtCores);
									VariantClear(&vtThreads);
									VariantClear(&vtPid);
									pclsObj->Release();
								}
								else
								{
									DBFMT(ConsoleHandle, L"WMI枚举 Win32_Processor 失败或无数据 hr=0x%08X uReturn=%u",
										(unsigned)hr, (unsigned)uReturn);
								}
								pEnumerator->Release();
							}
							else
							{
								DBFMT(ConsoleHandle, L"ExecQuery 失败 hr=0x%08X", (unsigned)hr);
							}
						}
						else
						{
							DBFMT(ConsoleHandle, L"CoSetProxyBlanket 失败 hr=0x%08X", (unsigned)hr);
						}
					}
					else
					{
						DBFMT(ConsoleHandle, L"ConnectServer 失败 hr=0x%08X", (unsigned)hr);
					}

					if (pSvc) pSvc->Release();
					if (pLoc) pLoc->Release();
				}
				else
				{
					DBFMT(ConsoleHandle, L"CoCreateInstance WbemLocator 失败 hr=0x%08X", (unsigned)hr);
				}
			}
		}

		if (needUninit)
			CoUninitialize();

		//核心 / 线程数未获取到时，使用系统API统计
		if (processorCoreCount == 0 || logicalProcessorCount == 0)
		{
			DB(ConsoleHandle, L"进入CPU核心/线程 fallback 统计逻辑...");
			DWORD len = 0;
			GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && len > 0)
			{
				std::unique_ptr<BYTE[]> buffer(new BYTE[len]);
				if (GetLogicalProcessorInformationEx(RelationProcessorCore,
					reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get()), &len))
				{
					size_t offset = 0;
					DWORD cores = 0;
					DWORD threads = 0;
					while (offset < len)
					{
						PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX p =
							reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get() + offset);
						if (p->Relationship == RelationProcessorCore)
						{
							++cores;
							KAFFINITY m = p->Processor.GroupMask[0].Mask;
							DWORD t = 0;
							while (m) { t += (m & 1); m >>= 1; }
							threads += (t == 0 ? 1 : t);
						}
						offset += p->Size;
					}
					if (cores > 0 && threads > 0)
					{
						processorCoreCount = cores;
						logicalProcessorCount = threads;
						DBFMT(ConsoleHandle, L"fallback 获取成功 cores=%u threads=%u", cores, threads);
					}
				}
			}

			//失败则GetSystemInfo获取
			if (processorCoreCount == 0 || logicalProcessorCount == 0)
			{
				SYSTEM_INFO si;
				GetSystemInfo(&si);
				if (logicalProcessorCount == 0)
					logicalProcessorCount = si.dwNumberOfProcessors;
				if (processorCoreCount == 0)
					processorCoreCount = logicalProcessorCount; 
				DBFMT(ConsoleHandle, L"使用 GetSystemInfo 退化 cores=%u threads=%u", processorCoreCount, logicalProcessorCount);
			}
		}

		// 如果processorId获取失败，则输出提示
		if (processorId.empty() || processorId == "NULL")
		{
			DB(ConsoleHandle, L"processorId获取失败!");
		}

		// 设置 CPU 信息
		cpuNode["name"] = cpuBrandString[0] ? cpuBrandString : "NULL";
		cpuNode["cores"] = static_cast<int>(processorCoreCount);
		cpuNode["threads"] = static_cast<int>(logicalProcessorCount);
		cpuNode["serial_number"] = processorId;

		DBFMT(ConsoleHandle, L"最终CPU信息: name=%hs cores=%d threads=%d serial=%hs",
			cpuBrandString[0] ? cpuBrandString : "NULL",
			(int)processorCoreCount,
			(int)logicalProcessorCount,
			processorId.c_str());
	}
	catch (...)
	{
		cpuNode["name"] = "NULL";
		cpuNode["cores"] = 0;
		cpuNode["threads"] = 0;
		cpuNode["serial_number"] = "NULL";
		DB(ConsoleHandle, L"获取CPU信息发生异常，已使用默认值。");
	}
}

void theApp::GetGpuInfo(Json::Value& gpuNode)
{
	try {
		std::string gpuName = "NULL";
		std::string driverVersion = "NULL";
		std::string serialNumber = "NULL";
		bool foundDiscreteGPU = false;

		HRESULT hr = CoInitialize(NULL);
		if (SUCCEEDED(hr)) {
			IWbemLocator* pLoc = NULL;
			hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

			if (SUCCEEDED(hr)) {
				IWbemServices* pSvc = NULL;
				hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

				if (SUCCEEDED(hr)) {
					hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
						RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

					if (SUCCEEDED(hr)) {
						IEnumWbemClassObject* pEnumerator = NULL;
						hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_VideoController"),
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

						if (SUCCEEDED(hr)) {
							IWbemClassObject* pclsObj = NULL;
							ULONG uReturn = 0;

							// 第一次遍历：寻找独立显卡（NVIDIA, AMD等）
							while (pEnumerator) {
								hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

								if (0 == uReturn) break;

								VARIANT vtName;
								VariantInit(&vtName);

								hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);

								if (SUCCEEDED(hr) && vtName.vt == VT_BSTR) {
									std::string name = _bstr_t(vtName.bstrVal);

									// 检查是否为独显（NVIDIA或AMD）
									if (name.find("NVIDIA") != std::string::npos ||
										name.find("AMD") != std::string::npos ||
										name.find("Radeon") != std::string::npos) {

										VARIANT vtDriverVersion;
										VARIANT vtPNPDeviceID; // 用作序列号

										VariantInit(&vtDriverVersion);
										VariantInit(&vtPNPDeviceID);

										hr = pclsObj->Get(L"DriverVersion", 0, &vtDriverVersion, 0, 0);
										hr = pclsObj->Get(L"PNPDeviceID", 0, &vtPNPDeviceID, 0, 0);

										gpuName = name;

										if (vtDriverVersion.vt == VT_BSTR)
											driverVersion = _bstr_t(vtDriverVersion.bstrVal);

										if (vtPNPDeviceID.vt == VT_BSTR)
										{
											std::string deviceId = _bstr_t(vtPNPDeviceID.bstrVal);
											serialNumber = GenerateGpuSerialFromDeviceID(deviceId);
										}

										VariantClear(&vtDriverVersion);
										VariantClear(&vtPNPDeviceID);

										foundDiscreteGPU = true;

										VariantClear(&vtName);
										pclsObj->Release();
										break; // 找到独显，停止搜索
									}
								}

								VariantClear(&vtName);
								pclsObj->Release();
							}

							// 如果没有找到独显，重新查询获取第一个显卡（通常是集成显卡）
							if (!foundDiscreteGPU) {
								pEnumerator->Reset();

								if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == WBEM_S_NO_ERROR && uReturn > 0) {
									VARIANT vtName;
									VARIANT vtDriverVersion;
									VARIANT vtPNPDeviceID;

									VariantInit(&vtName);
									VariantInit(&vtDriverVersion);
									VariantInit(&vtPNPDeviceID);

									hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);
									hr = pclsObj->Get(L"DriverVersion", 0, &vtDriverVersion, 0, 0);
									hr = pclsObj->Get(L"PNPDeviceID", 0, &vtPNPDeviceID, 0, 0);

									if (vtName.vt == VT_BSTR)
										gpuName = _bstr_t(vtName.bstrVal);

									if (vtDriverVersion.vt == VT_BSTR)
										driverVersion = _bstr_t(vtDriverVersion.bstrVal);

									if (vtPNPDeviceID.vt == VT_BSTR)
									{
										std::string deviceId = _bstr_t(vtPNPDeviceID.bstrVal);
										serialNumber = GenerateGpuSerialFromDeviceID(deviceId);
									}

									VariantClear(&vtName);
									VariantClear(&vtDriverVersion);
									VariantClear(&vtPNPDeviceID);

									pclsObj->Release();
								}
							}

							pEnumerator->Release();
						}
					}
					pSvc->Release();
				}
				pLoc->Release();
			}
			CoUninitialize();
		}

		// 设置GPU信息
		gpuNode["name"] = gpuName;
		gpuNode["driver_version"] = driverVersion;
		gpuNode["serial_number"] = serialNumber;
	}
	catch (...) {
		gpuNode["name"] = "NULL";
		gpuNode["driver_version"] = "NULL";
		gpuNode["serial_number"] = "NULL";
	}
}

void theApp::GetMemoryInfo(Json::Value& memoryNode)
{
	MEMORYSTATUSEX memStatus;
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatusEx(&memStatus);

	// 计算总内存（转换为GB）
	double totalMemoryInGB = static_cast<double>(memStatus.ullTotalPhys) / (1024 * 1024 * 1024);
	std::stringstream stream;
	stream << static_cast<int>(totalMemoryInGB) << "GB";

	memoryNode["total"] = stream.str();
}

void theApp::GetDiskInfo(Json::Value& diskNode)
{
	try {
		std::string model = "NULL";
		std::string size = "NULL";
		std::string serialNumber = "NULL";

		// 获取系统盘盘符
		WCHAR systemDrive[MAX_PATH] = { 0 };
		GetSystemDirectoryW(systemDrive, MAX_PATH);
		WCHAR sysDriveLetter = systemDrive[0]; // 通常是 C

		HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (SUCCEEDED(hr)) {
			IWbemLocator* pLoc = NULL;
			hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

			if (SUCCEEDED(hr)) {
				IWbemServices* pSvc = NULL;
				hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

				if (SUCCEEDED(hr)) {
					hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
						RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

					if (SUCCEEDED(hr)) {
						// 首先查询系统盘的DeviceID
						WCHAR systemDriveQuery[256];
						swprintf_s(systemDriveQuery, L"SELECT * FROM Win32_LogicalDisk WHERE DeviceID='%c:'", sysDriveLetter);

						IEnumWbemClassObject* pEnumLogicalDisk = NULL;
						hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(systemDriveQuery),
							WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumLogicalDisk);

						std::wstring systemDiskID;

						if (SUCCEEDED(hr)) {
							IWbemClassObject* pLogicalDisk = NULL;
							ULONG uReturn = 0;

							if (pEnumLogicalDisk->Next(WBEM_INFINITE, 1, &pLogicalDisk, &uReturn) == WBEM_S_NO_ERROR && uReturn > 0) {
								// 从逻辑磁盘找对应的物理磁盘
								IEnumWbemClassObject* pEnumDiskPartition = NULL;
								WCHAR partitionQuery[256];
								swprintf_s(partitionQuery, L"ASSOCIATORS OF {Win32_LogicalDisk.DeviceID='%c:'} WHERE ResultClass=Win32_DiskPartition", sysDriveLetter);

								hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(partitionQuery),
									WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDiskPartition);

								if (SUCCEEDED(hr)) {
									IWbemClassObject* pDiskPartition = NULL;

									if (pEnumDiskPartition->Next(WBEM_INFINITE, 1, &pDiskPartition, &uReturn) == WBEM_S_NO_ERROR && uReturn > 0) {
										// 从分区找物理磁盘
										IEnumWbemClassObject* pEnumDiskDrive = NULL;
										std::wstring queryStr = L"ASSOCIATORS OF {Win32_DiskPartition.DeviceID='Disk #0, Partition #0'} WHERE ResultClass=Win32_DiskDrive";
										hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t(queryStr.c_str()),
											WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDiskDrive);

										if (SUCCEEDED(hr)) {
											IWbemClassObject* pDiskDrive = NULL;

											if (pEnumDiskDrive->Next(WBEM_INFINITE, 1, &pDiskDrive, &uReturn) == WBEM_S_NO_ERROR && uReturn > 0) {
												VARIANT vtModel;
												VARIANT vtSize;
												VARIANT vtSerialNumber;

												VariantInit(&vtModel);
												VariantInit(&vtSize);
												VariantInit(&vtSerialNumber);

												hr = pDiskDrive->Get(L"Model", 0, &vtModel, 0, 0);
												hr = pDiskDrive->Get(L"Size", 0, &vtSize, 0, 0);
												hr = pDiskDrive->Get(L"SerialNumber", 0, &vtSerialNumber, 0, 0);

												if (vtModel.vt == VT_BSTR)
													model = _bstr_t(vtModel.bstrVal);

												if (vtSize.vt == VT_BSTR) {
													ULONGLONG sizeInBytes = _strtoui64(_bstr_t(vtSize.bstrVal), NULL, 10);
													double sizeInGB = static_cast<double>(sizeInBytes) / (1024 * 1024 * 1024);

													std::stringstream stream;
													if (sizeInGB >= 1024) {
														stream << std::fixed << std::setprecision(1) << (sizeInGB / 1024) << "TB";
													}
													else {
														stream << std::fixed << std::setprecision(0) << sizeInGB << "GB";
													}
													size = stream.str();
												}

												if (vtSerialNumber.vt == VT_BSTR)
													serialNumber = _bstr_t(vtSerialNumber.bstrVal);

												VariantClear(&vtModel);
												VariantClear(&vtSize);
												VariantClear(&vtSerialNumber);

												pDiskDrive->Release();
											}
											pEnumDiskDrive->Release();
										}
										pDiskPartition->Release();
									}
									pEnumDiskPartition->Release();
								}
								pLogicalDisk->Release();
							}
							pEnumLogicalDisk->Release();
						}

						// 如果上面的关联查询方法失败，尝试直接查询所有磁盘驱动器
						if (model == "NULL") {
							IEnumWbemClassObject* pEnumerator = NULL;
							hr = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_DiskDrive"),
								WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

							if (SUCCEEDED(hr)) {
								IWbemClassObject* pclsObj = NULL;
								ULONG uReturn = 0;

								if (pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn) == WBEM_S_NO_ERROR && uReturn > 0) {
									VARIANT vtModel;
									VARIANT vtSize;
									VARIANT vtSerialNumber;

									VariantInit(&vtModel);
									VariantInit(&vtSize);
									VariantInit(&vtSerialNumber);

									hr = pclsObj->Get(L"Model", 0, &vtModel, 0, 0);
									hr = pclsObj->Get(L"Size", 0, &vtSize, 0, 0);
									hr = pclsObj->Get(L"SerialNumber", 0, &vtSerialNumber, 0, 0);

									if (vtModel.vt == VT_BSTR)
										model = _bstr_t(vtModel.bstrVal);

									if (vtSize.vt == VT_BSTR) {
										ULONGLONG sizeInBytes = _strtoui64(_bstr_t(vtSize.bstrVal), NULL, 10);
										double sizeInGB = static_cast<double>(sizeInBytes) / (1024 * 1024 * 1024);

										std::stringstream stream;
										if (sizeInGB >= 1024) {
											stream << std::fixed << std::setprecision(1) << (sizeInGB / 1024) << "TB";
										}
										else {
											stream << std::fixed << std::setprecision(0) << sizeInGB << "GB";
										}
										size = stream.str();
									}

									if (vtSerialNumber.vt == VT_BSTR)
										serialNumber = _bstr_t(vtSerialNumber.bstrVal);

									VariantClear(&vtModel);
									VariantClear(&vtSize);
									VariantClear(&vtSerialNumber);

									pclsObj->Release();
								}
								pEnumerator->Release();
							}
						}
					}
					pSvc->Release();
				}
				pLoc->Release();
			}
			CoUninitialize();
		}

		// 设置硬盘信息
		diskNode["model"] = model;
		diskNode["size"] = size;
		diskNode["serial_number"] = serialNumber;
	}
	catch (...) {
		diskNode["model"] = "NULL";
		diskNode["size"] = "NULL";
		diskNode["serial_number"] = "NULL";
	}
}

void theApp::GetDisplayInfo(Json::Value& displayNode)
{
	HDC hdcScreen = GetDC(NULL);
	int horizontalResolution = GetDeviceCaps(hdcScreen, HORZRES);
	int verticalResolution = GetDeviceCaps(hdcScreen, VERTRES);
	int dpi = GetDeviceCaps(hdcScreen, LOGPIXELSX);
	ReleaseDC(NULL, hdcScreen);

	// 格式化分辨率字符串
	std::stringstream resStream;
	resStream << horizontalResolution << "x" << verticalResolution;

	// 设置显示信息
	displayNode["resolution"] = resStream.str();
	displayNode["dpi"] = dpi;
}

void theApp::GetMachineGuid(Json::Value& displayNode)
{
	 auto machineGuid = GetMachineGuid();
	 if (machineGuid == "NULL")
		 DB(ConsoleHandle, L"获取machineGuid失败");
	 displayNode["machine_guid"] = machineGuid;
	 m_machine_guid = machineGuid;
	 return;
}

bool theApp::FetchUserInfo()
{
	std::string responseBuffer;
	bool success = false;

	// 初始化 CURL
	CURL* curl = curl_easy_init();
	if (!curl)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化 CURL 失败");
		return false;
	}

	try
	{
		// 设置 API Token
		std::string apiToken;
#if RELEASE_CODE == 1
		apiToken = m_appToken;
#else
		apiToken =
			"oKqdT4g_7O8yLxKtdUeOsoMr85qE22LfzKlotOfAydbcUIVnNklYe-XYmqvzkRORJxlvpHqVor0TEtB6ErJJbWQhMBGdVwCQFg-jxMQR_eY";
#endif

		DEBUG_CONSOLE_STR(ConsoleHandle, L"==================== USER INFO REQUEST ====================");
		DEBUG_CONSOLE_STR(ConsoleHandle, L"请求URL: http://scrnrec.appapr.com/api/device/info");
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"请求的Token:%s", LARSC::s2ws(apiToken).c_str());

		// 添加请求头
		struct curl_slist* headers = NULL;
		std::string tokenHeader = "x-api-token: " + apiToken;
		std::string TokenHeader = GlobalFunc::AnsiToUtf8(tokenHeader);
		headers = curl_slist_append(headers, TokenHeader.c_str());

		// 设置请求选项
		curl_easy_setopt(curl, CURLOPT_URL, "http://scrnrec.appapr.com/api/device/info");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UserInfoWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

		// 执行请求
		CURLcode res = curl_easy_perform(curl);

		// 获取HTTP状态码
		long httpCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		if (httpCode == 500)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"数据请求失败，错误代码500");
			if (App.m_loadingWindow)
			{
				App.m_loadingWindow->Destroy();
			}
			Ui_MessageModalDlg messageBox;
			messageBox.SetModal(L"极速录屏大师", L"Ops！出错了", L"无法获取用户信息", L"忽略");
			messageBox.DoModal();

			// 清理资源
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			return false;
		}

		DEBUG_CONSOLE_STR(ConsoleHandle, L"==================== USER INFO RESPONSE ====================");
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"HTTP状态码: %ld", httpCode);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"响应数据: %hs", responseBuffer.c_str());

		// 清理资源
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);

		if (res != CURLE_OK)
		{
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"请求失败: %hs", curl_easy_strerror(res));
			return false;
		}


		// x86平台使用旧版JSON API
		Json::Reader reader;
		Json::Value root;
		if (reader.parse(responseBuffer, root))
		{
			// 检查请求是否成功
			if (root.get("success", false).asBool())
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息请求成功");

				try
				{
					// 检查并提取用户信息
					if (root.isMember("data") && root["data"].isObject() &&
						root["data"].isMember("user") && root["data"]["user"].isObject())
					{
						Json::Value userData = root["data"]["user"];

						// 提取用户昵称
						if (userData.isMember("nickname") && !userData["nickname"].isNull())
						{
							m_userInfo.nickname = CString(userData["nickname"].asCString());
						}
						else
						{
							m_userInfo.nickname = _T("");
						}

						// 提取用户等级
						if (userData.isMember("level") && !userData["level"].isNull())
						{
							m_userInfo.level = CString(userData["level"].asCString());
						}
						else
						{
							m_userInfo.level = _T("");
						}

						// 提取过期时间
						if (userData.isMember("expires_at") && !userData["expires_at"].isNull())
						{
							m_userInfo.expiresAt = CString(userData["expires_at"].asCString());
						}
						else
						{
							m_userInfo.expiresAt = _T("");
						}

						// 提取当前绑定数量
						if (userData.isMember("current_bindings") && !userData["current_bindings"].isNull())
						{
							m_userInfo.currentBindings = userData["current_bindings"].asInt();
						}
						else
						{
							m_userInfo.currentBindings = 0;
						}

						// 提取是否付费
						if (userData.isMember("is_paid") && !userData["is_paid"].isNull())
						{
							m_userInfo.isPaid = userData["is_paid"].asBool();
						}
						else
						{
							m_userInfo.isPaid = false;
						}

						// 提取最大绑定数量
						if (userData.isMember("max_bindings") && !userData["max_bindings"].isNull())
						{
							m_userInfo.maxBindings = userData["max_bindings"].asInt();
						}
						else
						{
							m_userInfo.maxBindings = 0;
						}

						DEBUG_CONSOLE_FMT(ConsoleHandle, L"用户信息: nickname=%s, level=%s, expires_at=%s, current_bindings=%d, is_paid=%s, max_bindings=%d",
							m_userInfo.nickname,
							m_userInfo.level.IsEmpty() ? L"null" : m_userInfo.level,
							m_userInfo.expiresAt.IsEmpty() ? L"null" : m_userInfo.expiresAt,
							m_userInfo.currentBindings,
							m_userInfo.isPaid ? L"true" : L"false",
							m_userInfo.maxBindings);
						m_IsVip = IsValidPaidSubscription(App.m_userInfo);
						success = true;
					}
					else
					{
						DEBUG_CONSOLE_STR(ConsoleHandle, L"响应中缺少用户数据");
					}
				}
				catch (std::exception& e)
				{
					DEBUG_CONSOLE_FMT(ConsoleHandle, L"解析用户数据时发生异常: %hs", e.what());
				}
			}
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"解析JSON响应失败");
		}
	}
	catch (std::exception& e)
	{
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取用户信息时发生异常: %hs", e.what());
	}

	return success;
}

std::string theApp::GetMachineGuid()
{
	static bool        s_loaded = false;
	static std::string s_guid;

	if (s_loaded)
		return s_guid;

	const wchar_t* kSubKey = L"SOFTWARE\\Microsoft\\Cryptography";
	const wchar_t* kValueName = L"MachineGuid";
	auto ReadView = [&](REGSAM samDesired) -> std::wstring
		{
			HKEY hKey = nullptr;
			LONG ret = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, kSubKey, 0, samDesired, &hKey);
			if (ret != ERROR_SUCCESS)
				return L"";
			std::wstring result;
			wchar_t buffer[128] = { 0 };
			DWORD  type = 0;
			DWORD  bytes = sizeof(buffer);
			ret = ::RegGetValueW(hKey, nullptr, kValueName, RRF_RT_REG_SZ, &type, buffer, &bytes);
			::RegCloseKey(hKey);
			if (ret != ERROR_SUCCESS || type != REG_SZ)
				return L"";
			result.assign(buffer);
			return result;
		};
	std::wstring wGuid = ReadView(KEY_READ | KEY_WOW64_64KEY);
	if (wGuid.empty())
		wGuid = ReadView(KEY_READ | KEY_WOW64_32KEY);
	if (wGuid.empty())
		wGuid = ReadView(KEY_READ);

	if (wGuid.empty())
	{
		s_guid = "NULL";
		s_loaded = true;
		m_machine_guid = s_guid;
		return s_guid;
	}

	//转为 UTF-8
	s_guid = GlobalFunc::ConvertToUtf8(wGuid);
	s_loaded = true;
	m_machine_guid = s_guid;
	return s_guid;
}

void theApp::DenyMessageDlg(std::string DenyMessage)
{
	if (m_loadingWindow)
	{
		DB(ConsoleHandle, L"SDL加载窗口Hide调用");
		m_loadingWindow->Hide();
	}
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师", L"请求被拒绝", LARSC::s2ws(DenyMessage).c_str(), L"确认");
	MessageDlg.DoModal();
	if (m_loadingWindow)
	{
		DB(ConsoleHandle, L"SDL加载窗口RestoreFromHide调用");
		m_loadingWindow->RestoreFromHide();
	}
}

void theApp::ErrorMessageDlg(HttpRequestHandler& requestHandler)
{
	if (m_loadingWindow)
	{
		m_loadingWindow->Hide();
	}
	std::string errorMessage = requestHandler.GetErrorMessage();
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师", L"Ops!出错了!",
		L"连接不到服务器", L"确认");
	MessageDlg.DoModal();
	if (m_loadingWindow)
	{
		m_loadingWindow->RestoreFromHide();
	}
}

BOOL theApp::PreTranslateMessage(MSG* pMsg)
{
	return CWinApp::PreTranslateMessage(pMsg);
}
