#pragma once


// Ui_LoginDlg 对话框
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "CColorfulEdit.h"
#include "CLazerStaticText.h"
#include "WndShadow.h"
#include "json.h"
#include "HttpRequestHandler.h"
class Ui_LoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_LoginDlg)

public:

	Ui_LoginDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_LoginDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOGINPAGE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnSeedveritycode();
	afx_msg void OnBnClickedBtnLogin();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeEditPhoneNum();
	afx_msg void OnEnChangeEditVerityCode();
	afx_msg LRESULT OnNcHitTest(CPoint point);
public:
	void Ui_SetWindowRect(const CRect& rect);  //在创建窗口之前，预设置窗口矩形
	void Ui_UpdateWindowPos(const CRect& rect);//创建窗口之后，设置窗口矩形并显示
	void CleanUpGdiPngRes();//清理GDI资源
	void SetCornerRadius(int radius) { m_nCornerRadius = radius; }
private:
	void GetUserDPI();       //获取用户DPI
	void UpdateScale();      //调整控件和窗口的大小，以及控件的位置
	void InitCtrl();         //初始化控件
	void LoadRes();          //加载前端资源
	void SaveUserLoginInfo();//保存用户登录的信息，创建配置文件
	void DrawCheckBox(Gdiplus::Graphics* graphics);
	bool RequestLogin();
	void UpdateDeviceInfo(Json::Value& data_user_Root);
	void DenyMessageDlg(std::string DenyMessage);
	void ErrorMessageDlg(HttpRequestHandler& requestHandler);
	bool RequestVerityCode();
	void IsPhoneInLegal(bool& retflag);
	void ApplyRoundedRegion();
public:
	//窗口基本参数
	float m_Scale;
	CRect m_CRect_WindowRect;		//窗口矩形(相对于全局)
	Gdiplus::Rect m_Rect_WindowRect;//窗口GDI+矩形（相对于窗口）
	int m_WindowWidth;				//窗口长度
	int m_WindowHeight;				//窗口高度
	int m_nCornerRadius = 32;
	CString m_originalBtnText;
	int m_diffX;
	int m_diffY;

	//区域
	CRect m_CRect_CkBox;	               //复选框
	bool m_bool_IsAutoLoginChecked = true; //用户是否点击了自动登录按钮

	//控件
	CColorfulEdit m_Edit_PhoneNum;	     // 电话号码编辑框
	CColorfulEdit m_Edit_VerityCode;	 // 验证码编辑框

	CLarBtn m_Btn_SendVerityCode;        // 发送验证码按钮
	CLarBtn m_Btn_Login;		         // 登录
	CLarBtn m_Btn_LoginByPhone;		     // 手机号登录
	CLarBtn m_Btn_FeedBack;	             // 反馈

	CLarPngBtn m_Btn_Logo;			     // logo图
	CLarPngBtn m_Btn_Close;
	CLazerStaticText m_Stat_LogoText;	 // 极速录屏大师
	CLazerStaticText m_Stat_AutoLogin;	 // 自动登录
	CLazerStaticText m_Stat_PhoneNum;    // 电话号码文本
	CLazerStaticText m_Stat_VerityCode;	 // 验证码

	//窗口阴影
	CWndShadow m_Shadow;

	//资源
	Gdiplus::Bitmap* m_Bitmap_Gou;


};
