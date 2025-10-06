// UI_MouthMemberDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "theApp.h"
#include "WndShadow.h"
#include "Ui_LoginDlg.h"
#include "Ui_VipPayDlg.h"
#include "UI_MouthMemberDlg.h"
#include "LarPaymentPolling.h"
#define TIMER_MOUTHMEMBER_COUNTDOWN  6001
IMPLEMENT_DYNAMIC(UI_MouthMemberDlg, CDialogEx)
std::chrono::system_clock::time_point UI_MouthMemberDlg::s_EndTime;
bool UI_MouthMemberDlg::s_EndTimeInitialized = false;
size_t UI_SeasonMemberDlg_QRCodeWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	std::vector<unsigned char>* mem = (std::vector<unsigned char>*)userp;
	size_t oldSize = mem->size();
	mem->resize(oldSize + realsize);
	memcpy(mem->data() + oldSize, contents, realsize);
	return realsize;
}

UI_MouthMemberDlg::UI_MouthMemberDlg(CWnd* pParent)
	: CDialogEx(IDD_DIALOG_SEASONMEMBER, pParent)
{
	m_Scale = getUserDpi();
	m_Bitmap_QRCode = nullptr;
	m_WindowHeight = 438 * m_Scale;
	m_WindowWidth = 476 * m_Scale;
	m_x = 0;
	m_y = 0;
	m_QRCodeStatus = QR_INITIAL;
	m_PaymentPolling = nullptr;
	m_Bitmap_QRCodeLoginBk = nullptr;
	m_Dlg_Login = nullptr;
	m_Rect_QRcode = Gdiplus::RectF();
}

UI_MouthMemberDlg::~UI_MouthMemberDlg()
{
	if (m_PaymentPolling)
	{
		m_PaymentPolling->stopPolling();
		delete m_PaymentPolling;
		m_PaymentPolling = nullptr;
	}
	//释放倒计时字体画刷
	if (m_CountDown.font)
	{ 
		delete m_CountDown.font; 
		m_CountDown.font = nullptr; 
	}
	if (m_CountDown.soildBrush)
	{ 
		delete m_CountDown.soildBrush; 
		m_CountDown.soildBrush = nullptr; 
	}
}

void UI_MouthMemberDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, SEASONMEMBERDLG_BTN_CLOSE, m_btn_close);
	DDX_Control(pDX, SEASONMEMBERDLG_STAT_PRICE, m_stat_price);
	DDX_Control(pDX, SEASONMEMBERDLG_BTN_SUPTICKETANDSERVICE, m_btn_supTicketAndService);
	DDX_Control(pDX, SEASONDLG_DLG_RMBLOGO, m_stat_rmblogo);
}

