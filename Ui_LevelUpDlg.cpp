// Ui_LevelUpDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "theApp.h"
#include "afxdialogex.h"
#include "WndShadow.h"
#include "LarPaymentPolling.h"
#include "Ui_LevelUpDlg.h"
#include "ModalDialogFunc.h"
#include "LarStringConversion.h"
#include "GlobalFunc.h"
#include "CDebug.h"
// Ui_LevelUpDlg 对话框
extern HANDLE ConsoleHandle;
size_t Ui_LevelUpDlg_PriceInfoWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	std::string* response = (std::string*)userp;
	response->append((char*)contents, realsize);
	return realsize;
}

size_t Ui_LevelUpDlg_QRCodeWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	std::vector<unsigned char>* mem = (std::vector<unsigned char>*)userp;
	size_t oldSize = mem->size();
	mem->resize(oldSize + realsize);
	memcpy(mem->data() + oldSize, contents, realsize);
	return realsize;
}

IMPLEMENT_DYNAMIC(Ui_LevelUpDlg, CDialogEx)

Ui_LevelUpDlg::Ui_LevelUpDlg(int x, int y, int w, int h, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_LEVELUP, pParent)
{
	m_Scale = getUserDpi();
	m_x = x;
	m_y = y;
	m_windowWidth = w;
	m_windowHeight = h;
	m_Bitmap_SetmealBk1 = nullptr;
	m_Bitmap_SetmealBk2 = nullptr;
	m_Bitmap_TitleLogo = nullptr;
	m_Bitmap_PayLogo = nullptr;
	m_Bitmap_gp = nullptr;
	m_Graphics_mem = nullptr;
	m_Bitmap_QRCode = nullptr;
	m_ClientBrush = new SolidBrush(Color(21, 21, 21));
	m_QRCodeStatus = QR_INITIAL;
	m_PaymentPolling = nullptr;
}

Ui_LevelUpDlg::~Ui_LevelUpDlg()
{
	if (m_MemBitmap) { delete m_MemBitmap; m_MemBitmap = nullptr; }
	if (m_Bitmap_gp)
	{
		delete m_Bitmap_gp;
		m_Bitmap_gp = nullptr;
	}
	if (m_Graphics_mem)
	{
		delete m_Graphics_mem;
		m_Graphics_mem = nullptr;
	}
	if (m_ClientBrush)
	{
		delete m_ClientBrush;
		m_ClientBrush = nullptr;
	}
}

void Ui_LevelUpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, LEVELUPDLG_BTN_CLOSE, m_btn_close);
	DDX_Control(pDX, LEVELUPDLG_STAT_PRICE, m_stat_price);
	DDX_Control(pDX, LEVELUPDLG_STAT_ORIPRICE, m_stat_oriprice);
	DDX_Control(pDX, LEVELUPDLG_BTN_SUPTICKETANDSERVICE, m_btn_supTicketAndService);
}


