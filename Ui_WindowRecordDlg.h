#pragma once


// Ui_WindowRecordDlg 对话框
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "CLazerStaticText.h"
#include "CLarEdit.h"
#include "CLarListBox.h"
#include "WndShadow.h"
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

class Ui_VipPayDlg;

// 页码导航区域结构
struct NavButton {
	Gdiplus::RectF rect;     // 按钮区域
	int action;              // 动作类型: -2前一页, -1上一页, 0...n页码, n+1下一页, n+2后一页
	bool enabled;            // 是否可用
};

// 导航相关的常量
static const int NAV_PREV_TEXT = -2;    // "上一页"文本
static const int NAV_PREV_ARROW = -1;   // 左箭头
static const int NAV_NEXT_ARROW = -3;   // 右箭头
static const int NAV_NEXT_TEXT = -4;    // "下一页"文本

class Ui_WindowRecordDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_WindowRecordDlg)
	// 存储每个缩略图区域信息
	struct ThumbnailInfo 
	{
		Gdiplus::Rect rect;    // 缩略图区域
		HWND hwnd;             // 对应的窗口句柄
	};
	struct RecordRuntimeUI
	{
		Gdiplus::Pen* borderPen;
		Gdiplus::SolidBrush* bkBrush;
		Gdiplus::SolidBrush* timeTextBrush;
		Gdiplus::SolidBrush* sizeTextBrush;
		Gdiplus::Font* timerFont;
		Gdiplus::Font* sizeFont;

		Gdiplus::Rect          rect_timeBk;
		Gdiplus::RectF         rect_timerText;
		Gdiplus::RectF         rect_sizeText;

		std::wstring           timerStr;
		std::wstring           sizeStr;

		RecordRuntimeUI() :
			borderPen(nullptr),
			bkBrush(nullptr),
			timeTextBrush(nullptr),
			sizeTextBrush(nullptr),
			timerFont(nullptr),
			sizeFont(nullptr)
		{}
	};

public:
	Ui_WindowRecordDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_WindowRecordDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WINDOWRECORD };
#endif

public:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnExitSizeMove();
	afx_msg void OnMouseLeave();
	afx_msg void OnNcMouseLeave();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);

public:

	// 本窗口控件响应
	afx_msg LRESULT OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnReturn();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnRecordwindowoption();
	afx_msg void OnBnClickedBtnRecordmode();
	afx_msg void OnBnClickedBtnSystemaudiooption();
	afx_msg void OnBnClickedBtnMicroaudiooption();
	afx_msg void OnBnClickedBtnStartrecord();
	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnBnClickedBtnMinimal();
	afx_msg void OnBnClickedBtnCombox();
	afx_msg void OnBnClickedBtnAdvanceoptionsystemaudio();
	afx_msg void OnBnClickedBtnAdvancemicrooption();
	afx_msg void OnBnClickedBtnReselectrecordwindow();
	afx_msg void OnBnClickedBtnConfig();
	afx_msg void OnBnClickedBtnMenu();
	afx_msg void OnBnClickedBtnPauseWindowRecord();
	afx_msg void OnBnClickedBtnResumeWindowRecord();
	afx_msg void OnBnClickedBtnStopWindowRecord();
	afx_msg void OnBnClickedBtnProfileicon();
	afx_msg void OnBnClickedBtnUsername();
	afx_msg void OnBnClickedBtnOpenvip();
	afx_msg void OnBnClickedBtnTipsofrecordmode();
	afx_msg void OnBnClickedBtnNoselect();
	
	afx_msg LRESULT On_WindowRecFileSizeUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_TopRec_StartRecord(WPARAM wParam, LPARAM lParam);  
	afx_msg LRESULT On_TopRec_StopRecord(WPARAM wParam, LPARAM lParam);   
	afx_msg LRESULT On_TopRec_PauseRecord(WPARAM wParam, LPARAM lParam);  
	afx_msg LRESULT On_TopRec_ResumeRecord(WPARAM wParam, LPARAM lParam); 
public:
	void Ui_SetWindowRect(const CRect& rect);
	void Ui_UpdateWindowPos(const CRect& rect);
	void CleanUpGdiPngRes();//清理GDI资源
	void CleanUpWindowImage();
	void ResetWindowRecordUIState(bool clearTimerAndSize = true);
