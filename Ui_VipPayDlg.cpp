// Ui_VipPayDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CLarBtn.h"
#include "CLarPngBtn.h"
#include "CLazerStaticText.h"
#include "Ui_VipPayDlg.h"
#include "afxdialogex.h"
#include "PngLoader.h"
#include "theApp.h"
#include "LarStringConversion.h"
#include "ModalDialogFunc.h"
#include "GlobalFunc.h"
#include "Ui_LoginDlg.h"
#include "CTimer.h"
const int DiscountAmount = 140;
static bool g_isAlreadyHasRedpaketCoupon = false;   //是否领取过红包优惠券
bool Ui_VipPayDlg::g_offerExpireAtInited = false;
std::chrono::system_clock::time_point Ui_VipPayDlg::g_offerExpireAt;
bool Ui_VipPayDlg::m_isHasRedPacketCoupon = false;
std::chrono::milliseconds Ui_VipPayDlg::m_timeLest = std::chrono::milliseconds(std::chrono::hours(23));
std::chrono::milliseconds Ui_VipPayDlg::m_InitialTimeLest = std::chrono::milliseconds(0);
int Ui_VipPayDlg::m_RedPacketCouponAmount = 0;
std::string Ui_VipPayDlg::m_urlCouponAmount = "";
//二维码信息回调
size_t QRCodeWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    std::vector<unsigned char>* mem = (std::vector<unsigned char>*)userp;
    size_t oldSize = mem->size();
    mem->resize(oldSize + realsize);
    memcpy(mem->data() + oldSize, contents, realsize);
    return realsize;
}

IMPLEMENT_DYNAMIC(Ui_VipPayDlg, CDialogEx)

Ui_VipPayDlg::Ui_VipPayDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_VIPDLG, pParent)
{
    m_Bitmap_Sugbuy = nullptr;
    m_Bitmap_CouponActivityInfo = nullptr;
    m_Bitmap_SeasonVip = nullptr;
    m_Dlg_Login = nullptr;
    m_bool_IsInitReady.store(false);
    m_bool_IsDarker.store(false);
    m_MouthBillPrice = 0;
    m_preOrderNo = "";
    m_timerIdOfferCountdown = 0;
    m_Mode = Mode::LowPriceWithMouthBill;

}

Ui_VipPayDlg::~Ui_VipPayDlg()
{
    // 停止轮询
    m_PaymentPolling.stopPolling();
    CleanUpRes();
}

void Ui_VipPayDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, OPENVIPDLG_BTN_CLOSE, m_Btn_Close);
    DDX_Control(pDX, OPENVIPDLG_STAT_PAYLOGO, m_Stat_PayText);
    DDX_Control(pDX, OPENVIPDLG_STAT_PAYPRICE, m_Stat_PayPriceText);
    DDX_Control(pDX, OPENVIPDLG_STAT_PRICE, m_Stat_Price);
    DDX_Control(pDX, OPENVIPDLG_STAT_ORIPRICE, m_Stat_OriPrice);
    DDX_Control(pDX, OPENVIPDLG_BTN_PAYWAY, m_Btn_PayWay);
    DDX_Control(pDX, OPENVIPDLG_STAT_PROTOCOL, m_Stat_UserProtocol);
    DDX_Control(pDX, OPENVIPDLG_STAT_TITLETEXT, m_Stat_TitleText);
    DDX_Control(pDX, VIPDLG_STAT_PERMENTAMOUNT, m_Stat_PermentVip);
    DDX_Control(pDX, VIPDLG_STAT_YEARAMOUNT, m_Stat_YearAmount);
    DDX_Control(pDX, VIPDLG_STAT_HALFYEARAMOUNT, m_Stat_HalfYearAmount);
    DDX_Control(pDX, OPENVIP_STAT_VIPTYPE1, m_Stat_VipType1);
    DDX_Control(pDX, OPENVIP_STAT_VIPTYPE2, m_Stat_VipType2);
    DDX_Control(pDX, OPENVIP_STAT_VIPTYPE3, m_Stat_VipType3);
    DDX_Control(pDX, OPENVIP_STAT_BINGDEVICENUM1, m_Stat_BindDevice1);
    DDX_Control(pDX, OPENVIP_STAT_BINGDEVICENUM2, m_Stat_BindDevice2);
    DDX_Control(pDX, OPENVIP_STAT_BINGDEVICENUM3, m_Stat_BindDevice3);
    DDX_Control(pDX, VIPDLG_STAT_COUPONACTIVITY, m_stat_CouponActivityInfo);
    DDX_Control(pDX, VIPPAYDLG_STAT_TYPE1ORIPRICE, m_stat_type1OriPrice);
    DDX_Control(pDX, VIPPAYDLG_STAT_TYPE2ORIPRICE, m_stat_type2OriPrice);
    DDX_Control(pDX, VIPPAYDLG_STAT_TYPE3ORIPRICE, m_stat_Type3OriPrice);
    DDX_Control(pDX, OPENVIP_STAT_VIPTYPE4, m_stat_vipType4);
    DDX_Control(pDX, VIPDLG_STAT_SEASONAMOUNT, m_Stat_SeasonAMount);
    DDX_Control(pDX, OPENVIP_STAT_TYPE1RMBLOGO, m_stat_type1RmbLogo);
    DDX_Control(pDX, OPENVIP_STAT_TYPE2RMBLOGO, m_stat_type2RmbLogo);
    DDX_Control(pDX, OPENVIP_STAT_TYPE3RMBLOGO, m_stat_type3RmbLogo);
    DDX_Control(pDX, OPENVIP_STAT_TYPE4RMBLOGO, m_stat_type4RmbLogo);
    DDX_Control(pDX, OPENVIP_STAT_TYPE1LOWTODAY, m_stat_type1LowToDay);
    DDX_Control(pDX, OPENVIP_STAT_TYPE2LOWTODAY, m_stat_type2LowToDay);
    DDX_Control(pDX, OPENVIP_STAT_TYPE3LOWTODAY, m_stat_type3LowToDay);
    DDX_Control(pDX, OPENVIP_STAT_TYPE4LOWTODAY, m_stat_type4LowToDay);
    DDX_Control(pDX, OPENVIP_STAT_PRICERMBLOGO, m_stat_priceRmbLogo);
    DDX_Control(pDX, VIPPAYDLG_STAT_TYPE4ORIPRICE, m_stat_type4OriPrice);
    DDX_Control(pDX, OPENVIP_STAT_BINGDEVICENUM4, m_stat_bindDevice4);
    DDX_Control(pDX, VIPDLG_BTN_SUPTICKETANDSERVICE, m_btn_supTicketAndService);
    DDX_Control(pDX, VIPDLG_STAT_TYPE1BUDDLELOWTODAY, m_stat_type1BuddleLowToday);
    DDX_Control(pDX, VIPDLG_STAT_OFFEREXPIRATION, m_stat_offerExpiration);
}

void Ui_VipPayDlg::GetUserDpi()
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    if (screen == NULL) {
        AfxMessageBox(L"无法获取屏幕 DC。");
        return;
    }
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);

    // 计算缩放因子（基准 DPI 为 96）
    double scaleX = static_cast<double>(dpiX) / 96.0;
    double scaleY = static_cast<double>(dpiY) / 96.0;
    m_Scale = scaleY;
}