BEGIN_MESSAGE_MAP(Ui_LevelUpDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(LEVELUPDLG_BTN_CLOSE, &Ui_LevelUpDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(LEVELUPDLG_BTN_SUPTICKETANDSERVICE, &Ui_LevelUpDlg::OnBnClickedBtnSupticketandservice)

	ON_MESSAGE(MSG_LEVELUP_STARTPAYMENTPOLLING, &Ui_LevelUpDlg::StartNewPaymentPolling)
	ON_MESSAGE(MSG_LEVELUP_UPDATEUIMODELDATA, &Ui_LevelUpDlg::UpdateUiModelData)
END_MESSAGE_MAP()


// Ui_LevelUpDlg 消息处理程序


BOOL Ui_LevelUpDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}


void Ui_LevelUpDlg::OnPaint()
{
	using namespace Gdiplus;
	CPaintDC dc(this);
	m_Shadow.Show(m_hWnd);
	if(!m_Bitmap_gp)
		m_Bitmap_gp = new Bitmap(m_windowWidth, m_windowHeight);
	if (!m_Graphics_mem)
		m_Graphics_mem = new Graphics(m_Bitmap_gp);
	m_Graphics_mem->FillRectangle(
		m_ClientBrush,
		0,
		0,
		m_windowWidth,
		m_windowHeight
	);
	m_Graphics_mem->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	// 渲染位图资源
	m_Graphics_mem->DrawImage(m_Bitmap_TitleLogo, m_Rect_TitleLogo);
	m_Graphics_mem->DrawImage(m_Bitmap_PayLogo, m_Rect_PayLogo);

	// 二维码 & “正在加载中.....” 显示
	{
		const INT qrX = static_cast<INT>(47 * m_Scale);
		const INT qrY = static_cast<INT>(382 * m_Scale);
		const INT qrSize = static_cast<INT>(107 * m_Scale);

		// 统一高质量渲染设置
		m_Graphics_mem->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		m_Graphics_mem->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		m_Graphics_mem->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

		if (m_QRCodeStatus == QR_LOADED && m_Bitmap_QRCode)
		{
			//渲染二维码
			Rect destRect(qrX, qrY, qrSize, qrSize);
			m_Graphics_mem->DrawImage(m_Bitmap_QRCode, destRect);
		}
		else if (m_QRCodeStatus == QR_INITIAL || m_QRCodeStatus == QR_LOADING)
		{
			// 未加载完成文本
			StringFormat fmt;
			fmt.SetAlignment(StringAlignmentNear);
			fmt.SetLineAlignment(StringAlignmentNear);
			fmt.SetFormatFlags(StringFormatFlagsNoWrap);
			Gdiplus::Font font(L"微软雅黑", 12.0f * static_cast<REAL>(m_Scale), FontStyleBold, UnitPixel);
			Gdiplus::SolidBrush brush(Color(251, 215, 170)); 
			RectF textRect(
				static_cast<REAL>(qrX),
				static_cast<REAL>(qrY),
				static_cast<REAL>(qrSize),
				static_cast<REAL>(qrSize)
			);
			m_Graphics_mem->DrawString(L"正在加载中.....", -1, &font, textRect, &fmt, &brush);
		}
	}

	// 渲染 SetMeal 卡片
	{
		for (auto& kv : m_Map_SetMeals)
		{
			const int key = kv.first;
			const SetMeal& sm = kv.second;
			const bool isSelected = (key == m_SelectedSetMeal);

			// 背景
			m_Graphics_mem->DrawImage(
				isSelected ? m_Bitmap_SetmealBk1 : m_Bitmap_SetmealBk2,
				sm.int_Left, sm.int_top, sm.int_width, sm.int_height
			);

			// 文本格式
			StringFormat fmtCenter;
			fmtCenter.SetAlignment(StringAlignmentCenter);
			fmtCenter.SetLineAlignment(StringAlignmentCenter);

			//根据是否选中，切换普通画刷与点击态画刷
			auto* brTypeName = isSelected && sm.clickSB_TypeName ? sm.clickSB_TypeName : sm.sb_TypeName;
			auto* brPrice = isSelected && sm.clickSB_Price ? sm.clickSB_Price : sm.sb_Price;
			auto* brOriPrice = isSelected && sm.clickSB_oriPrice ? sm.clickSB_oriPrice : sm.sb_oriPrice;
			auto* brDeviceBinds = isSelected && sm.clickSB_deviceBinds ? sm.clickSB_deviceBinds : sm.sb_deviceBinds;
			auto* brLowToday = isSelected && sm.clickSB_lowToday ? sm.clickSB_lowToday : sm.sb_lowToday;

			// 文本绘制
			if (sm.font_TypeName && brTypeName && !sm.text_TypeName.empty())
				m_Graphics_mem->DrawString(sm.text_TypeName.c_str(), -1, sm.font_TypeName, sm.rect_TypeName,
					&fmtCenter, brTypeName);
			if (sm.font_Price && brPrice && !sm.text_Price.empty())
				m_Graphics_mem->DrawString(sm.text_Price.c_str(), -1, sm.font_Price, sm.rect_Price, 
					&fmtCenter, brPrice);
			if (sm.font_oriPrice && brOriPrice && !sm.text_OriPrice.empty())
				m_Graphics_mem->DrawString(sm.text_OriPrice.c_str(), -1, sm.font_oriPrice, sm.rect_oriPrice,
					&fmtCenter, brOriPrice);
			if (sm.font_deviceBinds && brDeviceBinds && !sm.text_DeviceBinds.empty())
				m_Graphics_mem->DrawString(sm.text_DeviceBinds.c_str(), -1, sm.font_deviceBinds, sm.rect_deviceBinds, 
					&fmtCenter, brDeviceBinds);
			if (sm.font_lowToday && brLowToday && !sm.text_LowToday.empty())
				m_Graphics_mem->DrawString(sm.text_LowToday.c_str(), -1, sm.font_lowToday, sm.rect_lowToday,
					&fmtCenter, brLowToday);
		}
	}

	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(m_Bitmap_gp, 0, 0,
		static_cast<INT>(m_windowWidth), static_cast<INT>(m_windowHeight));
	DB(ConsoleHandle, L"Ui_LevelUpDlg::repaint");
}


void Ui_LevelUpDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	int oldSelected = m_SelectedSetMeal;
	int newSelected = -1;

	// 点击测试
	for (auto& kv : m_Map_SetMeals)
	{
		const SetMeal& sm = kv.second;
		RECT rc = { sm.int_Left, sm.int_top, sm.int_Left + sm.int_width, sm.int_top + sm.int_height };
		if (::PtInRect(&rc, point))
		{
			newSelected = kv.first;
			break;
		}
	}

	// 更新选中状态
	m_SelectedSetMeal = newSelected;
	if (oldSelected != -1)
	{
		const auto itOld = m_Map_SetMeals.find(oldSelected);
		if (itOld != m_Map_SetMeals.end())
		{
			RECT rcOld = { itOld->second.int_Left, itOld->second.int_top,
						   itOld->second.int_Left + itOld->second.int_width,
						   itOld->second.int_top + itOld->second.int_height };
			::InflateRect(&rcOld, 2, 2);
			InvalidateRect(&rcOld, FALSE); 
		}
	}
	if (m_SelectedSetMeal != -1)
	{
		const auto itNew = m_Map_SetMeals.find(m_SelectedSetMeal);
		if (itNew != m_Map_SetMeals.end())
		{
			RECT rcNew = { itNew->second.int_Left, itNew->second.int_top,
						   itNew->second.int_Left + itNew->second.int_width,
						   itNew->second.int_top + itNew->second.int_height };
			::InflateRect(&rcNew, 2, 2);
			InvalidateRect(&rcNew, FALSE); 
		}
	}
	if (m_SelectedSetMeal != -1)
	{
		SetSelectedVipType(m_SelectedSetMeal);
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

double Ui_LevelUpDlg::getUserDpi()
{
	// 获取系统 DPI
	HDC screen = ::GetDC(NULL);
	int dpi = GetDeviceCaps(screen, LOGPIXELSX);
	::ReleaseDC(NULL, screen);
	double scale = static_cast<double>(dpi) / 96.0;
	return scale;
}

void Ui_LevelUpDlg::initCtrl()
{
	m_btn_close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_btn_close.SetBackgroundColor(RGB(26, 27, 32));
	m_btn_close.SetHoverEffectColor(30, 255, 255, 200);
	m_btn_close.SetStretchMode(0.75f);

	m_btn_supTicketAndService.LarSetTextSize(14);
	m_btn_supTicketAndService.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
	m_btn_supTicketAndService.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_btn_supTicketAndService.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_btn_supTicketAndService.LarSetBorderColor(Gdiplus::Color(255, 21, 21, 21));
	m_btn_supTicketAndService.LarSetEraseBkEnable(false);
	m_btn_supTicketAndService.LarSetNormalFiilBrush(SolidBrush(Color(255, 21, 21, 21)));
	m_btn_supTicketAndService.LarSetHoverFillBrush(SolidBrush(Color(255, 21, 21, 21)));
	m_btn_supTicketAndService.LarSetClickedFillBrush(SolidBrush(Color(255, 21, 21, 21)));
	m_btn_supTicketAndService.LarSetTextCenter(false);
	m_btn_supTicketAndService.LarSetBtnTextMultLine(false);

	m_stat_price.LarSetTextColor(RGB(209, 53, 35));
	m_stat_price.LarSetTextSize(34);
	m_stat_price.LarSetTextCenter();
	
	m_stat_oriprice.LarSetStrikeOut(true);
	m_stat_oriprice.LarSetTextColor(RGB(139, 139, 130));
	m_stat_oriprice.LarSetTextSize(14);
	m_stat_oriprice.LarSetTextCenter();
}

void Ui_LevelUpDlg::initSetMeal()
{
	// 初始化三个SetMeal控件，并将其加入到m_Map_SetMeals中
	using namespace Gdiplus;
	m_Map_SetMeals.clear();
	const INT marginX = static_cast<INT>(43 * m_Scale);
	const INT marginY = static_cast<INT>(130 * m_Scale);
	const INT gap = static_cast<INT>(24 * m_Scale);
	const INT cardWidth = 160 * m_Scale;
	const INT cardHeight = 188 * m_Scale;
	const INT paddingX = static_cast<INT>(20 * m_Scale);
	const INT paddingTop = static_cast<INT>(28 * m_Scale);
	const INT lineGap = static_cast<INT>(8 * m_Scale);
	for (INT i = 0; i < 3; ++i)
	{
		SetMeal sm = {};

		// 卡片外框位置与尺寸
		sm.int_Left = marginX + i * (cardWidth + gap);
		sm.int_top = marginY;
		sm.int_width = cardWidth;
		sm.int_height = cardHeight;

		// 字体（像素单位，跟随DPI）
		sm.font_TypeName = new 
			Gdiplus::Font(L"微软雅黑", 16.0f * static_cast<REAL>(m_Scale), FontStyleBold, UnitPixel);
		sm.font_Price = 
			new Gdiplus::Font(L"微软雅黑", 28.0f * static_cast<REAL>(m_Scale), FontStyleBold, UnitPixel);
		sm.font_oriPrice = 
			new Gdiplus::Font(L"微软雅黑", 12.0f * static_cast<REAL>(m_Scale), FontStyleRegular, UnitPixel);
		sm.font_deviceBinds = 
			new Gdiplus::Font(L"微软雅黑", 12.0f * static_cast<REAL>(m_Scale), FontStyleRegular, UnitPixel);
		sm.font_lowToday = 
			new Gdiplus::Font(L"微软雅黑", 14.0f * static_cast<REAL>(m_Scale), FontStyleBold, UnitPixel);

		// 正常文本画刷颜色
		sm.sb_TypeName = new SolidBrush(Color(235, 235, 235));		// 套餐名/标题
		sm.sb_Price = new SolidBrush(Color(219, 57, 43));			// 价格（红色）
		sm.sb_oriPrice = new SolidBrush(Color(145, 145, 145));		// 原价（灰）
		sm.sb_deviceBinds = new SolidBrush(Color(185, 185, 185));	// 几台同时登录（灰）
		sm.sb_lowToday = new SolidBrush(Color(210, 210, 210));		// 底部提示（“低至/永久有效”区域）

		//点击态文本颜色
		sm.clickSB_TypeName = new SolidBrush(Color(255, 255, 255));		// 标题：纯白
		sm.clickSB_Price = new SolidBrush(Color(245, 66, 51));			// 价格：更亮的红
		sm.clickSB_oriPrice = new SolidBrush(Color(200, 200, 200));		// 原价：更亮的灰
		sm.clickSB_deviceBinds = new SolidBrush(Color(230, 230, 230));  // 同时登录：更亮的灰
		sm.clickSB_lowToday = new SolidBrush(Color(252, 220, 170));		// 底部提示：与主题相近的暖色

		INT x = sm.int_Left;                              
		INT y = sm.int_top + paddingTop;
		sm.rect_TypeName = RectF(x, y, sm.int_width, static_cast<INT>(24 * m_Scale)); 
		y += sm.rect_TypeName.Height + lineGap + static_cast<INT>(4 * m_Scale);
		sm.rect_Price = RectF(x, y, sm.int_width, static_cast<INT>(34 * m_Scale));   
		y += sm.rect_Price.Height + lineGap;
		sm.rect_oriPrice = RectF(x, y, sm.int_width, static_cast<INT>(18 * m_Scale)); 
		y += sm.rect_oriPrice.Height + lineGap + static_cast<INT>(4 * m_Scale);
		sm.rect_deviceBinds = RectF(x, y, sm.int_width, static_cast<INT>(20 * m_Scale));

		// 底部条区域（“低至/永久有效”）
		const INT bottomBarH = static_cast<INT>(40 * m_Scale);
		sm.rect_lowToday = RectF(
			sm.int_Left, 
			sm.int_top + cardHeight - bottomBarH + 5 * m_Scale, 
			cardWidth, 
			bottomBarH
		);

		m_Map_SetMeals[i] = sm;
	}
}

void Ui_LevelUpDlg::UpdateScale()
{
	m_Rect_TitleLogo.Width = 313 * m_Scale;
	m_Rect_TitleLogo.Height = 47 * m_Scale;
	m_Rect_TitleLogo.X = 145 * m_Scale;
	m_Rect_TitleLogo.Y = 44 * m_Scale;

	m_Rect_PayLogo.Width = 236 * m_Scale;
	m_Rect_PayLogo.Height = 204 * m_Scale;
	m_Rect_PayLogo.X = 40 * m_Scale;
	m_Rect_PayLogo.Y = 346 * m_Scale;

	{
		int w = 28 * m_Scale;
		int h = 28 * m_Scale;
		int x = 571 * m_Scale;
		int y = 22 * m_Scale;
		m_btn_close.MoveWindow(x, y, w, h);
	}

	{
		int w = 115 * m_Scale;
		int h = 46 * m_Scale;
		int x = 228 * m_Scale;
		int y = 395 * m_Scale;
		m_stat_price.MoveWindow(x, y, w, h);
	}

	{
		int w = 65 * m_Scale;
		int h = 19 * m_Scale;
		int x = 335 * m_Scale;
		int y = 408 * m_Scale;
		m_stat_oriprice.MoveWindow(x, y, w, h);
	}

	{
		int w = 123 * m_Scale;
		int h = 15 * m_Scale;
		int x = 486 * m_Scale;
		int y = 531 * m_Scale;
		m_btn_supTicketAndService.MoveWindow(x, y, w, h);
	}
}

void Ui_LevelUpDlg::LoadRes()
{
	m_Bitmap_SetmealBk1 = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(LEVELUPDLG_PNG_SETMEALBK1),
		L"PNG");
	m_Bitmap_SetmealBk2 = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(LEVELUPDLG_PNG_SETMEALBK2),
		L"PNG");
	m_Bitmap_TitleLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(LEVELUPDLG_PNG_TITLELOGO),
		L"PNG");
	m_Bitmap_PayLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(LEVELUPDLG_PNG_PAYLOGO),
		L"PNG");
}