public:
	void GetUserDPI();//获取用户DPI
	void UpdateScale();	//调整控件和窗口的大小，以及控件的位置
	void InitCtrl();
	void SetBtnStyle(Gdiplus::SolidBrush& BkBrush, CLarBtn& Btn, CString BtnText);
	//初始化控件
	void EnableShadow();//开启阴影
	void LoadRes();//加载前端资源
	void CaptureAllWindowImages();//捕获所有窗口句柄的窗口截图
	bool DrawWindowThumbnails(Gdiplus::Graphics* graphics);//绘画窗口句柄截图到圆角矩形区域内
	void HandleNavButtonClick(int action);// 处理页码导航点击
	void DrawBorder(Gdiplus::Pen* pen, const CRect& rect, Gdiplus::Graphics* graphics);//绘画边框
	void DrawAngleRect(
		Gdiplus::Rect& rect,
		Gdiplus::Graphics* graphics,
		float cornerRadius = 10.0f,
		float borderWidth = 0.0f,
		Gdiplus::Color borderColor = Gdiplus::Color(0, 0, 0),
		Gdiplus::Color fillColor = Gdiplus::Color(36, 37, 40));//绘画圆角矩形
	LRESULT OnNavButtonAction(WPARAM wParam, LPARAM lParam);//页面导航按钮响应
	LRESULT OnBnClickBtnAreaRecord(WPARAM wParam, LPARAM lParam);//区域录制按钮按下
	LRESULT OnCaptureCompelete(WPARAM wParam, LPARAM lParam);//窗口缩略图捕获完毕
	LRESULT OnRecordWindowMinimal(WPARAM wParam, LPARAM lParam);//被录制的窗口最小化
	LRESULT On_BroadCast_UserLogin(WPARAM wParam, LPARAM lParam);
	void HandleThumbnailClick(HWND hwnd);// 处理缩略图点击
	void InitDropListData();//初始化下拉框数据
	void InitDroplistCtrl();//初始化下拉框
	void CaptureWindowImageThread();//捕获窗口句柄截图线程
	void ReturnToPage();
	void ClearHoverState();
	void DrawThumbnailsUiMode(Gdiplus::Graphics* memGraphics);
	void DrawWindowImageUiMode(Gdiplus::Graphics* memGraphics);
	void ShowHWndOnTop(HWND hwnd);
	void SetWindowToAppWindow();	//设置窗口为应用程序级窗口
	void InitWindowRecordRuntimeRect();
	void SetWindowRecordCountTimer();
	void StopWindowRecordCountTimer();
	bool InitWindowRecordFreeSpaceFromPath(const std::wstring& path);
	void UpdateWindowRecordSizeUI(double sizeMB);
	void StartNonVipAutoStopTimerIfNeeded();     // 启动（初始化）一次性可暂停计时
	void PauseNonVipAutoStopTimer();             // 在录制暂停时调用
	void ResumeNonVipAutoStopTimer();            // 在录制恢复时调用
	void ResetNonVipAutoStopTimerState();        // 手动/自动停止后调用（清理）
	void NotifyRecTop_Start();
	void NotifyRecTop_Stop(); 
