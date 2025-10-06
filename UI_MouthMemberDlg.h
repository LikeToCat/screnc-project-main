#pragma once
#include "afxdialogex.h"
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "CLazerStaticText.h"
#include "Ui_VipPayDlg.h"
// UI_MouthMemberDlg 对话框
class UI_MouthMemberDlg : public CDialogEx
{
	DECLARE_DYNAMIC(UI_MouthMemberDlg)
public:
	UI_MouthMemberDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~UI_MouthMemberDlg();
	struct CountDown
	{
		std::wstring str_hours;
		std::wstring str_minutes;
		std::wstring str_seconds;
		Gdiplus::Rect rect_hours;
		Gdiplus::Rect rect_minutes;
		Gdiplus::Rect rect_seconds;
		Gdiplus::Font* font;
		Gdiplus::SolidBrush* soildBrush;

		CountDown():
			font(nullptr),
			soildBrush(nullptr)
		{}
	};
	enum QRCodeStatus
	{
		QR_INITIAL,    // 初始状态
		QR_LOADING,    // 正在加载
		QR_LOADED,     // 加载成功
		QR_FAILED,     // 加载失败
		QR_TIMEOUT     // 超时，需要刷新
	};
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SEASONMEMBER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnSupticketandservice();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg LRESULT StartNewPaymentPolling(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPaymentSuccess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoginShow(WPARAM wParam, LPARAM lParam);
public:
	double getUserDpi();
	void UpdateScale();
	void InitCtrl();
	void InitCountDown();
	void loadRes();
	void SetWindowXY(int x, int y);
	void LoadQRCodeThread();
	inline int getW() { return m_WindowWidth; }
	inline int getH() { return m_WindowHeight; }
private:
	//窗口基本参数
	CWndShadow m_Shadow;
	int m_WindowHeight;
	int m_WindowWidth;
	int m_x;
	int m_y;
	double m_Scale;
	QRCodeStatus m_QRCodeStatus;

	//位图
	Gdiplus::Bitmap* m_Bitmap_WinPriceLogo;
	Gdiplus::Bitmap* m_Bitmap_TitleLogo;
	Gdiplus::Bitmap* m_Bitmap_PayLogo;
	Gdiplus::Bitmap* m_Bitmap_QRCode;
	Gdiplus::Bitmap* m_Bitmap_QRCodeLoginBk;

	Gdiplus::Rect m_Rect_WinPriceLogo;
	Gdiplus::Rect m_Rect_TitleLogo;
	Gdiplus::Rect m_Rect_PayLogo;
	Gdiplus::RectF m_Rect_QRcode;

	//控件		
	CLazerStaticText m_stat_price;
	CLazerStaticText m_stat_rmblogo;
	CLarPngBtn m_btn_close;
	CLarBtn m_btn_supTicketAndService;
	CountDown m_CountDown;

	LarPaymentPolling* m_PaymentPolling;
	static std::chrono::system_clock::time_point s_EndTime;   // 结束时间点（首次创建后固定）
	static bool s_EndTimeInitialized;                         // 是否已设置结束时间

	Ui_LoginDlg* m_Dlg_Login;
};
