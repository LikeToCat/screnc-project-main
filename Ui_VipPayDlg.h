#pragma once
#include <thread>
#include "WndShadow.h"
#include "LarPaymentPolling.h"
// Ui_VipPayDlg 对话框
class Ui_LoginDlg;
class Ui_VipPayDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Ui_VipPayDlg)
#define OPENVIP_CUSSTAT_PRIVILEGE 1103 
public:
	enum class Mode
	{
		RedPacketCoupon,
		LowPriceWithMouthBill
	};

	struct cus_stat
	{
		Gdiplus::SolidBrush* txtB;
		Gdiplus::SolidBrush* txthovB;
		Gdiplus::SolidBrush* finalTxtB;
		Gdiplus::Font* font;
		std::wstring str;
		UINT uId;

		Gdiplus::Rect txtR;
		Gdiplus::Rect btnArea;

		std::function<void()> clickCallBack;

		cus_stat()
			: txtB(nullptr)
			, txthovB(nullptr)
			, finalTxtB(nullptr)
			, font(nullptr)
			, str()
			, uId(0)
			, txtR(0, 0, 0, 0)
			, btnArea(0, 0, 0, 0)
			, clickCallBack(nullptr)
		{
		}
	};

	//顶部优惠倒计时
	struct CountDownOfferExpiration
	{
		std::wstring wstr_days;
		std::wstring wstr_hours;
		std::wstring wstr_minute;
		std::wstring wstr_seconds;
		std::wstring wstr_millSeconds;

		Gdiplus::Font* font_days;
		Gdiplus::Font* font_hours;
		Gdiplus::Font* font_minute;
		Gdiplus::Font* font_seconds;
		Gdiplus::Font* font_millSeconds;
		Gdiplus::Font* font_unit;

		Gdiplus::Rect rect_days;
		Gdiplus::Rect rect_hours;
		Gdiplus::Rect rect_minutes;
		Gdiplus::Rect rect_seconds;
		Gdiplus::Rect rect_millSeconds;
		std::vector<Gdiplus::Rect> vec_rect_units;
		int unit_interval;

		Gdiplus::SolidBrush* sb_days;
		Gdiplus::SolidBrush* sb_hours;
		Gdiplus::SolidBrush* sb_minutes;
		Gdiplus::SolidBrush* sb_seconds;
		Gdiplus::SolidBrush* sb_millSeconds;
		Gdiplus::SolidBrush* sb_unit;

		CountDownOfferExpiration() :
			sb_days(nullptr),
			sb_hours(nullptr),
			sb_minutes(nullptr),
			sb_seconds(nullptr),
			sb_millSeconds(nullptr),
			sb_unit(nullptr),
			font_days(nullptr),
			font_hours(nullptr),
			font_minute(nullptr),
			font_seconds(nullptr),
			font_millSeconds(nullptr),
			font_unit(nullptr)
		{
		}
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

	//二维码相关
	enum QRCodeStatus
	{
		QR_INITIAL,    // 初始状态
		QR_LOADING,    // 正在加载
		QR_LOADED,     // 加载成功
		QR_FAILED,     // 加载失败
		QR_TIMEOUT     // 超时，需要刷新
	};
	Ui_VipPayDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Ui_VipPayDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VIPDLG };
#endif
public:
	void CleanUpGdiRes();
	void Ui_SetWindowPos(const CRect& rect);
	void Ui_UpdateWindowPos(const CRect& rect);
	inline int getMouthBillPrice() { return m_MouthBillPrice; }
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();

	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnBnClickedBtnSupticketandservice();
