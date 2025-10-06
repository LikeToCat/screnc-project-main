
// MFCMediaPlayerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ScreenRecorder.h"
#include "CameraCapture.h"
#include "AreaSelectDlg.h"

// CMFCMediaPlayerDlg 对话框
class CMFCMediaPlayerDlg : public CDialogEx
{
// 构造
public:
	CMFCMediaPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数
// 对话框数据
	enum { IDD = IDD_MFCMEDIAPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
protected:
	HICON m_hIcon;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtnScreenrecording();
	afx_msg void OnBnClickedBtnStopScreenrecording();
	afx_msg void OnOpenCameraClickedButton();
	afx_msg void OnCloseCameraDisplayClickedButton();
	afx_msg void OnAreaRecordingClickedBtnScreenrecording();
private:
	CButton m_ScreenRecordingBtn;// 录屏按钮
	CStatic m_DisplayArea;// 画面显示区域
	ScreenRecorder* m_ScreenRecorder;
	CameraCapture* m_CameraCapture;
private:
public:
	afx_msg void OnHandleRecordingClickedBtn();
};
