#include "stdafx.h"
#include "GlobalFunc.h"
#include "Ui_RaiSDL.h"
#include "Ui_RadarTimerSDL.h"
#include "Ui_RedPacketSDL.h"
#include "UI_MouthMemberDlg.h"
#include "theApp.h"
#include "ModalDialogFunc.h"
#include "LarStringConversion.h"

extern HANDLE ConsoleHandle;
int ModalDlg_MFC::ShowModal_PaySuccess(CWnd* pParCWnd)
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"感谢您的支持！支付成功", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_TrialOver(CWnd* pParCWnd)
{
	return ShowModal_Priview(pParCWnd);
}

int ModalDlg_MFC::ShowModal_Priview(CWnd* pParCWnd)
{
	bool isModoal = false;

	if (pParCWnd && pParCWnd->IsIconic())
		pParCWnd = nullptr;

	if(isModoal)
	{
		Ui_MessageModalDlg MessageDlg;
		if (!App.m_IsVip && !App.m_IsNonUserPaid)
			MessageDlg.SetModal(L"极速录屏大师", L"录制结束", L"您的试用录制已结束,点击预览查看视频，开通VIP可无限录制",
				L"开通VIP");
		else
			MessageDlg.SetModal(L"极速录屏大师", L"录制结束", L"您的录制已结束,点击预览查看视频", L"取消");
		MessageDlg.AddButton(
			L"预览",
			Gdiplus::Color(36, 37, 40),
			Gdiplus::Color(215, 215, 215),
			Gdiplus::Color(225, 225, 225),
			Gdiplus::Color(255, 255, 255),
			Gdiplus::Color(0, 0, 0),
			Gdiplus::Color(0, 0, 0),
			Gdiplus::Color(215, 215, 215),
			[=, &MessageDlg]()
			{
				MessageDlg.OnDialogOk();
				App.m_Dlg_Main.On_ModalBnClick_Prview();
			}
		);
		MessageDlg.SetDefaultBtnCallback([=, &MessageDlg]()
			{
				CString btnText;
				MessageDlg.m_Btn_OpenVip.GetWindowTextW(btnText);
				MessageDlg.m_Shadow.SetSize(0);
				MessageDlg.OnDialogCancel();
				if (btnText == L"取消")
				{
					MessageDlg.EndDialog(IDCANCEL);
					return;
				}
				MessageDlg.EndDialog(IDCANCEL);
				if (::IsWindowVisible(App.m_Dlg_Main.GetSafeHwnd()))
					App.m_Dlg_Main.OnBnClickedBtnOpenvip();
				else if (::IsWindowVisible(App.m_Dlg_Main.m_Dlg_Carmera->GetSafeHwnd()))
					App.m_Dlg_Main.m_Dlg_Carmera->OnBnClickedBtnOpenvip();
				else if (::IsWindowVisible(App.m_Dlg_Main.m_Dlg_Gaming->GetSafeHwnd()))
					App.m_Dlg_Main.m_Dlg_Gaming->OnBnClickedBtnOpenvip();
				else if (::IsWindowVisible(App.m_Dlg_Main.m_Dlg_WindowRecord->GetSafeHwnd()))
					App.m_Dlg_Main.m_Dlg_WindowRecord->OnBnClickedBtnOpenvip();
			});
		if (!App.m_IsVip && !App.m_IsNonUserPaid)
		{
			MessageDlg.SetDefaultBtnGradientColor(
				Gdiplus::Color(255, 255, 232, 213), Gdiplus::Color(255, 224, 184, 140),
				Gdiplus::Color(255, 224, 184, 140), Gdiplus::Color(255, 255, 232, 213),
				Gdiplus::Color(255, 224, 184, 140), Gdiplus::Color(255, 255, 232, 213)
			);
		}
		MessageDlg.HideCloseBtn();
		return MessageDlg.DoModal();
	}
	else
	{
		static Ui_MessageModalDlg* pDlg = nullptr;
		if (pDlg)
		{
			pDlg->DestroyWindow();
			pDlg = nullptr;
		}
		pDlg = new Ui_MessageModalDlg(pParCWnd);
		pDlg->AddButton(
			L"预览",
			Gdiplus::Color(36, 37, 40),
			Gdiplus::Color(215, 215, 215),
			Gdiplus::Color(225, 225, 225),
			Gdiplus::Color(255, 255, 255),
			Gdiplus::Color(0, 0, 0),
			Gdiplus::Color(0, 0, 0),
			Gdiplus::Color(215, 215, 215),
			[=]()
			{
				pDlg->OnDialogOk();
				pDlg->m_Shadow.HideShadow();
				App.m_Dlg_Main.On_ModalBnClick_Prview();
			}
		);
		if (!App.m_IsVip && !App.m_IsNonUserPaid)
		{
			pDlg->SetDefaultBtnGradientColor(
				Gdiplus::Color(255, 255, 232, 213), Gdiplus::Color(255, 224, 184, 140),
				Gdiplus::Color(255, 245, 222, 203), Gdiplus::Color(255, 214, 174, 130),
				Gdiplus::Color(255, 234, 212, 193), Gdiplus::Color(255, 204, 164, 120)
			);
		}
		pDlg->HideCloseBtn();
		if (!pDlg->Create(IDD_DIALOG_NONEVIP, pParCWnd))
		{
			delete pDlg;
			return IDCANCEL;
		}
		pDlg->m_Stat_AppText.SetWindowTextW(L"极速录屏大师");
		pDlg->m_Stat_MessageType.SetWindowTextW(L"录制结束");
		if (!App.m_IsVip && !App.m_IsNonUserPaid)
		{
			pDlg->m_Stat_MessageInfo.SetWindowTextW(L"您的试用录制已结束,点击预览查看视频，开通VIP可无限录制");
			pDlg->m_Btn_OpenVip.SetWindowTextW(L"开通VIP");
		}
		else
		{
			pDlg->m_Stat_MessageInfo.SetWindowTextW(L"您的录制已结束,点击预览查看视频");
			pDlg->m_Btn_OpenVip.SetWindowTextW(L"取消");
		}

		if (pParCWnd)
		{
			// 将对话框移动到父窗口中心
			CRect dlgRect, parRect;
			pDlg->GetWindowRect(dlgRect);
			pParCWnd->GetWindowRect(parRect);
			int newX = parRect.left + (parRect.Width() - dlgRect.Width()) / 2;
			int newY = parRect.top + (parRect.Height() - dlgRect.Height()) / 2;
			dlgRect.MoveToXY(newX, newY);
			::MoveWindow(pDlg->m_hWnd, dlgRect.left, dlgRect.top, dlgRect.Width(), dlgRect.Height(), FALSE);

		}
		else
		{
			// 将窗口移动到屏幕中心
			CRect dlgRect;
			pDlg->GetWindowRect(dlgRect);
			int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYSCREEN);
			int dlgWidth = dlgRect.Width();
			int dlgHeight = dlgRect.Height();
			int newX = (screenWidth - dlgWidth) / 2;
			int newY = (screenHeight - dlgHeight) / 2;
			dlgRect.MoveToXY(newX, newY);
			::MoveWindow(pDlg->m_hWnd, dlgRect.left, dlgRect.top, dlgRect.Width(), dlgRect.Height(), FALSE);
		}
		pDlg->ShowWindow(SW_SHOW);
		pDlg->SetForegroundWindow();
		::SetWindowPos(pDlg->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return IDCANCEL; 
	}
}