BEGIN_MESSAGE_MAP(UI_MouthMemberDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()

	ON_BN_CLICKED(SEASONMEMBERDLG_BTN_CLOSE, &UI_MouthMemberDlg::OnBnClickedBtnClose)
	ON_BN_CLICKED(SEASONMEMBERDLG_BTN_SUPTICKETANDSERVICE, &UI_MouthMemberDlg::OnBnClickedBtnSupticketandservice)

	ON_MESSAGE(MSG_SEASONMEMBER_STARTPAYMENTPOLLING,&UI_MouthMemberDlg::StartNewPaymentPolling)
	ON_MESSAGE(MSG_MOUTHMEMBER_ONPAYMENTSUCCESS, &UI_MouthMemberDlg::OnPaymentSuccess)
	ON_MESSAGE(MSG_VIPDLG_LOGINSHOW, &UI_MouthMemberDlg::OnLoginShow)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

BOOL UI_MouthMemberDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void UI_MouthMemberDlg::OnPaint()
{
	using namespace Gdiplus;
	CPaintDC dc(this);
	m_Shadow.Show(m_hWnd);
	Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
	Graphics memGraphics(&memBitmap);
	SolidBrush ClientBrush(Color(31, 36, 37));
	memGraphics.FillRectangle(
		&ClientBrush,
		0,
		0,
		m_WindowWidth,
		m_WindowHeight
	);
	// 渲染贴图
	memGraphics.DrawImage(m_Bitmap_WinPriceLogo, m_Rect_WinPriceLogo);
	memGraphics.DrawImage(m_Bitmap_PayLogo, m_Rect_PayLogo);
	memGraphics.DrawImage(m_Bitmap_TitleLogo, m_Rect_TitleLogo);
	{
		// 与倒计时统一的文本渲染优化设置
		memGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
		memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		memGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

		if (App.m_isLoginIn || App.m_CanGuestBuy)
		{
			// 二维码已加载成功，绘制二维码
			if (m_QRCodeStatus == QR_LOADED && m_Bitmap_QRCode)
			{
				memGraphics.DrawImage(m_Bitmap_QRCode, m_Rect_QRcode);
			}
			// 二维码未加载完成，在指定位置显示“正在加载中.....”
			else if (m_QRCodeStatus == QR_INITIAL || m_QRCodeStatus == QR_LOADING)
			{
				StringFormat fmt;
				fmt.SetAlignment(StringAlignmentCenter);
				fmt.SetLineAlignment(StringAlignmentCenter);
				fmt.SetFormatFlags(StringFormatFlagsNoWrap);
				Gdiplus::Font* font = new Gdiplus::Font(L"微软雅黑", 7 * m_Scale);
				Gdiplus::SolidBrush* brush = new Gdiplus::SolidBrush(Color(0, 0, 0));
				if (font && brush)
				{
					memGraphics.DrawString(L"正在加载中", -1, font, m_Rect_QRcode, &fmt, brush);
				}
			}
		}
		else
		{
			memGraphics.DrawImage(m_Bitmap_QRCodeLoginBk, m_Rect_QRcode);
		}
	}

	if (m_CountDown.font && m_CountDown.soildBrush)
	{
		// 文本渲染优化为 ClearType
		memGraphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
		memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		memGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
		//memGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);

		// 居中对齐，不换行
		StringFormat fmt;
		fmt.SetAlignment(StringAlignmentCenter);
		fmt.SetLineAlignment(StringAlignmentCenter);
		fmt.SetFormatFlags(StringFormatFlagsNoWrap);

		// 工具函数：从整型 Rect 转为 RectF
		auto ToRectF = [](const Gdiplus::Rect& r) -> Gdiplus::RectF {
			return Gdiplus::RectF(
				static_cast<REAL>(r.X),
				static_cast<REAL>(r.Y),
				static_cast<REAL>(r.Width),
				static_cast<REAL>(r.Height)
			);
			};

		// 绘制“时”
		memGraphics.DrawString(
			m_CountDown.str_hours.c_str(),
			-1,
			m_CountDown.font,
			ToRectF(m_CountDown.rect_hours),
			&fmt,
			m_CountDown.soildBrush
		);

		// 绘制“分”
		memGraphics.DrawString(
			m_CountDown.str_minutes.c_str(),
			-1,
			m_CountDown.font,
			ToRectF(m_CountDown.rect_minutes),
			&fmt,
			m_CountDown.soildBrush
		);

		// 绘制“秒”
		memGraphics.DrawString(
			m_CountDown.str_seconds.c_str(),
			-1,
			m_CountDown.font,
			ToRectF(m_CountDown.rect_seconds),
			&fmt,
			m_CountDown.soildBrush
		);
	}


	//一次性绘画到窗口上
	Graphics graphice(dc.GetSafeHdc());
	graphice.DrawImage(&memBitmap, 0, 0,
		static_cast<INT>(m_WindowWidth), static_cast<INT>(m_WindowHeight));
	DB(ConsoleHandle, L"UI_SeasonMemberDlg::repaint");
}

double UI_MouthMemberDlg::getUserDpi()
{
	HDC screen = ::GetDC(NULL);
	if (screen == NULL)
	{
		AfxMessageBox(L"无法获取屏幕 DC。");
		DB(ConsoleHandle, L"can't not obtain screen dc");
		throw std::runtime_error("can't not obtain screen dc");
	}
	int dpi = GetDeviceCaps(screen, LOGPIXELSX);
	::ReleaseDC(NULL, screen);
	double scale = static_cast<double>(dpi) / 96.0;
	return scale;
}

void UI_MouthMemberDlg::UpdateScale()
{
	{
		int w = 22 * m_Scale;
		int h = 22 * m_Scale;
		int x = 445 * m_Scale;
		int y = 16 * m_Scale;
		m_btn_close.MoveWindow(x, y, w, h);
	}

	m_Rect_PayLogo.Width = 271 * m_Scale;
	m_Rect_PayLogo.Height = 91 * m_Scale;
	m_Rect_PayLogo.X = 125 * m_Scale;
	m_Rect_PayLogo.Y = 289 * m_Scale;

	m_Rect_TitleLogo.Width = 322 * m_Scale;
	m_Rect_TitleLogo.Height = 55 * m_Scale;
	m_Rect_TitleLogo.X = 85 * m_Scale;
	m_Rect_TitleLogo.Y = 42 * m_Scale;

	m_Rect_WinPriceLogo.Width = 402 * m_Scale;
	m_Rect_WinPriceLogo.Height = 107 * m_Scale;
	m_Rect_WinPriceLogo.X = 43 * m_Scale;
	m_Rect_WinPriceLogo.Y = 133 * m_Scale;

	m_Rect_QRcode.Width = 81 * m_Scale;
	m_Rect_QRcode.Height = 81 * m_Scale;
	m_Rect_QRcode.X = 130 * m_Scale;
	m_Rect_QRcode.Y = 291 * m_Scale;

	//联系客服/开具发票
	{
		int w = 140 * m_Scale;
		int h = 23 * m_Scale;
		int x = 224 * m_Scale;
		int y = 293 * m_Scale;
		m_btn_supTicketAndService.MoveWindow(x, y, w, h);
	}

	{
		int w = 104 * m_Scale;
		int h = 48 * m_Scale;
		int x = 84 * m_Scale;
		int y = 166 * m_Scale;
		m_stat_price.MoveWindow(x, y, w, h);
	}

	{
		int w = 24 * m_Scale;
		int h = 24 * m_Scale;
		int x = 80 * m_Scale;
		int y = 178 * m_Scale;
		m_stat_rmblogo.MoveWindow(x, y, w, h);
	}
}

void UI_MouthMemberDlg::InitCtrl()
{
	m_btn_close.LoadPNG(MAINDLG_PNG_CLOSE);
	m_btn_close.SetBackgroundColor(RGB(31, 36, 37));
	m_btn_close.SetHoverEffectColor(30, 255, 255, 200);
	m_btn_close.SetStretchMode(0.75f);

	m_stat_price.LarSetTextStyle(true, false, false);
	m_stat_price.LarSetTextColor(RGB(209, 53, 35));
	m_stat_price.LarSetTextSize(36);
	m_stat_price.LarSetTextCenter();
	std::wstring mouthBillPriceStr = std::to_wstring(App.m_CouponPrice.amount);
	m_stat_price.SetWindowTextW(mouthBillPriceStr.c_str());

	m_stat_rmblogo.LarSetTextColor(RGB(209, 53, 35));
	m_stat_rmblogo.LarSetTextSize(20);
	m_stat_rmblogo.LarSetTextCenter();

	m_btn_supTicketAndService.LarSetTextSize(16);
	m_btn_supTicketAndService.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
	m_btn_supTicketAndService.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
	m_btn_supTicketAndService.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
	m_btn_supTicketAndService.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
	m_btn_supTicketAndService.LarSetEraseBkEnable(false);
	m_btn_supTicketAndService.LarSetNormalFiilBrush(SolidBrush(Color(255, 31, 36, 37)));
	m_btn_supTicketAndService.LarSetHoverFillBrush(SolidBrush(Color(255, 31, 36, 37)));
	m_btn_supTicketAndService.LarSetClickedFillBrush(SolidBrush(Color(255, 31, 36, 37)));
	m_btn_supTicketAndService.LarSetTextCenter(false);
	m_btn_supTicketAndService.LarSetBtnTextMultLine(false);
}

void UI_MouthMemberDlg::InitCountDown()
{
	if (m_CountDown.font) { delete m_CountDown.font; m_CountDown.font = nullptr; }
	if (m_CountDown.soildBrush) { delete m_CountDown.soildBrush; m_CountDown.soildBrush = nullptr; }
	m_CountDown.font = new Gdiplus::Font(L"微软雅黑", 9 * m_Scale, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	m_CountDown.soildBrush = new Gdiplus::SolidBrush(Color(251, 215, 170));
	m_CountDown.rect_hours.X = 299 * m_Scale;
	m_CountDown.rect_hours.Y = 208 * m_Scale;
	m_CountDown.rect_hours.Width = 20 * m_Scale;
	m_CountDown.rect_hours.Height = 12 * m_Scale;

	m_CountDown.rect_minutes.Width = 20 * m_Scale;
	m_CountDown.rect_minutes.Height = 12 * m_Scale;
	m_CountDown.rect_minutes.X = 344 * m_Scale;
	m_CountDown.rect_minutes.Y = m_CountDown.rect_hours.Y;

	m_CountDown.rect_seconds.Width = 20 * m_Scale;
	m_CountDown.rect_seconds.Height = 12 * m_Scale;
	m_CountDown.rect_seconds.X = 388 * m_Scale;
	m_CountDown.rect_seconds.Y = m_CountDown.rect_hours.Y;

	//若首次进入，初始化结束时间 = 当前 + 22 小时；否则保持之前的继续走
	if (!UI_MouthMemberDlg::s_EndTimeInitialized)
	{
		UI_MouthMemberDlg::s_EndTime = std::chrono::system_clock::now() + std::chrono::hours(22);
		UI_MouthMemberDlg::s_EndTimeInitialized = true;
	}
	m_CountDown.str_hours = L"--";
	m_CountDown.str_minutes = L"--";
	m_CountDown.str_seconds = L"--";
}

void UI_MouthMemberDlg::loadRes()
{
	m_Bitmap_WinPriceLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(SEASONMEMBERDLG_PNG_WINPRICELOGO),
		L"PNG"
	);
	m_Bitmap_TitleLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(SEASONMEMBERDLG_PNG_TITLELOGO),
		L"PNG"
	);
	m_Bitmap_PayLogo = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(SEASONMEMBERDLG_PNG_PAYLOGO),
		L"PNG"
	);
	m_Bitmap_QRCodeLoginBk = LARPNG::LoadPngFromResource(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(VIPDLG_PNG_LOGINQRCODEBK),
		L"PNG"
	);
}