bool Ui_LevelUpDlg::getPriceInfo()
{
	bool success = false;

	// 清空现有数据
	m_PriceInfos.clear();
	m_PreOrderNo.clear();

	CURL* curl = curl_easy_init();
	if (!curl) 
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"CURL 初始化失败");
		return false;
	}
	std::string responseBuffer;

	// 设置请求URL和选项
	curl_easy_setopt(curl, CURLOPT_URL, "http://scrnrec.appapr.com/api/pricing");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Ui_LevelUpDlg_PriceInfoWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

	// 设置请求头
	struct curl_slist* headers = NULL;
	std::string tokenHeader;
#if RELEASE_CODE == 1
	tokenHeader = "x-api-token:" + App.m_appToken;
#else
	tokenHeader = "x-api-token: oKqdT4g_7O8yLxKtdUeOsoMr85qE22LfzKlotOfAydbcUIVnNklYe-XYmqvzkRORJxlvpHqVor0TEtB6ErJJbWQhMBGdVwCQFg-jxMQR_eY";
#endif
	headers = curl_slist_append(headers, tokenHeader.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	// 执行请求
	DBFMT(ConsoleHandle, L"url:http://scrnrec.appapr.com/api/pricing\n data:%s",
		LARSC::s2ws(tokenHeader).c_str());
	CURLcode res = curl_easy_perform(curl);

	// 检查是否成功
	if (res != CURLE_OK)
	{
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"GET请求失败: %hs", curl_easy_strerror(res));
		ModalDlg_MFC::ShowModal_NetWorkError();
		EndDialog(IDCANCEL);
	}
	else
	{
		// 使用UTF-8转换处理响应数据
		std::wstring wideResponse = LARSC::Utf8ToWideString(responseBuffer);
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"接收到的响应: %s", wideResponse.c_str());

		// 解析JSON响应
		Json::Reader reader;
		Json::Value root;

		if (reader.parse(responseBuffer, root))
		{
			// 检查请求是否成功
			bool isSuccess = root.get("success", false).asBool();

			if (isSuccess)
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"价格信息请求成功");
				// 先解析额外字段
				if (root["data"].isMember("coupon_price") && !root["data"]["coupon_price"].isNull())
				{
					m_CouponPrice.id =
						root["data"]["coupon_price"]["id"].isNull() ?
						-1 : root["data"]["coupon_price"]["id"].asUInt();
					m_CouponPrice.name =
						root["data"]["coupon_price"]["name"].isNull() ?
						L"" : LARSC::Utf8ToWideString(root["data"]["coupon_price"]["name"].asString());
					m_CouponPrice.badge =
						root["data"]["coupon_price"]["badge"].isNull() ?
						L"" : LARSC::Utf8ToWideString(root["data"]["coupon_price"]["badge"].asString());
					m_CouponPrice.amount =
						root["data"]["coupon_price"]["amount"].isNull() ?
						0 : root["data"]["coupon_price"]["amount"].asUInt();
					m_CouponPrice.desc =
						root["data"]["coupon_price"]["desc"].isNull() ?
						0 : LARSC::Utf8ToWideString(root["data"]["coupon_price"]["desc"].asString());
				}

				// 解析预订单号
				if (root["data"].isMember("pre_order_no") && !root["data"]["pre_order_no"].isNull())
				{
					m_PreOrderNo = root["data"]["pre_order_no"].asString();
				}

				// 解析价格信息
				if (root["data"].isMember("prices") && root["data"]["prices"].isArray())
				{
					Json::Value prices = root["data"]["prices"];
					for (Json::Value::ArrayIndex i = 0; i < prices.size(); i++)
					{
						PriceInfo info;

						// 获取ID
						info.id = prices[i].get("id", 0).asInt();

						// 获取名称 - 处理中文编码
						if (prices[i].isMember("name") && !prices[i]["name"].isNull())
						{
							std::string nameStr = GlobalFunc::Utf8ToAnsi(prices[i]["name"].asString());
							std::wstring wideName = LARSC::s2ws(nameStr);
							info.name = wideName.c_str();
						}

						// 获取标签 (可能为null)
						if (prices[i].isMember("badge") && !prices[i]["badge"].isNull())
						{
							std::string badgeStr = prices[i]["badge"].asString();
							std::wstring wideBadge = LARSC::s2ws(badgeStr);
							info.badge = wideBadge.c_str();
						}

						// 获取价格
						info.amount = prices[i].get("amount", 0.0).asDouble();

						// 获取描述
						if (prices[i].isMember("desc") && !prices[i]["desc"].isNull())
						{
							std::string descStr = prices[i]["desc"].asString();
							std::wstring wideDesc = LARSC::Utf8ToWideString(descStr);
							info.desc = wideDesc.c_str();
						}

						// 添加到数组
						m_PriceInfos.push_back(info);

						DEBUG_CONSOLE_FMT(ConsoleHandle, L"解析到价格信息: ID=%d, 名称=%s, 价格=%.2f, 描述=%s",
							info.id, (LPCTSTR)info.name, info.amount, (LPCTSTR)info.GetFormattedDesc());
					}

					success = true;
				}
				else
				{
					DEBUG_CONSOLE_STR(ConsoleHandle, L"未找到有效的价格数组");
				}

				//解析优惠券数额
				if (root["data"].isMember("coupon_amount") && !root["data"]["coupon_amount"].isNull())
				{
					m_int_Coupon = root["data"]["coupon_amount"].asInt();
				}
				else
				{
					m_int_Coupon = 0;
					DB(ConsoleHandle, L"未解析到Coupon，可能为空或者非法字符");
				}
			}
			else
			{
				std::string message = root.get("message", "unknown error").asString();
				std::wstring wideMessage = LARSC::s2ws(message);
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"价格信息请求失败: %s", wideMessage.c_str());
				Ui_MessageModalDlg MessageDlg;
				MessageDlg.SetModal(L"极速录屏大师", L"Ops！出错了！", L"价格信息请求失败!", L"确认");
				MessageDlg.DoModal();
				EndDialog(IDCANCEL);
			}
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"JSON解析失败");
		}
	}

	// 清理资源
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	return success;
}

