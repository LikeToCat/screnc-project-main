#pragma once


// Ui_GamingDlg 对话框
#include "CLazerStaticText.h"
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "WndShadow.h"
#include "Ui_UserProfileSDL.h"
#include "CLarListBox.h"
#include "GameFocusDetector.h"
#include <memory>
#include <thread>
#include <algorithm> 

class Ui_GamingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_GamingDlg)

public:
	Ui_GamingDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_GamingDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_GAMINGDLG };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnExitSizeMove();
	afx_msg void OnEnterSizeMove();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);

	//按钮控件响应
public:
	afx_msg void OnBnClickedBtnCkaudio();
	afx_msg void OnBnClickedBtnCknoaudio();
	afx_msg void OnBnClickedBtnMinimal();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnScreen();
	afx_msg void OnBnClickedBtnArearecord();
	afx_msg void OnBnClickedBtnWindowrecord();
	afx_msg void OnBnClickedBtnStartrecord();
	afx_msg void OnBnClickedBtnCkmicro();
	afx_msg void OnBnClickedBtnCknomicro();
	afx_msg void OnBnClickedBtnVideolist();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnBnClickedBtnPhone();
	afx_msg void OnBnClickedBtnOpenvip();
	afx_msg void OnBnClickedBtnAppicon();
	afx_msg void OnBnClickedBtnConfig();
	afx_msg void OnBnClickedBtnRecordedGame();
	afx_msg void OnBnClickedBtnAdvanceopt();
	afx_msg void OnBnClickedBtnMore();
	afx_msg void OnBnClickedBtnGaimgrecord();
	afx_msg void OnBnClickedBtnFeedback();
	afx_msg void OnBnClickedBtnSelectgame();

	//SDL窗口的消息响应
	afx_msg LRESULT On_SDLBnClick_UserLogOut(WPARAM wParam, LPARAM lParam);

	//广播消息
	afx_msg LRESULT On_BroadCast_UserLogin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_BroadCast_UserLogOut(WPARAM wParam, LPARAM lParam);

	//其他MFC窗口的消息响应
	afx_msg LRESULT On_UserProfileDlg_WindowHidenByTimer(WPARAM wParam, LPARAM lParam);

	//下拉框点击
	LRESULT OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam);

	//录制窗口最小化消息
	LRESULT OnRecordWindowMinimal(WPARAM wParam, LPARAM lParam);
public:
	void Ui_SetWindowRect(const CRect& rect);  //在创建窗口之前，预设置窗口矩形
	void Ui_UpdateWindowPos(const CRect& rect);//创建窗口之后，设置窗口矩形并显示
	void CleanUpGdiPngRes();//清理GDI资源
	void ShowLoginUi();
	void ShowNoneUserPayUi();
	void UpdateLogOutUi();
	void RegGameDetector();
	void unRegGameDetector();
	Gdiplus::Bitmap* GetIconOfHwnd(HWND hWnd);						//获取窗口图标
	void SetGameBtnIcon(Gdiplus::Bitmap* bitmap);					//设置游戏手柄的图标
	void AddRecGameItem(const CString& item);						//添加可录制的游戏窗口到下拉框
	void AddGameIcon(std::wstring name, Gdiplus::Bitmap* bitmap);	//添加窗口图标
	bool JudegeIsWindowCaptured(const CString& WindowName);			//判断是否以及捕获过当前游戏焦点窗口
	void SetCurrentRecGameWindow(HWND hWnd, std::wstring windowName); //设置当前录制的窗口
private:
	//窗口基本函数
	void GetUserDPI();//获取用户DPI
	void UpdateScale();	//调整控件和窗口的大小，以及控件的位置
	void InitCtrl();//初始化控件
	void LoadRes();//加载前端资源
	void MinimizeToTaskbar();
	void HideUserProfile();
	void ShowUserProfileSDL();

	//功能函数
	bool HandleRecording(float systemAudioVolume, float microAudioVolume);

	//调整控件位置和大小相关
	void UpdateTitleBarCtrl();
	void UpdateNavAreaCtrl();
	void UpdateMainAreaCtrl();

	//初始化控件样式相关
	void InitTitleBarCtrl();
	void InitNavAreaCtrl();
	void InitMainAreaCtrl();