void UI_MouthMemberDlg::SetWindowXY(int x, int y)
{
	m_x = x;
	m_y = y;
}

void UI_MouthMemberDlg::LoadQRCodeThread()
{
	//进入加载状态并立即刷新界面
	m_QRCodeStatus = QR_LOADING;
	Invalidate(FALSE);
	std::thread([this]()
		{
			App.RequestPrice();

			// 清除旧的二维码图像
			if (m_Bitmap_QRCode)
			{
				delete m_Bitmap_QRCode;
				m_Bitmap_QRCode = nullptr;
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
				"&price_id=" + std::to_string(App.m_CouponPrice.id) +
				"&pre_order_no=" + App.m_preOrderNo;
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"请求二维码URL: %hs", url.c_str());

			// 使用CURL获取二维码图像
			CURL* curl = curl_easy_init();
			if (!curl)
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"CURL初始化失败");
				m_QRCodeStatus = QR_FAILED;
				Invalidate(FALSE);
				return;
			}

			// 存储二维码图像数据
			std::vector<unsigned char> imageBuffer;

			// 设置CURL选项
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UI_SeasonMemberDlg_QRCodeWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
			CURLcode res = curl_easy_perform(curl);// 执行请求
			curl_easy_cleanup(curl);
			if (res != CURLE_OK)
			{
				DEBUG_CONSOLE_FMT(ConsoleHandle, L"下载二维码失败: %hs", curl_easy_strerror(res));
				m_QRCodeStatus = QR_FAILED;
				Invalidate(FALSE);
				return;
			}

			// 检查是否获取到图像数据
			if (imageBuffer.empty())
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"二维码数据为空");
				m_QRCodeStatus = QR_FAILED;
				Invalidate(FALSE);
				return;
			}

			// 创建内存流并用它加载位图
			IStream* stream = nullptr;
			HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
			if (FAILED(hr) || !stream)
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"创建内存流失败");
				m_QRCodeStatus = QR_FAILED;
				Invalidate(FALSE);
				return;
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
				return;
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
				Invalidate(FALSE);
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"二维码加载成功");
				m_QRCodeStatus = QR_LOADED;
				Invalidate(FALSE);
				PostMessage(MSG_SEASONMEMBER_STARTPAYMENTPOLLING);
			}
		}).detach();
}

