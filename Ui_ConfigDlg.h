#pragma once


// Ui_ConfigDlg 对话框
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "CLazerStaticText.h"
#include "CLarListBox.h"
#include "WndShadow.h"
#include "Ui_NoneVipDlg.h"
#include <memory>
struct LineBoundary//分界线结构体
{
	Gdiplus::Point p1;
	Gdiplus::Point p2;
};

class Ui_ConfigDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_ConfigDlg)

public:
	Ui_ConfigDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_ConfigDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONFIGDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnExitSizeMove();
	afx_msg void OnEnterSizeMove();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

public:
	//按钮响应
	afx_msg void OnBnClickedBtnFolderselect();
	afx_msg void OnBnClickedBtnFolderconfig();
	afx_msg void OnBnClickedBtnVideoconfig();
	afx_msg void OnBnClickedBtnAudioconfig();
	afx_msg void OnBnClickedBtnMouseconfig();
	afx_msg void OnBnClickedBtnGeneralconfig();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnDisplaymouse();
	afx_msg void OnBnClickedBtnStartupfirst();
	afx_msg void OnBnClickedBtnMinialtobar();
	afx_msg void OnBnClickedBtnExitexe();
	afx_msg void OnBnClickedBtnWater();

	//下拉框按钮相关
	afx_msg void OnBnClickedBtnAudiocapturedevice();
	afx_msg void OnBnClickedBtnMicrodevice();
	afx_msg void OnBnClickedBtnVideoformat();
	afx_msg void OnBnClickedBtnVideoquilaty();
	afx_msg void OnBnClickedBtnFps();
	afx_msg void OnBnClickedBtnVideocodec();
	afx_msg void OnBnClickedBtnVideobitratemode();
	afx_msg void OnBnClickedBtnVideobitratepercent();
	afx_msg void OnBnClickedBtnAudiobitrate();
	afx_msg void OnBnClickedBtnAudiokbps();

	//下拉框项点击
	afx_msg LRESULT OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam);

	bool AdjustVipMode();
	bool JudgePathValid(std::string Path);
public:
	void Ui_SetWindowRect(const CRect& rect);  //在创建窗口之前，预设置窗口矩形
	void Ui_UpdateWindowPos(const CRect& rect);//创建窗口之后，设置窗口矩形并显示
	void CleanUpGdiPngRes();	  //清理GDI资源（在GDI关闭之前调用）
	BOOL LoadConfigFromFile();
	BOOL SaveConfigToFile();
	void SetMinimalToBar(bool isEnable);
	void SetQuitExe(bool isQuitExe);
	void GetExeQuitWay(bool* IsQuitExe, bool* IsMinimalTobar);
	const CString getRecSavePath();
private:
	void ConfigAreaMove(int mode);// 设置区域滚动
	void InitializeUI();          // 初始化界面
	void LoadResources();         // 加载资源
	void CleanupResources();      // 释放资源
	void GetUserDPI();            // 获取用户DPI
	void UpdateScale();           // 更新缩放
	void EnableShadow();	      // 开启阴影
	// 设置LarBtn按钮样式,0为默认样式,1为下拉框样式
	void SetButtonStyle(
		Gdiplus::Color BorderColor,
		Gdiplus::Color TextColor,
		Gdiplus::Color BkColor,
		Gdiplus::Color HoverColor,
		Gdiplus::Color ClickColor,
		CLarBtn& larbtn,
		int mode = 0
	);
	void DrawRoundedRectangle(Gdiplus::Graphics* graphics,
		int x, int y, int width, int height, int cornerRadius = 0);

	// 下拉列表相关
	void InitDropListData();  // 初始化下拉列表数据
	void TipOfOpenVip();