int ModalDlg_MFC::ShowModal_RecordFalse()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"很抱歉，录制失败！当前音频设备或麦克风设备异常，请尝试静音录制", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_NoDeviceAvailable()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"很抱歉，录制失败！当前没有设备可用，无法进行录制", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_ScreenShotCompelete()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"截屏完成", L"确认");
	int cx = GetSystemMetrics(SM_CXSCREEN),
		cy = GetSystemMetrics(SM_CYSCREEN);
	int w = MessageDlg.m_WindowWidth,
		h = MessageDlg.m_WindowHeight;
	MessageDlg.SetInitialPosition((cx - w) / 2, (cy - h) / 2);
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_NeedLogin()
{
	return ModalDlg_MFC::ShowModal_Priview(nullptr);
}

int ModalDlg_MFC::ShowModal_NetWorkError()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"Ops！出错了", L"网络请求失败了", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_RecordHwndInvalid()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"您点击了关闭按钮，您需要：", L"", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_IsCloseToBar()
{
	static bool IsNoMoreTips = false;
	if (IsNoMoreTips)
		return IDOK;

	bool IsMinialTobar, IsQuitExe;
	App.m_Dlg_Main.m_Dlg_Config->GetExeQuitWay(&IsQuitExe, &IsMinialTobar);

	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"", L"确认");
	MessageDlg.AddButton(
		L"取消",
		Color(36, 37, 40),
		Color(215, 215, 215),
		Color(225, 225, 225),
		Color(255, 255, 255),
		Color(0, 0, 0),
		Color(0, 0, 0),
		Color(215, 215, 215),
		[=, &MessageDlg]()
		{
			MessageDlg.OnDialogCancel();
		});
	MessageDlg.AddCkBox(
		L"最小化到系统托盘，不退出程序",
		Color(178, 178, 178),
		20,
		2,
		Color(178, 178, 178),
		Ui_MessageModalDlg::CheckBoxBorderStyle::Eclipse,
		0, 0, 1, IsMinialTobar
	);
	MessageDlg.AddCkBox(
		L"退出程序",
		Color(178, 178, 178),
		20,
		2,
		Color(178, 178, 178),
		Ui_MessageModalDlg::CheckBoxBorderStyle::Eclipse,
		0, 0, 1, IsQuitExe
	);
	MessageDlg.AddCkBox(
		L"不再提示",
		Color(178, 178, 178),
		20,
		2,
		Color(178, 178, 178),
		Ui_MessageModalDlg::CheckBoxBorderStyle::Rect,
		0,
		GlobalFunc::GetUserDPI() * 15
	);
	::SetWindowPos(MessageDlg.GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	if (MessageDlg.DoModal() == IDOK)
	{
		std::vector<int> selectIds;
		MessageDlg.GetSelectCkUid(&selectIds);
		int minimalSelectUid = MessageDlg.GetStartCkUid();
		int quitExeSelectUid = minimalSelectUid + 1;
		int nomoretipsSelectUid = quitExeSelectUid + 1;
		for (const auto& selectUid : selectIds)
		{
			if (selectUid == minimalSelectUid)
			{
				App.m_Dlg_Main.m_Dlg_Config->SetMinimalToBar(true);
			}
			else if (selectUid == quitExeSelectUid)
			{
				App.m_Dlg_Main.m_Dlg_Config->SetQuitExe(true);
			}
			else if (selectUid == nomoretipsSelectUid)
			{
				IsNoMoreTips = true;
			}
		}
		return IDOK;
	}
	else
	{
		return IDCANCEL;
	}
}