void UI_MouthMemberDlg::OnCancel()
{
	//CDialogEx::OnCancel();
}

void UI_MouthMemberDlg::OnOK()
{
	//CDialogEx::OnOK();
}

BOOL UI_MouthMemberDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	loadRes();
	UpdateScale();
	InitCtrl();
	InitCountDown();
	LoadQRCodeThread();

	SetTimer(TIMER_MOUTHMEMBER_COUNTDOWN, 1000, NULL);
	OnTimer(TIMER_MOUTHMEMBER_COUNTDOWN);

	::MoveWindow(m_hWnd, m_x, m_y, m_WindowWidth, m_WindowHeight, FALSE);
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	//m_Shadow.Create(m_hWnd);
	return TRUE;  
}

void UI_MouthMemberDlg::OnBnClickedBtnClose()
{
	EndDialog(IDCANCEL);
}

void UI_MouthMemberDlg::OnBnClickedBtnSupticketandservice()
{
	App.OpenFeedBackLink(
		this->GetSafeHwnd(),
		L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
	);
}

LRESULT UI_MouthMemberDlg::StartNewPaymentPolling(WPARAM wParam, LPARAM lParam)
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
		App.m_preOrderNo,
		[this]()
		{
			::PostMessage(m_hWnd, MSG_MOUTHMEMBER_ONPAYMENTSUCCESS, NULL, NULL);
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

LRESULT UI_MouthMemberDlg::OnPaymentSuccess(WPARAM wParam, LPARAM lParam)
{
	this->EndDialog(IDOK);
	m_PaymentPolling->stopPolling();
	return 0;
}

LRESULT UI_MouthMemberDlg::OnLoginShow(WPARAM wParam, LPARAM lParam)
{
	if (!App.m_isLoginIn)
	{
		m_Shadow.SetDarkness(0);
		// 确保释放之前的对话框（如果有）
		if (m_Dlg_Login != nullptr)
		{
			DB(ConsoleHandle, L"重新创建登录对话框");
			m_Dlg_Login->DestroyWindow();
			delete m_Dlg_Login;
			m_Dlg_Login = nullptr;
		}

		// 创建新的对话框实例 525 676
		m_Dlg_Login = new Ui_LoginDlg(this);
		int loginWindowWidth = 425 * m_Scale;
		int loginWindowHeight = 397 * m_Scale;

		// 设置位置
		CRect WindowRect, loginWindowRect;
		GetWindowRect(WindowRect);
		loginWindowRect.left = WindowRect.left + (WindowRect.Width() - loginWindowWidth) / 2;
		loginWindowRect.top = WindowRect.top + (WindowRect.Height() - loginWindowHeight) / 2;
		loginWindowRect.right = loginWindowRect.left + loginWindowWidth;
		loginWindowRect.bottom = loginWindowRect.top + loginWindowHeight;
		m_Dlg_Login->Ui_SetWindowRect(loginWindowRect);

		// 以模态方式显示对话框
		DEBUG_CONSOLE_STR(ConsoleHandle, L"显示登录对话框");
		this->SetForegroundWindow();
		if (m_Dlg_Login->DoModal() == IDOK)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"登录成功，开始获取用户信息");
			if (App.RequestDeviceInfo())
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取成功");
			}
			else
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"用户信息获取失败");
			}
			App.m_Dlg_Main.ShowLoginInUi();
			OnInitDialog();
			Invalidate(false);
		}
		else
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"用户取消了登录");
		}
		m_Shadow.SetDarkness();
	}
	else
	{
		App.m_Dlg_Main.OnBnClickedBtnUserProfile();
	}
	m_Dlg_Login = nullptr;
	return LRESULT();
}


