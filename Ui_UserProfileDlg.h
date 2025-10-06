#pragma once


// Ui_UserProfileDlg 对话框
#include "WndShadow.h"
#include "CLarBtn.h"
#include "CLarPngBtn.h"
#include "CLazerStaticText.h"
class Ui_UserProfileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_UserProfileDlg)

public:
	Ui_UserProfileDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_UserProfileDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_USERPROFILE };
#endif
public:
	void ShowAtXY(int x, int y);
	void UpdateUserInfo();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	//按钮控件响应
	afx_msg void OnBnClickedBtnLoginout();
	afx_msg void OnBnClickedBtnContactservice();
	afx_msg void OnBnClickedBtnOpenvip();

	afx_msg LRESULT On_AppMsg_LogoutSuccess(WPARAM wp, LPARAM lp);
private://辅助功能函数
	enum { IDT_MOUSE_CHECK = 1001 };
	void initCtrl();
	void UpdateScale();
	float GetUserDPI();
private:
	//窗口基本参数
	int m_WindowWidth;		//窗口宽s
	int m_WindowHeight;		//窗口高
	float m_Scale;			//用户缩放系数
	int m_BoundaryLineY;	//分界线
	CWndShadow m_Shadow;	//窗口阴影

	//标志
	BOOL m_bool_isEnterIn;	//鼠标是否进入过该窗口
	BOOL m_bool_isInitlize;	//窗口是否创建完毕并被初始化

	//图片按钮控件
	CLarPngBtn m_btn_UserIcon1;        // 用户图标1
	CLarPngBtn m_btn_UserIcon2;        // 用户图标2

	//文字按钮 
	CLarBtn m_btn_logOut;	           // 退出登录
	CLarBtn m_btn_contactservice;	   // 联系客服
	CLarBtn m_btn_openVip;				// 开通会员

	//文本控件
	CLazerStaticText m_stat_phonenum1; // 手机号1
	CLazerStaticText m_stat_phonenum2; // 手机号2
	CLazerStaticText m_stat_memberinfo;// 会员信息
	CLazerStaticText m_stat_maxbind;   // 可绑定数量
	CLazerStaticText m_stat_expiretime;// 到期时间
};