bool Ui_LevelUpDlg::loadQRCode()
{
	m_QRCodeStatus = QR_LOADING;
	Invalidate(FALSE);

	// 清除旧的二维码图像
	if (m_Bitmap_QRCode)
	{
		delete m_Bitmap_QRCode;
		m_Bitmap_QRCode = nullptr;
	}

	// 确保价格信息和预订单号
	if (m_PriceInfos.empty() || m_PreOrderNo.empty())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"加载二维码失败：没有价格信息或预订单号");
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 获取当前选中套餐类型
	int selectedPriceId = 0;
	for (const auto& price : m_PriceInfos)
	{
		if ((m_SelectedSetMeal == 0) ||
			(m_SelectedSetMeal == 1) ||
			(m_SelectedSetMeal == 2))
		{
			selectedPriceId = price.id;
			break;
		}
	}

	if (selectedPriceId == 0)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"加载二维码失败：无法确定选中的价格ID");
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 构建二维码URL
	std::string tokenValue;
#if RELEASE_CODE == 1
	tokenValue = App.m_appToken;
#else
	tokenValue = "oKqdT4g_7O8yLxKtdUeOsoMr85qE22LfzKlotOfAydbcUIVnNklYe-XYmqvzkRORJxlvpHqVor0TEtB6ErJJbWQhMBGdVwCQFg-jxMQR_eY";
