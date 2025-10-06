#pragma once
#include "afxdialogex.h"
#include "CLarPngBtn.h"
#include "CLazerStaticText.h"
#include "CLarBtn.h"
class LarPaymentPolling;
// Ui_LevelUpDlg 对话框
class Ui_LevelUpDlg : public CDialogEx
{

	DECLARE_DYNAMIC(Ui_LevelUpDlg)
	struct SetMeal
	{
		std::wstring text_TypeName;   
		std::wstring text_Price;      
		std::wstring text_OriPrice;   
		std::wstring text_DeviceBinds;
		std::wstring text_LowToday;   

		Gdiplus::Font* font_TypeName;		//套餐名（终身VIP...）
		Gdiplus::Font* font_Price;			//中间价格
		Gdiplus::Font* font_oriPrice;		//原价
		Gdiplus::Font* font_deviceBinds;	//几台同时登录
		Gdiplus::Font* font_lowToday;		//低至几天,如果永久就是永久

		Gdiplus::SolidBrush* sb_TypeName;
		Gdiplus::SolidBrush* sb_Price;
		Gdiplus::SolidBrush* sb_oriPrice;
		Gdiplus::SolidBrush* sb_deviceBinds;
		Gdiplus::SolidBrush* sb_lowToday;

		Gdiplus::SolidBrush* clickSB_TypeName;
		Gdiplus::SolidBrush* clickSB_Price;
		Gdiplus::SolidBrush* clickSB_oriPrice;
		Gdiplus::SolidBrush* clickSB_deviceBinds;
		Gdiplus::SolidBrush* clickSB_lowToday;

		Gdiplus::RectF rect_TypeName;
		Gdiplus::RectF rect_Price;
		Gdiplus::RectF rect_oriPrice;
		Gdiplus::RectF rect_deviceBinds;
		Gdiplus::RectF rect_lowToday;

		int int_Left;
		int int_top;
		int int_width;
		int int_height;

		SetMeal()
			: font_TypeName(nullptr), font_Price(nullptr), font_oriPrice(nullptr),
			font_deviceBinds(nullptr), font_lowToday(nullptr),
			sb_TypeName(nullptr), sb_Price(nullptr), sb_oriPrice(nullptr),
			sb_deviceBinds(nullptr), sb_lowToday(nullptr),
			int_Left(0), int_top(0), int_width(0), int_height(0),
			clickSB_TypeName(nullptr), clickSB_Price(nullptr), 
			clickSB_oriPrice(nullptr),clickSB_deviceBinds(nullptr), clickSB_lowToday(nullptr),
			text_TypeName(L""), text_Price(L""), text_OriPrice(L""),
			text_DeviceBinds(L""), text_LowToday(L"")
		{}
	};

	// 价格信息结构体
	struct PriceInfo
	{
		int id;
		CString name;
		CString badge;  // 可能为null
		double amount;
		CString desc;

		CString GetFormattedDesc() const
		{
			if (desc.IsEmpty() || wcschr(desc, L'?') != nullptr)
			{
				switch (id)
				{
				case 8: return _T("5台同时登录");
				case 6: return _T("3台同时登录");
				case 5: return _T("5台同时登录");
				default: return _T("多台设备登录");
				}
			}
			return desc;
		}
	};

	// 额外字段(月度会员)
	struct CouponPrice
	{
		int32_t id;
		std::wstring name;
		std::wstring badge;
		uint32_t amount;
		std::wstring desc;

		CouponPrice() :
			id(-1),
			name(L""),
			badge(L""),
			amount(0),
			desc(L"")
		{
		}
	};

	enum QRCodeStatus
	{
		QR_INITIAL,    // 初始状态
		QR_LOADING,    // 正在加载
		QR_LOADED,     // 加载成功
		QR_FAILED,     // 加载失败
		QR_TIMEOUT     // 超时，需要刷新
	};
public:
	Ui_LevelUpDlg(int x, int y, int w, int h, CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_LevelUpDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LEVELUP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	//按钮接口 
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnSupticketandservice();

	afx_msg LRESULT StartNewPaymentPolling(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT UpdateUiModelData(WPARAM wParam, LPARAM lParam);
public:
	double getUserDpi();
	void initCtrl();
	void initSetMeal();
	void UpdateScale();
	void LoadRes();
	bool getPriceInfo();
	bool loadQRCode();
	void RequestDatas();
	void SetSelectedVipType(int type);
private:
	//窗口基本参数
	int m_windowWidth;
	int m_windowHeight;
	int m_x;
	int m_y;
	double m_Scale;
	SolidBrush* m_ClientBrush;	//填充色
	QRCodeStatus m_QRCodeStatus;
	LarPaymentPolling* m_PaymentPolling;
	
	//Gdiplus渲染
	Gdiplus::Bitmap* m_Bitmap_gp = nullptr;
	Gdiplus::Graphics* m_Graphics_mem = nullptr;

	//位图
	Gdiplus::Bitmap* m_MemBitmap = nullptr;
	Gdiplus::Bitmap* m_Bitmap_SetmealBk1 = nullptr;
	Gdiplus::Bitmap* m_Bitmap_SetmealBk2 = nullptr;
	Gdiplus::Bitmap* m_Bitmap_TitleLogo = nullptr;
	Gdiplus::Bitmap* m_Bitmap_PayLogo = nullptr;
	Gdiplus::Bitmap* m_Bitmap_QRCode = nullptr;

	//区域
	Gdiplus::Rect m_Rect_TitleLogo;
	Gdiplus::Rect m_Rect_PayLogo;

	//控件
	std::map<INT, SetMeal> m_Map_SetMeals;
	CLarPngBtn m_btn_close;
	CLazerStaticText m_stat_price;
	CLazerStaticText m_stat_oriprice;
	CLarBtn m_btn_supTicketAndService;

	//阴影框架
	CWndShadow m_Shadow;

	int m_SelectedSetMeal = 0;		//记录当前被点击的套餐卡片下标，-1 表示未选中

	//套餐信息相关
	std::vector<PriceInfo> m_PriceInfos;			// 价格信息数组
	CouponPrice m_CouponPrice;
	std::string m_PreOrderNo;
	int m_int_Coupon;
};
