#pragma once
#include <afxwin.h>
#include <functional>
#include <string>
#include "Ui_NoneVipDlg.h"
#include "Ui_RaiSDL.h"
#include "Ui_VipPayDlg.h"
class CouponPrice;
namespace ModalDlg_MFC
{
	int ShowModal_OverBindsTipsBeforeRecord();
	int ShowModal_PaySuccess(CWnd* pParCWnd);
	int ShowModal_TrialOver(CWnd* pParCWnd);
	int ShowModal_Priview(CWnd* pParCWnd);
	int ShowModal_RecordFalse();
	int ShowModal_NoDeviceAvailable();
	int ShowModal_ScreenShotCompelete();
	int ShowModal_NeedLogin();
	int ShowModal_NetWorkError();
	int ShowModal_RecordHwndInvalid();
	int ShowModal_IsCloseToBar();
	int ShowModal_RecordWindowMinimal();
	int ShowModal_WindowMinimalRecordTips();
	int ShowModal_OverBindsTips();
	int ShowModal_MouseRecordAreaSelectCompelete(int width, int height);
	int ShowModal_LoginSuccess();
	int ShowModal_SignOutSuccess();
	int ShowModal_Test();
	int ShowModal_WindowHwndInvalid();
	int ShowModal_NoGameWindowSelect();
	int ShowModal_OpenVideoFailed();
	int ShowModal_NoCarmerDevice();
	int ShowModal_OpenVipFailed();
	int ShowModal_MouthMemberDlg(CWnd* parCwnd);
}

namespace ModalDlg_SDL
{
	bool ShowModal_CountDown(
		int CountDownSeconds,
		const std::string& textInfo,
		std::function<void()> callback,
		bool IsAutoRelease
	);
	bool ShowModal_Rai(
		const CWnd* pWnd,
		float SystemDPI,
		Ui_RaiSDL::ShowMode showMode = Ui_RaiSDL::ShowMode::RecordOrOpenVip
	);
	int ShowModal_RedPacket(
		HWND hWnd, 
		float SystemDPI,
		int CouponValue, 
		std::chrono::milliseconds& timeLest, 
		std::chrono::milliseconds initaltimeLest
	);
}