public:
	//窗口基本参数
	float m_Scale;
	CRect m_CRect_WindowRect;	//窗口矩形(相对于全局)
	Gdiplus::Rect m_Rect_WindowRect;//窗口GDI+矩形（相对于窗口）
	Gdiplus::Rect m_Rect_NavArea;//左侧导航栏区域
	Gdiplus::Rect m_Rect_TitleBar;//标题栏区域
	Gdiplus::Rect m_Rect_MainArea;//主要互动区域 
	int m_WindowWidth;//窗口长度
	int m_WindowHeight;//窗口高度

	//图片资源
	Gdiplus::Bitmap* m_Bitmap_VipLogo = nullptr;
	Gdiplus::Rect m_Rect_Viplogo;

	//图标按钮
	CLarPngBtn m_Btn_ScreenRecord;		// 全屏录制
	CLarPngBtn m_Btn_AreaRecord;		// 区域录制
	CLarPngBtn m_Btn_GamingRecord;		// 游戏录制
	CLarPngBtn m_Btn_WindowRecord;		// 应用窗口录制
	CLarPngBtn m_Btn_RecordGame;		// 录全屏
	CLarPngBtn m_Btn_Rec;				// Rec
	CLarPngBtn m_Btn_SystemAudioIcon;	// 扬声器图标
	CLarPngBtn m_Btn_MicroAudioIcon;	// 麦克风图标
	CLarPngBtn m_Btn_More;				// 更多
	CLarPngBtn m_Btn_TitleIcon;			//应用程序图标
	CLarPngBtn m_Btn_VideoList;			// 视频列表
	CLarPngBtn m_Btn_UserIcon;			// 图标
	CLarPngBtn m_Btn_OpenVip;			// 开通会员
	CLarPngBtn m_Btn_Minimal;			// 最小化
	CLarPngBtn m_Btn_Close;				// 关闭

	//文本
	CLazerStaticText m_Stat_SystemAudio;		// 系统声音
	CLazerStaticText m_Stat_MicroAudio;			// 麦克风声音						
	CLazerStaticText m_Stat_SystemAudioPercent;	// 系统声音百分比
	CLazerStaticText m_Stat_MicroAudioPercent;	// 麦克风声音百分比
	CLazerStaticText m_Stat_TitleText;			// 极速录屏大师
	CLazerStaticText m_Stat_ScreenRecord;		// 全屏录制文本
	CLazerStaticText m_Stat_AreaRecord;			// 区域录制
	CLazerStaticText m_Stat_GamingRecord;		// 游戏录制
	CLazerStaticText m_Stat_WindowRecord;		// 应用窗口录制
	CLazerStaticText m_stat_hotKeyStartRecord;

	//文字按钮
	CLarBtn m_Btn_Login;		// 登录
	CLarBtn m_Btn_Config;		// 设置
	CLarBtn m_Btn_Phone;		// 手机号
	CLarBtn m_Btn_FeedBack;		// 反馈
	CLarBtn m_Btn_AdvanceOpt;	// 高级选项	
	CLarBtn m_btn_selectGame;	// 选择游戏

	//复选框			
	CLarPngBtn m_Btn_CkAudio;// 录制系统声音
	CLarPngBtn m_Btn_CkNoAudio;// 不录制系统声音
	CLarPngBtn m_Btn_CkMicro;// 录制麦克风声音
	CLarPngBtn m_Btn_CkNoMIicro;// 不录制麦克风声音

	//下拉框
	CLarPopupListBoxs m_listBoxs;	//下拉框 
	CStringArray m_Csa_Game;		//游戏列表

	//音量条相关
	Gdiplus::RectF m_Rect_AudioBar;		//系统音量条
	Gdiplus::RectF m_Rect_MicroBar;		//麦克风音量条
	Gdiplus::RectF m_Rect_AudioThumb;   //系统音量条滑块
	Gdiplus::RectF m_Rect_MicroThumb;   //麦克风音量条滑块
	Gdiplus::Bitmap* m_Bitmap_FullBar = nullptr;//满载的音量条
	Gdiplus::Bitmap* m_Bitmap_Bar = nullptr;	//空载的音量条
	Gdiplus::Bitmap* m_Bitmap_Thumb = nullptr;	//音量条滑块
	CPoint m_ptDragStart;     // 拖动的起始点
	CPoint m_ptDragCurrent;   // 拖动过程中的当前点
	bool m_bDragging = false;         // 是否正在进行拖动操作
	bool m_bDraggingAudio = false;    // 是否正在拖动音频滑块（否则是麦克风滑块）	
	int m_nInitialThumbPos;   // 拖动前滑块的初始X位置

	//状态框相关
	bool m_IsRecordSystemAudio = true;	//是否录制系统声音
	bool m_IsRecordMicroAudio = true;	//是否录制麦克风声音

	//录制相关
	bool m_IsRecording = false;
	HWND m_RecHwnd;						//当前录制的游戏窗口句柄

	//窗口阴影
	CWndShadow m_Shadow;

	//SDL窗口
	std::unique_ptr<std::thread> m_Thread_UserProfileWindow;
	
	//添加的游戏窗口
	std::map<std::wstring, Gdiplus::Bitmap*> m_map_captureGames;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
