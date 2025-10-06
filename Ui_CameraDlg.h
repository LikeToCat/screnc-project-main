#pragma once

// Ui_CameraDlg 对话框
#include "CLazerStaticText.h"
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "CLarListBox.h"  
#include "Ui_CameraDisplaySDL.h"
#include "WndShadow.h"
#include "Ui_UserProfileSDL.h"
#include "Ui_VipPayDlg.h"
#include <thread>
#include <memory>
class Ui_ConfigDlg;
class Ui_CameraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_CameraDlg)

public:
	Ui_CameraDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_CameraDlg();
	struct CameraParams {
		std::string format;       // 视频格式，如"mjpeg"
		int width;				  // 视频宽度，如1920
		int height;				  // 视频高度，如1080
		int fps;				  // 帧率，如30
	};
	struct CameraDeviceInfo
	{
		std::string deviceName; //设备名
		std::string desc;		//设备描述
	};
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CAMERADLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL OnNcActivate(BOOL bActive);
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMove(int x, int y);	//窗口正在被拖动
	afx_msg void OnEnterSizeMove();		//窗口开始被拖动
	afx_msg void OnExitSizeMove();		//窗口拖动完成
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);

	//广播
	afx_msg LRESULT On_BroadCast_UserLogin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_BroadCast_UserLogOut(WPARAM wParam, LPARAM lParam);
	
	//SDL窗口响应
	afx_msg LRESULT On_SDLBnClick_UserLogOut(WPARAM wParam, LPARAM lParam);

	//其他MFC窗口消息的响应
	afx_msg LRESULT On_UserProfileDlg_WindowHidenByTimer(WPARAM wParam, LPARAM lParam);

	//本窗口控件响应
public:
	afx_msg LRESULT OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnCbcarmeradevice();
	afx_msg void OnBnClickedBtnCbcameraparam();
	afx_msg void OnBnClickedBtnCbaudiodevice();
	afx_msg void OnBnClickedBtnCbmicrodevice();
	afx_msg void OnBnClickedBtnRecordopt();
	afx_msg void OnBnClickedBtnStartrecord();
	afx_msg void OnBnClickedBtnMinmal();
	afx_msg void OnBnClickedBtnOpencarmera();
	afx_msg void OnBnClickedBtnUsericon();
	afx_msg void OnBnClickedBtnOpenvip();
	afx_msg void OnBnClickedBtnPhone();
	afx_msg void OnBnClickedBtnConfig();
	afx_msg void OnBnClickedBtnAdvanceopt();
	afx_msg void OnBnClickedBtnMicroadvance();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnReturn();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnMenu();
public:
	void Ui_SetWindowRect(const CRect& rect);  //在创建窗口之前，预设置窗口矩形
	void Ui_UpdateWindowPos(const CRect& rect);//创建窗口之后，设置窗口矩形并显示
	void CleanUpGdiPngRes();//清理GDI资源
	void UpdateLoginUi();
	void UpdateNoneUserPayUI();
	void UpdateSignOutUi();
	void SetUiModeDuringRecord();
	void ResumeUiDuringNormal();
private:
	//窗口基本函数
	void GetUserDPI();			//获取用户DPI
	void UpdateScale();			//调整控件和窗口的大小，以及控件的位置
	void InitCtrl();			//初始化控件
	void LoadRes();				//加载前端资源
	void EnableShadow();		//启动窗口阴影
	void ReturnToPage();		//点击录制模式选择下拉框中的项是返回的页面
	void OpenConfigDlg();		//打开设置界面
	void HideUserProfile();		//隐藏用户界面（MFC窗口）
	void ShowUserProfileSDL();	//显示用户界面（SDL窗口）
	void SetWindowToAppWindow();//设置当前窗口为应用程序级窗口

	//调整控件位置和大小相关
	void UpdateTitleBarCtrl();
	void UpdateMainAreaCtrl();

	//初始化控件样式相关
	void InitTitleBarCtrl();
	void InitMainAreaCtrl();
	void SetButtonStyle(
		Gdiplus::Color BorderColor,
		Gdiplus::Color TextColor,
		Gdiplus::Color BkColor,
		CLarBtn& larbtn,
		int mode = 0);

	//初始化SDL窗口相关
	void InitalizeUi_CameraDisplaySDL();//初始化SDL摄像头显示渲染窗口接口

	// 下拉列表相关
	void InitDropListData();  // 初始化下拉列表数据

	//辅助功能函数
	CameraParams ParseCameraParamString(const CString& paramStr);//提取摄像头参数下拉框中的值
	void OpenCamera();//打开摄像头
