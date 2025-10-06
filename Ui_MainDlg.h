#pragma once


// Ui_MainDlg 对话框
#include <afxwin.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include "WndShadow.h"
#include "CLarBtn.h"
#include "CLarEdit.h"
#include "CLazerStaticText.h"
#include "CLarPngBtn.h"
#include "Ui_ChildDlg.h"
#include "Ui_WindowRecordDlg.h"
#include "Ui_VideoListDlg.h"
#include "Ui_VipPayDlg.h"
#include "Ui_ConfigDlg.h"
#include "Ui_GamingDlg.h"
#include "Ui_CameraDlg.h"
#include "Ui_AreaRecordingSDL.h"
#include "Ui_DropdownMenuSDL.h"
#include "Ui_DeviceBindingSDL.h"
#include "Ui_LoginDlg.h"
#include "Ui_UserProfileSDL.h"
#include "AreaSelectDlg.h"
#include "Ui_RecTopDlg.h"
#include "PngLoader.h"
#include "AppCloseManager.h"

#pragma comment ( lib, "dwmapi.lib")
#pragma comment ( lib, "gdiplus.lib")

class Ui_UserProfileDlg;
class Ui_LevelUpDlg;
class Ui_MainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_MainDlg)
public:
	// 设备绑定信息结构体
	struct DeviceBindingInfo
	{
		//std::string code;         // 设备代码
		std::string os_name;      // 操作系统名称
		std::string created_at;   // 创建时间
		bool is_current;          // 是否为当前设备
		DeviceBindingInfo() : is_current(false) {}
	};

	Ui_MainDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_MainDlg();
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MAINDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	//本MFC窗口消息
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
	
	//来自其他MFC窗口的消息的响应
	afx_msg LRESULT On_Ui_ChildDlg_ReturnBtnClicked(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_Ui_NoneVipDlg_OpenVipBtnClicked(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_Ui_NoneVipDlg_UnBindClicked(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_Ui_NoneVipDlg_Login(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_Ui_RequestConfigDlg_Close(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_Update_DevicBingList(WPARAM lp, LPARAM wp);
	afx_msg LRESULT OnUi_ProfileDlg_WindowHidenByTimer(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_UiProfile_SignOut(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_UiProfile_ShowLogoutSuccessDlg(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_UiProfile_OpenVip(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_UiOpenVip_VipPrivilegeContrast(WPARAM lp, LPARAM wp);

	//广播消息响应
	afx_msg LRESULT On_BroadCast_UserLogin(WPARAM lp, LPARAM wp);
	afx_msg LRESULT On_BroadCast_UserLogOut(WPARAM wParam, LPARAM lParam);

	//本窗口按钮响应
	afx_msg void OnBnClickedBtnScreenRecord();
	afx_msg void OnBnClickedBtnAppwindowrecording();
	afx_msg void OnBnClickedBtnVideolist();
	afx_msg void OnBnClickedBtnOpenvip();
	afx_msg void OnBnClickedBtnConfig();
	afx_msg void OnBnClickedBtnGamingrecording();
	afx_msg void OnBnClickedBtnArearecording();
	afx_msg void OnBnClickedBtnMore();
	afx_msg void OnBnClickedBtnDevicebinding();
	afx_msg void OnBnClickedBtnMinimal();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnUserProfile();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnFeedback();

	//SDL窗口消息的响应
	afx_msg LRESULT On_UiRecordArea_WindowChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_UiDropDowmMeau_CameraRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_UiDropDowmMeau_MouseRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_UiDropDowmMeau_SystemMircroAudioRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_UiDeviceBind_UnBindAndClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SDLAreaReocrd_HideMainWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SDLAreaReocrd_StartRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SDLAreaReocrd_StopRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SDLAreaReocrd_CloseWindow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SDLAreaReocrd_PauseRecord(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SDLAreaReocrd_ResumeRecord(WPARAM wParam, LPARAM lParam);

	//App消息
	afx_msg LRESULT OnMinimalAndTipsUserRecording(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFirstOpenToOpenVip(WPARAM wParam, LPARAM lParam);

public:
	//其他窗口可调用的窗口控制
	void CleanUpGdiPngRes();		//清理GDI资源
	void InitializeApp();			//初始化应用程序
	void PreCreateAllChildDialogs();//预创建程序所有的窗口
	void ShowLoginInUi();			//显示登录状态UI
	void ShowSignOutUi();			//显示未登录状态UI
	void CloseAreaRecordSDL();		//关闭区域录制SDL窗口
	void SetNeedReduceBinds(bool IsNeedReduceBinds);		  //窗口是否需要进行绑定设备SDL窗口的弹出
	void ShowVideoList();									  //显示视频列表附加窗口
	void HideVideoList();									  //隐藏视频列表附加窗口
	void SetTrayClickCallback(std::function<void()> callback);//托盘点击回调
	void On_ModalBnClick_Prview();	                          //预览提示窗口预览按钮点击回调
	void UpdateRecentRecordFile(const CString& filePath);	  //更新最近一次录制的视频文件路径
	void UpdateQuitWay();				//调用关闭管理器，更新程序退出方式
	void HandleClose();					//处理窗口关闭时的执行
	void HideUserProfile();				//隐藏用户信息窗口
	void JudgeDeviceBindBtnShowable();	//判断设备列表按钮是隐藏还是显示
	void SetWindowShadowSize(int size);	//设置窗口阴影大小
	void ShowNoneUserPayUi();			//设置未登录用户但已支付状态下的Ui
	void CloseDropdownMenu();			//统一关闭“更多”下拉菜单的便捷方法
	void BnClickedMouthBill();
	void ResetPhoneBtnImage();
	void SetPhoneBtnImageUp();

	//获取某一个窗口
	inline CWnd* getProfileDlg() { return (CWnd*)m_Dlg_UserPrifile; }

	//获取用户设置录制参数
	RecordingParams GetLastRecordParam() { return m_RecordP; }	//获取最近一次录制设定的录制参数
	RecordingParams CollectRecordingParams();					//获取当前程序的录制参数

	//SDL窗口控制
	void CreateSDLAreaRecordWindow(const CRect WindowRect, bool IsRecording);	//SDL区域录制窗口启动
	void CloseSDLAreaRecordWindow();											//SDL区域录制窗口关闭
	void ShowUserProfileSDL();													//SDL用户信息窗口启动
private:
	void UpdateUserInfoDisplay();
	void LoadRes();     //加载前端资源
	void GetUserDPI();  //获取用户DPI
	void UpdateScale();	//调整控件和窗口的大小，以及控件的位置
	void InitCtrl();    //初始化控件
	void EnableShadow();//开启阴影
public:
	std::vector<DeviceBindingInfo> m_Vec_DeviceBindingList; //用户所有的绑定设备信息
	std::unique_ptr<AppCloseManager> m_closeManager;        //关闭管理器
	RecordingParams m_RecordP;	                            //最近一次录制选项

	//sign
	bool m_Bool_IsNeedReduceBinds = false;//是否需要解绑的标志（用于是否再窗口显示后的一段时间弹出设备解绑窗口）
	bool m_IsVip = false;				  //用户是否是VIP
	bool m_VideoListVisible = false;	  //视频列表窗口是否显示
	bool m_bool_IsMoreMenuShow = false;	  //更多按钮菜单窗口是否显示

	//PNG资源
	Gdiplus::Bitmap* m_Bitmap_VipLogo = nullptr;
	Gdiplus::Bitmap* m_Bitmap_MouthBill = nullptr;
	Gdiplus::Rect m_Rect_Viplogo;
	Gdiplus::Rect m_Rect_MouthBill;

	//Child Dlg
	Ui_ChildDlg* m_Dlg_Child = nullptr;              //点击全屏录制，区域录制，游戏录制后显示的子界面

	//Pop Dlg
	Ui_WindowRecordDlg* m_Dlg_WindowRecord = nullptr;//点击应用窗口后出来的界面
	Ui_VideoListDlg* m_Dlg_Videolist = nullptr;      //点击视频列表后出来的界面
	Ui_VipPayDlg* m_Dlg_VipPay = nullptr;            //点击开通会员后出来的界面 
	Ui_ConfigDlg* m_Dlg_Config = nullptr;            //点击设置后出来的界面
	Ui_GamingDlg* m_Dlg_Gaming = nullptr;            //点击游戏录制后出来的界面
	Ui_CameraDlg* m_Dlg_Carmera = nullptr;           //点击摄像头录制出来后的界面
	Ui_LoginDlg* m_Dlg_Login = nullptr;	             //点击登录后出来的界面
	Ui_MessageModalDlg* m_Dlg_MessageDlg = nullptr;  //模态消息对话框
	Ui_RecTopDlg* m_Dlg_RecTopDlg = nullptr;	     //除摄像头录制和应用窗口录制以及游戏录制外的界面弹出时，顶部弹出的工具栏窗口
	Ui_UserProfileDlg* m_Dlg_UserPrifile = nullptr;  //点击用户名后出来的用户信息窗口
	CRect m_CRect_DlgChild;                          //Ui_ChildDlg子窗口的显示区域

	//SDL Window
	std::unique_ptr<Ui_AreaRecordingSDL> m_SDL_RecordingWindow; //区域录制窗口
	std::unique_ptr<std::thread> m_Thread_SDLWindow;			//区域录制窗口线程
	std::unique_ptr<Ui_DropdownMenuSDL> m_SDL_DropdownMenu;     //更多按钮菜单SDL窗口（点击更多按钮弹出）
	std::unique_ptr<std::thread> m_Thread_DropdownMenu;			//更多按钮菜单窗口线程
	std::unique_ptr<std::thread> m_Thread_DeviceBindWindow;		//设备绑定S窗口线程
	std::unique_ptr<std::thread> m_Thread_UserProfileWindow;	//用户信息窗口线程
	CRect m_CRect_SelectArea = CRect(0, 0, 0, 0);               //选区录制的区域

	//Area Rect
	Gdiplus::Rect m_Rect_Top;   //上矩形（绘画标题栏背景）
	Gdiplus::Rect m_Rect_Bottom;//下矩形(绘画客户区背景)

	//UI Ctrl
	CLarPngBtn m_Btn_AppIcon;            // 图标
	CLazerStaticText m_Stat_logoText;    // 极速录屏大师
	CLazerStaticText m_Stat_ScreenRecord;// 全屏录制文本
	CLazerStaticText m_Stat_AreaRecord;  // 区域录制文本
	CLazerStaticText m_Stat_GamingRecord;// 游戏录制文本
	CLazerStaticText m_Stat_WindowRecord;// 应用窗口录制文本
	//CLazerStaticText m_stat_mouthPrice;	 // 月度会员价格
	CLarBtn m_btn_Phone;                 // 手机号
	CLarBtn m_btn_Login;		         // 登录
	CLarPngBtn m_Btn_ProfileIcon;        // 头像
	CLarPngBtn m_Btn_OpenVIP;            // 开通VIP
	CLarBtn m_Btn_Config;                // 设置
	CLarBtn m_Btn_FeedBack;              // 菜单
	CLarPngBtn m_Btn_Minimal;            // 最小化
	CLarPngBtn m_Btn_Close;              // 关闭
	CLarPngBtn m_Btn_ScreenRecord;	     // 全屏录制
	CLarPngBtn m_Btn_AreaRecord;         // 区域录制
	CLarPngBtn m_Btn_GamingRecord;       // 游戏录制
	CLarPngBtn m_Btn_WindowRecord;       // 应用窗口录制
	CLarBtn m_Btn_More;					 // 更多
	CLarBtn m_Btn_VideoList;             // 视频列表
	CLarBtn m_Btn_DeviceBinding;         // 设备绑定

	//窗口基本参数
	float m_WindowWidth;
	float m_WindowHeight;
	float m_WindowX;
	float m_WindowY;
	float m_Scale;                 //用户DPI
	float m_BoundaryLine = 0.836;  //分界线Y坐标是窗口的几分之几(用于绘画窗口底部工具栏的边界线)

	//窗口阴影框架
	CWndShadow m_Shadow;
	afx_msg void OnBnClickedButton2();
};