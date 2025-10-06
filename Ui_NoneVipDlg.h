#pragma once


// Ui_MessageModalDlg 对话框
#include "CLazerStaticText.h"
#include "CLarPngBtn.h"
#include "CLarBtn.h"
#include "WndShadow.h"
#include <functional>
#include <map>
#define MODALDLG_BTN_EXTRASTART         5233
#define MODALDLG_CK_EXTRASTART         5333
class Ui_MessageModalDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_MessageModalDlg)

public:
	Ui_MessageModalDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_MessageModalDlg();
	enum CheckBoxBorderStyle
	{
		Rect,
		Eclipse
	};
	struct ExtraBtn
	{
		CString BtnText;
		Gdiplus::Color BkColoc;
		Gdiplus::Color hoverColor;
		Gdiplus::Color clickColor;
		Gdiplus::Color TextColor;
		Gdiplus::Color HoverTextColor;
		Gdiplus::Color ClickTextColor;
		Gdiplus::Color BorderColor;
		std::function<void()> BtnCallBack;
		UINT Uid;
	};
	struct ExtraCheckBox
	{
		CString CkText;
		Gdiplus::Color TextColor;
		int textSize;
		int borderSize;
		Gdiplus::Color borderColor;
		CRect CkRect;
		CRect CkBorderRect;
		UINT Uid;
		CheckBoxBorderStyle broderStyle;
		CLazerStaticText* stat;
		int diffX;
		int diffY;
		bool IsSelect;
		int MutualExclusion; //互斥，如果复选框控件中这个值相同，则复选互斥
	};
	std::vector<CLarBtn*> m_ExtraBtnControls;
	std::vector<CLazerStaticText*> m_ExtraCheckBox;
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_NONEVIP };
#endif
public:
	void Ui_SetWindowRect(const CRect& rect);  //在创建窗口之前，预设置窗口矩形
	void CleanUpGdiPngRes();//清理GDI资源
	void SetModal(CString title, CString Type, CString info, CString BtnText);
	void SetCornerRadius(int radius);
	void SetMessageTypeIcon(UINT uId);
	void SetDefaultBtnCallback(std::function<void()> SureCallBack);
	void SetDefaultBtnGradientColor(
		Gdiplus::Color c1, Gdiplus::Color c2,
		Gdiplus::Color hc1, Gdiplus::Color hc2,
		Gdiplus::Color cc1, Gdiplus::Color cc2
	);
	void SetInitialPosition(int diffx, int diffy);
	void AddButton(
		const CString BtnText,
		Gdiplus::Color BkColoc,
		Gdiplus::Color hoverColor,
		Gdiplus::Color clickColor,
		Gdiplus::Color TextColor,
		Gdiplus::Color HoverTextColor,
		Gdiplus::Color ClickTextColor,
		Gdiplus::Color BorderColor,
		std::function<void()> BtnCallBack
	);
	void AddCkBox(CString CkText, Gdiplus::Color TextColor, int textSize, int borderSize, Gdiplus::Color borderColor,
		CheckBoxBorderStyle borderType, int diffx = 0, int diffy = 0, int mutualExclusion = -1, bool isSelect = false);
	inline int GetStartCkUid() { return MODALDLG_CK_EXTRASTART; }
	void OnDialogOk();
	void OnDialogCancel();
	void GetSelectCkUid(std::vector<int>* selectIdvec);
	void HideCloseBtn();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	//本窗控件消息响应
	afx_msg void OnBnClickedBtnSure();
	afx_msg void OnBnClickedBtnClose();	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	void GetUserDPI();  //获取用户DPI
	void UpdateScale();	//调整控件和窗口的大小，以及控件的位置
	void InitCtrl();    //初始化控件
	void ApplyRoundCorner();
	void MutualExCk(int mutualEx);
	bool IsNoMutalSelect(int mutualEx);
public:
	bool m_bPosSet = false;
	int  m_InitPosX = 0;
	int  m_InitPosY = 0;
	int m_WindowWidth = 420 * m_Scale;
	int m_WindowHeight = 220 * m_Scale;
	int m_CkRectborderWidth;
	int	m_CkRectbroderHeight;
	int m_CkEcBorderWidth;
	int m_CkEcBorderHeight;
	float m_Scale;
	int m_CornerRadius;
	Gdiplus::Rect m_Rect_TitleBarArea;
	Gdiplus::Rect m_Rect_ClientArea;
	Gdiplus::Color m_gd1;
	Gdiplus::Color m_gd2;
	Gdiplus::Color m_hgd1;
	Gdiplus::Color m_hgd2;
	Gdiplus::Color m_cgd1;
	Gdiplus::Color m_cgd2;
	bool m_bIsDefaultBtnGradient;

	std::map<UINT, ExtraBtn> m_Map_ExtraBtns;
	std::map<UINT, ExtraCheckBox> m_Map_ExtraCk;
	UINT Btn_Identification_Start = MODALDLG_BTN_EXTRASTART;
	UINT Ck_Identification_Start = MODALDLG_CK_EXTRASTART;

	CLazerStaticText m_Stat_AppText;// 极速录屏大师
	CLarPngBtn m_Btn_Close;	// 关闭
	CLarPngBtn m_Btn_Icon;	    // 图标
	CLazerStaticText m_Stat_MessageType;// 消息类型
	CLazerStaticText m_Stat_MessageInfo;// 消息内容
	CLarBtn m_Btn_OpenVip;// 开通vip
	CWndShadow m_Shadow;

	CString m_MessageType;
	CString m_MessageInfo;
	CString m_Title;
	CString m_BtnText;
	bool m_IsModal;
	bool m_IsHideClose;
	std::function<void()> m_defaultBtnCallBack;


	virtual void OnOK();
	virtual void OnCancel();

};