#endif

	// 构建URL
	std::string url;
	url = "http://scrnrec.appapr.com/pay/qrcode?token=" + tokenValue +
		"&price_id=" + std::to_string(m_PriceInfos[m_SelectedSetMeal].id) +
		"&pre_order_no=" + m_PreOrderNo;
	DEBUG_CONSOLE_FMT(ConsoleHandle, L"请求二维码URL: %hs", url.c_str());

	// 使用CURL获取二维码图像
	CURL* curl = curl_easy_init();
	if (!curl)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"CURL初始化失败");
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 存储二维码图像数据
	std::vector<unsigned char> imageBuffer;

	// 设置CURL选项
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Ui_LevelUpDlg_QRCodeWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

	// 执行请求
	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
	{
		DEBUG_CONSOLE_FMT(ConsoleHandle, L"下载二维码失败: %hs", curl_easy_strerror(res));
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 检查是否获取到图像数据
	if (imageBuffer.empty())
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"二维码数据为空");
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 创建内存流并用它加载位图
	IStream* stream = nullptr;
	HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
	if (FAILED(hr) || !stream)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"创建内存流失败");
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 写入图像数据到流
	ULONG written;
	hr = stream->Write(imageBuffer.data(), static_cast<ULONG>(imageBuffer.size()), &written);
	if (FAILED(hr))
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"写入图像数据到流失败");
		stream->Release();
		m_QRCodeStatus = QR_FAILED;
		Invalidate(FALSE);
		return false;
	}

	// 从流中加载位图
	m_Bitmap_QRCode = Gdiplus::Bitmap::FromStream(stream);
	stream->Release();

	if (!m_Bitmap_QRCode || m_Bitmap_QRCode->GetLastStatus() != Gdiplus::Ok)
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"从数据流创建位图失败");
		delete m_Bitmap_QRCode;
		m_Bitmap_QRCode = nullptr;
		m_QRCodeStatus = QR_FAILED;
	}
	else
	{
		DEBUG_CONSOLE_STR(ConsoleHandle, L"二维码加载成功");
		m_QRCodeStatus = QR_LOADED;
	}
	Invalidate(FALSE);
}

