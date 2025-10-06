
// MFCMediaPlayer.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

// theApp: 
// 有关此类的实现，请参阅 MFCMediaPlayer.cpp
//
#include <curl.h>
#include "json.h"
#include <sstream>
#include <intrin.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iomanip>
#include <memory>
#include <map>
#include <fstream>   
#pragma comment(lib, "wbemuuid.lib")
#include "Ui_MainDlg.h"
#include "AppCloseManager.h"
#include "Ui_NoneVipDlg.h"
#include "Ui_LoadingSDL.h"
#include "Ui_RadarTimerSDL.h"
#include "HttpRequestHandler.h"
#include "DialogManager.h"
/*
*
dxgi.lib
d3d11.lib
d3dcompiler.lib
*/
//配置文件类型说明
	// CPU信息结构体
struct CPUInfo
{
	std::string name;
	int cores;
	int threads;
};

// GPU信息结构体
struct GPUInfo
{
	std::string name;
	std::string driverVersion;
};

// 硬盘信息结构体
struct DiskInfo
{
	std::string model;
	std::string size;
	std::string serialNumber;
};

// 显示信息结构体
struct DisplayInfo
{
	std::string resolution;
	int dpi;
};

// 用户信息结构体
struct UserInfo
{
	CString nickname;
	CString level;
	CString expiresAt;
	int currentBindings;
	bool isPaid;
	int maxBindings;

	UserInfo():
		nickname(L""),
		level(L""),
		expiresAt(L""),
		currentBindings(1),
		isPaid(false),
		maxBindings(1)
	{}
};

// 价格信息结构体
struct PriceInfo
{
	int id;
	CString name;
	CString badge;  // 可能为null
	double amount;
	CString desc;

	CString GetFormattedDesc() const
	{
		if (desc.IsEmpty() || wcschr(desc, L'?') != nullptr)
		{
			switch (id)
			{
			case 8: return _T("5台同时登录");
			case 6: return _T("3台同时登录");
			case 5: return _T("5台同时登录");
			default: return _T("多台设备登录");
			}
		}
		return desc;
	}
};

// 额外字段(月度会员)
struct CouponPrice
{
	int32_t id;
	std::wstring name;
	std::wstring badge;
	uint32_t amount;
	std::wstring desc;

	CouponPrice() :
		id(-1),
		name(L""),
		badge(L""),
		amount(0),
		desc(L"")
	{
	}
};
class UserActionLogger;
class theApp : public CWinApp
{
public:
	theApp();
	bool InitApp();

	//接口
	bool RequestPrice();
	bool RequestToken();
	bool RequestToken(std::string appToken);
	bool RequestDeviceInfo();
	bool RequestDeviceList();
	bool RequestDeviceUnbind(std::string DeviceCode);
	bool RequestSignOut();
	bool RequestSignIn();
	bool RequestSeedCode();

	//行为记录
	bool UserOpenMain(bool isSuccess);
	bool UserScreenRecord(bool isSuccess);
	bool UserAreaRecord(bool isSuccess);
	bool UserGamingRecord(bool isSuccess);
	bool UserWindowRecord(bool isSuccess);
	bool UserOnlyAudioRecord(bool isSuccss);
	bool UserCarmeraRecord(bool isSuccess);
	bool UserFollowMouseRecord(bool isSuccess);
	bool UserOpenVip(bool isSuccess);
	bool UserPay(bool isSuccess);
	bool UserShutDownExe(bool isSuccess);
	bool UserTriggerRecord();
	bool UserRecordWithCarmera(bool isSuccess);
	bool UserCloseCarmeraPreviewWithRecord(bool isSuccess);
	bool UserOpenCarmerPreview_InCameraDlg(bool isSuccess);
	bool UserCloseCarmerPreview_InCameraDlg(bool isSuccess);
	bool UserNoneVipRecordSuccess();
	bool UserRecordSuccess();
	bool UserCloseOpenVip();
	bool UserOpenMouthBiil();
	bool UserCloseMouthBill();
	bool UserClickMeal(int id, CString name, int price);

	//录制预热
	void RecordPreHotThread();
public:
	//辅助函数
	bool IsValidPaidSubscription(const UserInfo& userInfo);
	bool IsNotOverBinds(const UserInfo& userInfo);
	bool ExtractUserDeviceInfo(Json::Value& data_user_Root);
	bool IsWindows7OrHigher();
	std::string MapArchitecture();
	std::string MapWindowsVersion();
	void GetCpuInfo(Json::Value& cpuNode);						// 获取CPU信息
	void GetGpuInfo(Json::Value& gpuNode);						// 获取GPU信息
	void GetMemoryInfo(Json::Value& memoryNode);				// 获取内存信息
	void GetDiskInfo(Json::Value& diskNode);					// 获取硬盘信息
	void GetDisplayInfo(Json::Value& displayNode);				// 获取显示信息
	void GetMachineGuid(Json::Value& displayNode);				// 获取用户MachineGuid
	std::string GetDetailedOSVersion();							// 获取详细的操作系统版本信息
	void DenyMessageDlg(std::string DenyMessage);
	void ErrorMessageDlg(HttpRequestHandler& requestHandler);
	bool ReadConfig();											//读取用户配置和自动登录的函数
	void SaveUserAddedVideos();
	void SaveAppTokenToConfigFile();
	bool JudgeSingleAppIns();
	bool RegisterUninstallEntry(LPCWSTR displayName);
	void CleanUpWindows();
	void CleanUpGDIRes();
	void CleanUpGDI();
	void CleanUpInterface();
	void UpdateUserInfo(UserInfo userInfo);
	UserInfo getUserInfo();
	bool IsDeviceOverBind();
	void OpenFeedBackLink(HWND hWnd, LPCTSTR linkUrl);//打开客服链接
	bool IsWin7Over();
	void initGdiPlus();
	bool FetchUserInfo();
	std::string GetMachineGuid();
	bool Test();
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

public:
	//应用程序基本实例成员
	Ui_MainDlg m_Dlg_Main;
	ULONG_PTR m_gdiToken;
	Ui_LoadingSDL* m_loadingWindow = nullptr;
	Ui_MessageModalDlg m_Message_Dlg;
	float m_Scale;
	static HANDLE m_hInstanceMutex;

	//用户信息相关
	UserInfo m_userInfo;
	bool m_isLoginIn = false;
	bool m_IsOverBinds = false;
	bool m_IsVip = false;
	bool m_IsAutoLogin = false;
	bool m_IsInitialSuccess = false;
	bool m_IsWin7Over = false;
	bool m_IsNonUserPaid = false;
	bool m_IsHasOpenVip = false;
	bool m_IsNeedShowMouthBill = false;
	bool m_IsHasMouthBillWindowShowed = false;
	bool m_CanGuestBuy = false;

	//Token和配置文件
	std::string m_appToken = "NULL";
	std::string m_InstallerToken = "NULL";
	std::string m_ConfigPath = "NULL";

	//用户机器唯一标识
	std::string m_machine_guid;

	//组件
	UserActionLogger* m_userActionLogger;

	//套餐信息
	std::vector<PriceInfo> m_PriceInfos;
	CouponPrice m_CouponPrice;
	std::string m_preOrderNo;
	int m_MouthBillPrice;
	int m_ReadPacketAmount;

	//控件信息相关(这两个软件早期开发的祖传代码，我已经忘了它是干什么的，不能动)
	CRect m_Rect_UserIcon;	//登录前图标位置
	CRect m_Rect_Login;		//登陆前登录按钮位置

	//子线程
	std::thread m_Thread_RecordPreHot;
};

extern theApp App;