public:
	//窗口基本参数
	float m_Scale;
	CRect m_CRect_WindowRect;	//窗口矩形(相对于全局)
	Gdiplus::Rect m_Rect_WindowRect;//窗口GDI+矩形（相对于窗口）
	Gdiplus::Rect m_Rect_TitleBar;//标题栏区域
	Gdiplus::Rect m_Rect_MainArea;//主要互动区域
	Gdiplus::Rect m_Rect_CameraDisplay;//摄像头画面显示区域
	int m_WindowWidth;//窗口长度
	int m_WindowHeight;//窗口高度
	bool m_IsRecording = false;//是否正在录制
	bool m_IsCameraOpening = false;//是否打开了摄像头
	CRect m_Rect_BtnUserIcon_SignOutRect;
	bool m_IsloginUi = false;
	bool m_IsHasCameraDevice = true;

	//图片按钮
	CLarPngBtn m_Btn_Refresh;// 刷新
	CLarPngBtn m_Btn_Rec;	// Rec	
	CLarPngBtn m_Btn_OpenVIP;// 开通VIP
	CLarPngBtn m_Btn_UserIcon;// 用户头像
	CLarPngBtn m_Btn_AppIcon;// 图标
	CLarPngBtn m_Btn_Minimal;	// 最小化
	CLarPngBtn m_Btn_Close;// 关闭
	CLarPngBtn m_Btn_Return;// 返回
	CLarPngBtn m_Btn_VipLogo;//VIP标识

	//文本按钮
	CLarBtn m_Btn_Config;// 设置
	CLarBtn m_Btn_FeedBack;// 菜单
	CLarBtn m_Btn_RecordOpt;// 录制选项
	CLarBtn m_Btn_CameraDevice;// 摄像头设备
	CLarBtn m_Btn_CameraParam;// 摄像头参数
	CLarBtn m_Btn_SystemAudioDevice;// 系统声音设备
	CLarBtn m_Btn_MicroAudio;// 麦克风声音
	CLarBtn m_Btn_SystemAudioAdvanceOpt;	// 系统声音高级选项
	CLarBtn m_Btn_MicroAudioOpt;	        // 麦克风声音高级选项
	CLarBtn m_Btn_OpenCamera;	// 打开摄像头
	CLarBtn m_Btn_Phone;	//手机号
	CLarBtn m_Btn_login;	//登录

	//文本
	CLazerStaticText m_Stat_TitleText;// 标题文本
	CLazerStaticText m_Stat_SelectCarmera;// 选择摄像头
	CLazerStaticText m_Stat_CameraParam;// 摄像头参数
	CLazerStaticText m_Stat_SystemAudio;// 系统声音
	CLazerStaticText m_Stat_MicroAudio;// 麦克风声音
	CLazerStaticText m_stat_hotKeyStartRecord;

	CLarPopupListBoxs m_PopupListBox;			 // 下拉框控件
	CStringArray m_Array_CameraDeviceList;       // 摄像头设备列表
	CStringArray m_Array_CameraParamList;        // 摄像头参数列表
	CStringArray m_Array_SystemAudioList;        // 系统声音设备列表
	CStringArray m_Array_MicroAudioList;         // 麦克风设备列表
	CStringArray m_Array_RecordOptList;          // 录制选项列表

	//SDL窗口接口
	static void ptrCusDel_CameraDisplaySDL(Ui_CameraDisplaySDL* ptr)
	{
		if (ptr)
			Ui_CameraDisplaySDL::ReleaseInstance();
	}
	std::unique_ptr<Ui_CameraDisplaySDL, void(*)(Ui_CameraDisplaySDL* ptr)>
		m_Interface_pCDSDL{ nullptr, &ptrCusDel_CameraDisplaySDL };

	//窗口阴影相关
	CWndShadow m_Shadow;

	//窗口用户相关
	std::unique_ptr<std::thread> m_Thread_UserProfileWindow;


	//VIP支付界面相关
	Ui_VipPayDlg* m_Dlg_VipPay = nullptr;
	Ui_ConfigDlg* m_Dlg_Config = nullptr;

	CString m_Str_LastRecordFilePath;

	afx_msg void OnBnClickedBtnViplogo();
	afx_msg void OnBnClickedBtnRefreash();
	
};