void Ui_VipPayDlg::UpdateScale()
{
    // 设定窗口大小为 507x304 像素
    int windowWidth = 507 * m_Scale;
    int windowHeight = 304 * m_Scale;

    // 如果 m_CRect_WindowRect 已定义，则使用其尺寸，否则使用默认尺寸
    if (m_CRect_WindowRect.Width() > 0 && m_CRect_WindowRect.Height() > 0)
    {
        windowWidth = m_CRect_WindowRect.Width();
        windowHeight = m_CRect_WindowRect.Height();
    }
    else
        m_CRect_WindowRect.SetRect(0, 0, windowWidth, windowHeight);
    SetWindowPos(NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);

    //Vip详细信息矩形
    m_Rect_VipInfoRect.Width = 358.52 * m_Scale;
    m_Rect_VipInfoRect.Height = 193.99 * m_Scale;
    m_Rect_VipInfoRect.X = 27.8 * m_Scale;
    m_Rect_VipInfoRect.Y = 343.5 * m_Scale;
    
    //定义标题栏区域 
    m_Rect_Caption.X = 0;
    m_Rect_Caption.Y = 0;
    m_Rect_Caption.Width = m_CRect_WindowRect.Width();
    m_Rect_Caption.Height = 56 * m_Scale;

    //定义内部支付区域 
    m_Rect_InnerPay.Width = 328 * m_Scale;
    m_Rect_InnerPay.Height = 145 * m_Scale;
    m_Rect_InnerPay.X = 0.308 * m_CRect_WindowRect.Width();
    m_Rect_InnerPay.Y = 0.468 * m_CRect_WindowRect.Height();

    // 标题 
    float TitleTextWidth = 55 * m_Scale * 2.0f;
    float TitleTextHeight = 16 * m_Scale * 1.5f;
    float TitleTextX = 10 * m_Scale;
    float TitleTextY = (m_Rect_Caption.Height - TitleTextHeight) / 2;
    m_Stat_TitleText.MoveWindow(TitleTextX, TitleTextY, TitleTextWidth, TitleTextHeight);

    // 定义永久 VIP 按钮位置和大小
    float permVipWidth = 160 * m_Scale;
    float permVipHeight = 188 * m_Scale;
    float permVipX = 28 * m_Scale;
    float permVipY = 109 * m_Scale;
    m_Rect_PermentVip.Width = permVipWidth;
    m_Rect_PermentVip.Height = permVipHeight;
    m_Rect_PermentVip.X = permVipX;
    m_Rect_PermentVip.Y = permVipY;

    //标题新用户logo
    m_Rect_TitleLogoNewUser.Width = 305 * m_Scale;
    m_Rect_TitleLogoNewUser.Height = 25 * m_Scale;
    m_Rect_TitleLogoNewUser.X = (m_CRect_WindowRect.Width() - m_Rect_TitleLogoNewUser.Width) / 2;
    m_Rect_TitleLogoNewUser.Y = (m_Rect_Caption.Height - m_Rect_TitleLogoNewUser.Height) / 2;

    // 季度VIP按钮区域
    m_Rect_SeasonVip.Width = 160 * m_Scale;
    m_Rect_SeasonVip.Height = 188 * m_Scale;
    m_Rect_SeasonVip.X = 581 * m_Scale;
    m_Rect_SeasonVip.Y = m_Rect_PermentVip.Y;

    // 定义年度 VIP 按钮位置和大小
    float yearVipWidth = 160 * m_Scale;
    float yearVipHeight = 188 * m_Scale;
    float yearVipX = permVipX + permVipWidth + 24 * m_Scale;
    float yearVipY = permVipY;
    m_Rect_YearVip.Width = yearVipWidth;
    m_Rect_YearVip.Height = yearVipHeight;
    m_Rect_YearVip.X = yearVipX;
    m_Rect_YearVip.Y = yearVipY;

    //开具发票/联系客服
    float supTicketAndServiceW = 123 * m_Scale;
    float supTicketAndServiceH = 15 * m_Scale;
    float supTicketAndServiceX = m_CRect_WindowRect.Width() - supTicketAndServiceW - 11 * m_Scale;
    float supTicketAndServiceY = m_CRect_WindowRect.Height() - supTicketAndServiceH - 19 * m_Scale;
    m_btn_supTicketAndService.MoveWindow(
        supTicketAndServiceX,
        supTicketAndServiceY,
        supTicketAndServiceW,
        supTicketAndServiceH
    );

    //定义年度 VIP价格文字位置和大小
    float YearPriceWdith = 84 * m_Scale;
    float YearPriceHeight = 27 * m_Scale;
    float YearPriceX = yearVipX + (m_Rect_YearVip.Width - YearPriceWdith) / 2;
    float YearPriceY = yearVipY + 54 * m_Scale;
    m_Stat_YearAmount.MoveWindow(YearPriceX, YearPriceY, YearPriceWdith, YearPriceHeight);

    //定义年度vip原价
    {
        int w = 87 * m_Scale;
        int h = 16 * m_Scale;
        int x = m_Rect_YearVip.X + (m_Rect_YearVip.Width - w) / 2;
        int y = m_Rect_YearVip.Y + 89 * m_Scale;
        m_stat_type2OriPrice.MoveWindow(x, y, w, h);
    }

    //定义永久vip价格文字位置和大小
    float PermentPriceWdith = 88 * m_Scale;
    float PermentPriceHeight = 35 * m_Scale;
    float PermentPriceX = m_Rect_PermentVip.X + (m_Rect_PermentVip.Width - PermentPriceWdith) / 2;;
    float PermentPriceY = m_Rect_PermentVip.Y + 54 * m_Scale;
    m_Stat_PermentVip.MoveWindow(PermentPriceX, PermentPriceY, PermentPriceWdith, PermentPriceHeight);

    //定义永久vip原价
    {
        int w = 87 * m_Scale;
        int h = 20 * m_Scale;
        int x = m_Rect_PermentVip.X + (m_Rect_PermentVip.Width - w) / 2;
        int y = m_Rect_PermentVip.Y + 89 * m_Scale;
        m_stat_type1OriPrice.MoveWindow(x, y, w, h);
    }

    // 定义半年 VIP 按钮位置和大小
    float halfVipWidth = 160 * m_Scale;
    float halfVipHeight = 188 * m_Scale;
    float halfVipX = (yearVipX + yearVipWidth + 25 * m_Scale);
    float halfVipY = permVipY;
    m_Rect_HalfYear.Width = halfVipWidth;
    m_Rect_HalfYear.Height = halfVipHeight;
    m_Rect_HalfYear.X = halfVipX;
    m_Rect_HalfYear.Y = halfVipY;


    //定义VIP类型1的描述信息文本
    float StatType1Width = 58 * m_Scale;
    float StatType1Height = 19 * m_Scale;
    float StatType1X = m_Rect_PermentVip.X + (m_Rect_PermentVip.Width - StatType1Width) / 2;
    float StatType1Y = m_Rect_PermentVip.Y + 32 * m_Scale;
    m_Stat_VipType1.MoveWindow(StatType1X, StatType1Y, StatType1Width, StatType1Height);

    //定义设备绑定数量1描述文本
    float StatDeviceNum1Width = 77 * m_Scale;
    float StatDeviceNum1Height = 16 * m_Scale;
    float StatDeviceNum1X = m_Rect_PermentVip.X + (m_Rect_PermentVip.Width - StatDeviceNum1Width) / 2;
    float StatDeviceNum1Y = m_Rect_PermentVip.Y + 115 * m_Scale;
    m_Stat_BindDevice1.MoveWindow(StatDeviceNum1X, StatDeviceNum1Y, StatDeviceNum1Width, StatDeviceNum1Height);

    //定义VIP类型2的描述信息文本 
    float StatType2Width = 60 * m_Scale;
    float StatType2Height = StatType1Height;
    float StatType2X = m_Rect_YearVip.X + (m_Rect_YearVip.Width - StatType2Width) / 2;
    float StatType2Y = StatType1Y;
    m_Stat_VipType2.MoveWindow(StatType2X, StatType2Y, StatType2Width, StatType2Height);

    //定义设备绑定数量2描述文本
    float StatDeviceNum2Width = 77 * m_Scale;
    float StatDeviceNum2Height = 16 * m_Scale;
    float StatDeviceNum2X = m_Rect_YearVip.X + (m_Rect_YearVip.Width - StatDeviceNum2Width) / 2;
    float StatDeviceNum2Y = StatDeviceNum1Y;
    m_Stat_BindDevice2.MoveWindow(StatDeviceNum2X, StatDeviceNum2Y, StatDeviceNum2Width, StatDeviceNum2Height);

    //定义VIP类型3的描述信息文本
    float StatType3Width = 58 * m_Scale;
    float StatType3Height = 19 * m_Scale;
    float StatType3X = m_Rect_HalfYear.X + (m_Rect_HalfYear.Width - StatType3Width) / 2;
    float StatType3Y = StatType2Y;
    m_Stat_VipType3.MoveWindow(StatType3X, StatType3Y, StatType3Width, StatType3Height);

    //定义设备绑定数量3描述文本
    float StatDeviceNum3Width = 77 * m_Scale;
    float StatDeviceNum3Height = 16 * m_Scale;
    float StatDeviceNum3X = m_Rect_HalfYear.X + (m_Rect_HalfYear.Width - StatDeviceNum3Width) / 2;
    float StatDeviceNum3Y = StatDeviceNum2Y;
    m_Stat_BindDevice3.MoveWindow(StatDeviceNum3X, StatDeviceNum3Y, StatDeviceNum3Width, StatDeviceNum3Height);

    //定义VIP类型4的描述信息文本m_stat_vipType4
    float StatType4Width = 58 * m_Scale;
    float StatType4Height = 19 * m_Scale;
    float StatType4X = m_Rect_SeasonVip.X + (m_Rect_SeasonVip.Width - StatType4Width) / 2;
    float StatType4Y = StatType3Y;
    m_stat_vipType4.MoveWindow(StatType4X, StatType4Y, StatType4Width, StatType4Height);

    //定义VIP类型4价格文本
    float vipType4PriceInfoW = 68 * m_Scale;
    float vipType4PriceInfoH = 31 * m_Scale;
    float vipType4PriceInfoX = m_Rect_SeasonVip.X + (m_Rect_SeasonVip.Width - vipType4PriceInfoW) / 2;
    float vipType4PriceInfoY = PermentPriceY;
    m_Stat_SeasonAMount.MoveWindow(vipType4PriceInfoX, vipType4PriceInfoY, vipType4PriceInfoW, vipType4PriceInfoH);
    m_Stat_SeasonAMount.SetWindowTextW(L"价格");

    //定义半年 VIP 价格文字位置和大小
    float HalfYearPriceWdith = 84 * m_Scale;
    float HalfYearPriceHeight = 35 * m_Scale;
    float HalfYearPriceX = m_Rect_HalfYear.X + (m_Rect_HalfYear.Width - HalfYearPriceWdith) / 2;
    float HalfYearPriceY = YearPriceY;
    m_Stat_HalfYearAmount.MoveWindow(HalfYearPriceX, HalfYearPriceY, HalfYearPriceWdith, HalfYearPriceHeight);

    //定义半年vip原价
    {
        int w = 87 * m_Scale;
        int h = 16 * m_Scale;
        int x = HalfYearPriceX + (HalfYearPriceWdith - w) / 2;
        int y = m_Rect_HalfYear.Y + 89 * m_Scale;
        m_stat_Type3OriPrice.MoveWindow(x, y, w, h);
    }

    // 定义关闭按钮位置和大小
    float closeWidth = 28 * m_Scale;
    float closeHeight = 28 * m_Scale;
    float closeX = 737.2 * m_Scale;
    float closeY = (m_Rect_Caption.Height - closeHeight) / 2;
    m_Btn_Close.MoveWindow(closeX, closeY, closeWidth, closeHeight);

    // 定义支付文本位置和大小
    float payTextWidth = 64 * m_Scale;
    float payTextHeight = 21 * m_Scale;
    float payTextX = 405 * m_Scale;
    float payTextY = 351 * m_Scale;
    m_Stat_PayText.MoveWindow(payTextX, payTextY, payTextWidth, payTextHeight);

    // 定义支付金额文本位置和大小
    float payPriceTextWidth = 57 * m_Scale;
    float payPriceTextHeight = 19 * m_Scale;
    float payPriceTextX = 537.2 * m_Scale;
    float payPriceTextY = 421.5 * m_Scale;
    m_Stat_PayPriceText.MoveWindow(payPriceTextX, payPriceTextY, payPriceTextWidth, payPriceTextHeight);

    // 定义价格文本位置和大小
    float priceWidth = 95 * m_Scale;
    float priceHeight = 46 * m_Scale;
    float priceX = payPriceTextX + payPriceTextWidth + 8.3 * m_Scale;
    float priceY = 410.5 * m_Scale;
    m_Stat_Price.MoveWindow(priceX, priceY, priceWidth, priceHeight);

    // 定义原价位置和大小
    float oriPriceWidth = 65 * m_Scale;
    float oriPriceHeight = 19 * m_Scale;
    float oriPriceX = priceX + priceWidth - 10 * m_Scale;
    float oriPriceY = 427.5 * m_Scale;
    m_Stat_OriPrice.MoveWindow(oriPriceX, oriPriceY, oriPriceWidth, oriPriceHeight);

    // 定义二维码区域
    float qrCodeWidth = 106 * m_Scale;
    float qrCodeHeight = 107 * m_Scale;
    float qrCodeX = 412 * m_Scale;
    float qrCodeY = 387 * m_Scale;
    m_Rect_QRCode.X = qrCodeX;
    m_Rect_QRCode.Y = qrCodeY;
    m_Rect_QRCode.Width = qrCodeWidth;
    m_Rect_QRCode.Height = qrCodeHeight;
    m_Rect_QRCodeBK.Width = 121 * m_Scale;
    m_Rect_QRCodeBK.Height = 118 * m_Scale;
    m_Rect_QRCodeBK.X = 405 * m_Scale;
    m_Rect_QRCodeBK.Y = 381 * m_Scale;

    // 定义用户协议文本位置
    float ProtocalWidth = 117 * m_Scale;
    float ProtocalHeight = 38 * m_Scale;
    float ProtocalX = 409 * m_Scale;
    float ProtocalY = 517 * m_Scale;
    m_Stat_UserProtocol.MoveWindow(ProtocalX, ProtocalY, ProtocalWidth, ProtocalHeight);

    //定义支付方式信息
    float PayWayWidth = 104 * m_Scale;
    float PayWayHeight = 22 * m_Scale;
    float PayWayY = 473 * m_Scale;
    float PayWayX = 537 * m_Scale;
    m_Btn_PayWay.MoveWindow(PayWayX, PayWayY, PayWayWidth, PayWayHeight);

    //定义 新人特惠logo
    m_Rect_CouponActivityInfo.Width = 138 * m_Scale;
    m_Rect_CouponActivityInfo.Height = 26.25 * m_Scale;
    m_Rect_CouponActivityInfo.X = 611 * m_Scale;
    m_Rect_CouponActivityInfo.Y = 389 * m_Scale;
    {
        float CaWidth = 117 * m_Scale;
        float CaHeight = 16 * m_Scale;
        float CaX = 622 * m_Scale;
        float CaY = 392 * m_Scale;
        m_stat_CouponActivityInfo.MoveWindow(
            CaX,
            CaY,
            CaWidth,
            CaHeight
        );
    }

    //热卖推荐
    m_Rect_Sugbuy.Width = 100 * m_Scale;
    m_Rect_Sugbuy.Height = 25 * m_Scale;
    m_Rect_Sugbuy.X = m_Rect_PermentVip.X;
    m_Rect_Sugbuy.Y = m_Rect_PermentVip.Y - m_Rect_Sugbuy.Height;

    //VIP特权对比
    int vpcW = 150 * m_Scale;
    int vpcH = 20 * m_Scale;
    int vpcX = 264 * m_Scale;
    int vpcY = 356 * m_Scale;
    m_cusstat_vipPrivilegeContrast.btnArea.X = vpcX;
    m_cusstat_vipPrivilegeContrast.btnArea.Y = vpcY;
    m_cusstat_vipPrivilegeContrast.btnArea.Width = vpcW;
    m_cusstat_vipPrivilegeContrast.btnArea.Height = vpcH;

    //低至xx一天(type1)
    int type1W = 56 * m_Scale;
    int type1H = 19 * m_Scale;
    int type1lowToDayY = m_Rect_PermentVip.Y + 144 * m_Scale;
    int type1lowToDayX = m_Rect_PermentVip.X + (m_Rect_PermentVip.Width - type1W) / 2;
    m_stat_type1LowToDay.MoveWindow(type1lowToDayX, type1lowToDayY, type1W, type1H);

    //低至xx一天(type2)
    int type2W = 76 * m_Scale;
    int type2H = 19 * m_Scale;
    int type2lowToDayY = type1lowToDayY;
    int type2lowToDayX = m_Rect_YearVip.X + (m_Rect_YearVip.Width - type2W) / 2;
    m_stat_type2LowToDay.MoveWindow(type2lowToDayX, type2lowToDayY, type2W, type2H);

    //低至xx一天(type3)
    int type3W = 76 * m_Scale;
    int type3H = 19 * m_Scale;
    int type3lowToDayY = type1lowToDayY;
    int type3lowToDayX = m_Rect_HalfYear.X + (m_Rect_HalfYear.Width - type3W) / 2;
    m_stat_type3LowToDay.MoveWindow(type3lowToDayX, type3lowToDayY, type3W, type3H);

    //低至xx一天(type4)
    int type4W = 76 * m_Scale;
    int type4H = 19 * m_Scale;
    int type4lowToDayY = type1lowToDayY;
    int type4lowToDayX = m_Rect_SeasonVip.X + (m_Rect_SeasonVip.Width - type4W) / 2;
    m_stat_type4LowToDay.MoveWindow(type4lowToDayX, type4lowToDayY, type4W, type4H);

    //价格旁边￥标识
    int RmbLogoY = m_Rect_PermentVip.Y + 66 * m_Scale;
    int RmbLogoW = 9 * m_Scale;
    int RmbLogoH = 21 * m_Scale;
    {
        int x1 = m_Rect_PermentVip.X + 35 * m_Scale;
        int x2 = m_Rect_YearVip.X + 41 * m_Scale;
        int x3 = m_Rect_HalfYear.X + 41 * m_Scale;
        int x4 = m_Rect_SeasonVip.X + 41 * m_Scale;
        m_stat_type1RmbLogo.MoveWindow(x1, RmbLogoY, RmbLogoW, RmbLogoH);
        m_stat_type2RmbLogo.MoveWindow(x2, RmbLogoY, RmbLogoW, RmbLogoH);
        m_stat_type3RmbLogo.MoveWindow(x3, RmbLogoY, RmbLogoW, RmbLogoH);
        m_stat_type4RmbLogo.MoveWindow(x4, RmbLogoY, RmbLogoW, RmbLogoH);
    }

    //最终支付价格旁的￥标识
    {
        int PriceRmbLogoW = 9 * m_Scale;
        int PriceRmbLogoH = 21 * m_Scale;
        int PriceRmbLogoX = priceX - PriceRmbLogoW;
        int PriceRmbLogoY = priceY + priceHeight - PriceRmbLogoH - 5 * m_Scale;
        m_stat_priceRmbLogo.MoveWindow(PriceRmbLogoX, PriceRmbLogoY, PriceRmbLogoW, PriceRmbLogoH);
    }

    {
        int w = 87 * m_Scale;
        int h = 16 * m_Scale;
        int x = m_Rect_SeasonVip.X + (m_Rect_SeasonVip.Width - w) / 2;
        int y = m_Rect_SeasonVip.Y + 89 * m_Scale;
        m_stat_type4OriPrice.MoveWindow(x, y, w, h);
    }
    {
        int w = 77 * m_Scale;
        int h = 16 * m_Scale;
        int x = m_Rect_SeasonVip.X + (m_Rect_SeasonVip.Width - w) / 2;
        int y = StatDeviceNum3Y;
        m_stat_bindDevice4.MoveWindow(x, y, w, h);
    }

    //Type1左上角的小气泡文本
    int buddleType1LowTodayW = 88 * m_Scale;
    int buddleType1LowToDayH = 23 * m_Scale;
    int buddleType1LowToDayX = m_Rect_PermentVip.X + 72 * m_Scale;
    int buddleType1LowToDayY = m_Rect_PermentVip.Y;
    m_stat_type1BuddleLowToday.MoveWindow(
        buddleType1LowToDayX,
        buddleType1LowToDayY,
        buddleType1LowTodayW,
        buddleType1LowToDayH
    );

    ///Type1左上角的小气泡背景位图区域
    m_Rect_BuddleType1LowToday.Width = buddleType1LowTodayW;
    m_Rect_BuddleType1LowToday.Height = buddleType1LowToDayH;
    m_Rect_BuddleType1LowToday.X = buddleType1LowToDayX;
    m_Rect_BuddleType1LowToday.Y = buddleType1LowToDayY;

    //优惠倒计时文本
    int offerEW = 73 * m_Scale;
    int offerEH = 19 * m_Scale;
    int offerEX = 25 * m_Scale;
    int offerEY = 74 * m_Scale;
    m_stat_offerExpiration.MoveWindow(offerEX, offerEY, offerEW, offerEH);
}