private:
	//辅助方法
	void GetUserDpi();
	void UpdateScale();
	void InitCtrl();
	void InitCusStat();
	void InitCountDownOfferExpiration();
	void EnableShadow();
	void LoadRes();
	void CleanUpRes();
	float GetNumericPrice(const CString& priceText);
	void ReloadBitmapBySelect(int type);
	void SetMemberTypeStat_SelectColor(int type);
	void SetLowToDayStatNormalStyle(CLazerStaticText* stat);
	void SetLowToDayStatSelectStyle(CLazerStaticText* stat);
	void SetRmbLogoStatStyle(CLazerStaticText* stat);
	void SetNormalCtrlUiAlpha();
	void SetDarkCtrlUi();
	void UpdateOfferCountdownDisplay();
	void UpdatePriceNewUserCouponInfo(std::chrono::milliseconds lestTime);

	// 二维码与轮询的处理方法
	void LoadQRCode();													// 开始加载二维码（异步）
	void QRCodeLoadingThread();											// 二维码加载线程
	void DrawQRCodeOrStatus(Gdiplus::Graphics* graphics);				// 绘制二维码或状态信息
	bool IsPointInQRCodeArea(CPoint point);								// 检查点击是否在二维码区域
	void StartPaymentPolling();											// 开始支付状态轮询
	void OnPaymentSuccess();											// 支付成功回调
	void OnPaymentTimeout();											// 支付超时回调
	void RefreshQRCode();												// 刷新二维码
	LRESULT OnPaymentTimeoutMsg(WPARAM wParam, LPARAM lParam);			// 支付超时消息处理
	LRESULT OnPaymentSuccessMsg(WPARAM wParam, LPARAM lParam);			// 支付成功消息处理
	LRESULT OnStartPaymentPollingMsg(WPARAM wParam, LPARAM lParam);		// 启动支付轮询消息处理
	LRESULT OnStopPaymentPollingAndQuit(WPARAM wParam, LPARAM lParam);	// 轮询失败时处理
	LRESULT OnLoginShow(WPARAM wParam, LPARAM lParam);					// 登录窗口弹窗
	LRESULT OnInRedPacketMode(WPARAM wParam, LPARAM lParam);			// 领取过红包，且再次打开当前界面
	LRESULT OnUpdateScaleWithMode(WPARAM wParam, LPARAM lParam);		// 判断出当前显示模式后，更新界面布局

	//套餐和更新获取相关
	void UpdatePriceDisplay();			// 更新价格显示
	bool IsPointInRect(const CPoint& point, const Gdiplus::Rect& rect);// 检查点是否在矩形内
	void SetSelectedVipType(int type);	// 设置选中的 VIP 类型