void Ui_LevelUpDlg::RequestDatas()
{
	std::thread([this]()
		{
			if (!getPriceInfo())
			{
				DB(ConsoleHandle, L"Ui_LevelUpDlg::RequestDatas: failed,获取价格失败");
				return;
			}
			if (!loadQRCode())
			{
				DB(ConsoleHandle, L"Ui_LevelUpDlg::RequestDatas: failed,获取或加载二维码失败");
				return;
			}
			PostMessage(MSG_LEVELUP_STARTPAYMENTPOLLING);	// 发送消息，启动支付轮询
			PostMessage(MSG_LEVELUP_UPDATEUIMODELDATA);		// 发送消息，更新界面价格显示
		}).detach();
}

void Ui_LevelUpDlg::SetSelectedVipType(int type)
{
	if (type < 0 || type >= static_cast<int>(m_PriceInfos.size()))
		return;
	m_SelectedSetMeal = type;
	const double amount = m_PriceInfos[type].amount;

	//更新价格
	CString priceText;
	priceText.Format(L"¥%.2f", amount);
	m_stat_price.SetWindowTextW(priceText);

	//更新原价
	m_stat_oriprice.SetWindowTextW(m_Map_SetMeals[m_SelectedSetMeal].text_OriPrice.c_str());
	Invalidate(FALSE);
}