void Ui_VipPayDlg::InitCtrl()
{
    // 关闭按钮
    m_Btn_Close.LoadPNG(VIDEOLISTDLG_BTN_CLOSE);
    m_Btn_Close.SetBackgroundColor(RGB(16, 23, 24));
    m_Btn_Close.SetHoverEffectColor(15, 255, 255, 255);
    m_Btn_Close.SetStretchMode(0.60f);

    // 原价
    m_Stat_OriPrice.LarSetBreakLine(false);
    m_Stat_OriPrice.LarSetTextSize(15);
    m_Stat_OriPrice.LarSetTextColor(RGB(159, 159, 159));
    m_Stat_OriPrice.LarSetStrikeOut(TRUE);
    m_Stat_OriPrice.LarSetTextLeft();

    // 微信支付宝扫码支付
    m_Btn_PayWay.LoadPNG(OPENVIPDLG_PNG_PAYWAY);
    m_Btn_PayWay.SetBackgroundColor(RGB(26, 31, 37));

    //价格字体设置
    m_Stat_Price.LarSetBreakLine(false);
    m_Stat_Price.LarSetTextSize(32);
    m_Stat_Price.LarSetEraseColor(RGB(26, 31, 37));
    m_Stat_Price.LarSetTextColor(RGB(241, 175, 57));
    m_Stat_Price.LarSetTextStyle(true, false, false);
    m_Stat_Price.LarSetTextLeft();

    //新人特想优惠立减140元
    m_stat_CouponActivityInfo.LarSetBreakLine(false);
    m_stat_CouponActivityInfo.LarSetTextSize(16);
    m_stat_CouponActivityInfo.LarSetTextColor(RGB(255, 255, 255));
    m_stat_CouponActivityInfo.LarSetTextCenter();

    m_stat_type1BuddleLowToday.LarSetBreakLine(false);
    m_stat_type1BuddleLowToday.LarSetTextSize(17);
    m_stat_type1BuddleLowToday.LarSetTextColor(RGB(255, 255, 255));
    m_stat_type1BuddleLowToday.LarSetTextCenter();

    m_stat_offerExpiration.LarSetBreakLine(false);
    m_stat_offerExpiration.LarSetTextSize(17);
    m_stat_offerExpiration.LarSetTextColor(RGB(255, 255, 255));
    m_stat_offerExpiration.LarSetTextCenter();

    // 永久会员价格
    m_Stat_PermentVip.LarSetTextSize(30);
    m_Stat_PermentVip.LarSetEraseColor(RGB(26, 31, 37));
    m_Stat_PermentVip.LarSetTextColor(RGB(250, 210, 158));
    m_Stat_PermentVip.LarSetTextStyle(true, false, false);
    m_Stat_PermentVip.LarSetTextCenter();
    m_Stat_SeasonAMount.LarSetTextSize(30);
    m_Stat_SeasonAMount.LarSetEraseColor(RGB(26, 31, 37));
    m_Stat_SeasonAMount.LarSetTextColor(RGB(250, 210, 158));
    m_Stat_SeasonAMount.LarSetTextStyle(true, false, false);
    m_Stat_SeasonAMount.LarSetTextCenter();
    m_stat_type1OriPrice.LarSetBreakLine(false);
    m_stat_type1OriPrice.LarSetTextSize(18);
    m_stat_type1OriPrice.LarSetTextColor(RGB(184, 184, 184));
    m_stat_type1OriPrice.LarSetStrikeOut(TRUE);

    m_stat_type4OriPrice.LarSetBreakLine(false);
    m_stat_type4OriPrice.LarSetTextSize(18);
    m_stat_type4OriPrice.LarSetTextColor(RGB(184, 184, 184));
    m_stat_type4OriPrice.LarSetStrikeOut(TRUE);

    // 年度会员价格
    m_Stat_YearAmount.LarSetTextSize(30);
    m_Stat_YearAmount.LarSetEraseColor(RGB(26, 31, 37));
    m_Stat_YearAmount.LarSetTextColor(RGB(250, 210, 158));
    m_Stat_YearAmount.LarSetTextStyle(true, false, false);
    m_Stat_YearAmount.LarSetTextCenter();
    m_stat_type2OriPrice.LarSetBreakLine(false);
    m_stat_type2OriPrice.LarSetTextSize(18);
    m_stat_type2OriPrice.LarSetTextColor(RGB(184, 184, 184));
    m_stat_type2OriPrice.LarSetStrikeOut(TRUE);

    // 半年会员价格
    m_Stat_HalfYearAmount.LarSetTextSize(30);
    m_Stat_HalfYearAmount.LarSetEraseColor(RGB(26, 31, 37));
    m_Stat_HalfYearAmount.LarSetTextColor(RGB(250, 210, 158));
    m_Stat_HalfYearAmount.LarSetTextStyle(true, false, false);
    m_Stat_HalfYearAmount.LarSetTextCenter();
    m_stat_Type3OriPrice.LarSetBreakLine(false);
    m_stat_Type3OriPrice.LarSetTextSize(18);
    m_stat_Type3OriPrice.LarSetTextColor(RGB(184, 184, 184));
    m_stat_Type3OriPrice.LarSetStrikeOut(TRUE);

    // 其他字体设置
    m_Stat_UserProtocol.LarSetTextColor(RGB(112, 112, 112));
    m_Stat_UserProtocol.LarSetTextSize(17);
    m_Stat_UserProtocol.LarSetTextStyle(false, false, false);
    m_Stat_UserProtocol.LarSetTextLeft();
    m_Stat_PayText.LarSetBreakLine(false);
    m_Stat_PayText.LarSetTextSize(20);
    m_Stat_PayText.LarSetTextStyle(false, false, false);
    m_Stat_PayPriceText.LarSetBreakLine(false);
    m_Stat_PayPriceText.LarSetTextSize(18);
    m_Stat_PayPriceText.LarSetTextStyle(false, false, false);
    m_Stat_PayPriceText.LarSetTextLeft();
    m_Stat_TitleText.LarSetTextSize(24);
    m_Stat_TitleText.LarSetTextStyle(false, false, false);

    //套餐信息描述
    m_Stat_VipType1.LarSetTextColor(RGB(215, 215, 215));
    m_Stat_VipType1.LarSetTextSize(18);
    m_Stat_VipType1.LarSetTextCenter();

    m_Stat_VipType2.LarSetTextColor(RGB(215, 215, 215));
    m_Stat_VipType2.LarSetTextSize(18);
    m_Stat_VipType2.LarSetTextCenter();

    m_Stat_VipType3.LarSetTextColor(RGB(215, 215, 215));
    m_Stat_VipType3.LarSetTextSize(18);
    m_Stat_VipType3.LarSetTextCenter();

    m_stat_vipType4.LarSetTextColor(RGB(215, 215, 215));
    m_stat_vipType4.LarSetTextSize(18);
    m_stat_vipType4.LarSetTextCenter();

    m_stat_bindDevice4.LarSetTextColor(RGB(165, 165, 165));
    m_stat_bindDevice4.LarSetTextSize(18);
    m_stat_bindDevice4.LarSetTextCenter();

    m_Stat_BindDevice1.LarSetTextColor(RGB(184, 184, 184));
    m_Stat_BindDevice1.LarSetTextSize(18);
    m_Stat_BindDevice1.LarSetTextCenter();

    m_Stat_BindDevice2.LarSetTextColor(RGB(184, 184, 184));
    m_Stat_BindDevice2.LarSetTextSize(18);
    m_Stat_BindDevice2.LarSetTextCenter();

    m_Stat_BindDevice3.LarSetTextColor(RGB(184, 184, 184));
    m_Stat_BindDevice3.LarSetTextSize(18);
    m_Stat_BindDevice3.LarSetTextCenter();

    //￥符号
    SetRmbLogoStatStyle(&m_stat_type1RmbLogo);
    SetRmbLogoStatStyle(&m_stat_type2RmbLogo);
    SetRmbLogoStatStyle(&m_stat_type3RmbLogo);
    SetRmbLogoStatStyle(&m_stat_type4RmbLogo);
    SetRmbLogoStatStyle(&m_stat_priceRmbLogo);

    //低至xx一天
    SetLowToDayStatSelectStyle(&m_stat_type1LowToDay);
    SetLowToDayStatNormalStyle(&m_stat_type2LowToDay);
    SetLowToDayStatNormalStyle(&m_stat_type3LowToDay);
    SetLowToDayStatNormalStyle(&m_stat_type4LowToDay);

    //联系客服/开局发票
    m_btn_supTicketAndService.LarSetTextSize(16);
    m_btn_supTicketAndService.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
    m_btn_supTicketAndService.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
    m_btn_supTicketAndService.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
    m_btn_supTicketAndService.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
    m_btn_supTicketAndService.LarSetEraseBkEnable(false);
    m_btn_supTicketAndService.LarSetNormalFiilBrush(SolidBrush(Color(255, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetHoverFillBrush(SolidBrush(Color(255, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetClickedFillBrush(SolidBrush(Color(255, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetTextCenter();
    m_btn_supTicketAndService.LarSetBtnTextMultLine(false);

    // VIP特权对比 
    {
        using namespace Gdiplus;
        // 文本画刷
        if (m_cusstat_vipPrivilegeContrast.txtB == nullptr)
            m_cusstat_vipPrivilegeContrast.txtB = new SolidBrush(Color(255, 253, 229, 187));
        if (m_cusstat_vipPrivilegeContrast.txthovB == nullptr)
            m_cusstat_vipPrivilegeContrast.txthovB = new SolidBrush(Color(255, 255, 255, 255));

        if (m_cusstat_vipPrivilegeContrast.font)
        {
            delete m_cusstat_vipPrivilegeContrast.font;
            m_cusstat_vipPrivilegeContrast.font = nullptr;
        }
        m_cusstat_vipPrivilegeContrast.font = new
            Gdiplus::Font(L"微软雅黑", 13.0f * m_Scale, FontStyleRegular, UnitPixel);
        if (m_cusstat_vipPrivilegeContrast.font &&
            m_cusstat_vipPrivilegeContrast.font->GetLastStatus() != Gdiplus::Ok)
        {
            delete m_cusstat_vipPrivilegeContrast.font;
            m_cusstat_vipPrivilegeContrast.font = nullptr;
        }

        // 文本与区域
        m_cusstat_vipPrivilegeContrast.str = L"全部权益>>";
        m_cusstat_vipPrivilegeContrast.uId = OPENVIP_CUSSTAT_PRIVILEGE;
        m_cusstat_vipPrivilegeContrast.txtR = m_cusstat_vipPrivilegeContrast.btnArea;
        m_cusstat_vipPrivilegeContrast.finalTxtB = m_cusstat_vipPrivilegeContrast.txtB;
        m_bVipPrivilegeHover = false;

        //设置点击回调
        m_cusstat_vipPrivilegeContrast.clickCallBack = [this]()
            {
                DB(ConsoleHandle, L"全部权益文本按钮回调执行");
                m_PaymentPolling.stopPolling();
                this->ShowWindow(SW_HIDE);
                ::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                ModalDlg_SDL::ShowModal_Rai(this, m_Scale, Ui_RaiSDL::ShowMode::ReturnToOriPage);
                this->ShowWindow(SW_SHOW);
                ::PostMessage(this->GetSafeHwnd(), WM_APP + 1003, NULL, NULL);
            };
    }
}

void Ui_VipPayDlg::InitCusStat()
{

}

void Ui_VipPayDlg::InitCountDownOfferExpiration()
{
    auto& c = m_countDownOfferExpiration;

    // 先清理旧资源
    if (c.font_days) { delete c.font_days;    c.font_days = nullptr; }
    if (c.font_hours) { delete c.font_hours;   c.font_hours = nullptr; }
    if (c.font_minute) { delete c.font_minute;  c.font_minute = nullptr; }
    if (c.font_seconds) { delete c.font_seconds; c.font_seconds = nullptr; }
    if (c.font_unit) { delete c.font_unit;    c.font_unit = nullptr; }
    if (c.sb_days) { delete c.sb_days;      c.sb_days = nullptr; }
    if (c.sb_hours) { delete c.sb_hours;     c.sb_hours = nullptr; }
    if (c.sb_minutes) { delete c.sb_minutes;   c.sb_minutes = nullptr; }
    if (c.sb_seconds) { delete c.sb_seconds;   c.sb_seconds = nullptr; }
    if (c.sb_unit) { delete c.sb_unit;      c.sb_unit = nullptr; }

    // 默认显示文案
    c.wstr_days = L"00";
    c.wstr_hours = L"00";
    c.wstr_minute = L"00";
    c.wstr_seconds = L"00";
    c.wstr_millSeconds = L"9";

    // 字体设置
    const wchar_t* kFontFamily = L"微软雅黑";
    const REAL     kDigitSize = 22.0f * m_Scale;
    const REAL     kUnitSize = 14.0f * m_Scale;
    c.font_days = new Gdiplus::Font(kFontFamily, kDigitSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    c.font_hours = new Gdiplus::Font(kFontFamily, kDigitSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    c.font_minute = new Gdiplus::Font(kFontFamily, kDigitSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    c.font_seconds = new Gdiplus::Font(kFontFamily, kDigitSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    c.font_unit = new Gdiplus::Font(kFontFamily, kUnitSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    c.font_millSeconds = new Gdiplus::Font(kFontFamily, kDigitSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

    // 颜色画刷（数字暖色，单位中性灰）
    const Gdiplus::Color kDigitColor(255, 232, 185, 133);
    const Gdiplus::Color kUnitColor(255, 199, 199, 199);
    c.sb_days = new Gdiplus::SolidBrush(kDigitColor);
    c.sb_hours = new Gdiplus::SolidBrush(kDigitColor);
    c.sb_minutes = new Gdiplus::SolidBrush(kDigitColor);
    c.sb_seconds = new Gdiplus::SolidBrush(kDigitColor);
    c.sb_unit = new Gdiplus::SolidBrush(kUnitColor);
    c.sb_millSeconds = new Gdiplus::SolidBrush(kDigitColor);

    // 数字与单位之间的像素间距
    c.unit_interval = 6;

    // 倒计时整体左上角
    const INT base_x = static_cast<INT>(470 * m_Scale + 0.5f);
    const INT base_y = static_cast<INT>(73 * m_Scale + 0.5f);

    // 创建临时 Graphics 用于测量
    Gdiplus::Bitmap bmp(1, 1, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&bmp);
    g.SetPageUnit(Gdiplus::UnitPixel);
    Gdiplus::StringFormat sf;
    sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsNoClip);

    // 为避免不同 DPI / ClearType 下数字被裁切，使用较宽的样本 "88"，再增加额外 padding
    const std::wstring kDigitMeasureSample = L"88";
    const INT kDigitExtraPadding = static_cast<INT>(4 * m_Scale + 0.5f); 
    Gdiplus::RectF layout(0, 0, 1000.0f, 1000.0f), bound;
    g.MeasureString(kDigitMeasureSample.c_str(),
        static_cast<INT>(kDigitMeasureSample.length()),
        c.font_days, layout, &sf, &bound);
    const INT digit_w = static_cast<INT>(bound.Width + 0.5f) + kDigitExtraPadding;
    const INT digit_h = static_cast<INT>(bound.Height + 0.5f);

    // 单位字符串
    const std::wstring u_day = L"天";
    const std::wstring u_hrs = L"时";
    const std::wstring u_min = L"分";
    const std::wstring u_sec = L"秒";

    g.MeasureString(u_day.c_str(), 1, c.font_unit, layout, &sf, &bound);
    const INT unit_w_day = static_cast<INT>(bound.Width + 0.5f);
    const INT unit_h_day = static_cast<INT>(bound.Height + 0.5f);

    g.MeasureString(u_hrs.c_str(), 1, c.font_unit, layout, &sf, &bound);
    const INT unit_w_hrs = static_cast<INT>(bound.Width + 0.5f);
    const INT unit_h_hrs = static_cast<INT>(bound.Height + 0.5f);

    g.MeasureString(u_min.c_str(), 1, c.font_unit, layout, &sf, &bound);
    const INT unit_w_min = static_cast<INT>(bound.Width + 0.5f);
    const INT unit_h_min = static_cast<INT>(bound.Height + 0.5f);

    g.MeasureString(u_sec.c_str(), 1, c.font_unit, layout, &sf, &bound);
    const INT unit_w_sec = static_cast<INT>(bound.Width + 0.5f);
    const INT unit_h_sec = static_cast<INT>(bound.Height + 0.5f);

    // 数字与单位间距（放大与缩放系数绑定）
    const INT gap_num_unit = static_cast<INT>(c.unit_interval * m_Scale + 0.5f);
    // 组之间的额外间距
    const INT gap_group = static_cast<INT>(5 * m_Scale + 0.5f);

    INT x = base_x;
    const INT y = base_y;

    // 天
    c.rect_days = Gdiplus::Rect(x, y, digit_w, digit_h);
    Gdiplus::Rect rect_unit_day(
        c.rect_days.X + c.rect_days.Width + gap_num_unit,
        y + (digit_h - unit_h_day) / 2,
        unit_w_day, unit_h_day);
    x = rect_unit_day.X + rect_unit_day.Width + gap_group;

    // 时
    c.rect_hours = Gdiplus::Rect(x, y, digit_w, digit_h);
    Gdiplus::Rect rect_unit_hrs(
        c.rect_hours.X + c.rect_hours.Width + gap_num_unit,
        y + (digit_h - unit_h_hrs) / 2,
        unit_w_hrs, unit_h_hrs);
    x = rect_unit_hrs.X + rect_unit_hrs.Width + gap_group;

    // 分
    c.rect_minutes = Gdiplus::Rect(x, y, digit_w, digit_h);
    Gdiplus::Rect rect_unit_min(
        c.rect_minutes.X + c.rect_minutes.Width + gap_num_unit,
        y + (digit_h - unit_h_min) / 2,
        unit_w_min, unit_h_min);
    x = rect_unit_min.X + rect_unit_min.Width + gap_group;

    // 秒
    c.rect_seconds = Gdiplus::Rect(x, y, digit_w, digit_h);
    Gdiplus::Rect rect_unit_sec(
        c.rect_seconds.X + c.rect_seconds.Width + gap_num_unit,
        y + (digit_h - unit_h_sec) / 2,
        unit_w_sec, unit_h_sec);
    x = rect_unit_sec.X + rect_unit_sec.Width + gap_group;

    // 毫秒
    //c.rect_millSeconds = Gdiplus::Rect(x, y, digit_w, digit_h);

    // 保存单位矩形
    c.vec_rect_units.clear();
    c.vec_rect_units.reserve(4);
    c.vec_rect_units.push_back(rect_unit_day);
    c.vec_rect_units.push_back(rect_unit_hrs);
    c.vec_rect_units.push_back(rect_unit_min);
    c.vec_rect_units.push_back(rect_unit_sec);
}

void Ui_VipPayDlg::EnableShadow()
{
    // 启用非客户区渲染
    DWORD dwPolicy = DWMNCRP_ENABLED;
    HRESULT hr = DwmSetWindowAttribute(this->GetSafeHwnd(), DWMWA_NCRENDERING_POLICY, &dwPolicy, sizeof(dwPolicy));
    if (FAILED(hr))
    {
        TRACE(_T("DwmSetWindowAttribute 调用失败, HRESULT=0x%x\n"), hr);
    }

    // 扩展窗口客户区以显示阴影，margins 可根据需求调整
    MARGINS margins = { 5, 5, 5, 5 };
    hr = DwmExtendFrameIntoClientArea(this->GetSafeHwnd(), &margins);
    if (FAILED(hr))
    {
        TRACE(_T("DwmExtendFrameIntoClientArea 调用失败, HRESULT=0x%x\n"), hr);
    }
}

void Ui_VipPayDlg::LoadRes()
{
    m_Bitmap_VipInfo = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIPDLG_PNG_VIPINFO),
        L"PNG");
    m_Bitmap_PermentVip = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIPDLG_PNG_PERMAMENTVIP),
        L"PNG");
    m_Bitmap_YearVip = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIPDLG_PNG_YEARVIP),
        L"PNG");
    m_Bitmap_HalfYear = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
        L"PNG");
    m_Bitmap_Sugbuy = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIPDLG_PNG_SUGESTIONBUY),
        L"PNG");
    m_Bitmap_CouponActivityInfo = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIPDLG_PNG_FRESHLOGO),
        L"PNG");
    m_Bitmap_SeasonVip = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(VIPPAY_PNG_SEASONBK),
        L"PNG");
    m_Bitmap_QRCodeBk = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIP_PNG_QRCODEBK),
        L"PNG");
    m_Bitmap_BuddleType1LowToday = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(VIPDLG_PNG_TYPE1BUDDLELOWTODAY),
        L"PNG");
    m_Bitmap_TitleLogoNewUser = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(VIPDLG_PNG_NEWUSERTITLELOGO),
        L"PNG");
    m_Bitmap_VipPrivilegeLower = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(OPENVIP_PNG_VIPPRIVILEGELOWER),
        L"PNG");
    m_Bitmap_LoginQRCodeBk = LARPNG::LoadPngFromResource(
        AfxGetInstanceHandle(),
        MAKEINTRESOURCE(VIPDLG_PNG_LOGINQRCODEBK),
        L"PNG");
}