public:
	//窗口显示参数
	CRect m_CRect_WindowRect;//窗口显示区域
	float m_Scale;	// 缩放系数
	Gdiplus::Rect m_Rect_Top;// 标题栏区域
	float m_WindowWidth;//窗口长
	float m_WindowHeight;//窗口宽
	int m_currentPage = 0;  // 当前显示的页码
	Gdiplus::Rect m_Rect_WindowsDiplayArea;//窗口显示区域

	CLarPngBtn m_Btn_AppIcon;// 图标
	CLazerStaticText m_Stat_LogoText;// 极速录屏大师
	CLarPngBtn m_Btn_PrifileIcon;// 用户头像
	CLarBtn m_Btn_Username;// 用户名
	CLarPngBtn m_Btn_OpenVip;// 升级会员
	CLarBtn m_Btn_Config;// 设置
	CLarBtn m_Btn_FeedBack;// 菜单
	CLarPngBtn m_Btn_Minimal;	// 最小化
	CLarPngBtn m_Btn_Close;	// 关闭
	CLarPngBtn m_Btn_Return;	// 返回
	CLarBtn m_Btn_ComBox;	// 选择录制选项(录制窗口，还是录制全屏，还是选区录制)
	CLarPngBtn m_Btn_Refresh;	// 刷新
	CLarPngBtn m_Btn_StartRecord;	// 开始录制
	CLarBtn m_Btn_RecordWindowOption;	// 录制窗口下拉框
	CLarBtn m_Btn_RecordMode;	// 录制模式下拉框
	CLarBtn m_Btn_SystemAudioOption;	// 系统声音下拉框
	CLarBtn m_Btn_MicroAudioOption;	// 麦克风声音下拉框
	CLazerStaticText m_Stat_RecordWindow;	// 录制窗口文本
	CLazerStaticText m_Stat_RecordMode;	// 录制模式文本
	CLazerStaticText m_Stat_SystemAudio;	// 系统声音文本
	CLazerStaticText m_Stat_MicroAudio;	// 麦克风声音文本			
	CLazerStaticText m_stat_hotKeyStartRecord;
	CLarBtn m_Btn_RecordWindowReselect;	// 录制窗口重新选择按钮
	CLarPngBtn m_Btn_TipsOfRecordMode;	// 录制模式提示
	CLarBtn m_Btn_SystemAudioAdvanceOption;	// 系统声音
	CLarBtn m_Btn_MicroAudioAdvanceOption;	// 麦克风声音高级选项
	CLarBtn m_Btn_NoSelect;					// 未选择
	CLarPngBtn m_Btn_WindowRecPause;
	CLarPngBtn m_Btn_WindowRecStop;
	CLarPngBtn m_Btn_WindowRecResume;

	// 窗口录制文件大小与时间UI成员
	RecordRuntimeUI m_windowRecordUI;
	int  m_windowRecordElapsedSec = 0;
	std::wstring m_windowLastOutputPath;
	double m_windowFreeSpaceGB = 0.0;      // 录制开始时的剩余空间(GB)
	bool   m_windowHasFreeSpace = false;

	//窗口显示区域相关
	std::mutex m_imagesMutex;
	std::map<HWND, Gdiplus::Image*> m_WindowImages;//获取到的窗口句柄的截图
	std::vector<NavButton> m_NavButtons;// 页码导航按钮数组
	std::vector<ThumbnailInfo> m_thumbnailRects;  // 保存所有缩略图位置信息
	std::thread m_Thread_CaptureWindowImage;//获取窗口截图子线程
	std::atomic<bool> m_Bool_IsWindowImageReady = false;
	bool m_Bool_IsMouseInThumbnail = false;
	bool m_Bool_IsWindowDisplayNeedRedraw = true;
	HWND m_selectedWindowHandle = NULL;
	Gdiplus::Rect m_Rect_CurrnetHovertTumbnailRect;
	Gdiplus::Rect m_Rect_LastHovertTumbnailRect;
	BOOL m_bMouseLeaveTrack = FALSE; // 标记是否已启动 WM_MOUSELEAVE 追踪
	HWND m_Hwnd_CurSelect = nullptr;


	// 下拉框控件相关
	CLarPopupListBoxs m_PopupListBox;
	CStringArray m_Array_RecordWindowList;      // 录制窗口列表
	CStringArray m_Array_RecordModeList;        // 录制模式列表
	CStringArray m_Array_SystemAudioList;       // 系统声音设备列表
	CStringArray m_Array_MicroAudioList;        // 麦克风设备列表
	CStringArray m_Array_RecordOptions;			// 录制选项
	bool m_IsRecording = false;

	//资源
	Gdiplus::Bitmap* m_Bitmap_AddWindow;

	CString m_outputfileName; //录制的视频文件路径
	CWndShadow m_Shadow;//窗口阴影层
	
	//窗口捕获标志
	int m_captureflag;

	//子窗口
	Ui_VipPayDlg* m_Dlg_VipPay = nullptr;

	int        m_nonVipRemainingMs = 0;          // 非VIP自动停止剩余毫秒
	ULONGLONG  m_nonVipTimerStartTick = 0;       // 最近启动/恢复的 GetTickCount64()
	bool       m_nonVipTimerActive = false;      // 是否正在倒计时
};