int ModalDlg_MFC::ShowModal_RecordWindowMinimal()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"录制的窗口最小化或关闭，停止录制窗口", L"确认");
	MessageDlg.AddButton(
		L"预览",
		Color(36, 37, 40),
		Color(215, 215, 215),
		Color(225, 225, 225),
		Color(255, 255, 255),
		Color(0, 0, 0),
		Color(0, 0, 0),
		Color(215, 215, 215),
		[=, &MessageDlg]()
		{
			MessageDlg.OnDialogOk();
			App.m_Dlg_Main.On_ModalBnClick_Prview();
		}
	);
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_WindowMinimalRecordTips()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"录制的窗口不能最小化，是否确认?", L"确认");
	MessageDlg.AddButton(
		L"取消",
		Color(36, 37, 40),
		Color(215, 215, 215),
		Color(225, 225, 225),
		Color(255, 255, 255),
		Color(0, 0, 0),
		Color(0, 0, 0),
		Color(215, 215, 215),
		[=, &MessageDlg]()
		{
			MessageDlg.OnDialogCancel();
		});
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_OverBindsTips()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"由于您的设备绑定数量已经超限，录制停止，请在主界面中点击设备绑定减少绑定数量", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_OverBindsTipsBeforeRecord()
{
	if (App.m_IsNonUserPaid)
		return IDOK;
	if(!App.m_IsVip)
		return IDOK;

	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"由于您的设备绑定数量已经超限，是否确认录制?", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_MouseRecordAreaSelectCompelete(int width,int height)
{
	CString mesInfo;
	mesInfo.Format(L"以鼠标为中心，录制长%d,宽%d的矩形区域", width, height);

	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"鼠标周围录制区域大小选择完毕", mesInfo, L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_LoginSuccess()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"登录成功!", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_SignOutSuccess()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"退出登录成功!", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_Test()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"健身房会员管理系统",
		L"耀伟的温馨提示~", L"由于您不是用户角色，无法进入会员办理界面", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_WindowHwndInvalid()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"录制的窗口已经无效，请重新选择", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_NoGameWindowSelect()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(
		L"极速录屏大师",
		L"温馨提示",
		L"未检测到游戏程序，请进入游戏后再点击REC进行录制，部分游戏也可开启全屏模式进行录制",
		L"全屏录制"
	);
	MessageDlg.AddButton(
		L"我知道了",
		Color(36, 37, 40),
		Color(215, 215, 215),
		Color(225, 225, 225),
		Color(255, 255, 255),
		Color(0, 0, 0),
		Color(0, 0, 0),
		Color(215, 215, 215),
		[=, &MessageDlg]()
		{
			MessageDlg.OnDialogCancel();
		});
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_OpenVideoFailed()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"程序权限不足，播放视频失败！", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_NoCarmerDevice()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"对不起，未找到摄像头设备！", L"确认");
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_OpenVipFailed()
{
	Ui_MessageModalDlg MessageDlg;
	MessageDlg.SetModal(L"极速录屏大师",
		L"温馨提示", L"对不起！由于未知原因开通VIP失败!请联系客服解决或重启软件", L"联系客服");
	MessageDlg.SetDefaultBtnCallback([=, &MessageDlg]()
		{
			App.m_Dlg_Main.OnBnClickedBtnFeedback();
			MessageDlg.EndDialog(IDCANCEL);
		});
	return MessageDlg.DoModal();
}

int ModalDlg_MFC::ShowModal_MouthMemberDlg(CWnd* parCwnd)
{
	UI_MouthMemberDlg smdlg(parCwnd);
	CRect parentGlobalRect;
	parCwnd->GetWindowRect(parentGlobalRect);
	int x = parentGlobalRect.left + (parentGlobalRect.Width() - smdlg.getW()) / 2;
	int y = parentGlobalRect.top + (parentGlobalRect.Height() - smdlg.getH()) / 2;
	smdlg.SetWindowXY(x, y);
	return smdlg.DoModal();
}

bool ModalDlg_SDL::ShowModal_CountDown(
	int CountDownSeconds,
	const std::string& textInfo,
	std::function<void()> callback,
	bool IsAutoRelease
)
{
#ifdef _DEBUG
	return true;
#endif

	// 获取系统 DPI
	HDC screen = ::GetDC(NULL);
	if (screen == NULL)
	{
		AfxMessageBox(L"无法获取屏幕 DC。");
		return false;
	}
	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	::ReleaseDC(NULL, screen);
	double scale = static_cast<double>(dpiX) / 96.0;

	// 如果存在实例并且在运行，则停止运行并释放，重新创建
	auto RadarTimerIns = Ui_RadarTimerSDL::GetInstance();
	if (RadarTimerIns->IsWindowRunning())
	{
		RadarTimerIns->ReleaseInstance();
		RadarTimerIns = Ui_RadarTimerSDL::GetInstance();
	}

	//定义雷达倒计时窗口大小
	CRect WindowRect;
	int DesktopHeight = GetSystemMetrics(SM_CYSCREEN);
	int DesktopWidth = GetSystemMetrics(SM_CXSCREEN);
	int WindowWidth = 340 * scale;
	int WindowHeight = 430 * scale;
	int WindowY = (DesktopHeight - WindowHeight) / 2;
	int WindowX = (DesktopWidth - WindowWidth) / 2;
	WindowRect.left = WindowX;
	WindowRect.top = WindowY;
	WindowRect.right = WindowRect.left + WindowWidth;
	WindowRect.bottom = WindowRect.top + WindowHeight;

	// 初始化雷达倒计时窗口
	std::string UTF8Text = GlobalFunc::AnsiToUtf8(textInfo);
	RadarTimerIns->Initialize(WindowRect, scale);
	RadarTimerIns->SetCountdown(CountDownSeconds);
	RadarTimerIns->SetAutoRelease(IsAutoRelease);
	RadarTimerIns->SetCountDownCallBack(callback);
	RadarTimerIns->Run();

	while (Ui_RadarTimerSDL::IsInsExist())
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return true;
}
 
bool ModalDlg_SDL::ShowModal_Rai(
	const CWnd* pWnd, 
	float SystemDPI,
	Ui_RaiSDL::ShowMode showMode
)
{
	if (App.m_IsNonUserPaid)
		return true;

	if (App.m_IsVip && showMode != Ui_RaiSDL::ShowMode::ReturnToOriPage)
		return true;

	DB(ConsoleHandle, L"开始显示权益窗口");

	int ret = 0;
	{
		//准备权益窗口矩形参数
		CRect rRect;
		pWnd->GetWindowRect(&rRect);
		int width = static_cast<int>(800 * SystemDPI);
		int height = static_cast<int>(650 * SystemDPI);
		int left = rRect.left + (rRect.Width() - width) / 2;
		int top = rRect.top + (rRect.Height() - height) / 2;
		CRect rect(left, top, left + width, top + height);

		//准备权益窗口阴影参数
		Ui_RaiSDL::ShadowParam sp;
		sp.color = RGB(31, 36, 37);
		sp.Darkness = 255;
		sp.size = 5;
		sp.sharpness = 90;

		Ui_RaiSDL pRai;
		pRai.SetShowMode(showMode);
		pRai.Run(rect, false, 60, sp);
		DB(ConsoleHandle, L"权益窗口开始显示，开始peekMFC消息");
		//阻塞MFC窗口循环，直到Ui_RaiSDL窗口关闭
		MSG msg;
		while (pRai.isRunning())
		{
			while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		ret = pRai.getRet();
	}
	//根据窗口结束码返回真假
	if (ret == 0)
		return true;
	else if (ret == MSG_UIRAI_OPENVIP)
		return false;
	else
		return true;
}

int ModalDlg_SDL::ShowModal_RedPacket(
	HWND hWnd, 
	float SystemDPI, 
	int CouponValue, 
	std::chrono::milliseconds& timeLest,
	std::chrono::milliseconds initaltimeLest
)
{
	//定义阴影参数（不显示）
	LLarSDL::ShadowParam shadowParam{};
	shadowParam.color = RGB(255, 255, 255);
	shadowParam.Darkness = 0;
	shadowParam.sharpness = 0;
	shadowParam.size = 0;

	//获取被附加红包窗口的大小
	CRect WindowRect;
	::GetWindowRect(hWnd, WindowRect);

	// 配置红包窗口
	Ui_RedPacketSDL::RedPacketConfig config;
	CRect readPacketR;
	int redPacketW = 650 * SystemDPI;
	int redPacketH = 650 * SystemDPI;
	readPacketR.left = (WindowRect.Width() - redPacketW) / 2;
	readPacketR.top = (WindowRect.Height() - redPacketH) / 2;
	readPacketR.right = readPacketR.left + redPacketW;
	readPacketR.bottom = readPacketR.top + redPacketH;
	config.ReadPacket = readPacketR;
	config.redPacketImagePath = L"\\SDLAssets\\redpacket.png";
	config.backgroundHWnd = hWnd;
	config.dimParam = 0.35f;
	config.CouponNum = CouponValue;
	config.initialCountdownTime = initaltimeLest;

	// 从HWND窗口创建背景
	Ui_RedPacketSDL redPacketWindow;
	redPacketWindow.SetRedPacketConfig(config);
	redPacketWindow.Run(WindowRect, false, 60, shadowParam);	// 运行红包窗口

	//阻塞MFC循环，直到Ui_RedPacketSDL窗口关闭
	MSG msg;
	while (redPacketWindow.isRunning())
	{
		while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	timeLest = redPacketWindow.GetRemainingTime();
	int result = redPacketWindow.getRet();
	return result;
}