void Ui_VipPayDlg::CleanUpRes()
{
    delete m_Bitmap_VipInfo;
    delete m_Bitmap_PermentVip;
    delete m_Bitmap_YearVip;
    delete m_Bitmap_HalfYear;
    m_Bitmap_VipInfo = nullptr;
    m_Bitmap_PermentVip = nullptr;
    m_Bitmap_YearVip = nullptr;
    m_Bitmap_HalfYear = nullptr;
    // 清理二维码图像
    if (m_Bitmap_QRCode)
    {
        delete m_Bitmap_QRCode;
        m_Bitmap_QRCode = nullptr;
    }

    // 释放“VIP特权对比”
    if (m_cusstat_vipPrivilegeContrast.txtB)
    {
        delete m_cusstat_vipPrivilegeContrast.txtB;
        m_cusstat_vipPrivilegeContrast.txtB = nullptr;
    }
    if (m_cusstat_vipPrivilegeContrast.txthovB)
    {
        delete m_cusstat_vipPrivilegeContrast.txthovB;
        m_cusstat_vipPrivilegeContrast.txthovB = nullptr;
    }
    if (m_cusstat_vipPrivilegeContrast.font)
    {
        delete m_cusstat_vipPrivilegeContrast.font;
        m_cusstat_vipPrivilegeContrast.font = nullptr;
    }
    m_cusstat_vipPrivilegeContrast.finalTxtB = nullptr;
}

float Ui_VipPayDlg::GetNumericPrice(const CString& priceText)
{
    CString numericOnly = priceText;
    for (int i = 0; i < numericOnly.GetLength(); i++)
    {
        if (!_istdigit(numericOnly[i]) && numericOnly[i] != _T('.'))
        {
            numericOnly.SetAt(i, _T(' '));
        }
    }
    numericOnly.Remove(_T(' '));
    return _ttof(numericOnly);
}

void Ui_VipPayDlg::CleanUpGdiRes()
{
    m_Btn_Close.ClearImages();		    // 关闭按钮
    m_Btn_PayWay.ClearImages();		    // 微信支付宝扫码支付
}

void Ui_VipPayDlg::Ui_SetWindowPos(const CRect& rect)
{
    m_CRect_WindowRect.left = rect.left;
    m_CRect_WindowRect.right = rect.right;
    m_CRect_WindowRect.top = rect.top;
    m_CRect_WindowRect.bottom = rect.bottom;
}

void Ui_VipPayDlg::Ui_UpdateWindowPos(const CRect& rect)
{
    m_CRect_WindowRect.left = rect.left;
    m_CRect_WindowRect.right = rect.right;
    m_CRect_WindowRect.top = rect.top;
    m_CRect_WindowRect.bottom = rect.bottom;
    MoveWindow(m_CRect_WindowRect.left, m_CRect_WindowRect.top, m_CRect_WindowRect.Width(), m_CRect_WindowRect.Height());
}

BEGIN_MESSAGE_MAP(Ui_VipPayDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_NCACTIVATE()
    ON_WM_NCHITTEST()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_SETCURSOR()
    ON_WM_MOVE()
    ON_WM_TIMER()

    ON_BN_CLICKED(OPENVIPDLG_BTN_CLOSE, &Ui_VipPayDlg::OnBnClickedBtnClose)
    ON_BN_CLICKED(VIPDLG_BTN_SUPTICKETANDSERVICE, &Ui_VipPayDlg::OnBnClickedBtnSupticketandservice)

    ON_MESSAGE(WM_APP + 1001, OnPaymentSuccessMsg)
    ON_MESSAGE(WM_APP + 1002, OnPaymentTimeoutMsg)
    ON_MESSAGE(WM_APP + 1003, OnStartPaymentPollingMsg)
    ON_MESSAGE(MSG_OPENVIPDLG_PAYMENTLOOPERROR, OnStopPaymentPollingAndQuit)
    ON_MESSAGE(MSG_VIPDLG_LOGINSHOW, OnLoginShow)
    ON_MESSAGE(MSG_VIPDLG_INREADPACKETMODE,OnInRedPacketMode)
    ON_MESSAGE(MSG_VIPDLG_UPDATESCALEWITHMODE,OnUpdateScaleWithMode)

    ON_WM_DESTROY()
END_MESSAGE_MAP()

// Ui_VipPayDlg 消息处理程序

BOOL Ui_VipPayDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    App.UserOpenVip(false);
    try
    {
        GetUserDpi();
        // 初始化控件和布局
        UpdateScale();
        InitCtrl();
        InitCountDownOfferExpiration();
        LoadRes();

        //初始化“优惠倒计时”为7天
        {
            using namespace std::chrono;
            if (!g_offerExpireAtInited)
            {
                g_offerExpireAt = system_clock::now() + hours(24 * 24);
                g_offerExpireAtInited = true;
            }
            m_offerExpireAt = g_offerExpireAt;
        }

        //设置窗口双缓冲
        ModifyStyleEx(0, WS_EX_COMPOSITED);
        std::thread([this]()
            {
                App.RequestPrice();

                // 拷贝 App 中的价格数据到本地成员
                m_PriceInfos.clear();
                for (const auto& p : App.m_PriceInfos)
                {
                    PriceInfo x;
                    x.id = p.id;
                    x.name = p.name;
                    x.badge = p.badge;
                    x.amount = p.amount;
                    x.desc = p.desc;
                    m_PriceInfos.push_back(x);
                }

                // 预订单号与月度会员价格
                m_PreOrderNo = CString(App.m_preOrderNo.c_str());
                m_MouthBillPrice = App.m_MouthBillPrice;
                m_RedPacketCouponAmount = App.m_ReadPacketAmount;

                //测试补丁(红包模式)
                //m_MouthBillPrice = -1;
                //m_RedPacketCouponAmount = 5;

                //测试补丁(低价格模式)
                //m_MouthBillPrice = 28;
                //m_RedPacketCouponAmount = 0;

                //判断模式
                if (m_MouthBillPrice == -1 && m_RedPacketCouponAmount > 0)
                    m_Mode = Mode::RedPacketCoupon;
                else if (m_MouthBillPrice != -1 && m_RedPacketCouponAmount == 0)
                    m_Mode == Mode::LowPriceWithMouthBill;

                // 刷新价格显示与加载二维码
                UpdatePriceDisplay();
                LoadQRCode();

                // 刷新优惠倒计数
                UpdateOfferCountdownDisplay();
                m_timerIdOfferCountdown = SetTimer(VIPDLG_TIMER_COUNTDOWNOFFEREXPIRATION, 1000, NULL);

                // 标记初始化完成与设置默认选中套餐
                m_bool_IsInitReady = true;
                SetSelectedVipType(0);

                if (!App.m_isLoginIn && !App.m_CanGuestBuy)
                    ::PostMessage(m_hWnd, MSG_VIPDLG_LOGINSHOW, NULL, NULL);

                //发送消息，更新对应模式的显示布局
                ::PostMessage(m_hWnd, MSG_VIPDLG_UPDATESCALEWITHMODE, NULL, NULL);
                //if (m_isHasRedPacketCoupon)
                //    ::PostMessage(m_hWnd, MSG_VIPDLG_INREADPACKETMODE, NULL, NULL);
                
            }).detach();
    }
    catch (std::exception& e)
    {
        std::wstring wideError = LARSC::s2ws(e.what());
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"初始化对话框时发生异常: %s", wideError.c_str());
    }

    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    App.UserOpenVip(true);
    App.UserPay(false);
    return TRUE;
}

void Ui_VipPayDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类
    //CDialogEx::OnOK();
}

void Ui_VipPayDlg::OnCancel()
{
    // 停止轮询
    //m_PaymentPolling.stopPolling();
    //CDialogEx::OnCancel();
}

