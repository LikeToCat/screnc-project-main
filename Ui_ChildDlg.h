#pragma once


// Ui_ChildDlg 对话框
#include <afxwin.h>
#include <dwmapi.h>
#include <gdiplus.h>

//Ui控件相关
#include "CLarBtn.h"
#include "CLarEdit.h"
#include "CLazerStaticText.h"
#include "CLarPngBtn.h"
#include "CLarListBox.h"

//核心功能相关
#include "CameraCapture.h"   //摄像头画面捕获器
#include "ScreenRecorder.h"  //屏幕录制器
#include "Ui_CameraPreviewSDL.h"
//外部依赖
#pragma comment ( lib, "dwmapi.lib")
#pragma comment ( lib, "gdiplus.lib")
#include <cmath>

class CCustomSliderCtrl : public CSliderCtrl
{
	DECLARE_DYNAMIC(CCustomSliderCtrl)

public:
	CCustomSliderCtrl();
	virtual ~CCustomSliderCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

private:
	bool m_bDragging;
	int CalculatePositionFromPoint(CPoint point);
};

class Ui_ChildDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_ChildDlg)

public:
	struct RecordTimerRect
	{
		Gdiplus::Pen* sb_borderPenB;
		Gdiplus::SolidBrush* sb_bkB;
		Gdiplus::SolidBrush* sb_timeTextB;
		Gdiplus::SolidBrush* sb_CurRecordSize;

		Gdiplus::Rect rect_timeTextBkArea;
		Gdiplus::RectF rect_timerTextArea;
		Gdiplus::RectF rect_CurRecordSizeArea;

		Gdiplus::Font* timerFont;
		Gdiplus::Font* sizeFont;

		std::wstring timerStr;
		std::wstring sizeStr;

		RecordTimerRect() :
			sb_borderPenB(nullptr),
			sb_bkB(nullptr),
			sb_timeTextB(nullptr),
			sb_CurRecordSize(nullptr),
			timerFont(nullptr),
			sizeFont(nullptr)
		{
		}
	};

	Ui_ChildDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_ChildDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHILDDIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	void CloseCarmeraPreview();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	//本窗口控件响应
	afx_msg void OnBnClickedBtnSystemaudio();		//系统音量图标按钮按下
	afx_msg void OnBnClickedBtnMicro();				//麦克风图标按钮点击
	afx_msg void OnBnClickedBtnCamera();			//点击了摄像头图标
	afx_msg void OnBnClickedBtnStartrecord();		//点击了录制按钮
	afx_msg void OnBnClickedBtnMouseareapreset();
	afx_msg void OnBnClickedBtnCarmeraoption();
	afx_msg void OnBnClickedBtnFollowmouserecord();
	afx_msg void OnBnClickedBtnPause();
	afx_msg void OnBnClickedBtnStoprecordingduringrecord();
	afx_msg void OnBnClickedBtnResume();
	afx_msg void OnBnClickedBtnReturn();
	afx_msg void OnBnClickedBtnCombox();
	afx_msg LRESULT OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam);//下拉框项点击
	afx_msg LRESULT On_UpdateVolume(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_SliderMoveOnEdge(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_ScrencFileSizeUpdate(WPARAM wParam, LPARAM lParam);

	//来自其他MFC窗口消息的响应
	afx_msg LRESULT On_Ui_TopRecDlg_BnClickStartReocrd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT On_Ui_TopRecDlg_BnClickStopReocrd(WPARAM wParam, LPARAM lParam);
public:
	void CleanUpGdiPngRes();
	void SetDlgDisplayRect(CRect DisplayRect);
	void SetRecordArea(const CRect& AreaRect);
	void UpdateRecordMouseAreaUi();
	void ResetRecordMouseAreaUi();
	void UpdateAudioRecord();
	void ResetAudioRecord();
	void UpdateAreaRecordRect(const CRect AreaRecordRect);
	void GetRecordMode(bool* IsNormalRecord, bool* IsAreaRecord, bool* isMouseAreaRecord);
	void OnMouseMovedAnywhere();
	void HideListBox();
	void initRecordingCtrlPos();
	void SetRecordCountTimer();
	void GetUserDPI();
	void initCtrlPos();
	void initRecordTimerRect();
	void initCtrl();
	void InitDropListData();
	void SetRecordOptBtnText(CString text);

	//实时更新录制大小相关
	bool InitTotalDiskSizeFromPath(const std::wstring& path);	// 根据路径初始化磁盘总容量(只在开始录制时调用一次)
	void UpdateRecordSizeUI(double sizeMB);		// 更新录制大小显示字符串
public:
	////--------Ui相关
	float m_Scale;//DPI系数
	float m_WindowWidth;//窗口宽度
	float m_WindowHeight;//窗口高度

	//滑块控件相关
	CCustomSliderCtrl m_Slider_SystemAudio;// 系统声音滑块
	CCustomSliderCtrl m_Slider_Micro;		 // 麦克风滑块

	//图片按钮
	CLarPngBtn m_Btn_Return;			// 返回
	CLarPngBtn m_Btn_SysteamAudio;		// 系统声音
	CLarPngBtn m_Btn_Micro;				// 麦克风
	CLarPngBtn m_Btn_Camera;			// 摄像头
	CLarPngBtn m_Btn_StartRecording;	// 开始										
	CLarPngBtn m_Btn_MouseRecordArea;	// 鼠标录制区域
	CLarPngBtn m_btn_pause;
	CLarPngBtn m_btn_stopRecordingDuringRecord;
	CLarPngBtn m_btn_resume;

	//文本控件
	CLazerStaticText m_Stat_Micro;// 麦克风
	CLazerStaticText m_Stat_Camera;// 摄像头
	CLazerStaticText m_Stat_SystemAudio;// 系统声音
	CLazerStaticText m_stat_HotkeyStartRecord;	// Alt + B 开始停止录制快捷键提示

	//文字按钮
	CLarBtn m_Btn_MouseRecordAreaPreset;	// 鼠标录制区域文本
	CLarBtn m_Btn_CameraOption;				//摄像头选项参数下拉框
	CLarBtn m_btn_Combox;

	CLarPopupListBoxs m_ListBoxs;
	CStringArray m_Array_MouseRecordAreaPreset;
	CStringArray m_Array_CarmerOptions;
	RecordTimerRect m_recordTimerRect;

	////----功能相关
	//摄像头功能相关
	bool m_IsHasCameraDeive = true;
	bool m_IsCameraRecord = false;
	int m_CameraOption_Width = 640;
	int m_CameraOption_Height = 480;
	int m_CameraOption_Fps = 30;
	std::string m_string_CameraPix = "yuyv422";
	std::string m_string_CameraName = "";
	std::unique_ptr<std::thread> m_Thread_CameraPrviewSDL;

	////录制功能相关
	bool m_IsRecording = false;		//是否正在录制
	bool m_IsOnlyAudioRecord = false;	//是否只录制声音
	bool m_IsMouseAreaRecord = false;

	//区域录制相关
	CRect m_CRect_RecordRect = CRect(0, 0, 0, 0); //选区录制时录制的区域

	//跟随鼠标录制相关
	POINT m_LastMousePos;     // 上次记录的鼠标位置
	bool m_MousePosInitialized = false;  // 鼠标位置是否已初始化

	//点击喇叭静音与恢复逻辑相关
	int m_LastAudioVolumePos;
	int m_LastMicVolumePos;
	bool m_IsAudioSilent = false;
	bool m_IsMicSilent = false;

	// 防止重复停止
	bool m_IsStopping = false;

	//录制时长计时相关
	int m_recordElapsedSec = 0;

	//记录输出文件路径与磁盘容量相关
	std::wstring m_lastRecordOutputPath;// 最后一次录制输出文件路径
	double m_totalDiskSizeGB = 0.0;		// 录制开始时该盘符的剩余空间(GB)
	bool m_hasTotalDiskSize = false;	// 是否已成功获取该剩余空间

	CStringArray m_Array_RecordOptions;	// 录制选项下拉框数据
};