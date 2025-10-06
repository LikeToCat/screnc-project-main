#pragma once


// Ui_RecTopDlg 对话框
#include "CLarPngBtn.h"
#include "CLazerStaticText.h"
#include "CLarBtn.h"
#include "CLarToolsTips.h"
#include "WndShadow.h"
#include <functional>
#include <chrono>
#include <sstream>
class Ui_PopupMenuDlg;
class Ui_RecTopDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_RecTopDlg)

public:
	Ui_RecTopDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_RecTopDlg();

	enum class RecContextType
	{
		None = 0,
		Child,      // 全屏/区域/游戏 等通过 Ui_ChildDlg
		Window      // 应用窗口录制 Ui_WindowRecordDlg
	};

	void SetRecordContext_None();         
	void SetRecordContext_Child();        
	void SetRecordContext_Window();       
	RecContextType GetRecordContext() const { return m_ctxType; } 

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_RECTOPWINDOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//本窗口按钮响应
	afx_msg void OnBnClickedBtnTransparent();
	afx_msg void OnBnClickedBtnNail();
	afx_msg void OnBnClickedBtnStartrecordpng();
	afx_msg void OnBnClickedBtnStartrecord();
	afx_msg void OnBnClickedBtnRecording();
	afx_msg void OnBnClickedBtnScreenphoto();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnPause();
	afx_msg void OnBnClickedBtnResumerecord();

	//其他消息响应
	afx_msg LRESULT OnBnScreenShotCompelete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTimerCountStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopTimeCountAndShowNormalUi(WPARAM wParam, LPARAM lParam);
public:
	void SetWindowRect(CRect rect);
	void HideAllTooltips();
	void CleanUpGdiPngRes();
	void SetPauseUIMode();
	void ResumeFromPauseUiMode();
private:
	void GetUserDPI();  //获取用户DPI
	void UpdateScale();	//调整控件和窗口的大小，以及控件的位置
	void InitCtrl();    //初始化控件
	void InitToolWindow(bool IsMinimalWithMain);	//初始化为工具窗口（不会再任务栏中显示）
	void LoadRes();     //加载前端资源
	void CreateMenu();  //创建菜单
	void SetOpacity(int percent);//设置窗口透明度
public:
	//窗口基本属性
	CRect m_Rect_WindowRect;
	float m_Scale;
	int m_WindowWidth;
	int m_WindowHeight;
	int m_WindowX;
	int m_WindowY;
	bool m_IsRecording;
	bool m_IsHideDuringRecord;

	//控件
	CLarPngBtn m_Btn_Nail;				// 别针
	CLarPngBtn m_Btn_Transparent;		// 不透明按钮
	CLarPngBtn m_Btn_ScreenShot;		// 截屏
	CLarPngBtn m_btn_Close;				// 关闭
	CLarPngBtn m_btn_StartRecord;		// 开始录制
	CLarPngBtn m_btn_Recording;		    // 录制中
	CLarPngBtn m_btn_pause;
	CLarPngBtn m_btn_resumeRecord;
	CLarBtn m_btn_StartRecordText;		// 开始录制文字按钮
	CLazerStaticText m_Stat_Pixel;		// 分辨率
	CLazerStaticText m_Stat_TimeCount;	// 时间计数

	//工具控件
	CLarToolsTips m_toolTips;			// 提示窗口

	//菜单
	Ui_PopupMenuDlg* m_Menu_Transp;		// 透明度选择菜单
	
	//录制计数相关
	std::chrono::steady_clock::time_point m_Time_Start;
	std::wstringstream m_stream;

	//暂停相关状态，用于“暂停计时冻结，恢复后继续累加”
	bool m_IsPaused = false;                                                
	std::chrono::steady_clock::time_point m_PauseStart;                     
	std::chrono::steady_clock::duration m_PausedAccum = std::chrono::seconds::zero();	

	CWndShadow m_Shadow;			//阴影框架
	RecContextType m_ctxType;		//当前顶栏控制上下文
};