public:
	//窗口显示参数
	float m_Scale;
	CRect m_CRect_WindowRect;		// 窗口矩形
	std::atomic<bool> m_bool_IsInitReady = false;
	std::atomic<bool> m_bool_IsDarker = false;
	Mode m_Mode;

	//背景位图
	Gdiplus::Bitmap* m_Bitmap_VipInfo = nullptr;			// VIP特权logo
	Gdiplus::Bitmap* m_Bitmap_PermentVip = nullptr;			//永久vip
	Gdiplus::Bitmap* m_Bitmap_YearVip = nullptr;			//年度
	Gdiplus::Bitmap* m_Bitmap_HalfYear = nullptr;			//半年 
	Gdiplus::Bitmap* m_Bitmap_Sugbuy = nullptr;				//热卖推荐
	Gdiplus::Bitmap* m_Bitmap_CouponActivityInfo = nullptr;	//新人立享.....
	Gdiplus::Bitmap* m_Bitmap_SeasonVip = nullptr;
	Gdiplus::Bitmap* m_Bitmap_QRCodeBk = nullptr;
	Gdiplus::Bitmap* m_Bitmap_BuddleType1LowToday = nullptr;//type1套餐的顶部的低至
	Gdiplus::Bitmap* m_Bitmap_TitleLogoNewUser = nullptr;
	//Gdiplus::Bitmap* m_Bitmap_MouthBill = nullptr;
	Gdiplus::Bitmap* m_Bitmap_VipPrivilegeLower = nullptr;
	Gdiplus::Bitmap* m_Bitmap_LoginQRCodeBk = nullptr;

	Gdiplus::Rect m_Rect_QRCode;	// 二维码矩形区域
	Gdiplus::Rect m_Rect_Caption;	// 标题栏区域
	Gdiplus::Rect m_Rect_InnerPay;	// 支付信息区域
	Gdiplus::Rect m_Rect_SeasonVip;				//季度VIP
	Gdiplus::Rect m_Rect_PermentVip;			//永久vip
	Gdiplus::Rect m_Rect_YearVip;				//年度
	Gdiplus::Rect m_Rect_HalfYear;				//半年 
	Gdiplus::Rect m_Rect_Sugbuy;				//热卖推荐
	Gdiplus::Rect m_Rect_CouponActivityInfo;	//新人立享.....
	Gdiplus::Rect m_Rect_VipInfoRect;			//Vip详细信息区域
	Gdiplus::Rect m_Rect_BuddleType1LowToday;
	Gdiplus::Rect m_Rect_QRCodeBK;
	Gdiplus::Rect m_Rect_TitleLogoNewUser;
	//Gdiplus::Rect m_Rect_MouthBill;

	//控件
	CLarPngBtn m_Btn_Close;					// 关闭按钮
	CLarPngBtn m_Btn_PayWay;				// 微信支付宝扫码支付	
	CLazerStaticText m_Stat_TitleText;		// 开通会员标题
	CLazerStaticText m_Stat_UserProtocol;	// 用户协议
	CLazerStaticText m_Stat_PayText;	    // 支付订单
	CLazerStaticText m_Stat_PayPriceText;	// 支付金额文本
	CLazerStaticText m_Stat_Price;		    // 价格
	CLazerStaticText m_Stat_PermentVip;		// 永久会员价格
	CLazerStaticText m_Stat_YearAmount;		// 年度会员价格
	CLazerStaticText m_Stat_HalfYearAmount;	// 半年会员价格
	CLazerStaticText m_Stat_OriPrice;		// 原价
	CLazerStaticText m_Stat_VipType1;
	CLazerStaticText m_Stat_VipType2;
	CLazerStaticText m_Stat_VipType3;
	CLazerStaticText m_stat_vipType4;		// VIP类型4
	CLazerStaticText m_Stat_BindDevice1;
	CLazerStaticText m_Stat_BindDevice2;
	CLazerStaticText m_Stat_BindDevice3;
	CLazerStaticText m_stat_CouponActivityInfo;	// 新人特享优惠立减140元
	CLazerStaticText m_stat_type1OriPrice;		// 套餐1的原价
	CLazerStaticText m_stat_type2OriPrice;		// 套餐2原价
	CLazerStaticText m_stat_Type3OriPrice;		// 套餐3
	CLazerStaticText m_stat_type4OriPrice;
	CLazerStaticText m_Stat_SeasonAMount;		// 季度会员价格
	CLazerStaticText m_stat_type1RmbLogo;
	CLazerStaticText m_stat_type2RmbLogo;
	CLazerStaticText m_stat_type3RmbLogo;
	CLazerStaticText m_stat_type4RmbLogo;
	CLazerStaticText m_stat_type1LowToDay;
	CLazerStaticText m_stat_type2LowToDay;
	CLazerStaticText m_stat_type3LowToDay;
	CLazerStaticText m_stat_type4LowToDay;
	CLazerStaticText m_stat_priceRmbLogo;
	CLazerStaticText m_stat_bindDevice4;
	CLazerStaticText m_stat_type1BuddleLowToday;
	CLarBtn m_btn_supTicketAndService;
	CLazerStaticText m_stat_offerExpiration;			// 优惠倒计时
	CountDownOfferExpiration m_countDownOfferExpiration;
	//CLazerStaticText m_stat_mouthBillPrice;	// 价格

	//点击套餐相关
	int m_SelectedVipType = 0;// 当前选中的 VIP 类型 (0=套餐1, 1=套餐2, 2=套餐3)
	int m_HoverVipType = -1;	// 鼠标悬停的区域 (-1=无悬停, 0=永久, 1=年度, 2=半年)

	Gdiplus::Bitmap* m_Bitmap_QRCode = nullptr;  // 二维码图像
	QRCodeStatus m_QRCodeStatus = QR_INITIAL;    // 二维码状态
	std::thread m_Thread_LoadQRCode;             // 加载二维码的线程
	bool m_ThreadRunning = false;                // 线程运行状态

	// 支付轮询相关
	LarPaymentPolling m_PaymentPolling;          // 支付轮询管理器

	//窗口阴影相关
	CWndShadow m_Shadow;

	//红包相关
	static int m_RedPacketCouponAmount;
	static bool m_isHasRedPacketCoupon;
	static std::chrono::milliseconds m_timeLest;
	static std::chrono::milliseconds m_InitialTimeLest;
	static std::string m_urlCouponAmount;

	//套餐信息
	CString m_PreOrderNo;							// 预订单号
	std::vector<PriceInfo> m_PriceInfos;			// 价格信息数组

	//更多权益文本按钮相关
	cus_stat m_cusstat_vipPrivilegeContrast;	// vip权益对比
	bool m_bVipPrivilegeHover = false;

	//额外的月度会员字段
	//CouponPrice m_CouponPrice;
	std::string m_preOrderNo;
	int m_MouthBillPrice;

	//顶部优惠倒计时
	static std::chrono::system_clock::time_point g_offerExpireAt;              //活动截止时间
	std::chrono::system_clock::time_point m_offerExpireAt;
	static bool g_offerExpireAtInited;
	UINT_PTR m_timerIdOfferCountdown = 0; //倒计时Timer ID
	Ui_LoginDlg* m_Dlg_Login = nullptr;
};