LRESULT Ui_LevelUpDlg::StartNewPaymentPolling(WPARAM wParam, LPARAM lParam)
{
	//创建一个新的轮询
	if (m_PaymentPolling)
	{
		m_PaymentPolling->stopPolling();
		delete m_PaymentPolling;
		m_PaymentPolling = nullptr;
	}
	m_PaymentPolling = new LarPaymentPolling;
	m_PaymentPolling->startPolling(
		m_PreOrderNo,
		[this]()
		{
			EndDialog(IDOK);
		},
		[this]()
		{
			if (m_Bitmap_QRCode)
				delete m_Bitmap_QRCode;
			m_Bitmap_QRCode = nullptr;
			m_QRCodeStatus = QR_TIMEOUT;
			Invalidate(FALSE);
		}
	);
	return LRESULT();
}

LRESULT Ui_LevelUpDlg::UpdateUiModelData(WPARAM wParam, LPARAM lParam)
{
	const int count = static_cast<int>(m_PriceInfos.size());
	const int n = min(3, count);
	for (int i = 0; i < n; ++i)
	{
		auto it = m_Map_SetMeals.find(i);
		if (it == m_Map_SetMeals.end()) continue;
		SetMeal& sm = it->second;

		// 名称
		sm.text_TypeName = std::wstring((LPCTSTR)m_PriceInfos[i].name);

		// 现价
		{
			CString s; s.Format(L"¥%.2f", m_PriceInfos[i].amount);
			sm.text_Price = std::wstring(s.GetString());
		}

		// 设备绑定数
		{
			CString desc = m_PriceInfos[i].GetFormattedDesc();
			sm.text_DeviceBinds = std::wstring(desc.GetString());
		}
	}

	//固定值
	for (int i = 0; i < n; i++)
	{
		auto it = m_Map_SetMeals.find(i);
		if (it == m_Map_SetMeals.end()) continue;
		SetMeal& sm = it->second;

		switch (i)
		{
		case 0:
			sm.text_LowToday = L"永久有效";
			sm.text_OriPrice = L"原价：598";
			break;
		case 1:
			sm.text_LowToday = L"永久有效";
			sm.text_OriPrice = L"原价：388";
			break;
		case 2:
			sm.text_LowToday = L"永久有效";
			sm.text_OriPrice = L"原价：268";
			break;
		}

	}
	SetSelectedVipType(m_SelectedSetMeal);
	Invalidate(FALSE);
	return LRESULT();
}

BOOL Ui_LevelUpDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	LoadRes();
	UpdateScale();
	initCtrl();
	initSetMeal();
	RequestDatas();

	::MoveWindow(m_hWnd, m_x, m_y, m_windowWidth, m_windowHeight, FALSE);
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	m_Shadow.Create(m_hWnd);
	return TRUE;
}

void Ui_LevelUpDlg::OnBnClickedBtnClose()
{
	EndDialog(IDCANCEL);
}

void Ui_LevelUpDlg::OnBnClickedBtnSupticketandservice()
{
	
}