public:
	//子对话框
	Ui_MessageModalDlg* m_Dlg_NoneVip = nullptr;

	// 窗口基本参数
	float m_Scale;                // DPI缩放系数
	CRect m_CRect_WindowRect;     // 窗口区域
	int m_WindowWidth;            // 窗口宽度
	int m_WindowHeight;           // 窗口高度
	int m_WindowX;                // 窗口左上角X坐标
	int m_WindowY;                // 窗口左上角Y坐标
	Gdiplus::Rect m_Rect_TitleBar;    // 标题栏区域
	Gdiplus::Rect m_Rect_ConfigNavBtn;// 窗口左侧设置导航按钮
	Gdiplus::Rect m_Rect_ConfigArea;  // 录制参数设置区域
	LineBoundary m_Boundary1;	  // 保存目录和视频参数设置区域的分界线
	LineBoundary m_Boundary2;	  // 视频参数和音频参数设置区域的分界线
	LineBoundary m_Boundary3;	  // 音频参数和鼠标设置设置区域的分界线
	LineBoundary m_Boundary4;	  // 鼠标设置和通用设置设置区域的分界线

	//图片按钮
	CLarPngBtn m_Btn_TitleText;		// 设置
	CLarPngBtn m_Btn_FolderSelect;	// 目录选择
	CLarPngBtn m_Btn_Close;		    // 关闭

	enum NavClickType
	{
		FolderConfig,
		VideoConfig,
		AudioConfig,
		MouseConfig,
		GeneralConfig
	};
	NavClickType m_NavClickType = FolderConfig;

	//按钮控件
	CLarBtn m_Btn_FolderConfig;			// 保存目录
	CLarBtn m_Btn_VideoConfig;			// 视频参数
	CLarBtn m_Btn_AudioConfig;			// 音频参数
	CLarBtn m_Btn_MouseConfig;			// 鼠标设置
	CLarBtn m_Btn_GeneralConfig;		// 通用设置
	CLarBtn m_Btn_Path;					// 路径
	CLarBtn m_Btn_VideoFormat;			// 视频格式
	CLarBtn m_Btn_VideoQuality;			// 录制分辨率
	CLarBtn m_Btn_Fps;					// 帧率下拉框
	CLarBtn m_Btn_VideoCodec;			// 视频编码器下拉框
	CLarBtn m_Btn_VideoBitrateMode;		// 画质下拉框
	CLarBtn m_Btn_VideoBitRatePercent;	// 比特率百分比选择下拉框
	CLarBtn m_Btn_AudioSampleRate;		// 采样率下拉框
	CLarBtn m_Btn_BitRate;				// 比特率下拉框
	CLarBtn m_Btn_AudioDevice;			// 音频设备下拉框
	CLarBtn m_Btn_MicroDevice;			// 麦克风设备
	CLarPngBtn m_Btn_DisplayMouse;//显示鼠标
	CLarPngBtn m_Btn_OpenStart;	  // 开机启动
	CLarPngBtn m_Btn_MInimalTobar;// 最小化到托盘
	CLarPngBtn m_Btn_QuitExe;	  // 退出程序
	CLarPngBtn m_Btn_Water;		// 水印

	bool m_Bool_IsDisplayMouse = false;
	bool m_Bool_IsOpenStart = false;
	bool m_Bool_MInimalTobar = false;
	bool m_Bool_QuitExe = false;
	bool m_Bool_IsWaterOn = true;

	//文本控件
	CLazerStaticText m_Stat_FolderConfig;	 // 提示设置界面为保存目录设置的文本
	CLazerStaticText m_Stat_SavePath;		 // 保存目录设置文本
	CLazerStaticText m_Stat_VideoFormat;	 // 文件格式
	CLazerStaticText m_Stat_VideoConfig;	 // 视频参数文本
	CLazerStaticText m_Stat_Measure;		 // 尺寸
	CLazerStaticText m_Stat_Fps;			 // 帧率
	CLazerStaticText m_Stat_VideoCodec;		 // 视频编码器
	CLazerStaticText m_Stat_VideoQualityMode;// 画质
	CLazerStaticText m_Stat_AudioConfig;	 // 音频参数
	CLazerStaticText m_Stat_SampleRate;		 // 采样率
	CLazerStaticText m_Stat_BitRate;		 // 比特率
	CLazerStaticText m_Stat_MouseConfig;	 // 鼠标设置
	CLazerStaticText m_Stat_GeneralConfig;   // 通用设置
	CLazerStaticText m_Stat_StartupFirst;    // 开机启动
	CLazerStaticText m_Stat_CloseExe;        // 关闭软件
	CLazerStaticText m_Stat_MouseDisplay;    // 鼠标显示
	CLazerStaticText m_Stat_AudioDevice;	 // 音频设备
	CLazerStaticText m_Stat_MicroDevice;     // 麦克风设备

	// 下拉列表相关
	CLarPopupListBoxs m_ListBoxs;
	CStringArray m_Array_VideoFormat;
	CStringArray m_Array_VideoPix;
	CStringArray m_Array_VideoFps;
	CStringArray m_Array_VideoQuality;
	CStringArray m_Array_VideoMbps;
	CStringArray m_Array_VideoCodec;
	CStringArray m_Array_AudioSampleRate;
	CStringArray m_Array_AudioMbps;
	CStringArray m_Array_AudioDevice;
	CStringArray m_Array_MicroDevice;

	enum {
		TIMER_DELAYED_REDRAW = 1001  // 延迟repaint定时器ID
	};
	int m_redrawTimerCount = 0;


	static int diffY;//滚动偏差


	//窗口阴影
	CWndShadow m_Shadow;
	bool m_bool_IsEraseTopMost = false;
};