void Ui_VipPayDlg::OnPaint()
{
    CPaintDC dc(this);
    m_Shadow.Show(m_hWnd);

    //预缓冲Gdiplus对象
    using namespace Gdiplus;
    static Bitmap* memBitmap = new Bitmap(m_CRect_WindowRect.Width(), m_CRect_WindowRect.Height());
    Graphics memGraphics(memBitmap);

    //设置最高质量绘画
    memGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    memGraphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    memGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    //memGraphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
    memGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    //渲染标题栏背景和绘画客户区背景
    SolidBrush CaptionBrush(Color(16, 23, 24));
    SolidBrush ClientBrush(Color(31, 36, 37));
    memGraphics.FillRectangle(&CaptionBrush, m_Rect_Caption);//绘画标题栏背景
    memGraphics.FillRectangle(//绘画客户区背景
        &ClientBrush,
        0,
        m_Rect_Caption.GetBottom(),
        m_CRect_WindowRect.Width(),
        m_CRect_WindowRect.Height()
    );

    if (!m_bool_IsInitReady.load())
    {
        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentCenter);
        fmt.SetLineAlignment(StringAlignmentCenter);
        fmt.SetFormatFlags(StringFormatFlagsNoWrap);

        SolidBrush textBrush(Color(255, 250, 210, 158));
        Gdiplus::Font font(L"微软雅黑", 40.0f * m_Scale, FontStyleBold, UnitPixel);
        RectF textRect(0.0f, 0.0f, (REAL)m_CRect_WindowRect.Width(), (REAL)m_CRect_WindowRect.Height());
        memGraphics.DrawString(L"正在加载中...", -1, &font, textRect, &fmt, &textBrush);


        Graphics graphice(dc.GetSafeHdc());
        graphice.DrawImage(memBitmap, 0, 0,
            static_cast<INT>(m_CRect_WindowRect.Width()), static_cast<INT>(m_CRect_WindowRect.Height()));
        return;
    }

    //  VIP 信息 Logo绘画
    memGraphics.DrawImage(
        App.m_IsNeedShowMouthBill ? m_Bitmap_VipPrivilegeLower : m_Bitmap_VipInfo,
        m_Rect_VipInfoRect
    );

    // 渲染VIP选项，并添加选中/悬停效果
    memGraphics.DrawImage(m_Bitmap_PermentVip, m_Rect_PermentVip);
    memGraphics.DrawImage(m_Bitmap_YearVip, m_Rect_YearVip);
    memGraphics.DrawImage(m_Bitmap_HalfYear, m_Rect_HalfYear);
    memGraphics.DrawImage(m_Bitmap_SeasonVip, m_Rect_SeasonVip);

    // 渲染热卖推荐和新人立享背景气泡
    //memGraphics.DrawImage(m_Bitmap_Sugbuy, m_Rect_Sugbuy);
    memGraphics.DrawImage(m_Bitmap_CouponActivityInfo, m_Rect_CouponActivityInfo);

    // 渲染二维码蒙版
    memGraphics.DrawImage(m_Bitmap_QRCodeBk, m_Rect_QRCodeBK);

    // 渲染选中效果
    Pen selectedPen(Color(255, 255, 165, 0), 2); // 橙色边框
    Pen hoverPen(Color(128, 255, 255, 255), 1);  // 半透明白色边框

    // 渲染选中效果
    //switch (m_SelectedVipType)
    //{
    //case 0:
    //    memGraphics.DrawRectangle(&selectedPen, m_Rect_PermentVip);
    //    break;
    //case 1:
    //    memGraphics.DrawRectangle(&selectedPen, m_Rect_YearVip);
    //    break;
    //case 2:
    //    memGraphics.DrawRectangle(&selectedPen, m_Rect_HalfYear);
    //    break;
    //}

    // 渲染悬停效果
    if (m_HoverVipType != m_SelectedVipType && m_HoverVipType >= 0)
    {
        switch (m_HoverVipType)
        {
        case 0:
            memGraphics.DrawRectangle(&hoverPen, m_Rect_PermentVip);
            break;
        case 1:
            memGraphics.DrawRectangle(&hoverPen, m_Rect_YearVip);
            break;
        case 2:
            memGraphics.DrawRectangle(&hoverPen, m_Rect_HalfYear);
            break;
        }
    }

    // 渲染二维码或状态信息
    if (App.m_isLoginIn || App.m_CanGuestBuy)
        DrawQRCodeOrStatus(&memGraphics);
    else
        memGraphics.DrawImage(m_Bitmap_LoginQRCodeBk, m_Rect_QRCode);

    //低价格模式
    auto& c = m_countDownOfferExpiration;
    if (c.font_days && c.font_hours && c.font_minute && c.font_seconds &&
        c.font_unit && c.sb_days && c.sb_hours && c.sb_minutes && c.sb_seconds && c.sb_unit)
    {
        // 开关：隐藏“天” (及其数字) 但保持布局占位
        bool kHideDays = true;

        StringFormat fmtNum;
        fmtNum.SetAlignment(StringAlignmentNear);
        fmtNum.SetLineAlignment(StringAlignmentCenter);
        fmtNum.SetFormatFlags(StringFormatFlagsNoWrap);

        StringFormat fmtUnit;
        fmtUnit.SetAlignment(StringAlignmentNear);
        fmtUnit.SetLineAlignment(StringAlignmentCenter);
        fmtUnit.SetFormatFlags(StringFormatFlagsNoWrap);

        RectF r;

        // 天（若隐藏则不绘制，保留空白区域以维持原布局）
        if (!kHideDays)
        {
            r = RectF((REAL)c.rect_days.X, (REAL)c.rect_days.Y, (REAL)c.rect_days.Width, (REAL)c.rect_days.Height);
            memGraphics.DrawString(c.wstr_days.c_str(), -1, c.font_days, r, &fmtNum, c.sb_days);
        }

        // 时
        r = RectF((REAL)c.rect_hours.X, (REAL)c.rect_hours.Y, (REAL)c.rect_hours.Width, (REAL)c.rect_hours.Height);
        memGraphics.DrawString(c.wstr_hours.c_str(), -1, c.font_hours, r, &fmtNum, c.sb_hours);

        // 分
        r = RectF((REAL)c.rect_minutes.X, (REAL)c.rect_minutes.Y, (REAL)c.rect_minutes.Width, (REAL)c.rect_minutes.Height);
        memGraphics.DrawString(c.wstr_minute.c_str(), -1, c.font_minute, r, &fmtNum, c.sb_minutes);

        // 秒
        r = RectF((REAL)c.rect_seconds.X, (REAL)c.rect_seconds.Y, (REAL)c.rect_seconds.Width, (REAL)c.rect_seconds.Height);
        memGraphics.DrawString(c.wstr_seconds.c_str(), -1, c.font_seconds, r, &fmtNum, c.sb_seconds);

        //毫秒
        //r = RectF((REAL)c.rect_millSeconds.X, (REAL)c.rect_millSeconds.Y, (REAL)c.rect_millSeconds.Width, (REAL)
        //c.rect_millSeconds.Height);
        //memGraphics.DrawString(c.wstr_millSeconds.c_str(), -1, c.font_millSeconds, r, &fmtNum, c.sb_millSeconds);

        // 单位
        const wchar_t* units[4] = { L"天", L"时", L"分", L"秒" };
        const size_t n = std::min<size_t>(4, c.vec_rect_units.size());
        for (size_t i = 0; i < n; ++i)
        {
            if (kHideDays && i == 0)
                continue; // 跳过绘制“天”
            const Gdiplus::Rect& ur = c.vec_rect_units[i];
            RectF urf((REAL)ur.X, (REAL)ur.Y, (REAL)ur.Width, (REAL)ur.Height);
            memGraphics.DrawString(units[i], 1, c.font_unit, urf, &fmtUnit, c.sb_unit);
        }
    }
    

    // 渲染“VIP特权对比”
    if (m_cusstat_vipPrivilegeContrast.font &&
        m_cusstat_vipPrivilegeContrast.finalTxtB &&
        !m_cusstat_vipPrivilegeContrast.str.empty())
    {
        StringFormat fmt;
        fmt.SetAlignment(StringAlignmentCenter);       // 水平居中
        fmt.SetLineAlignment(StringAlignmentCenter);   // 垂直居中
        fmt.SetFormatFlags(StringFormatFlagsNoWrap);   // 不换行

        RectF textRect(
            static_cast<REAL>(m_cusstat_vipPrivilegeContrast.txtR.X),
            static_cast<REAL>(m_cusstat_vipPrivilegeContrast.txtR.Y),
            static_cast<REAL>(m_cusstat_vipPrivilegeContrast.txtR.Width),
            static_cast<REAL>(m_cusstat_vipPrivilegeContrast.txtR.Height)
        );

        memGraphics.DrawString(
            m_cusstat_vipPrivilegeContrast.str.c_str(),
            -1,
            m_cusstat_vipPrivilegeContrast.font,
            textRect,
            &fmt,
            m_cusstat_vipPrivilegeContrast.finalTxtB
        );
    }

    // 渲染Type1套餐中右上方小气泡
    memGraphics.DrawImage(m_Bitmap_BuddleType1LowToday, m_Rect_BuddleType1LowToday);
    memGraphics.DrawImage(m_Bitmap_TitleLogoNewUser, m_Rect_TitleLogoNewUser);

    if (m_bool_IsDarker.load())
    {
        Gdiplus::SolidBrush brush(Gdiplus::Color(150, 0, 0, 0));
        memGraphics.FillRectangle(&brush, 0, 0, m_CRect_WindowRect.Width(), m_CRect_WindowRect.Height());
    }

    // 一次性绘画到窗口上
    Graphics graphice(dc.GetSafeHdc());
    graphice.DrawImage(memBitmap, 0, 0,
        static_cast<INT>(m_CRect_WindowRect.Width()), static_cast<INT>(m_CRect_WindowRect.Height()));

    DB(ConsoleHandle, L"Ui_VipPayDlg:repaint..");
}

BOOL Ui_VipPayDlg::OnNcActivate(BOOL bActive)
{
    return CDialogEx::OnNcActivate(bActive);
}

LRESULT Ui_VipPayDlg::OnNcHitTest(CPoint point)
{
    POINT clientPt = point;
    ScreenToClient(&clientPt);
    if (m_Rect_Caption.Contains(clientPt.x, clientPt.y))
        return HTCAPTION;
    return CDialogEx::OnNcHitTest(point);
}

BOOL Ui_VipPayDlg::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

bool Ui_VipPayDlg::IsPointInRect(const CPoint& point, const Gdiplus::Rect& rect)
{
    return (point.x >= rect.X && point.x <= rect.X + rect.Width &&
        point.y >= rect.Y && point.y <= rect.Y + rect.Height);
}

void Ui_VipPayDlg::SetSelectedVipType(int type)
{
    // 如果选择了不同的类型，需要重新加载二维码
    bool typeChanged = (m_SelectedVipType != type);

    m_SelectedVipType = type;
    switch (type)
    {
    case 0:
    {
        CString PriceText = m_Stat_PermentVip.LarGetText();
        m_Stat_Price.LarSetText(PriceText);
        CString type1OriPrice, OriPriceStr;
        m_stat_type1OriPrice.GetWindowTextW(type1OriPrice);
        std::wstring type1OriPricewstr(type1OriPrice);
        type1OriPricewstr = type1OriPricewstr.substr(type1OriPricewstr.find_last_of(L'：') + 1);
        OriPriceStr.Format(L"原价%s元", type1OriPricewstr.c_str());
        m_Stat_OriPrice.LarSetText(OriPriceStr);

        //更新新人特权优惠文本
        int oriPrice = _wtoi(type1OriPricewstr.c_str());
        CString type1PriceCStr = m_Stat_PermentVip.LarGetText();
        std::wstring type1PriceStr(type1PriceCStr);
        int type1Price = _wtoi(type1PriceStr.c_str());
        type1Price = oriPrice - type1Price;
        std::wstring CouponActivityInfoPriceStr = L"新人特权优惠:" + std::to_wstring(type1Price) + L"元";
        m_stat_CouponActivityInfo.SetWindowTextW(CouponActivityInfoPriceStr.c_str());
    }
    break;
    case 1: // 年度会员
    {
        CString PriceText = m_Stat_YearAmount.LarGetText();
        m_Stat_Price.LarSetText(PriceText);
        CString type2OriPrice, OriPriceStr;
        m_stat_type2OriPrice.GetWindowTextW(type2OriPrice);
        std::wstring type2OriPricewstr(type2OriPrice);
        type2OriPricewstr = type2OriPricewstr.substr(type2OriPricewstr.find_last_of(L'：') + 1);
        OriPriceStr.Format(L"原价%s元", type2OriPricewstr.c_str());
        m_Stat_OriPrice.LarSetText(OriPriceStr);

        //更新新人特权优惠文本
        int oriPrice = _wtoi(type2OriPricewstr.c_str());
        CString type2PriceCStr = m_Stat_YearAmount.LarGetText();
        std::wstring type2PriceStr(type2PriceCStr);
        int type2Price = _wtoi(type2PriceStr.c_str());
        type2Price = oriPrice - type2Price;
        std::wstring CouponActivityInfoPriceStr = L"新人特权优惠:" + std::to_wstring(type2Price) + L"元";
        m_stat_CouponActivityInfo.SetWindowTextW(CouponActivityInfoPriceStr.c_str());
    }
    break;
    case 2: // 半年会员
    {
        CString PriceText = m_Stat_HalfYearAmount.LarGetText();
        m_Stat_Price.LarSetText(PriceText);
        CString type3OriPrice, OriPriceStr;
        m_stat_Type3OriPrice.GetWindowTextW(type3OriPrice);
        std::wstring type3OriPricewstr(type3OriPrice);
        type3OriPricewstr = type3OriPricewstr.substr(type3OriPricewstr.find_last_of(L'：') + 1);
        OriPriceStr.Format(L"原价%s元", type3OriPricewstr.c_str());
        m_Stat_OriPrice.LarSetText(OriPriceStr);

        //更新新人特权优惠文本
        int oriPrice = _wtoi(type3OriPricewstr.c_str());
        CString type3PriceCStr = m_Stat_HalfYearAmount.LarGetText();
        std::wstring type3PriceStr(type3PriceCStr);
        int type3Price = _wtoi(type3PriceStr.c_str());
        type3Price = oriPrice - type3Price;
        std::wstring CouponActivityInfoPriceStr = L"新人特权优惠:" + std::to_wstring(type3Price) + L"元";
        m_stat_CouponActivityInfo.SetWindowTextW(CouponActivityInfoPriceStr.c_str());

    }
    break;
    case 3: // 季度会员 
    {
        CString PriceText = m_Stat_SeasonAMount.LarGetText();
        m_Stat_Price.LarSetText(PriceText);
        CString type4OriPrice, OriPriceStr;
        m_stat_type4OriPrice.GetWindowTextW(type4OriPrice);
        std::wstring type4OriPricewstr(type4OriPrice);
        type4OriPricewstr = type4OriPricewstr.substr(type4OriPricewstr.find_last_of(L'：') + 1);
        OriPriceStr.Format(L"原价%s元", type4OriPricewstr.c_str());
        m_Stat_OriPrice.LarSetText(OriPriceStr);
        m_Stat_OriPrice.LarSetText(OriPriceStr);

        int oriPrice = _wtoi(type4OriPricewstr.c_str());
        CString type4PriceCStr = m_Stat_SeasonAMount.LarGetText();
        std::wstring type4PriceStr(type4PriceCStr);
        int type4Price = _wtoi(type4PriceStr.c_str());
        type4Price = oriPrice - type4Price;
        std::wstring CouponActivityInfoPriceStr = L"新人特权优惠:" + std::to_wstring(type4Price) + L"元";
        m_stat_CouponActivityInfo.SetWindowTextW(CouponActivityInfoPriceStr.c_str());
    }
    break;
    default:
        break;
    }

    // 如果类型发生了变化，停止当前轮询，并重新加载二维码
    std::thread([=]()
        {
            m_QRCodeStatus = QR_INITIAL;
            if (typeChanged && !m_PriceInfos.empty() && !m_PreOrderNo.IsEmpty())
            {
                m_PaymentPolling.stopPolling();  // 停止轮询
                RefreshQRCode();// 加载新的二维码
            }
        }).detach();


    Invalidate(FALSE);// 刷新显示
}