void UI_MouthMemberDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_MOUTHMEMBER_COUNTDOWN)
	{
		using namespace std::chrono;
		auto now = system_clock::now();
		long long leftSec = duration_cast<seconds>(UI_MouthMemberDlg::s_EndTime - now).count();
		if (leftSec < 0) leftSec = 0;

		int hours = static_cast<int>(leftSec / 3600);
		int minutes = static_cast<int>((leftSec % 3600) / 60);
		int seconds = static_cast<int>(leftSec % 60);

		// 格式化成两位
		auto fmt2 = [](int v)->std::wstring {
			wchar_t b[4] = { 0 };
			_snwprintf_s(b, _TRUNCATE, L"%02d", v);
			return b;
			};

		std::wstring newH = fmt2(hours);
		std::wstring newM = fmt2(minutes);
		std::wstring newS = fmt2(seconds);

		bool changed = (newH != m_CountDown.str_hours) ||
			(newM != m_CountDown.str_minutes) ||
			(newS != m_CountDown.str_seconds);

		m_CountDown.str_hours = newH;
		m_CountDown.str_minutes = newM;
		m_CountDown.str_seconds = newS;

		// 到 0 后停止计时
		if (leftSec == 0)
		{
			KillTimer(TIMER_MOUTHMEMBER_COUNTDOWN);
		}

		if (changed)
		{
			// 仅重绘倒计时相关区域
			int left = m_CountDown.rect_hours.X;
			int top = m_CountDown.rect_hours.Y;
			int right = m_CountDown.rect_hours.GetRight();
			int bottom = m_CountDown.rect_hours.GetBottom();
			auto acc = [&](const Gdiplus::Rect& r) {
				left = min(left, r.X);
				top = min(top, r.Y);
				right = max(right, r.GetRight());
				bottom = max(bottom, r.GetBottom());
				};
			acc(m_CountDown.rect_minutes);
			acc(m_CountDown.rect_seconds);
			CRect inval(left, top, right, bottom);
			InvalidateRect(&inval, FALSE);
		}
		return;
	}
	// 其它计时器交给基类
	CDialogEx::OnTimer(nIDEvent);
}

BOOL UI_MouthMemberDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!App.m_isLoginIn || App.m_CanGuestBuy)
	{
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);
		if (m_Rect_QRcode.Contains(point.x, point.y))
		{
			::SetCursor(::LoadCursor(NULL, IDC_HAND));
			return TRUE;
		}
	}
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


void UI_MouthMemberDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_Rect_QRcode.Contains(point.x, point.y) || App.m_CanGuestBuy)
	{
		::SendMessage(m_hWnd, MSG_VIPDLG_LOGINSHOW, NULL, NULL);
		if (!App.m_isLoginIn)return;
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}