void Ui_VipPayDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_QRCodeStatus == QR_INITIAL || m_QRCodeStatus == QR_LOADING)
        return;

    // 检查点击是否在 VIP 区域内
    if (IsPointInRect(point, m_Rect_PermentVip) && (m_SelectedVipType != 0))
    {
        ReloadBitmapBySelect(0);
        SetMemberTypeStat_SelectColor(0);
        SetSelectedVipType(0); // 永久会员
        if(m_isHasRedPacketCoupon)
            UpdatePriceNewUserCouponInfo(m_timeLest);

        auto mealInfo = m_PriceInfos[0];
        App.UserClickMeal(mealInfo.id, mealInfo.name, mealInfo.amount);
    }
    else if (IsPointInRect(point, m_Rect_YearVip) && (m_SelectedVipType != 1))
    {
        ReloadBitmapBySelect(1);
        SetMemberTypeStat_SelectColor(1);
        SetSelectedVipType(1); // 年度会员
        if (m_isHasRedPacketCoupon)
            UpdatePriceNewUserCouponInfo(m_timeLest);

        auto mealInfo = m_PriceInfos[1];
        App.UserClickMeal(mealInfo.id, mealInfo.name, mealInfo.amount);
    }
    else if (IsPointInRect(point, m_Rect_HalfYear) && (m_SelectedVipType != 2))
    {
        ReloadBitmapBySelect(2);
        SetMemberTypeStat_SelectColor(2);
        SetSelectedVipType(2); // 半年会员
        if (m_isHasRedPacketCoupon)
            UpdatePriceNewUserCouponInfo(m_timeLest);

        auto mealInfo = m_PriceInfos[2];
        App.UserClickMeal(mealInfo.id, mealInfo.name, mealInfo.amount);
    }
    else if (IsPointInRect(point, m_Rect_SeasonVip) && (m_SelectedVipType != 3))
    {
        ReloadBitmapBySelect(3);
        SetMemberTypeStat_SelectColor(3);
        SetSelectedVipType(3);
        if (m_isHasRedPacketCoupon)
            UpdatePriceNewUserCouponInfo(m_timeLest);

        auto mealInfo = m_PriceInfos[3];
        App.UserClickMeal(mealInfo.id, mealInfo.name, mealInfo.amount);
    }

    //if (App.m_IsNeedShowMouthBill && IsPointInRect(point, m_Rect_MouthBill))
    //{
    //    if (!App.m_isLoginIn)
    //    {
    //        ::SendMessage(m_hWnd, MSG_VIPDLG_LOGINSHOW, NULL, NULL);
    //        if (!App.m_isLoginIn)return;
    //    }
    //
    //    m_PaymentPolling.stopPolling();
    //    ::SetWindowPos(this->GetSafeHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //    SetNormalCtrlUiAlpha();
    //    Invalidate(false);
    //    UpdateWindow();
    //    if (ModalDlg_MFC::ShowModal_MouthMemberDlg(this) == IDOK)
    //    {
    //        EndDialog(IDOK);
    //        return;
    //    }
    //    ::SetWindowPos(this->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    //    ::PostMessage(this->GetSafeHwnd(), WM_APP + 1003, NULL, NULL); // 恢复轮询
    //    SetDarkCtrlUi();
    //    Invalidate(false);
    //    return;
    //}

    // 检查点击是否在二维码区域if(!App.m_isLoginIn && m_Rect_QRCode.Contains(gdiPoint))
    if (App.m_isLoginIn || App.m_CanGuestBuy)
    {
        if (IsPointInQRCodeArea(point) && m_QRCodeStatus == QR_TIMEOUT)
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"点击了超时的二维码区域，刷新二维码");
            RefreshQRCode();
        }
    }
    else
    {
        if (IsPointInQRCodeArea(point))
        {
            ::PostMessage(m_hWnd, MSG_VIPDLG_LOGINSHOW, NULL, NULL);
        }
    }

    //检查是否点击权益窗口
    if (IsPointInRect(point, m_cusstat_vipPrivilegeContrast.btnArea))
    {
        if (m_cusstat_vipPrivilegeContrast.clickCallBack)
            m_cusstat_vipPrivilegeContrast.clickCallBack();
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_VipPayDlg::ReloadBitmapBySelect(int type)
{
    if (type == 0)
    {
        if (m_Bitmap_HalfYear)
            delete m_Bitmap_HalfYear;
        if (m_Bitmap_PermentVip)
            delete m_Bitmap_PermentVip;
        if (m_Bitmap_YearVip)
            delete m_Bitmap_YearVip;
        if (m_Bitmap_SeasonVip)
            delete m_Bitmap_SeasonVip;

        m_Bitmap_PermentVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_PERMAMENTVIP),
            L"PNG");
        m_Bitmap_YearVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_YEARVIP),
            L"PNG");
        m_Bitmap_HalfYear = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
        m_Bitmap_SeasonVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
    }
    else if (type == 1)
    {
        if (m_Bitmap_HalfYear)
            delete m_Bitmap_HalfYear;
        if (m_Bitmap_PermentVip)
            delete m_Bitmap_PermentVip;
        if (m_Bitmap_YearVip)
            delete m_Bitmap_YearVip;
        if (m_Bitmap_SeasonVip)
            delete m_Bitmap_SeasonVip;

        m_Bitmap_PermentVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_YEARVIP),
            L"PNG");
        m_Bitmap_YearVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_PERMAMENTVIP),
            L"PNG");
        m_Bitmap_HalfYear = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
        m_Bitmap_SeasonVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
    }
    else if (type == 2)
    {
        if (m_Bitmap_HalfYear)
            delete m_Bitmap_HalfYear;
        if (m_Bitmap_PermentVip)
            delete m_Bitmap_PermentVip;
        if (m_Bitmap_YearVip)
            delete m_Bitmap_YearVip;
        if (m_Bitmap_SeasonVip)
            delete m_Bitmap_SeasonVip;

        m_Bitmap_PermentVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_YEARVIP),
            L"PNG");
        m_Bitmap_YearVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
        m_Bitmap_HalfYear = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_PERMAMENTVIP),
            L"PNG");
        m_Bitmap_SeasonVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
    }
    else if (type == 3)
    {
        if (m_Bitmap_HalfYear)
            delete m_Bitmap_HalfYear;
        if (m_Bitmap_PermentVip)
            delete m_Bitmap_PermentVip;
        if (m_Bitmap_YearVip)
            delete m_Bitmap_YearVip;
        if (m_Bitmap_SeasonVip)
            delete m_Bitmap_SeasonVip;

        m_Bitmap_PermentVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_YEARVIP),
            L"PNG");
        m_Bitmap_YearVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
        m_Bitmap_HalfYear = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_HALFYEARVIP),
            L"PNG");
        m_Bitmap_SeasonVip = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_PERMAMENTVIP),
            L"PNG");
    }

    if (type != 0)
    {
        m_Bitmap_Sugbuy = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_SUGSTIONBUY_BLACK),
            L"PNG");
    }
    else
    {
        m_Bitmap_Sugbuy = LARPNG::LoadPngFromResource(
            AfxGetInstanceHandle(),
            MAKEINTRESOURCE(OPENVIPDLG_PNG_SUGESTIONBUY),
            L"PNG");
    }
    Invalidate();
}

void Ui_VipPayDlg::SetMemberTypeStat_SelectColor(int type)
{
    if (type == 0)
    {
        m_stat_type1LowToDay.LarSetTextColor(RGB(68, 39, 4));
        m_stat_type2LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type3LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type4LowToDay.LarSetTextColor(RGB(115, 115, 118));
    }
    else if (type == 1)
    {
        m_stat_type1LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type2LowToDay.LarSetTextColor(RGB(68, 39, 4));
        m_stat_type3LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type4LowToDay.LarSetTextColor(RGB(115, 115, 118));
    }
    else if (type == 2)
    {
        m_stat_type1LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type2LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type3LowToDay.LarSetTextColor(RGB(68, 39, 4));
        m_stat_type4LowToDay.LarSetTextColor(RGB(115, 115, 118));
    }
    else if (type == 3)
    {
        m_stat_type1LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type2LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type3LowToDay.LarSetTextColor(RGB(115, 115, 118));
        m_stat_type4LowToDay.LarSetTextColor(RGB(68, 39, 4));
    }
    Invalidate();
}

void Ui_VipPayDlg::SetLowToDayStatNormalStyle(CLazerStaticText* stat)
{
    stat->LarSetTextColor(RGB(115, 115, 118));
    stat->LarSetTextSize(16);
    stat->LarSetTextCenter();
}

void Ui_VipPayDlg::SetLowToDayStatSelectStyle(CLazerStaticText* stat)
{
    stat->LarSetTextColor(RGB(68, 39, 4));
    stat->LarSetTextSize(16);
    stat->LarSetTextCenter();
}

void Ui_VipPayDlg::SetRmbLogoStatStyle(CLazerStaticText* stat)
{
    stat->LarSetTextColor(RGB(250, 210, 158));
    stat->LarSetTextSize(16);
    stat->LarSetTextCenter();
}

void Ui_VipPayDlg::OnMouseMove(UINT nFlags, CPoint point)
{
    //{//原来的处理逻辑：改变光标的同时增加三个套餐按钮的悬停效果
    //    int oldHoverType = m_HoverVipType;// 保存旧的悬停状态
    //
    //    // 更新悬停状态
    //    if (IsPointInRect(point, m_Rect_PermentVip))
    //    {
    //        m_HoverVipType = 0;
    //    }
    //    else if (IsPointInRect(point, m_Rect_YearVip))
    //    {
    //        m_HoverVipType = 1;
    //    }
    //    else if (IsPointInRect(point, m_Rect_HalfYear))
    //    {
    //        m_HoverVipType = 2;
    //    }
    //    else if (IsPointInQRCodeArea(point) && m_QRCodeStatus == QR_TIMEOUT)
    //    {
    //        m_HoverVipType = 3;
    //    }
    //    else
    //    {
    //        m_HoverVipType = -1;
    //    }
    //    // 如果悬停状态变化，刷新光标
    //    if (oldHoverType != m_HoverVipType)
    //    {
    //        SetCursor(m_HoverVipType >= 0 ? LoadCursor(NULL, IDC_HAND) : LoadCursor(NULL, IDC_ARROW));
    //        Invalidate(false);
    //    }
    //}

    CDialogEx::OnMouseMove(nFlags, point);
    //权益对比文本按钮
    {
        Gdiplus::Point gpt(point.x, point.y);
        bool inBtn = m_cusstat_vipPrivilegeContrast.btnArea.Contains(gpt);
        if (inBtn != m_bVipPrivilegeHover)
        {
            m_bVipPrivilegeHover = inBtn;
            // 根据悬停状态切换最终文本画刷
            m_cusstat_vipPrivilegeContrast.finalTxtB =
                m_bVipPrivilegeHover ? m_cusstat_vipPrivilegeContrast.txthovB
                : m_cusstat_vipPrivilegeContrast.txtB;

            // 仅repaint按钮区域，提升性能
            CRect r(
                m_cusstat_vipPrivilegeContrast.btnArea.X,
                m_cusstat_vipPrivilegeContrast.btnArea.Y,
                m_cusstat_vipPrivilegeContrast.btnArea.X + m_cusstat_vipPrivilegeContrast.btnArea.Width,
                m_cusstat_vipPrivilegeContrast.btnArea.Y + m_cusstat_vipPrivilegeContrast.btnArea.Height
            );
            InvalidateRect(&r, FALSE);
        }
    }
}

BOOL Ui_VipPayDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    // 如果鼠标在 VIP 区域上，显示手型光标
    //if (m_HoverVipType >= 0)
    //{
    //    SetCursor(LoadCursor(NULL, IDC_HAND));
    //    return TRUE;
    //}
    if (!m_Dlg_Login)
    {
        // 获取当前鼠标位置
        CPoint point;
        GetCursorPos(&point);
        ScreenToClient(&point);

        // 转换为 Gdiplus::Point
        Gdiplus::Point gdiPoint(point.x, point.y);

        // 检查鼠标是否在三个矩形区域内
        if (m_Rect_PermentVip.Contains(gdiPoint) ||
            m_Rect_YearVip.Contains(gdiPoint) ||
            m_Rect_HalfYear.Contains(gdiPoint) ||
            m_Rect_SeasonVip.Contains(gdiPoint) ||
            m_cusstat_vipPrivilegeContrast.btnArea.Contains(gdiPoint) 
            //||(App.m_IsNeedShowMouthBill && m_Rect_MouthBill.Contains(gdiPoint))
            )
        {
            // 设置手型光标
            ::SetCursor(::LoadCursor(NULL, IDC_HAND));
            return TRUE;
        }

        //检查鼠标是否在二维码区域内（未登录情况下）
        if (!App.m_isLoginIn && !App.m_CanGuestBuy && m_Rect_QRCode.Contains(gdiPoint))
        {
            // 设置手型光标
            ::SetCursor(::LoadCursor(NULL, IDC_HAND));
            return TRUE;
        }
    }

    return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void Ui_VipPayDlg::OnBnClickedBtnClose()
{
    //如果二维码正在加载，那么不关闭窗口（为解决debug assetion，我临时这么写，后面需要用线程通信更加优雅的处理这个情况）
    if (m_QRCodeStatus == QR_LOADING)
        return;

    ::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);          //先取消置顶（保证新窗口覆盖当前窗口）

    //在这里立即更新显示变暗的背景图
    {
        SetNormalCtrlUiAlpha();
        m_PaymentPolling.pausePolling();
        Invalidate(false);
        UpdateWindow();
    }
    if (m_Mode == Mode::RedPacketCoupon &&
        m_RedPacketCouponAmount != 0 && m_RedPacketCouponAmount > 0 &&
        !m_isHasRedPacketCoupon && !App.m_IsVip &&
        ModalDlg_SDL::ShowModal_RedPacket(
            m_hWnd, m_Scale, m_RedPacketCouponAmount, m_timeLest, m_InitialTimeLest
        ) == 1)
    {
        //调整文本区域
        m_Rect_CouponActivityInfo.Width = 178 * m_Scale;
        m_Rect_CouponActivityInfo.Height = 26.25 * m_Scale;
        m_Rect_CouponActivityInfo.X = 571 * m_Scale;
        m_Rect_CouponActivityInfo.Y = 389 * m_Scale;
        {
            float CaWidth = m_Rect_CouponActivityInfo.Width;
            float CaHeight = m_Rect_CouponActivityInfo.Height;
            float CaX = m_Rect_CouponActivityInfo.X;
            float CaY = m_Rect_CouponActivityInfo.Y;
            m_stat_CouponActivityInfo.MoveWindow(
                CaX,
                CaY,
                CaWidth,
                CaHeight
            );
        }

        //启动定时器
        KillTimer(VIPDLG_TIMER_REDPACKETTIMEUPDATE);
        SetTimer(VIPDLG_TIMER_REDPACKETTIMEUPDATE, 1000, NULL);
        UpdatePriceNewUserCouponInfo(m_timeLest);
        m_isHasRedPacketCoupon = true;
        SetDarkCtrlUi();
        m_PaymentPolling.stopPolling();
        m_urlCouponAmount = "&coupon_amount=" + std::to_string(m_RedPacketCouponAmount);
        LoadQRCode();
        Invalidate(false);
        return;
    }
    //(!App.m_IsVip && !App.m_IsNonUserPaid && !App.m_IsHasMouthBillWindowShowed)
    else if (m_Mode == Mode::LowPriceWithMouthBill &&
        m_MouthBillPrice != -1 && m_MouthBillPrice >= 0 && 
        (!App.m_IsVip && !App.m_IsNonUserPaid && !App.m_IsHasMouthBillWindowShowed))
    {
        App.m_IsHasMouthBillWindowShowed = true;
        App.UserOpenMouthBiil();
        if (ModalDlg_MFC::ShowModal_MouthMemberDlg(this) == IDOK)
        {
            App.UserPay(true);
            EndDialog(IDOK);
            return;
        }
        else
        {
            App.UserCloseMouthBill();
        }
    }
    App.m_IsHasOpenVip = true;
    if ((!App.m_IsNonUserPaid && !App.m_IsVip && 
        App.m_IsHasMouthBillWindowShowed && m_Mode == Mode::LowPriceWithMouthBill))
        App.m_IsNeedShowMouthBill = true;
    else
        App.m_IsNeedShowMouthBill = false;
    m_PaymentPolling.stopPolling();
    EndDialog(IDCANCEL);
    m_Shadow.Show(m_hWnd);

}

void Ui_VipPayDlg::UpdatePriceNewUserCouponInfo(std::chrono::milliseconds lestTime)
{
    // 更新支付金额
    if (!m_isHasRedPacketCoupon)
    {
        CString PriceStr;
        switch (m_SelectedVipType)
        {
        case 0:
            PriceStr = m_Stat_PermentVip.LarGetText();
            break;
        case 1:
            PriceStr = m_Stat_YearAmount.LarGetText();
            break;
        case 2:
            PriceStr = m_Stat_HalfYearAmount.LarGetText();
            break;
        case 3:
            PriceStr = m_Stat_SeasonAMount.LarGetText();
            break;
        }
        std::wstring wstr_PriceStr = PriceStr;
        int price = _wtoi(wstr_PriceStr.c_str());
        price -= m_RedPacketCouponAmount;
        std::wstring newPrice = std::to_wstring(price) + L".00";
        m_Stat_Price.LarSetText(newPrice.c_str());

        // 解析并更新新人特权优惠
        int CouponActivityInfoPrice;
        {
            int oriprice;
            CString oripriceStr;
            switch (m_SelectedVipType)
            {
            case 0:
                m_stat_type1OriPrice.GetWindowTextW(oripriceStr);
                break;
            case 1:
                m_stat_type2OriPrice.GetWindowTextW(oripriceStr);
                break;
            case 2:
                m_stat_Type3OriPrice.GetWindowTextW(oripriceStr);
                break;
            case 3:
                m_stat_type4OriPrice.GetWindowTextW(oripriceStr);
                break;
            }
            std::wstring wstr_oripriceStr = oripriceStr;
            int pos1 = wstr_oripriceStr.find_last_of(L"：");
            wstr_oripriceStr = wstr_oripriceStr.substr(pos1 + 1);
            oriprice = _wtoi(wstr_oripriceStr.c_str()); ;
            CouponActivityInfoPrice = (oriprice - price + m_RedPacketCouponAmount);
            if (CouponActivityInfoPrice < 0) CouponActivityInfoPrice = 0;
        }

        // 使用 lestTime 计算小时/分钟/秒
        using namespace std::chrono;
        if (lestTime < milliseconds::zero())
        {
            lestTime = milliseconds::zero();
        }
        auto total_seconds = duration_cast<seconds>(lestTime).count();
        long long hours = static_cast<long long>(total_seconds / 3600);
        long long minutes = static_cast<long long>((total_seconds % 3600) / 60);
        long long seconds = static_cast<long long>(total_seconds % 60);

        // 格式化输出
        wchar_t buf[256];
        swprintf_s(buf, sizeof(buf) / sizeof(buf[0]),
            L"优惠%d元,剩余%lld时%02lld分%02lld秒",
            CouponActivityInfoPrice, hours, minutes, seconds);
        m_stat_CouponActivityInfo.LarSetText(buf);
    }
    else
    {
        CString PriceStr;
        switch (m_SelectedVipType)
        {
        case 0:
            PriceStr = m_Stat_PermentVip.LarGetText();
            break;
        case 1:
            PriceStr = m_Stat_YearAmount.LarGetText();
            break;
        case 2:
            PriceStr = m_Stat_HalfYearAmount.LarGetText();
            break;
        case 3:
            PriceStr = m_Stat_SeasonAMount.LarGetText();
            break;
        }
        std::wstring wstr_PriceStr = PriceStr;
        int price = _wtoi(wstr_PriceStr.c_str());
        price -= m_RedPacketCouponAmount;
        std::wstring newPrice = std::to_wstring(price) + L".00";
        m_Stat_Price.LarSetText(newPrice.c_str());

        int CouponActivityInfoPrice;
        {
            int oriprice;
            CString oripriceStr;
            switch (m_SelectedVipType)
            {
            case 0:
                m_stat_type1OriPrice.GetWindowTextW(oripriceStr);
                break;
            case 1:
                m_stat_type2OriPrice.GetWindowTextW(oripriceStr);
                break;
            case 2:
                m_stat_Type3OriPrice.GetWindowTextW(oripriceStr);
                break;
            case 3:
                m_stat_type4OriPrice.GetWindowTextW(oripriceStr);
                break;
            }
            std::wstring wstr_oripriceStr = oripriceStr;
            int pos1 = wstr_oripriceStr.find_last_of(L"：");
            wstr_oripriceStr = wstr_oripriceStr.substr(pos1 + 1);
            oriprice = _wtoi(wstr_oripriceStr.c_str()); ;
            CouponActivityInfoPrice = (oriprice - price + m_RedPacketCouponAmount);
            if (CouponActivityInfoPrice < 0) CouponActivityInfoPrice = 0;
        }

        // 使用 lestTime 计算小时/分钟/秒
        using namespace std::chrono;
        if (lestTime < milliseconds::zero())
        {
            lestTime = milliseconds::zero();
        }
        auto total_seconds = duration_cast<seconds>(lestTime).count();
        long long hours = static_cast<long long>(total_seconds / 3600);
        long long minutes = static_cast<long long>((total_seconds % 3600) / 60);
        long long seconds = static_cast<long long>(total_seconds % 60);

        // 格式化输出
        wchar_t buf[256];
        swprintf_s(buf, sizeof(buf) / sizeof(buf[0]),
            L"优惠%d元,剩余%lld时%02lld分%02lld秒",
            CouponActivityInfoPrice, hours, minutes, seconds);
        m_stat_CouponActivityInfo.LarSetText(buf);
    }
}

void Ui_VipPayDlg::SetNormalCtrlUiAlpha()
{
    m_bool_IsDarker.store(true);
    m_Stat_TitleText.LarSetTextAlpha(100);
    m_stat_offerExpiration.LarSetTextAlpha(100);
    m_Stat_PermentVip.LarSetTextAlpha(100);
    m_Stat_SeasonAMount.LarSetTextAlpha(100);
    m_stat_CouponActivityInfo.LarSetTextAlpha(100);
    m_stat_type1BuddleLowToday.LarSetTextAlpha(100);
    m_Stat_BindDevice1.LarSetTextAlpha(100);
    m_stat_bindDevice4.LarSetTextAlpha(100);
    m_Stat_Price.LarSetTextAlpha(100);
    m_stat_type1OriPrice.LarSetTextAlpha(100);
    m_stat_type2OriPrice.LarSetTextAlpha(100);
    m_stat_Type3OriPrice.LarSetTextAlpha(100);
    m_stat_type4OriPrice.LarSetTextAlpha(100);
    //m_stat_mouthBillPrice.LarSetTextAlpha(100);
    m_btn_supTicketAndService.LaSetTextColor(Gdiplus::Color(100, 255, 255, 255));
    m_btn_supTicketAndService.LaSetTextHoverColor(Gdiplus::Color(100, 245, 245, 245));
    m_btn_supTicketAndService.LaSetTextClickedColor(Gdiplus::Color(100, 235, 235, 235));
    m_btn_supTicketAndService.LarSetBorderColor(Gdiplus::Color(100, 31, 36, 37));
    m_btn_supTicketAndService.LarSetNormalFiilBrush(SolidBrush(Color(100, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetHoverFillBrush(SolidBrush(Color(100, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetClickedFillBrush(SolidBrush(Color(100, 31, 36, 37)));
}

void Ui_VipPayDlg::SetDarkCtrlUi()
{
    m_bool_IsDarker.store(false);
    m_Stat_TitleText.LarSetTextAlpha(255);
    m_stat_offerExpiration.LarSetTextAlpha(255);
    m_Stat_PermentVip.LarSetTextAlpha(255);
    m_Stat_SeasonAMount.LarSetTextAlpha(255);
    m_stat_CouponActivityInfo.LarSetTextAlpha(255);
    m_stat_type1BuddleLowToday.LarSetTextAlpha(255);
    m_Stat_Price.LarSetTextAlpha(255);
    m_Stat_BindDevice1.LarSetTextAlpha(255);
    m_stat_bindDevice4.LarSetTextAlpha(255);
    m_stat_type1OriPrice.LarSetTextAlpha(255);
    m_stat_type2OriPrice.LarSetTextAlpha(255);
    m_stat_Type3OriPrice.LarSetTextAlpha(255);
    m_stat_type4OriPrice.LarSetTextAlpha(255);
    //m_stat_mouthBillPrice.LarSetTextAlpha(255);
    m_btn_supTicketAndService.LarSetOpacity(255);
    m_btn_supTicketAndService.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
    m_btn_supTicketAndService.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
    m_btn_supTicketAndService.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
    m_btn_supTicketAndService.LarSetBorderColor(Gdiplus::Color(255, 31, 36, 37));
    m_btn_supTicketAndService.LarSetNormalFiilBrush(SolidBrush(Color(255, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetHoverFillBrush(SolidBrush(Color(255, 31, 36, 37)));
    m_btn_supTicketAndService.LarSetClickedFillBrush(SolidBrush(Color(255, 31, 36, 37)));
}

void Ui_VipPayDlg::UpdateOfferCountdownDisplay()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto& c = m_countDownOfferExpiration;

    // 保留总毫秒
    long long totalMs = 0;
    if (now < m_offerExpireAt)
    {
        totalMs = duration_cast<milliseconds>(m_offerExpireAt - now).count();
    }
    else
    {
        totalMs = 0;
        // 活动已结束，停止定时器
        if (m_timerIdOfferCountdown != 0)
        {
            KillTimer(m_timerIdOfferCountdown);
            m_timerIdOfferCountdown = 0;
        }
    }

    // 按“天 时 分 秒 毫秒”分解
    int days = static_cast<int>(totalMs / 86400000);
    long long remain = totalMs % 86400000;

    int hours = static_cast<int>(remain / 3600000); 
    remain %= 3600000;

    int minutes = static_cast<int>(remain / 60000); 
    remain %= 60000;

    int seconds = static_cast<int>(remain / 1000);        
    //int millis = static_cast<int>(remain % 1000);

    // 格式化为两位数
    auto fmt2 = [](int v) -> std::wstring
        {
            wchar_t buf[4] = { 0 };
            _snwprintf_s(buf, _TRUNCATE, L"%02d", v);
            return std::wstring(buf);
        };

    c.wstr_days = fmt2(days);
    c.wstr_hours = fmt2(hours);
    c.wstr_minute = fmt2(minutes);
    c.wstr_seconds = fmt2(seconds);
    //c.wstr_millSeconds = std::to_wstring(millis % 10);

    // 仅重绘倒计时区域，减少刷新
    int left = c.rect_days.X;
    int top = c.rect_days.Y;
    int right = c.rect_days.GetRight();
    int bottom = c.rect_days.GetBottom();

    auto acc_rect = [&](const Gdiplus::Rect& r)
        {
            left = min(left, r.X);
            top = min(top, r.Y);
            right = max(right, r.GetRight());
            bottom = max(bottom, r.GetBottom());
        };

    acc_rect(c.rect_hours);
    acc_rect(c.rect_minutes);
    acc_rect(c.rect_seconds);
    //acc_rect(c.rect_millSeconds);
    for (const auto& ur : c.vec_rect_units) acc_rect(ur);

    CRect invalid(left, top, right, bottom);
    InvalidateRect(&invalid, FALSE);
}

void Ui_VipPayDlg::UpdatePriceDisplay()
{
    if (m_PriceInfos.empty())
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"没有可用的价格信息，使用默认价格");
        return;
    }

    try
    {
        int idx = 0;
        for (const auto& p : m_PriceInfos)
        {
            CString priceText;
            priceText.Format(_T("%.2f"), p.amount);
            switch (idx)
            {
            case 3:
                m_Stat_SeasonAMount.LarSetText(priceText);
                m_stat_vipType4.LarSetText(p.name);
                m_stat_bindDevice4.LarSetText(p.desc);
                break;

            case 2:
                m_Stat_HalfYearAmount.LarSetText(priceText);
                m_Stat_VipType3.LarSetText(p.name);
                m_Stat_BindDevice3.LarSetText(p.desc);
                break;

            case 1:
                m_Stat_YearAmount.LarSetText(priceText);
                m_Stat_VipType2.LarSetText(p.name);
                m_Stat_BindDevice2.LarSetText(p.desc);
                break;

            case 0:
                m_Stat_PermentVip.LarSetText(priceText);
                m_Stat_VipType1.LarSetText(p.name);
                m_Stat_BindDevice1.LarSetText(p.desc);
                break;

            default:
                break;
            }
            if (++idx >= 4) break;
        }
    }
    catch (std::exception& e)
    {
        std::wstring wideError = LARSC::s2ws(e.what());
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"更新价格显示时发生异常: %s", wideError.c_str());
    }
}

void Ui_VipPayDlg::LoadQRCode()
{
    // 如果线程已在运行，不要重复启动
    if (m_ThreadRunning)
        return;

    // 设置状态为加载中
    m_QRCodeStatus = QR_LOADING;

    // repaint对话框，显示"加载中"状态
    Invalidate(FALSE);

    // 启动加载线程
    m_ThreadRunning = true;
    m_Thread_LoadQRCode = std::thread(&Ui_VipPayDlg::QRCodeLoadingThread, this);
    m_Thread_LoadQRCode.detach(); // 分离线程，让它独立运行
}

void Ui_VipPayDlg::QRCodeLoadingThread()
{
    try 
    {
        // 清除旧的二维码图像
        if (m_Bitmap_QRCode)
        {
            delete m_Bitmap_QRCode;
            m_Bitmap_QRCode = nullptr;
        }

        // 确保价格信息和预订单号
        if (m_PriceInfos.empty() || m_PreOrderNo.IsEmpty()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"加载二维码失败：没有价格信息或预订单号");
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        // 获取当前选中套餐的信息
        int selectedPriceId = 0;
        std::string amount_str;
        std::string title;
        if (m_SelectedVipType >= 0 && m_SelectedVipType < m_PriceInfos.size()) {
            selectedPriceId = m_PriceInfos[m_SelectedVipType].id;
            amount_str = std::to_string(m_PriceInfos[m_SelectedVipType].amount);
            title = LARSC::CStringToStdString(m_PriceInfos[m_SelectedVipType].name);
        }

        if (selectedPriceId == 0 || amount_str.empty() || title.empty()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"加载二维码失败：无法获取价格信息");
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        std::string machine_guid = App.m_machine_guid;

        // 构造GET URL
        std::string url = "http://localhost:5000/api/pay?machine_guid="
            + machine_guid + "&type=" + std::to_string(selectedPriceId)
            + "&pre_order_no=" + LARSC::CStringToStdString(m_PreOrderNo);

        // 使用CURL下载PNG
        CURL* curl = curl_easy_init();
        if (!curl) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"CURL初始化失败");
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        std::vector<unsigned char> imageBuffer;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, QRCodeWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"下载二维码失败: %hs", curl_easy_strerror(res));
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        // 检查是否获取到图像数据
        if (imageBuffer.empty()) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"二维码数据为空");
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        // 创建内存流并用它加载位图
        IStream* stream = nullptr;
        HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
        if (FAILED(hr) || !stream) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"创建内存流失败");
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        // 写入图像数据到流
        ULONG written;
        hr = stream->Write(imageBuffer.data(), static_cast<ULONG>(imageBuffer.size()), &written);
        if (FAILED(hr)) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"写入图像数据到流失败");
            stream->Release();
            m_QRCodeStatus = QR_FAILED;
            Invalidate(FALSE);
            m_ThreadRunning = false;
            return;
        }

        // 从流中加载位图
        m_Bitmap_QRCode = Gdiplus::Bitmap::FromStream(stream);
        stream->Release();

        if (!m_Bitmap_QRCode || m_Bitmap_QRCode->GetLastStatus() != Gdiplus::Ok) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"从数据流创建位图失败");
            delete m_Bitmap_QRCode;
            m_Bitmap_QRCode = nullptr;
            m_QRCodeStatus = QR_FAILED;
        }
        else 
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"二维码加载成功");
            m_QRCodeStatus = QR_LOADED;

            // 二维码加载成功后，开始支付轮询
            PostMessage(WM_APP + 1003); // 发送消息，启动支付轮询
        }
    }
    catch (std::exception& e) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"加载二维码时发生异常: %hs", e.what());
        m_QRCodeStatus = QR_FAILED;
    }

    // 线程结束，请求repaint
    m_ThreadRunning = false;
    Invalidate(FALSE);
}

void Ui_VipPayDlg::DrawQRCodeOrStatus(Gdiplus::Graphics* graphics)
{
    using namespace Gdiplus;

    switch (m_QRCodeStatus)
    {
    case QR_INITIAL:
    case QR_LOADING:
    {
        // 显示"获取二维码中..."
        SolidBrush textBrush(Color(0, 0, 0));
        Gdiplus::Font font(L"微软雅黑", 6 * m_Scale, FontStyleRegular);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        graphics->DrawString(
            L"获取二维码中...", -1, &font,
            RectF((REAL)m_Rect_QRCode.X, (REAL)m_Rect_QRCode.Y,
                (REAL)m_Rect_QRCode.Width, (REAL)m_Rect_QRCode.Height),
            &format, &textBrush);
    }
    break;

    case QR_LOADED:
        // 显示二维码图像
        if (m_Bitmap_QRCode)
        {
            graphics->DrawImage(m_Bitmap_QRCode, m_Rect_QRCode);
        }
        break;

    case QR_FAILED:
    {
        // 显示"二维码获取失败"
        SolidBrush textBrush(Color(255, 100, 100)); // 红色文本
        Gdiplus::Font font(L"微软雅黑", 12, FontStyleRegular);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        graphics->DrawString(
            L"二维码获取失败", -1, &font,
            RectF((REAL)m_Rect_QRCode.X, (REAL)m_Rect_QRCode.Y,
                (REAL)m_Rect_QRCode.Width, (REAL)m_Rect_QRCode.Height),
            &format, &textBrush);
    }
    break;

    case QR_TIMEOUT:
    {
        // 显示"超时，请刷新"
        SolidBrush textBrush(Color(255, 175, 100)); // 橙色文本
        Gdiplus::Font font(L"微软雅黑", 9, FontStyleRegular);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        graphics->DrawString(
            L"超时，请刷新", -1, &font,
            RectF((REAL)m_Rect_QRCode.X, (REAL)m_Rect_QRCode.Y,
                (REAL)m_Rect_QRCode.Width, (REAL)m_Rect_QRCode.Height),
            &format, &textBrush);
    }
    break;
    }
}

void Ui_VipPayDlg::StartPaymentPolling() 
{
    // 确保有预订单号
    if (m_PreOrderNo.IsEmpty()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"无法启动轮询：预订单号为空");
        return;
    }

    // 将 CString 转换为 std::string
    std::string preOrderNo;
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
    preOrderNo = CT2A(m_PreOrderNo);
#else
    // x86 平台使用更安全的转换
    CStringA ansiStr(m_PreOrderNo);
    preOrderNo = ansiStr.GetString();
#endif

    // 启动轮询，设置成功和超时回调
    m_PaymentPolling.startPolling(
        preOrderNo,
        [this]() { this->OnPaymentSuccess(); },
        [this]() { this->OnPaymentTimeout(); }
    );
}

void Ui_VipPayDlg::OnPaymentSuccess()
{
    // 在 UI 线程中处理
    this->PostMessage(WM_APP + 1001); // 自定义消息，处理支付成功
}

void Ui_VipPayDlg::OnPaymentTimeout()
{
    // 在 UI 线程中处理
    this->PostMessage(WM_APP + 1002); // 自定义消息，处理支付超时
}

void Ui_VipPayDlg::RefreshQRCode()
{
    // 如果轮询正在运行，先停止
    m_PaymentPolling.stopPolling();

    // 将状态重置为初始状态
    m_QRCodeStatus = QR_INITIAL;

    // 重新加载二维码
    LoadQRCode();

    // 刷新显示
    Invalidate(FALSE);
}

bool Ui_VipPayDlg::IsPointInQRCodeArea(CPoint point) {
    CRect qrRect(m_Rect_QRCode.X, m_Rect_QRCode.Y,
        m_Rect_QRCode.X + m_Rect_QRCode.Width,
        m_Rect_QRCode.Y + m_Rect_QRCode.Height);
    return qrRect.PtInRect(point);
}

LRESULT Ui_VipPayDlg::OnPaymentSuccessMsg(WPARAM wParam, LPARAM lParam)
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"收到支付成功消息，关闭对话框");
  
    // 停止轮询
    m_PaymentPolling.stopPolling();

    // 以 IDOK 方式关闭对话框
    EndDialog(IDOK);
    return 0;
}

LRESULT Ui_VipPayDlg::OnPaymentTimeoutMsg(WPARAM wParam, LPARAM lParam)
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"收到支付超时消息");

    // 更新状态为超时
    m_QRCodeStatus = QR_TIMEOUT;

    // 如果有二维码图像，释放它
    if (m_Bitmap_QRCode) {
        delete m_Bitmap_QRCode;
        m_Bitmap_QRCode = nullptr;
    }

    // 刷新显示
    Invalidate(FALSE);

    return 0;
}

LRESULT Ui_VipPayDlg::OnStartPaymentPollingMsg(WPARAM wParam, LPARAM lParam)
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"启动支付状态轮询");

    // 开始支付轮询
    StartPaymentPolling();

    return 0;
}

LRESULT Ui_VipPayDlg::OnStopPaymentPollingAndQuit(WPARAM wParam, LPARAM lParam)
{
    m_PaymentPolling.stopPolling();
    EndDialog(IDCANCEL);
    return 1;
}

LRESULT Ui_VipPayDlg::OnLoginShow(WPARAM wParam, LPARAM lParam)
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

LRESULT Ui_VipPayDlg::OnInRedPacketMode(WPARAM wParam, LPARAM lParam)
{
    UpdatePriceNewUserCouponInfo(m_timeLest);
    SetTimer(VIPDLG_TIMER_REDPACKETTIMEUPDATE, 1000, NULL);
    return LRESULT();
}

LRESULT Ui_VipPayDlg::OnUpdateScaleWithMode(WPARAM wParam, LPARAM lParam)
{
    if (m_Mode == Mode::RedPacketCoupon)
    {
        m_Rect_VipInfoRect.Width = 358.52 * m_Scale;
        m_Rect_VipInfoRect.Height = 193.99 * m_Scale;
        m_Rect_VipInfoRect.X = 27.8 * m_Scale;
        m_Rect_VipInfoRect.Y = 343.5 * m_Scale;

        //m_stat_mouthBillPrice.ShowWindow(SW_HIDE);

        int vpcW = 150 * m_Scale;
        int vpcH = 20 * m_Scale;
        int vpcX = 264 * m_Scale;
        int vpcY = 356 * m_Scale;
        m_cusstat_vipPrivilegeContrast.btnArea.X = vpcX;
        m_cusstat_vipPrivilegeContrast.btnArea.Y = vpcY;
        m_cusstat_vipPrivilegeContrast.btnArea.Width = vpcW;
        m_cusstat_vipPrivilegeContrast.btnArea.Height = vpcH;
        m_cusstat_vipPrivilegeContrast.txtR = m_cusstat_vipPrivilegeContrast.btnArea;

        //如果处于红包优惠券时限下（已经领取过）
        if (m_isHasRedPacketCoupon)
        {
            //调整文本区域
            m_Rect_CouponActivityInfo.Width = 178 * m_Scale;
            m_Rect_CouponActivityInfo.Height = 26.25 * m_Scale;
            m_Rect_CouponActivityInfo.X = 571 * m_Scale;
            m_Rect_CouponActivityInfo.Y = 389 * m_Scale;
            {
                float CaWidth = m_Rect_CouponActivityInfo.Width;
                float CaHeight = m_Rect_CouponActivityInfo.Height;
                float CaX = m_Rect_CouponActivityInfo.X;
                float CaY = m_Rect_CouponActivityInfo.Y;
                m_stat_CouponActivityInfo.MoveWindow(
                    CaX,
                    CaY,
                    CaWidth,
                    CaHeight
                );
            }

            //启动定时器
            KillTimer(VIPDLG_TIMER_REDPACKETTIMEUPDATE);
            SetTimer(VIPDLG_TIMER_REDPACKETTIMEUPDATE, 1000, NULL);
            UpdatePriceNewUserCouponInfo(m_timeLest);
        }
    }
    else if(m_Mode == Mode::LowPriceWithMouthBill)
    {
        //if (!App.m_IsNeedShowMouthBill)
        //{
        //    m_Rect_VipInfoRect.Width = 358.52 * m_Scale;
        //    m_Rect_VipInfoRect.Height = 193.99 * m_Scale;
        //    m_Rect_VipInfoRect.X = 27.8 * m_Scale;
        //    m_Rect_VipInfoRect.Y = 343.5 * m_Scale;
        //}
        //else
        //{
        //    m_Rect_VipInfoRect.Width = 358.52 * m_Scale;
        //    m_Rect_VipInfoRect.Height = 193.99 * m_Scale;
        //    m_Rect_VipInfoRect.X = 27.8 * m_Scale;
        //    m_Rect_VipInfoRect.Y = 400.5 * m_Scale;
        //}
        //
        ////if (App.m_IsNeedShowMouthBill)
        ////{
        ////    std::wstring mouthBillPriceStr = L"￥" + std::to_wstring(App.m_CouponPrice.amount);
        ////    m_stat_mouthBillPrice.SetWindowTextW(mouthBillPriceStr.c_str());
        ////    m_stat_mouthBillPrice.ShowWindow(SW_SHOW);
        ////}
        ////else
        ////{
        ////    m_stat_mouthBillPrice.ShowWindow(SW_HIDE);
        ////}
        //
        //int vpcW = 150 * m_Scale;
        //int vpcH = 20 * m_Scale;
        //int vpcX = 264 * m_Scale;
        //int vpcY = App.m_IsNeedShowMouthBill ? 417 * m_Scale : 356 * m_Scale;
        //m_cusstat_vipPrivilegeContrast.btnArea.X = vpcX;
        //m_cusstat_vipPrivilegeContrast.btnArea.Y = vpcY;
        //m_cusstat_vipPrivilegeContrast.btnArea.Width = vpcW;
        //m_cusstat_vipPrivilegeContrast.btnArea.Height = vpcH;
        //m_cusstat_vipPrivilegeContrast.txtR = m_cusstat_vipPrivilegeContrast.btnArea;
    }
    Invalidate(false);
    return LRESULT();
}

void Ui_VipPayDlg::OnMove(int x, int y)
{
    CDialogEx::OnMove(x, y);
    m_Shadow.Show(m_hWnd);
    // TODO: 在此处添加消息处理程序代码
}

void Ui_VipPayDlg::OnTimer(UINT_PTR nIDEvent)  
{  
   if (nIDEvent == VIPDLG_TIMER_REDPACKETTIMEUPDATE)  
   {  
       m_timeLest -= std::chrono::milliseconds(1000);  
       UpdatePriceNewUserCouponInfo(m_timeLest);  
       if (m_timeLest <= std::chrono::milliseconds(0))   // 如果m_timeLest减少到了0秒
       {  
           KillTimer(VIPDLG_TIMER_REDPACKETTIMEUPDATE);  
           int price = 0;
           int oriPrice = 999;
           CString oriPriceStr;
           CString PriceStr;
           m_Stat_Price.GetWindowTextW(PriceStr);
           switch (m_SelectedVipType)
           {
           case 0:
               m_stat_type1OriPrice.GetWindowTextW(oriPriceStr);
               break;
           case 1:
               m_stat_type2OriPrice.GetWindowTextW(oriPriceStr);
               break;
           case 2:
               m_stat_Type3OriPrice.GetWindowTextW(oriPriceStr);
               break;
           case 3:
               m_stat_type4OriPrice.GetWindowTextW(oriPriceStr);
               break;
           }
           price = _wtoi(PriceStr);
           price += m_RedPacketCouponAmount;
           oriPrice = _wtoi(oriPriceStr);
           auto activityInfoStr = L"新人特权优惠:" + std::to_wstring(oriPrice - price) + L"元";
           m_Stat_Price.SetWindowTextW(PriceStr);

           UpdateScale();
       }  
   }   
   else if (nIDEvent == VIPDLG_TIMER_COUNTDOWNOFFEREXPIRATION)
   {
       UpdateOfferCountdownDisplay();
       return;
   }
   CDialogEx::OnTimer(nIDEvent);  
}

void Ui_VipPayDlg::OnBnClickedBtnSupticketandservice()
{
    App.OpenFeedBackLink(
        this->GetSafeHwnd(),
        L"https://tb.53kf.com/code/client/f4280c6c9b370e90a15a44955146a1ca0/3"
    );
}

void Ui_VipPayDlg::OnDestroy()
{
    CDialogEx::OnDestroy();
    if (m_timerIdOfferCountdown != 0)
    {
        KillTimer(m_timerIdOfferCountdown);
        m_timerIdOfferCountdown = 0;
    }
    auto& c = m_countDownOfferExpiration;
    if (c.font_days) { delete c.font_days; c.font_days = nullptr; }
    if (c.font_hours) { delete c.font_hours; c.font_hours = nullptr; }
    if (c.font_minute) { delete c.font_minute; c.font_minute = nullptr; }
    if (c.font_seconds) { delete c.font_seconds; c.font_seconds = nullptr; }
    if (c.font_unit) { delete c.font_unit; c.font_unit = nullptr; }
    if (c.sb_days) { delete c.sb_days; c.sb_days = nullptr; }
    if (c.sb_hours) { delete c.sb_hours; c.sb_hours = nullptr; }
    if (c.sb_minutes) { delete c.sb_minutes; c.sb_minutes = nullptr; }
    if (c.sb_seconds) { delete c.sb_seconds; c.sb_seconds = nullptr; }
    if (c.sb_unit) { delete c.sb_unit; c.sb_unit = nullptr; }
}
