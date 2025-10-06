// Ui_ChildDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "Ui_MainDlg.h"
#include "Ui_ChildDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "CDebug.h"
#include "CMessage.h"
#include "GlobalFunc.h"
#include "LarStringConversion.h"
#include "theApp.h"
#include "ModalDialogFunc.h"
#include "Ui_RaiSDL.h"
#include "CTimer.h"
extern bool g_CameraWindowClosed;
extern HANDLE ConsoleHandle;
IMPLEMENT_DYNAMIC(CCustomSliderCtrl, CSliderCtrl)

CCustomSliderCtrl::CCustomSliderCtrl()
{
    m_bDragging = false;

}

CCustomSliderCtrl::~CCustomSliderCtrl()
{
}

BEGIN_MESSAGE_MAP(CCustomSliderCtrl, CSliderCtrl)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CCustomSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 计算点击位置对应的滑块值
    int newPos = CalculatePositionFromPoint(point);

    // 设置新位置
    SetPos(newPos);

    // 发送通知消息给父窗口
    GetParent()->SendMessage(WM_VSCROLL, MAKEWPARAM(TB_THUMBPOSITION, newPos), (LPARAM)GetSafeHwnd());

    // 开始拖拽
    m_bDragging = true;
    SetCapture();

    // 不调用基类的OnLButtonDown以避免默认行为
    // CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CCustomSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bDragging)
    {
        m_bDragging = false;
        ReleaseCapture();

        // 发送最终位置通知
        int pos = GetPos();
        GetParent()->SendMessage(WM_VSCROLL, MAKEWPARAM(TB_THUMBPOSITION, pos), (LPARAM)GetSafeHwnd());
    }

    CSliderCtrl::OnLButtonUp(nFlags, point);
}

void CCustomSliderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bDragging && (nFlags & MK_LBUTTON))
    {
        // 计算鼠标位置对应的滑块值
        int newPos = CalculatePositionFromPoint(point);

        // 设置新位置
        SetPos(newPos);

        // 获取滑块的范围
        int min, max;
        GetRange(min, max);

        // 检查是否滚动到边缘
        GetParent()->SendMessage(MSG_SILDERCTRL_ISSLIDERMOVING, (WPARAM)newPos, (LPARAM)GetSafeHwnd());


        // 发送通知消息
        GetParent()->SendMessage(WM_VSCROLL, MAKEWPARAM(TB_THUMBTRACK, newPos), (LPARAM)GetSafeHwnd());
    }

    CSliderCtrl::OnMouseMove(nFlags, point);
}

int CCustomSliderCtrl::CalculatePositionFromPoint(CPoint point)
{
    CRect rect;
    GetClientRect(&rect);

    int min, max;
    GetRange(min, max);

    // 获取滑块方向
    DWORD style = GetStyle();
    bool isVertical = (style & TBS_VERT) != 0;

    int pos;
    if (isVertical)
    {
        // 垂直滑块 - 从上到下递增
        int height = rect.Height();
        if (height <= 0) return min;

        pos = min + (point.y * (max - min)) / height;
    }
    else
    {
        // 水平滑块 - 从左到右递增
        int width = rect.Width();
        if (width <= 0) return min;

        pos = min + (point.x * (max - min)) / width;
    }

    // 确保位置在有效范围内
    if (pos < min) pos = min;
    if (pos > max) pos = max;

    return pos;
}

IMPLEMENT_DYNAMIC(Ui_ChildDlg, CDialogEx)

Ui_ChildDlg::Ui_ChildDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_CHILDDIALOG, pParent)
{
    m_IsHasCameraDeive = true;
    m_IsAudioSilent = false;
    m_IsMicSilent = false;
    m_IsStopping = false;
    m_recordElapsedSec = 0;
}

Ui_ChildDlg::~Ui_ChildDlg()
{
    KillTimer(1001);
    KillTimer(1002);

    if (m_recordTimerRect.sb_bkB)
        delete m_recordTimerRect.sb_bkB;
    if (m_recordTimerRect.sb_borderPenB)
        delete m_recordTimerRect.sb_borderPenB;
    if (m_recordTimerRect.sb_CurRecordSize)
        delete m_recordTimerRect.sb_CurRecordSize;
    if (m_recordTimerRect.sb_timeTextB)
        delete m_recordTimerRect.sb_timeTextB;
}

void Ui_ChildDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, CHILDDLG_BTN_RETURN, m_Btn_Return);
    DDX_Control(pDX, CHILDDLG_BTN_SYSTEMAUDIO, m_Btn_SysteamAudio);
    DDX_Control(pDX, CHILDDLG_SLIDER_SYSTEMAUDIO, m_Slider_SystemAudio);
    DDX_Control(pDX, CHILDDLG_BTN_MICRO, m_Btn_Micro);
    DDX_Control(pDX, CHILDDLG_SLIDER_MICRO, m_Slider_Micro);
    DDX_Control(pDX, CHILDDLG_BTN_CAMERA, m_Btn_Camera);
    DDX_Control(pDX, CHILDDLG_BTN_STARTRECORD, m_Btn_StartRecording);
    DDX_Control(pDX, CHILDDLG_STAT_SYSTEMAUDIO, m_Stat_SystemAudio);
    DDX_Control(pDX, CHILDDLG_STAT_MICRO, m_Stat_Micro);
    DDX_Control(pDX, CHILDDLG_STAT_CAMERA, m_Stat_Camera);
    DDX_Control(pDX, CHILDDLG_BTN_FOLLOWMOUSERECORD, m_Btn_MouseRecordArea);
    DDX_Control(pDX, CHILDDLG_BTN_MOUSEAREAPRESET, m_Btn_MouseRecordAreaPreset);
    DDX_Control(pDX, CHILDDLG_BTN_CARMERAOPTION, m_Btn_CameraOption);
    DDX_Control(pDX, CHILDDLG_STAT_HOTKEYSTARTRECORD, m_stat_HotkeyStartRecord);
    DDX_Control(pDX, CHILDDLG_BTN_PAUSE, m_btn_pause);
    DDX_Control(pDX, CHILDDLG_BTN_STOPRECORDINGDURINGRECORD, m_btn_stopRecordingDuringRecord);
    DDX_Control(pDX, CHILDDLG_BTN_RESUME, m_btn_resume);
    DDX_Control(pDX, CHILDDLG_BTN_COMBOX, m_btn_Combox);
}

void Ui_ChildDlg::GetUserDPI()
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

void Ui_ChildDlg::initCtrlPos()
{
    //系统声音按钮74 70
    float SystemAudioWidth = 49.3 * m_Scale;
    float SystemAudioHeight = 46.6 * m_Scale;
    float SystemAudioX = 0.121 * m_WindowWidth;
    float SystemAudioY = (m_WindowHeight - SystemAudioHeight) / 2;
    m_Btn_SysteamAudio.MoveWindow(SystemAudioX, SystemAudioY, SystemAudioWidth, SystemAudioHeight);

    //麦克风按钮 - 与系统声音按钮Y坐标相同，大小也相同
    float MicroX = 0.336 * m_WindowWidth;
    float MicroY = SystemAudioY;  // 与系统声音Y坐标相同
    float MicroWidth = SystemAudioWidth;
    float MicroHeight = SystemAudioHeight;
    m_Btn_Micro.MoveWindow(MicroX, MicroY, MicroWidth, MicroHeight);

    //摄像头按钮 - 与系统声音按钮Y坐标相同，大小也相同
    float CameraX = 0.579 * m_WindowWidth;
    float CameraY = SystemAudioY + 2 * m_Scale;  // 与系统声音Y坐标相同
    float CameraWidth = SystemAudioWidth;
    float CameraHeight = SystemAudioHeight;
    m_Btn_Camera.MoveWindow(CameraX, CameraY, CameraWidth, CameraHeight);

    //跟随鼠标录制按钮
    float MouseAreaRecordBtnWidth = SystemAudioWidth - 12 * m_Scale;
    float MouseAreaRecordBtnHeight = SystemAudioHeight - 6 * m_Scale;
    float MouseAreaRecordBtnX = CameraX + CameraWidth + 15 * m_Scale;
    float MouseAreaRecordBtnY = CameraY + (CameraHeight - MouseAreaRecordBtnHeight) / 2;
    m_Btn_MouseRecordArea.MoveWindow(MouseAreaRecordBtnX, MouseAreaRecordBtnY,
        MouseAreaRecordBtnWidth, MouseAreaRecordBtnHeight);

    //开始录制按钮
    float StartRecordingWidth = 106 * m_Scale;
    float StartRecordingHeight = 106 * m_Scale;
    float StartRecordingX = 0.796 * m_WindowWidth;
    float StartRecordingY = (m_WindowHeight - StartRecordingHeight) / 2;
    m_Btn_StartRecording.MoveWindow(StartRecordingX, StartRecordingY, StartRecordingWidth, StartRecordingHeight);

    //快捷键文本提示（Alt + B）
    float HotKeySRW = StartRecordingWidth + 100 * m_Scale;
    float HotKeySRH = 20 * m_Scale;
    float HotKeySRX = StartRecordingX + (StartRecordingWidth - HotKeySRW) / 2;
    float HotKeySRY = StartRecordingY + StartRecordingHeight + 5 * m_Scale;
    m_stat_HotkeyStartRecord.MoveWindow(HotKeySRX, HotKeySRY, HotKeySRW, HotKeySRH);

    //返回按钮27 35
    float ReturnX = 20 * m_Scale;
    float ReturnY = 10 * m_Scale;
    float ReturnWidth = 18 * m_Scale;
    float ReturnHeight = 23.3 * m_Scale;
    m_Btn_Return.MoveWindow(ReturnX, ReturnY, ReturnWidth, ReturnHeight);

    //下拉库录制选项m_btn_Combox 147 36
    float comboxW = 98 * m_Scale;
    float comboxH = 24 * m_Scale;
    float comboxX = ReturnX + ReturnWidth + 10 * m_Scale;
    float comboxY = ReturnY + (ReturnHeight - comboxH) / 2;
    m_btn_Combox.MoveWindow(comboxX, comboxY, comboxW, comboxH);

    //系统声音滑块
    float SystemAudioSliderX = SystemAudioX + SystemAudioWidth + 15 * m_Scale;
    float SystemAudioSliderY = SystemAudioY;
    float SystemAudioSliderWidth = 56.6 * m_Scale;
    float SystemAudioSliderHeight = 40 * m_Scale;
    m_Slider_SystemAudio.MoveWindow(SystemAudioSliderX, SystemAudioSliderY, SystemAudioSliderHeight, SystemAudioSliderWidth);

    //麦克风滑块
    float MicroSliderX = MicroX + MicroWidth + 15 * m_Scale;
    float MicroSliderY = MicroY;
    float MicroSliderWidth = SystemAudioSliderWidth;
    float MicroSliderHeight = 40 * m_Scale;
    m_Slider_Micro.MoveWindow(MicroSliderX, MicroSliderY, MicroSliderHeight, MicroSliderWidth);

    //系统声音文本
    float SystemAudioStatWidth = 73 * m_Scale;
    float SystemAudioStatHeight = 35 * m_Scale;
    float SystemAudioStatX = SystemAudioX + (SystemAudioWidth - SystemAudioStatWidth) / 2;
    float SystemAudioStatY = SystemAudioY + SystemAudioHeight + 5 * m_Scale;
    m_Stat_SystemAudio.MoveWindow(SystemAudioStatX, SystemAudioStatY, SystemAudioStatWidth, SystemAudioStatHeight);

    //麦克风文本
    float MicroStatWidth = 73 * m_Scale;
    float MicroStatHeight = 35 * m_Scale;
    float MicroStatX = MicroX + (MicroWidth - MicroStatWidth) / 2;
    float MicroStatY = MicroY + MicroHeight + 5 * m_Scale;
    m_Stat_Micro.MoveWindow(MicroStatX, MicroStatY, MicroStatWidth, MicroStatHeight);

    //摄像头文本
    float CameraStatWidth = CameraWidth + 120 * m_Scale;
    float CameraStatHeight = 55 * m_Scale;
    float CameraStatX = CameraX + (CameraWidth - CameraStatWidth) / 2 + 3 * m_Scale;
    float CameraStatY = CameraY + CameraHeight - 5 * m_Scale;
    m_Stat_Camera.MoveWindow(CameraStatX, CameraStatY, CameraStatWidth, CameraStatHeight);
    m_Btn_CameraOption.MoveWindow(CameraStatX, CameraStatY, CameraStatWidth, CameraStatHeight);

    //跟随录制文本按钮
    float MouseAreaRecordStatWidth = 0.135 * m_WindowWidth;
    float MouseAreaRecordStatHeight = 35 * m_Scale;
    float MouseAreaRecordStatX = MouseAreaRecordBtnX + (MouseAreaRecordBtnWidth - MouseAreaRecordStatWidth) / 2 + 3 * m_Scale;
    float MouseAreaRecordStatY = CameraStatY + 11 * m_Scale;
    m_Btn_MouseRecordAreaPreset.MoveWindow(MouseAreaRecordStatX, MouseAreaRecordStatY,
        MouseAreaRecordStatWidth, MouseAreaRecordStatHeight);

    m_btn_pause.ShowWindow(SW_HIDE);
    m_btn_stopRecordingDuringRecord.ShowWindow(SW_HIDE);
    m_btn_resume.ShowWindow(SW_HIDE);
    m_Btn_StartRecording.ShowWindow(SW_SHOW);
}

void Ui_ChildDlg::initRecordingCtrlPos()
{
    int moveOffset = 55 * m_Scale;
    int groupSpacingX = 65 * m_Scale;
    if (m_IsOnlyAudioRecord)
    {
        moveOffset = 15 * m_Scale;
        groupSpacingX = 105 * m_Scale;
    }

    initCtrlPos();
    initRecordTimerRect();

    CRect SysteamAudioR, SystemAudioTxt, SystemAudioSliderBarR;
    m_Btn_SysteamAudio.GetWindowRect(SysteamAudioR);
    m_Stat_SystemAudio.GetWindowRect(SystemAudioTxt);
    m_Slider_SystemAudio.GetWindowRect(SystemAudioSliderBarR);
    ScreenToClient(SysteamAudioR);
    ScreenToClient(SystemAudioTxt);
    ScreenToClient(SystemAudioSliderBarR);
    // 整体水平偏移（只改 X，不改 Y）
    SysteamAudioR.OffsetRect(-moveOffset, 0);
    SystemAudioTxt.OffsetRect(-moveOffset, 0);
    SystemAudioSliderBarR.OffsetRect(-moveOffset, 0);

    CRect MicroR, MicroTxt, MicroSliderR;
    m_Btn_Micro.GetWindowRect(MicroR);
    m_Stat_Micro.GetWindowRect(MicroTxt);
    m_Slider_Micro.GetWindowRect(MicroSliderR);
    ScreenToClient(MicroR);
    ScreenToClient(MicroTxt);
    ScreenToClient(MicroSliderR);
    // 先整体水平偏移相同量（保持 Y 不变）
    MicroR.OffsetRect(-moveOffset, 0);
    MicroTxt.OffsetRect(-moveOffset, 0);
    MicroSliderR.OffsetRect(-moveOffset, 0);
    // 将 Micro 组放到 SystemAudio 组右侧，间距为 groupSpacingX
    int desiredMicroLeft = SysteamAudioR.right + groupSpacingX;
    int microOffsetX = desiredMicroLeft - MicroR.left;
    MicroR.OffsetRect(microOffsetX, 0);
    MicroTxt.OffsetRect(microOffsetX, 0);
    MicroSliderR.OffsetRect(microOffsetX, 0);

    CRect CarmeraR, CarmeraTxt, CarmeraOptR;
    m_Btn_Camera.GetWindowRect(CarmeraR);
    m_Stat_Camera.GetWindowRect(CarmeraTxt);
    m_Btn_CameraOption.GetWindowRect(CarmeraOptR);
    ScreenToClient(CarmeraR);
    ScreenToClient(CarmeraTxt);
    ScreenToClient(CarmeraOptR);
    // 整体水平偏移
    CarmeraR.OffsetRect(-moveOffset, 0);
    CarmeraTxt.OffsetRect(-moveOffset, 0);
    CarmeraOptR.OffsetRect(-moveOffset, 0);
    // 将 Camera 组放到 Micro 组右侧，间距为 groupSpacingX
    int desiredCameraLeft = MicroR.right + groupSpacingX;
    int camOffsetX = desiredCameraLeft - CarmeraR.left;
    CarmeraR.OffsetRect(camOffsetX, 0);
    CarmeraTxt.OffsetRect(camOffsetX, 0);
    CarmeraOptR.OffsetRect(camOffsetX, 0);

    m_Btn_Camera.MoveWindow(CarmeraR);
    m_Stat_Camera.MoveWindow(CarmeraTxt);
    m_Btn_CameraOption.MoveWindow(CarmeraOptR);
    m_Btn_SysteamAudio.MoveWindow(SysteamAudioR);
    m_Stat_SystemAudio.MoveWindow(SystemAudioTxt);
    m_Slider_SystemAudio.MoveWindow(SystemAudioSliderBarR);
    m_Btn_Micro.MoveWindow(MicroR);
    m_Stat_Micro.MoveWindow(MicroTxt);
    m_Slider_Micro.MoveWindow(MicroSliderR);

    int WidthHeightPng = 72 * m_Scale;
    int WidthHeightPngResume = 81 * m_Scale;
    int x1 = 594 * m_Scale;
    int x2 = x1 + WidthHeightPng + 20 * m_Scale;
    int y = 59 * m_Scale;
    int x3 = x1 - 4.5 * m_Scale;
    int y3 = y - 4.5 * m_Scale;
    m_btn_pause.MoveWindow(x1, y, WidthHeightPng, WidthHeightPng);
    m_btn_stopRecordingDuringRecord.MoveWindow(x2, y, WidthHeightPng, WidthHeightPng);
    m_btn_resume.MoveWindow(x3, y3, WidthHeightPngResume, WidthHeightPngResume);

    m_stat_HotkeyStartRecord.ShowWindow(SW_HIDE);
    m_Btn_StartRecording.ShowWindow(SW_HIDE);
    m_Btn_MouseRecordArea.ShowWindow(SW_HIDE);
    m_Btn_MouseRecordAreaPreset.ShowWindow(SW_HIDE);
    m_btn_pause.ShowWindow(SW_SHOW);
    m_btn_stopRecordingDuringRecord.ShowWindow(SW_SHOW);
}

void Ui_ChildDlg::initCtrl()
{
    // 系统声音按钮
    m_Btn_SysteamAudio.LoadPNG(CHILDDLG_PNG_SYSTEMAUDIO);
    m_Btn_SysteamAudio.SetBackgroundColor(RGB(27, 31, 37));

    // 麦克风按钮
    m_Btn_Micro.LoadPNG(GAMINGDLG_PNG_MICROUNABLE);
    m_Btn_Micro.SetBackgroundColor(RGB(27, 31, 37));

    // 摄像头按钮
    m_Btn_Camera.LoadPNG(CHILDDLG_PNG_CAMERA);
    m_Btn_Camera.SetBackgroundColor(RGB(27, 31, 37));

    // 开始录制按钮
    m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_STARTRECORD);
    m_Btn_StartRecording.SetBackgroundColor(RGB(27, 31, 37));

    //停止录制按钮
    m_btn_stopRecordingDuringRecord.LoadPNG(CHILDDLG_PNG_STOPRECORDING);
    m_btn_stopRecordingDuringRecord.LoadClickPNG(CHILDDLG_PNG_STOPHOV);
    m_btn_stopRecordingDuringRecord.SetBackgroundColor(RGB(26, 31, 37));
    m_btn_stopRecordingDuringRecord.SetUseHoverImage(TRUE);

    //暂停按钮
    m_btn_pause.LoadPNG(CHILDDLG_PNG_PAUSE);
    m_btn_pause.LoadClickPNG(CHILDDLG_PNG_STOPRECORDINGHOV);
    m_btn_pause.SetBackgroundColor(RGB(26, 31, 37));
    m_btn_pause.SetUseHoverImage(TRUE);

    //恢复播放按钮
    m_btn_resume.LoadPNG(CHILDDLG_PNG_RESUME);
    m_btn_resume.LoadClickPNG(CHILDDLG_PNG_RESUMEHOV);
    m_btn_resume.SetBackgroundColor(RGB(26, 31, 37));
    m_btn_resume.SetUseHoverImage(TRUE);

    // 返回按钮
    m_Btn_Return.LoadPNG(CHILDDLG_PNG_RETURN);
    m_Btn_Return.SetBackgroundColor(RGB(27, 31, 37));

    //录制选项下拉框
    m_btn_Combox.LarSetNormalFiilBrush(SolidBrush(Color(31, 36, 37)));
    m_btn_Combox.LarSetHoverFillBrush(SolidBrush(Color(35, 41, 42)));
    m_btn_Combox.LarSetClickedFillBrush(SolidBrush(Color(35, 41, 42)));
    m_btn_Combox.LaSetTextColor(Color(234, 235, 235));
    m_btn_Combox.LaSetTextHoverColor(Color(244, 245, 245));
    m_btn_Combox.LaSetTextClickedColor(Color(254, 255, 255));
    m_btn_Combox.LarSetTextSize(17);
    m_btn_Combox.LarSetBtnNailImage(MAINDLG_PNG_DOWN,
        CLarBtn::NailImageLayout::Right, 14 * m_Scale, 10 * m_Scale, -5 * m_Scale
    );
    m_btn_Combox.LarAdjustTextDisplayPos(-18 * m_Scale, 0);
    m_btn_Combox.LarSetBorderColor(Color(73, 73, 73));

    // 设置文本属性
    m_Stat_SystemAudio.LarSetTextSize(20);
    m_Stat_Micro.LarSetTextSize(20);
    m_Stat_Camera.LarSetTextSize(20);
    m_Stat_Camera.LarSetTextColor(RGB(155, 155, 155));
    m_Stat_Camera.LarSetTextLeft();
    m_Stat_Camera.ShowWindow(SW_HIDE);

    m_stat_HotkeyStartRecord.LarSetTextSize(20);
    m_stat_HotkeyStartRecord.LarSetTextCenter();
    m_stat_HotkeyStartRecord.LarSetTextColor(RGB(255, 255, 255));

    //鼠标区域录制预设下拉框
    m_Btn_MouseRecordAreaPreset.LarSetTextSize(20);
    m_Btn_MouseRecordAreaPreset.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
    m_Btn_MouseRecordAreaPreset.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
    m_Btn_MouseRecordAreaPreset.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
    m_Btn_MouseRecordAreaPreset.LarSetBorderColor(Gdiplus::Color(255, 26, 31, 37));
    m_Btn_MouseRecordAreaPreset.LarSetEraseBkEnable(false);
    m_Btn_MouseRecordAreaPreset.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 31, 37)));

    //摄像头选项下拉框
    m_Btn_CameraOption.LarSetTextSize(20);
    m_Btn_CameraOption.LaSetTextColor(Gdiplus::Color(255, 255, 255, 255));
    m_Btn_CameraOption.LaSetTextHoverColor(Gdiplus::Color(255, 245, 245, 245));
    m_Btn_CameraOption.LaSetTextClickedColor(Gdiplus::Color(255, 235, 235, 235));
    m_Btn_CameraOption.LarSetBorderColor(Gdiplus::Color(255, 26, 31, 37));
    m_Btn_CameraOption.LarSetEraseBkEnable(false);
    m_Btn_CameraOption.LarSetNormalFiilBrush(SolidBrush(Color(255, 26, 31, 37)));

    //设置系统声音滑块
    m_Slider_SystemAudio.SetRange(0, 100, false);
    m_Slider_SystemAudio.SetPos(0);

    //设置麦克风滑块
    m_Slider_Micro.SetRange(0, 100, false);
    m_Slider_Micro.SetPos(0);

    //跟随鼠标录制下拉框
    CRect Rect1;
    m_Btn_MouseRecordAreaPreset.GetWindowRect(Rect1);
    int listWidth1 = Rect1.Width() + 15 * m_Scale;
    int listHeight1 = Rect1.Height() / 2 + 10 * m_Scale;
    m_ListBoxs.addListBox(listWidth1, listHeight1, this, m_Array_MouseRecordAreaPreset, L"鼠标跟随区域下拉框");
    m_ListBoxs.SetMaxDisplayItems(4, L"鼠标跟随区域下拉框");
    CString str1(L"区域大小\n");
    str1 += m_Array_MouseRecordAreaPreset.GetAt(m_Array_MouseRecordAreaPreset.GetCount() - 1);
    str1 += L"▼";
    m_Btn_MouseRecordAreaPreset.SetWindowTextW(str1);

    //摄像头下拉框
    CRect Rect2;
    m_Btn_CameraOption.GetWindowRect(Rect2);
    int btnWidth2 = Rect2.Width();
    int btnHeight2 = Rect2.Height() / 2;
    m_ListBoxs.addListBox(btnWidth2, btnHeight2, this, m_Array_CarmerOptions, L"摄像头选项下拉框");
    if (m_IsHasCameraDeive)
        m_ListBoxs.SetMaxDisplayItems(4, L"摄像头选项下拉框");
    CString str2(L"摄像头选项:\n");
    str2 += m_Array_CarmerOptions.GetAt(m_Array_CarmerOptions.GetCount() - 1);
    str2 += L"▼";
    m_Btn_CameraOption.SetWindowTextW(str2);

    //录制选项下拉框
    CRect rect;
    m_btn_Combox.GetWindowRect(rect);
    int listW = rect.Width();
    int itemH = rect.Height();
    m_ListBoxs.addListBox(listW, itemH, this, m_Array_RecordOptions, L"录制选项下拉框");
    m_btn_Combox.SetWindowTextW(m_Array_RecordOptions.GetAt(0));

    //设置跟随鼠标录制按钮
    m_Btn_MouseRecordArea.LoadPNG(CHILDDLG_PNG_MOUSEAREARECORD);
    m_Btn_MouseRecordArea.SetBackgroundColor(RGB(26, 31, 37));
    m_Btn_MouseRecordArea.ShowWindow(SW_HIDE);
    m_Btn_MouseRecordAreaPreset.ShowWindow(SW_HIDE);

    //下拉框整体样式
    m_ListBoxs.SetTextSize(12, L"");                //设置所有下拉框的字体大小
    m_ListBoxs.SetScrollbarWidth(10 * m_Scale);     //设置所有滑块的大小
    m_ListBoxs.SetListBoxHideWhenMouseLeave(false); //设置下拉框不自动隐藏
    m_ListBoxs.SetMaxDisplayItems(7, L"录制选项下拉框");  
    m_ListBoxs.SetBackColor(RGB(26, 31, 37));
    m_ListBoxs.SetScrollbarColors(RGB(26, 31, 37), RGB(73, 73, 73), RGB(26, 31, 37));
    m_ListBoxs.SetHoverColor(RGB(73, 73, 73));
}

void Ui_ChildDlg::InitDropListData()
{
    // 获取当前屏幕分辨率并添加简单的分辨率预设
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 添加当前完整分辨率
    CString currentRes;
    currentRes.Format(L"%dx%d", screenWidth, screenHeight);
    m_Array_MouseRecordAreaPreset.Add(currentRes);

    // 添加半屏分辨率
    int halfWidth = screenWidth / 2;
    int halfHeight = screenHeight / 2;

    // 确保是偶数
    halfWidth = halfWidth - (halfWidth % 2);
    halfHeight = halfHeight - (halfHeight % 2);
    CString halfRes;
    halfRes.Format(L"%dx%d", halfWidth, halfHeight);
    m_Array_MouseRecordAreaPreset.Add(halfRes);

    // 添加几个常用标准分辨率
    m_Array_MouseRecordAreaPreset.Add(L"1920x1080");
    m_Array_MouseRecordAreaPreset.Add(L"1280x720");
    m_Array_MouseRecordAreaPreset.Add(L"800x600");

    //添加摄像头选项
    auto deviceInfos = DeviceManager::GetInstance().GetCameraDevices();
    if (deviceInfos.size() != 0)
    {
        m_string_CameraName = deviceInfos.at(deviceInfos.size() - 1).nameA;
        auto CameraOptions = DeviceManager::GetInstance().GetCameraCapabilities(m_string_CameraName);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"ChildDlg界面：添加摄像头名%s", LARSC::s2ws(m_string_CameraName).c_str());
        for (auto& Option : CameraOptions)
        {
            CString PixelWHStr;
            PixelWHStr.Format(L"%dx%d,%dFPS", Option.width, Option.height, (int)Option.fps);
            m_Array_CarmerOptions.Add(PixelWHStr);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"添加选项:%s,格式:%s,摄像头名称:%s",
                PixelWHStr, LARSC::s2ws(Option.vcodec).c_str(), LARSC::s2ws(m_string_CameraName).c_str());
        }
    }
    else
    {//没有摄像头设备
        DEBUG_CONSOLE_STR(ConsoleHandle, L"没有摄像头设备");
        m_Array_CarmerOptions.Add(L"无可用设备");
        m_IsHasCameraDeive = false;
    }

    // 录制选项列表
    m_Array_RecordOptions.Add(L"录全屏");
    m_Array_RecordOptions.Add(L"录区域");
    m_Array_RecordOptions.Add(L"录游戏");
    m_Array_RecordOptions.Add(L"录窗口");
    m_Array_RecordOptions.Add(L"录摄像头");
    m_Array_RecordOptions.Add(L"录声音");
    m_Array_RecordOptions.Add(L"跟随鼠标");

}

void Ui_ChildDlg::SetRecordOptBtnText(CString text)
{
    m_btn_Combox.SetWindowTextW(text);
}

bool Ui_ChildDlg::InitTotalDiskSizeFromPath(const std::wstring& path)
{
    if (path.size() < 2 || path[1] != L':')
        return false;

    wchar_t rootPath[4] = { 0 };
    rootPath[0] = path[0];
    rootPath[1] = L':';
    rootPath[2] = L'\\';
    rootPath[3] = 0;

    ULARGE_INTEGER freeAvail{}, totalBytes{}, totalFree{};
    if (GetDiskFreeSpaceExW(rootPath, &freeAvail, &totalBytes, &totalFree))
    {
        // totalFree磁盘当前总体可用空间
        double gb = (double)totalFree.QuadPart / (1024.0 * 1024.0 * 1024.0);
        m_totalDiskSizeGB = gb;          // 保留两位小数
        m_hasTotalDiskSize = true;
        return true;
    }
    return false;
}

void Ui_ChildDlg::UpdateRecordSizeUI(double sizeMB)
{
    double value = 0.0;
    const wchar_t* unit = L"MB";

    if (sizeMB < 1.0)
    {   // 使用 KB
        value = sizeMB * 1024.0;
        unit = L"KB";
    }
    else if (sizeMB < 1024.0)
    {   // 使用 MB
        value = sizeMB;
        unit = L"MB";
    }
    else
    {   // 使用 GB
        value = sizeMB / 1024.0;
        unit = L"GB";
    }

    wchar_t leftBuf[64] = { 0 };
    swprintf_s(leftBuf, L"%.2f %s", value, unit);

    wchar_t rightBuf[32] = { 0 };
    if (m_hasTotalDiskSize && m_totalDiskSizeGB > 0.0)
    {
        swprintf_s(rightBuf, L"/%.2fGB", m_totalDiskSizeGB);// 右侧固定 GB 单位，不加小数
    }
    else
    {
        wcscpy_s(rightBuf, L"/--GB");
    }
    std::wstring leftNoExtraSpace = leftBuf;
    m_recordTimerRect.sizeStr = leftNoExtraSpace + rightBuf;
}

void Ui_ChildDlg::initRecordTimerRect()
{
    m_recordTimerRect.rect_timeTextBkArea.Width = 159 * m_Scale;
    m_recordTimerRect.rect_timeTextBkArea.Height = 51 * m_Scale;
    m_recordTimerRect.rect_timeTextBkArea.X = 410 * m_Scale;
    m_recordTimerRect.rect_timeTextBkArea.Y = 58 * m_Scale;

    m_recordTimerRect.rect_timerTextArea.Width = 136 * m_Scale;
    m_recordTimerRect.rect_timerTextArea.Height = 27 * m_Scale;
    m_recordTimerRect.rect_timerTextArea.X =
        m_recordTimerRect.rect_timeTextBkArea.X +
        (m_recordTimerRect.rect_timeTextBkArea.Width - m_recordTimerRect.rect_timerTextArea.Width) / 2;
    m_recordTimerRect.rect_timerTextArea.Y =
        m_recordTimerRect.rect_timeTextBkArea.Y +
        (m_recordTimerRect.rect_timeTextBkArea.Height - m_recordTimerRect.rect_timerTextArea.Height) / 2;

    m_recordTimerRect.rect_CurRecordSizeArea.Width = 133 * m_Scale;
    m_recordTimerRect.rect_CurRecordSizeArea.Height = 30 * m_Scale;
    m_recordTimerRect.rect_CurRecordSizeArea.X = m_recordTimerRect.rect_timeTextBkArea.X
        + (m_recordTimerRect.rect_timeTextBkArea.Width - m_recordTimerRect.rect_CurRecordSizeArea.Width) / 2;
    m_recordTimerRect.rect_CurRecordSizeArea.Y = m_recordTimerRect.rect_timeTextBkArea.Y
        + m_recordTimerRect.rect_timeTextBkArea.Height + 5 * m_Scale;

    if (!m_recordTimerRect.sb_bkB)
        m_recordTimerRect.sb_bkB = new SolidBrush(Color(16, 16, 16));
    if (!m_recordTimerRect.sb_borderPenB)
        m_recordTimerRect.sb_borderPenB = new Pen(&SolidBrush(Color(52, 52, 52)), 2.0f);
    if (!m_recordTimerRect.sb_CurRecordSize)
        m_recordTimerRect.sb_CurRecordSize = new SolidBrush(Color(209, 209, 209));
    if (!m_recordTimerRect.sb_timeTextB)
        m_recordTimerRect.sb_timeTextB = new SolidBrush(Color(255, 255, 255));
    if (!m_recordTimerRect.sizeFont)
        m_recordTimerRect.sizeFont = new Gdiplus::Font(L"微软雅黑", 6 * m_Scale);
    if (!m_recordTimerRect.timerFont)
        m_recordTimerRect.timerFont = new Gdiplus::Font(L"微软雅黑", 15 * m_Scale);

    m_recordTimerRect.timerStr = L"00:00:00";
    m_recordTimerRect.sizeStr = L"000 KB/000 GB";
}

LRESULT Ui_ChildDlg::OnBnClickedBtnListBoxSelected(WPARAM wParam, LPARAM lParam)
{
    MsgParam::LISTBOX_SELECT_INFO* pInfo = (MsgParam::LISTBOX_SELECT_INFO*)wParam;
    if (pInfo)
    {
        int nIndex = pInfo->nIndex;
        CString strText = pInfo->strText;
        std::wstring strBoxName = pInfo->strBoxName;
        // 根据不同的下拉框名称进行不同处理
        if (strBoxName == L"摄像头选项下拉框")
        {
            // 提取分辨率数字
            int width = 0, height = 0, fps = 0;
            wchar_t cameraBufW[128] = { 0 };
            int scanResult = swscanf_s(strText, L"%dx%d,%dFPS\n格式:%s",
                &width, &height, &fps,
                cameraBufW, (unsigned)_countof(cameraBufW));
            if (scanResult == 4 && width > 0 && height > 0 && cameraBufW != L"")
            {
                // 记录摄像头选项
                m_CameraOption_Fps = fps;
                m_CameraOption_Width = width;
                m_CameraOption_Height = height;
                m_string_CameraPix = LARSC::w2c(cameraBufW);
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"设置摄像头选项: %dx%d,%dFPS,格式:%s",
                    width, height, fps, cameraBufW);
            }

            //更新按钮文本
            CString str = L"摄像头选项:\n";
            str += strText;
            str += L"▼";
            m_Btn_CameraOption.SetWindowText(str);
            m_Btn_CameraOption.LarSetTextSize(20);
        }
        else if (strBoxName == L"鼠标跟随区域下拉框")
        {
            // 提取分辨率数字
            int width = 0, height = 0;
            int scanResult = swscanf_s(strText, L"%dx%d", &width, &height);
            if (scanResult == 2 && width > 0 && height > 0)
            {
                // 设置给记录区域矩形
                m_CRect_RecordRect.left = 0;
                m_CRect_RecordRect.top = 0;
                m_CRect_RecordRect.right = width;
                m_CRect_RecordRect.bottom = height;
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"设置跟随鼠标录制区域大小: %dx%d", width, height);
            }

            //跟随区域
            CString str = L"跟随区域大小\n";
            str += strText;
            str += L"▼";
            m_Btn_MouseRecordAreaPreset.SetWindowText(str);
            m_Btn_MouseRecordAreaPreset.LarSetTextSize(20);
        }
        else if (strBoxName == L"录制选项下拉框")
        {
            // 设置按钮文本
            CString btnText;
            m_btn_Combox.GetWindowTextW(btnText);
            m_btn_Combox.SetWindowTextW(strText);

            // 获取主窗口指针
            auto* pMain = reinterpret_cast<Ui_MainDlg*>(GetParent());
            if (!pMain || !::IsWindow(pMain->GetSafeHwnd()))
                return 1;
            if (strText == L"录全屏" && btnText != L"录全屏")
            {
                OnBnClickedBtnReturn();
                pMain->PostMessage(
                    WM_COMMAND,
                    MAKEWPARAM(MainDlg_Btn_ScreenRecording, BN_CLICKED),
                    (LPARAM)pMain->m_Btn_AreaRecord.GetSafeHwnd()
                );
            }
            else if (strText == L"录区域" && btnText != L"录区域")
            {
                OnBnClickedBtnReturn();
                std::thread([pMain]()
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        pMain->PostMessage(
                            WM_COMMAND,
                            MAKEWPARAM(MAINDLG_BTN_AREARECORDING, BN_CLICKED),
                            (LPARAM)pMain->m_Btn_AreaRecord.GetSafeHwnd()
                        );
                    }).detach();
            }
            else if (strText == L"录游戏")
            {
                OnBnClickedBtnReturn();
                pMain->PostMessage(
                    WM_COMMAND,
                    MAKEWPARAM(MAINDLG_BTN_GAMINGRECORDING, BN_CLICKED),
                    (LPARAM)pMain->m_Btn_AreaRecord.GetSafeHwnd()
                );
            }
            else if (strText == L"录窗口")
            {
                OnBnClickedBtnReturn();
                pMain->PostMessage(
                    WM_COMMAND,
                    MAKEWPARAM(MAINDLG_BTN_APPWINDOWRECORDING, BN_CLICKED),
                    (LPARAM)pMain->m_Btn_AreaRecord.GetSafeHwnd()
                );
            }
            else if (strText == L"录摄像头")
            {
                OnBnClickedBtnReturn();
                ::PostMessage(pMain->GetSafeHwnd(), MSG_UIDROPDOWNMENU_CAMERARECORD, NULL, NULL);
            }
            else if (strText == L"录声音" && btnText != L"录声音")
            {
                ::PostMessage(pMain->GetSafeHwnd(), MSG_UIDROPDOWNMENU_SYSTEMAUDIOMICRO, 0, 0);
            }
            else if (strText == L"跟随鼠标" && btnText != L"跟随鼠标")
            {
                ::PostMessage(pMain->GetSafeHwnd(), MSG_UIDROPDOWNMENU_MOUSERECORD, 0, 0);
            }
        }
    }
    return 1;
}

void Ui_ChildDlg::OnBnClickedBtnMouseareapreset()
{
    if (m_ListBoxs.IsListBoxVisiable(L"鼠标跟随区域下拉框"))
    {
        m_ListBoxs.HideListBox();
    }
    else
    {
        //更新下拉框的显示位置
        CRect Rect;
        m_Btn_MouseRecordAreaPreset.GetWindowRect(&Rect);
        m_ListBoxs.UpdateDroplistXY(L"鼠标跟随区域下拉框", Rect.left, Rect.top + Rect.Height());
    }
}

void Ui_ChildDlg::OnBnClickedBtnCarmeraoption()
{
    //更新下拉框的显示位置
    if (m_ListBoxs.IsListBoxVisiable(L"摄像头选项下拉框"))
    {
        m_ListBoxs.HideListBox();
    }
    else
    {
        CRect Rect;
        m_Btn_CameraOption.GetWindowRect(&Rect);
        m_ListBoxs.UpdateDroplistXY(L"摄像头选项下拉框", Rect.left, Rect.top + Rect.Height());
    }
}

void Ui_ChildDlg::CleanUpGdiPngRes()
{
    m_Btn_Return.ClearImages();
    m_Btn_SysteamAudio.ClearImages();
    m_Btn_Micro.ClearImages();
    m_Btn_Camera.ClearImages();
    m_Btn_StartRecording.ClearImages();
}

void Ui_ChildDlg::SetDlgDisplayRect(CRect DisplayRect)
{
    m_WindowWidth = DisplayRect.Width();
    m_WindowHeight = DisplayRect.Height();
}

void Ui_ChildDlg::SetRecordArea(const CRect& AreaRect)
{
    m_CRect_RecordRect.left = AreaRect.left;
    m_CRect_RecordRect.right = AreaRect.right;
    m_CRect_RecordRect.top = AreaRect.top;
    m_CRect_RecordRect.bottom = AreaRect.bottom;
}

void Ui_ChildDlg::UpdateRecordMouseAreaUi()
{
    //更新控件显示位置
    CRect CameraRectBtn, CameraStatRect, CameraOptionBtn;
    m_Btn_Camera.GetWindowRect(CameraRectBtn);
    m_Stat_Camera.GetWindowRect(CameraStatRect);
    m_Btn_CameraOption.GetWindowRect(CameraOptionBtn);

    ScreenToClient(CameraRectBtn);
    ScreenToClient(CameraStatRect);
    ScreenToClient(CameraOptionBtn);
    int newX = CameraRectBtn.left - 66 * m_Scale;
    CameraRectBtn.MoveToX(newX);
    CameraStatRect.MoveToX(newX);
    CameraOptionBtn.MoveToX(newX - 58 * m_Scale);
    m_Btn_Camera.MoveWindow(CameraRectBtn);
    m_Stat_Camera.MoveWindow(CameraStatRect);
    m_Btn_CameraOption.MoveWindow(CameraOptionBtn);

    // 麦克风控件
    CRect MicroRectBtn, MicroStatRect;
    m_Btn_Micro.GetWindowRect(MicroRectBtn);
    m_Stat_Micro.GetWindowRect(MicroStatRect);

    ScreenToClient(MicroRectBtn);
    ScreenToClient(MicroStatRect);
    newX = MicroRectBtn.left - 43 * m_Scale;
    MicroRectBtn.MoveToX(newX);
    MicroStatRect.MoveToX(newX - 12 * m_Scale);
    m_Btn_Micro.MoveWindow(MicroRectBtn);
    m_Stat_Micro.MoveWindow(MicroStatRect);

    // 系统音频控件
    CRect SystemAudioRectBtn, SystemAudioStatRect;
    m_Btn_SysteamAudio.GetWindowRect(SystemAudioRectBtn);
    m_Stat_SystemAudio.GetWindowRect(SystemAudioStatRect);

    ScreenToClient(SystemAudioRectBtn);
    ScreenToClient(SystemAudioStatRect);
    newX = SystemAudioRectBtn.left - 18 * m_Scale;
    SystemAudioRectBtn.MoveToX(newX);
    SystemAudioStatRect.MoveToX(newX - 12 * m_Scale);
    m_Btn_SysteamAudio.MoveWindow(SystemAudioRectBtn);
    m_Stat_SystemAudio.MoveWindow(SystemAudioStatRect);

    //更新滑块控件位置
    CRect MicroSilder, AudioSilder;
    m_Slider_Micro.GetWindowRect(MicroSilder);
    m_Slider_SystemAudio.GetWindowRect(AudioSilder);
    ScreenToClient(MicroSilder);
    ScreenToClient(AudioSilder);
    int newXM = MicroSilder.left - 38 * m_Scale;
    int nexXA = AudioSilder.left - 12 * m_Scale;
    MicroSilder.MoveToX(newXM);
    AudioSilder.MoveToX(nexXA);
    m_Slider_Micro.MoveWindow(MicroSilder);
    m_Slider_SystemAudio.MoveWindow(AudioSilder);

    //显示控件
    m_Btn_MouseRecordArea.ShowWindow(SW_SHOW);
    m_Btn_MouseRecordAreaPreset.ShowWindow(SW_SHOW);
}

void Ui_ChildDlg::ResetRecordMouseAreaUi()
{
    //撤销控件位置移动效果
    CRect CameraRectBtn, CameraStatRect, CameraOptionBtn;
    m_Btn_Camera.GetWindowRect(CameraRectBtn);
    m_Stat_Camera.GetWindowRect(CameraStatRect);
    m_Btn_CameraOption.GetWindowRect(CameraOptionBtn);

    ScreenToClient(CameraRectBtn);
    ScreenToClient(CameraStatRect);
    ScreenToClient(CameraOptionBtn);
    int newX = CameraRectBtn.left + 66 * m_Scale; // 向右移动相同距离
    CameraRectBtn.MoveToX(newX);
    CameraStatRect.MoveToX(newX);
    CameraOptionBtn.MoveToX(newX - 58 * m_Scale);
    m_Btn_Camera.MoveWindow(CameraRectBtn);
    m_Stat_Camera.MoveWindow(CameraStatRect);
    m_Btn_CameraOption.MoveWindow(CameraOptionBtn);

    // 麦克风控件
    CRect MicroRectBtn, MicroStatRect;
    m_Btn_Micro.GetWindowRect(MicroRectBtn);
    m_Stat_Micro.GetWindowRect(MicroStatRect);

    ScreenToClient(MicroRectBtn);
    ScreenToClient(MicroStatRect);
    newX = MicroRectBtn.left + 40 * m_Scale; // 向右移动相同距离
    MicroRectBtn.MoveToX(newX);
    MicroStatRect.MoveToX(newX - 12 * m_Scale);
    m_Btn_Micro.MoveWindow(MicroRectBtn);
    m_Stat_Micro.MoveWindow(MicroStatRect);

    // 系统音频控件
    CRect SystemAudioRectBtn, SystemAudioStatRect;
    m_Btn_SysteamAudio.GetWindowRect(SystemAudioRectBtn);
    m_Stat_SystemAudio.GetWindowRect(SystemAudioStatRect);

    ScreenToClient(SystemAudioRectBtn);
    ScreenToClient(SystemAudioStatRect);
    newX = SystemAudioRectBtn.left + 40 * m_Scale; // 向右移动相同距离
    SystemAudioRectBtn.MoveToX(newX);
    SystemAudioStatRect.MoveToX(newX - 12 * m_Scale);
    m_Btn_SysteamAudio.MoveWindow(SystemAudioRectBtn);
    m_Stat_SystemAudio.MoveWindow(SystemAudioStatRect);

    //撤销滑块控件位置
    CRect MicroSilder, AudioSilder;
    m_Slider_Micro.GetWindowRect(MicroSilder);
    m_Slider_SystemAudio.GetWindowRect(AudioSilder);
    ScreenToClient(MicroSilder);
    ScreenToClient(AudioSilder);
    int newXM = MicroSilder.left + 40 * m_Scale; // 向右移动相同距离
    int nexXA = AudioSilder.left + 40 * m_Scale; // 向右移动相同距离
    MicroSilder.MoveToX(newXM);
    AudioSilder.MoveToX(nexXA);
    m_Slider_Micro.MoveWindow(MicroSilder);
    m_Slider_SystemAudio.MoveWindow(AudioSilder);

    //隐藏控件
    m_Btn_MouseRecordArea.ShowWindow(SW_HIDE);
    m_Btn_MouseRecordAreaPreset.ShowWindow(SW_HIDE);
}

void Ui_ChildDlg::UpdateAudioRecord()
{
    m_Btn_Camera.ShowWindow(SW_HIDE);
    m_Btn_CameraOption.ShowWindow(SW_HIDE);

    CRect AudioBtnRect, MicroBtnRect, AudioRect, MicroRect, silderA, silderM;
    m_Btn_SysteamAudio.GetWindowRect(AudioBtnRect);
    m_Btn_Micro.GetWindowRect(MicroBtnRect);
    m_Stat_SystemAudio.GetWindowRect(AudioRect);
    m_Stat_Micro.GetWindowRect(MicroRect);
    m_Slider_SystemAudio.GetWindowRect(silderA);
    m_Slider_Micro.GetWindowRect(silderM);

    ScreenToClient(AudioBtnRect);
    ScreenToClient(MicroBtnRect);
    ScreenToClient(AudioRect);
    ScreenToClient(MicroRect);
    ScreenToClient(silderA);
    ScreenToClient(silderM);

    int diffX = 70 * m_Scale;
    int diffX1 = 30 * m_Scale;
    AudioBtnRect.MoveToX(AudioBtnRect.left + diffX - diffX1);
    MicroBtnRect.MoveToX(MicroBtnRect.left + diffX + diffX1);
    AudioRect.MoveToX(AudioRect.left + diffX - diffX1);
    MicroRect.MoveToX(MicroRect.left + diffX + diffX1);
    silderA.MoveToX(silderA.left + diffX - diffX1);
    silderM.MoveToX(silderM.left + diffX + diffX1);

    m_Btn_SysteamAudio.MoveWindow(AudioBtnRect);
    m_Btn_Micro.MoveWindow(MicroBtnRect);
    m_Stat_SystemAudio.MoveWindow(AudioRect);
    m_Stat_Micro.MoveWindow(MicroRect);
    m_Slider_SystemAudio.MoveWindow(silderA);
    m_Slider_Micro.MoveWindow(silderM);
}

void Ui_ChildDlg::ResetAudioRecord()
{
    m_Btn_Camera.ShowWindow(SW_SHOW);
    m_Btn_CameraOption.ShowWindow(SW_SHOW);

    CRect AudioBtnRect, MicroBtnRect, AudioRect, MicroRect, silderA, silderM;
    m_Btn_SysteamAudio.GetWindowRect(AudioBtnRect);
    m_Btn_Micro.GetWindowRect(MicroBtnRect);
    m_Stat_SystemAudio.GetWindowRect(AudioRect);
    m_Stat_Micro.GetWindowRect(MicroRect);
    m_Slider_SystemAudio.GetWindowRect(silderA);
    m_Slider_Micro.GetWindowRect(silderM);

    ScreenToClient(AudioBtnRect);
    ScreenToClient(MicroBtnRect);
    ScreenToClient(AudioRect);
    ScreenToClient(MicroRect);
    ScreenToClient(silderA);
    ScreenToClient(silderM);

    int diffX = 70 * m_Scale;
    int diffX1 = 30 * m_Scale;
    AudioBtnRect.MoveToX(AudioBtnRect.left - diffX + diffX1);
    MicroBtnRect.MoveToX(MicroBtnRect.left - diffX - diffX1);
    AudioRect.MoveToX(AudioRect.left - diffX + diffX1);
    MicroRect.MoveToX(MicroRect.left - diffX - diffX1);
    silderA.MoveToX(silderA.left - diffX + diffX1);
    silderM.MoveToX(silderM.left - diffX - diffX1);

    m_Btn_SysteamAudio.MoveWindow(AudioBtnRect);
    m_Btn_Micro.MoveWindow(MicroBtnRect);
    m_Stat_SystemAudio.MoveWindow(AudioRect);
    m_Stat_Micro.MoveWindow(MicroRect);
    m_Slider_SystemAudio.MoveWindow(silderA);
    m_Slider_Micro.MoveWindow(silderM);
}

void Ui_ChildDlg::UpdateAreaRecordRect(const CRect AreaRecordRect)
{
    m_CRect_RecordRect.CopyRect(AreaRecordRect);
}

void Ui_ChildDlg::GetRecordMode(bool* IsNormalRecord, bool* IsAreaRecord, bool* isMouseAreaRecord)
{
    if (m_CRect_RecordRect.IsRectEmpty())
    {
        if (IsNormalRecord)
            *IsNormalRecord = true;
    }
    else if (m_IsMouseAreaRecord)
    {
        if (isMouseAreaRecord)
            *isMouseAreaRecord = true;
    }
    else
    {
        if (IsAreaRecord)
            *IsAreaRecord = true;
    }
}

BEGIN_MESSAGE_MAP(Ui_ChildDlg, CDialogEx)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    ON_WM_HSCROLL()
    ON_WM_CTLCOLOR()
    ON_WM_TIMER()
    ON_WM_LBUTTONDOWN()

    ON_MESSAGE(MSG_MAINDLG_COLLECTVALUME, &Ui_ChildDlg::On_UpdateVolume)
    ON_MESSAGE(MSG_CLARLISTBOX_SELECTED, &Ui_ChildDlg::OnBnClickedBtnListBoxSelected)
    ON_MESSAGE(MSG_TOPRECDLG_STARTRECORD, &Ui_ChildDlg::On_Ui_TopRecDlg_BnClickStartReocrd)
    ON_MESSAGE(MSG_TOPRECDLG_STOPRECORD, &Ui_ChildDlg::On_Ui_TopRecDlg_BnClickStopReocrd)
    ON_MESSAGE(MSG_SILDERCTRL_ISSLIDERMOVING, &Ui_ChildDlg::On_SliderMoveOnEdge)
    ON_MESSAGE(MSG_SCRENC_FILESIZEUPDATE,&Ui_ChildDlg::On_ScrencFileSizeUpdate)

    ON_BN_CLICKED(CHILDDLG_BTN_MOUSEAREAPRESET, &Ui_ChildDlg::OnBnClickedBtnMouseareapreset)
    ON_BN_CLICKED(CHILDDLG_BTN_FOLLOWMOUSERECORD, &Ui_ChildDlg::OnBnClickedBtnFollowmouserecord)
    ON_BN_CLICKED(CHILDDLG_BTN_CARMERAOPTION, &Ui_ChildDlg::OnBnClickedBtnCarmeraoption)
    ON_BN_CLICKED(CHILDDLG_BTN_SYSTEMAUDIO, &Ui_ChildDlg::OnBnClickedBtnSystemaudio)
    ON_BN_CLICKED(CHILDDLG_BTN_RETURN, &Ui_ChildDlg::OnBnClickedBtnReturn)
    ON_BN_CLICKED(CHILDDLG_BTN_MICRO, &Ui_ChildDlg::OnBnClickedBtnMicro)
    ON_BN_CLICKED(CHILDDLG_BTN_CAMERA, &Ui_ChildDlg::OnBnClickedBtnCamera)
    ON_BN_CLICKED(CHILDDLG_BTN_STARTRECORD, &Ui_ChildDlg::OnBnClickedBtnStartrecord)
    ON_BN_CLICKED(CHILDDLG_BTN_PAUSE, &Ui_ChildDlg::OnBnClickedBtnPause)
    ON_BN_CLICKED(CHILDDLG_BTN_STOPRECORDINGDURINGRECORD, &Ui_ChildDlg::OnBnClickedBtnStoprecordingduringrecord)
    ON_BN_CLICKED(CHILDDLG_BTN_RESUME, &Ui_ChildDlg::OnBnClickedBtnResume)
    ON_BN_CLICKED(CHILDDLG_BTN_COMBOX, &Ui_ChildDlg::OnBnClickedBtnCombox)
END_MESSAGE_MAP()

void Ui_ChildDlg::OnBnClickedBtnSystemaudio()
{
    m_ListBoxs.HideListBox();
    if (!m_IsAudioSilent)
    {
        m_LastAudioVolumePos = m_Slider_SystemAudio.GetPos();
        m_Slider_SystemAudio.SetPos(100);
        m_IsAudioSilent = true;
        m_Btn_SysteamAudio.LoadPNG(GAMINGDLG_PNG_AUDIOUNABLE);
        m_Btn_SysteamAudio.LoadClickPNG(GAMINGDLG_PNG_AUDIOUNABLE);
        Invalidate();
    }
    else
    {
        m_Slider_SystemAudio.SetPos(m_LastAudioVolumePos);
        m_IsAudioSilent = false;
        m_Btn_SysteamAudio.LoadPNG(CHILDDLG_PNG_SYSTEMAUDIO);
        m_Btn_SysteamAudio.LoadClickPNG(CHILDDLG_PNG_SYSTEMAUDIO);
        Invalidate();
    }
}

void Ui_ChildDlg::OnBnClickedBtnReturn()
{
    m_ListBoxs.HideListBox();
    if (ScreenRecorder::GetInstance()->IsRecording())
    {
        Ui_MessageModalDlg MessageDlg;
        MessageDlg.SetModal(L"极速录屏大师", L"温馨提示", L"正在录制中，要停止录制吗?", L"确认");
        if (MessageDlg.DoModal() == IDOK)
        {
            KillTimer(TIMER_NONEVIP_RECORDTIME);
            this->OnBnClickedBtnStartrecord();
        }
        else
        {
            return;
        }
    }
    KillTimer(CHILDDLG_TIMER_TIMERECORDED);

    reinterpret_cast<Ui_MainDlg*>(GetParent())->CloseAreaRecordSDL();
    if (m_IsOnlyAudioRecord)
        ResetAudioRecord();

    ShowWindow(SW_HIDE);
    //通知父窗口本窗口关闭了
    GetParent()->PostMessage(WM_UI_CHILDDLG_RETURN);
    DEBUG_CONSOLE_STR(ConsoleHandle, L"子窗口关闭消息发送");

    if (m_IsMouseAreaRecord)
        ResetRecordMouseAreaUi();

    CloseCarmeraPreview();

    m_IsMouseAreaRecord = false;
    m_IsOnlyAudioRecord = false;
    m_IsRecording = false;
    m_CRect_RecordRect.SetRectEmpty();
    App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_HIDE);

    initCtrlPos();
}

void Ui_ChildDlg::CloseCarmeraPreview()
{
    m_Btn_Camera.LoadClickPNG(CHILDDLG_PNG_CAMERA);
    m_Btn_Camera.LoadPNG(CHILDDLG_PNG_CAMERA);
    m_IsCameraRecord = false;
    Invalidate();
    if (Ui_CameraPreviewSDL::IsInsExist())
    {
        if (Ui_CameraPreviewSDL::IsRunning())Ui_CameraPreviewSDL::GetInstance()->ReleaseInstance();
    }
}

BOOL Ui_ChildDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    GetUserDPI();
    initCtrlPos();
    InitDropListData();
    initCtrl();
    SetTimer(1001, 500, NULL);  // 500毫秒检查一次
    return TRUE;
}

void Ui_ChildDlg::OnPaint()
{
    CPaintDC dc(this);
    //预缓冲Gdiplus对象
    using namespace Gdiplus;
    Bitmap memBitmap(m_WindowWidth, m_WindowHeight);
    Graphics memGraphics(&memBitmap);

    Gdiplus::StringFormat sfTimerFormat{}, sfSizeFormat;
    sfTimerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    sfSizeFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    sfSizeFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    sfTimerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
    memGraphics.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    //绘画背景
    SolidBrush ClientBrush(Color(26, 31, 37));
    memGraphics.FillRectangle(//绘画客户区背景
        &ClientBrush, 0, 0,
        static_cast<INT>(m_WindowWidth),
        static_cast<INT>(m_WindowHeight)
    );

    if (m_IsRecording)
    {
        Gdiplus::Point p1(382 * m_Scale, 50 * m_Scale);
        Gdiplus::Point p2(382 * m_Scale, 150 * m_Scale);
        Gdiplus::Pen linePen(&SolidBrush(Color(0, 0, 0)), 2.0f);
        memGraphics.DrawLine(&linePen, p1, p2);

        //绘画计时框和大小预览
        memGraphics.FillRectangle(m_recordTimerRect.sb_bkB, m_recordTimerRect.rect_timeTextBkArea);
        memGraphics.DrawRectangle(m_recordTimerRect.sb_borderPenB, m_recordTimerRect.rect_timeTextBkArea);
        memGraphics.DrawString(m_recordTimerRect.timerStr.c_str(), -1, m_recordTimerRect.timerFont, m_recordTimerRect.rect_timerTextArea, &sfTimerFormat, m_recordTimerRect.sb_timeTextB);
        memGraphics.DrawString(m_recordTimerRect.sizeStr.c_str(), -1, m_recordTimerRect.sizeFont, m_recordTimerRect.rect_CurRecordSizeArea, &sfSizeFormat, m_recordTimerRect.sb_CurRecordSize);
    }

    //贴图预缓冲的内容
    Graphics graphics(dc.GetSafeHdc());
    graphics.DrawImage(&memBitmap, 0, 0);

    DB(ConsoleHandle, L"Ui_ChildDlg:repaint..");
}

BOOL Ui_ChildDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认
    return TRUE;
}

HBRUSH Ui_ChildDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    // 检查是否是滑块控件
    if (nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_SCROLLBAR)
    {
        int id = pWnd->GetDlgCtrlID();
        if (id == CHILDDLG_SLIDER_SYSTEMAUDIO || id == CHILDDLG_SLIDER_MICRO)
        {//设置指定滑块的背景色
            pDC->SetBkColor(RGB(26, 31, 37));
            static CBrush s_brushBackground(RGB(26, 31, 37));
            return (HBRUSH)s_brushBackground;
        }
    }

    return hbr;
}

void Ui_ChildDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类
    //CDialogEx::OnOK();
}

void Ui_ChildDlg::OnCancel()
{
    // TODO: 在此添加专用代码和/或调用基类
    //CDialogEx::OnCancel();
}

void Ui_ChildDlg::OnBnClickedBtnMicro()
{
    m_ListBoxs.HideListBox();
    if (!m_IsMicSilent)
    {
        m_LastMicVolumePos = m_Slider_Micro.GetPos();
        m_Slider_Micro.SetPos(100);//设置麦克风音量值为0
        m_IsMicSilent = true;
        m_Btn_Micro.LoadPNG(CHILDDLG_PNG_MICRO);
        m_Btn_Micro.LoadClickPNG(CHILDDLG_PNG_MICRO);
        Invalidate();
    }
    else
    {
        m_Slider_Micro.SetPos(m_LastMicVolumePos);
        m_IsMicSilent = false;
        m_Btn_Micro.LoadPNG(GAMINGDLG_PNG_MICROUNABLE);
        m_Btn_Micro.LoadClickPNG(GAMINGDLG_PNG_MICROUNABLE);
        Invalidate();
    }

}

void Ui_ChildDlg::OnBnClickedBtnCamera()
{
    m_ListBoxs.HideListBox();
    if (m_IsHasCameraDeive)
    {
        if (!m_IsCameraRecord)
        {
            App.UserRecordWithCarmera(false);

            //更新按钮图标
            m_Btn_Camera.LoadPNG(CHILDDLG_PNG_CAMERAOPEN);
            m_Btn_Camera.LoadClickPNG(CHILDDLG_PNG_CAMERAOPEN);
            m_Btn_Camera.SetBackgroundColor(RGB(26, 31, 37));


            // 设置线程异常处理
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: 创建SDL录制窗口");
            Ui_CameraPreviewSDL* sdlWindow = Ui_CameraPreviewSDL::GetInstance();// 创建SDL窗口实例

            // 获取摄像头设备信息和录制参数
            std::vector<DeviceInfo> Devices = DeviceManager::GetInstance().GetCameraDevices();
            if (Devices.empty()) {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"未找到摄像头设备");
                m_IsCameraRecord = false;
                return;
            }

            std::string alternateName;
            for (size_t i = 0; i < Devices.size(); i++)
            {
                if (Devices[i].nameA == m_string_CameraName)
                {
                    alternateName = Devices[i].alternateName;
                    break;
                }
            }

            // 构造摄像头录制所需参数结构体
            CameraOptions camerOption;
            camerOption.deviceDesc = alternateName;
            camerOption.deviceName = m_string_CameraName;
            camerOption.fps = m_CameraOption_Fps;
            camerOption.vcodec = m_string_CameraPix;
            camerOption.pixelX = m_CameraOption_Width;
            camerOption.pixelY = m_CameraOption_Height;
            CString PixelStr;
            PixelStr.Format(L"%dx%d", camerOption.pixelX, camerOption.pixelY);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"摄像头描述:%s\n摄像头名:%s\n摄像帧数:%d\n摄像头格式:%s\n摄像头分辨率:%s",
                LARSC::s2ws(camerOption.deviceDesc).c_str(),
                LARSC::s2ws(camerOption.deviceName).c_str(),
                camerOption.fps,
                LARSC::s2ws(camerOption.vcodec).c_str(),
                PixelStr
            );

            // 初始化和运行窗口
            CRect WindowRect;
            GetWindowRect(WindowRect);
            CRect SDLWindowRect;
            int SDLWindowRectWidth = 426 * m_Scale;
            int SDLWindowRectHeight = 426 * m_Scale;
            int left = 20 * m_Scale;
            int bottom = GetSystemMetrics(SM_CYSCREEN) - 20 * m_Scale;
            SDLWindowRect.left = left;
            SDLWindowRect.top = bottom - SDLWindowRectHeight;
            SDLWindowRect.right = SDLWindowRect.left + SDLWindowRectWidth;
            SDLWindowRect.bottom = SDLWindowRect.top + SDLWindowRectHeight;
            if (sdlWindow->SetWindowParam(SDLWindowRect, camerOption, m_Scale))
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口初始化成功，开始运行");
                sdlWindow->RunWindowThread();
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口已关闭");
            }
            else
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL摄像头预览窗口]: SDL窗口初始化失败");
            }
            m_IsCameraRecord = true;
            App.UserRecordWithCarmera(true);
        }
        else
        {
            App.UserCloseCarmeraPreviewWithRecord(false);
            Ui_CameraPreviewSDL::GetInstance()->ReleaseInstance();
            m_IsCameraRecord = false;
            m_Btn_Camera.LoadPNG(CHILDDLG_PNG_CAMERA);
            m_Btn_Camera.LoadClickPNG(CHILDDLG_PNG_CAMERA);
            m_Btn_Camera.SetBackgroundColor(RGB(26, 31, 37));
            App.UserCloseCarmeraPreviewWithRecord(true);
        }
    }
    else
    {
        ModalDlg_MFC::ShowModal_NoCarmerDevice();
    }
}

void Ui_ChildDlg::OnBnClickedBtnStartrecord()
{
    m_ListBoxs.HideListBox();
    DB(ConsoleHandle, L"OnBnClickedBtnStartrecord");
    if (!m_IsRecording) //如果当前没有进行录制
    {
        App.UserTriggerRecord();
        if (App.m_IsOverBinds)
        {
            if (ModalDlg_MFC::ShowModal_OverBindsTipsBeforeRecord() == IDCANCEL)
                return;
        }

        if (!(Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning()))
        {
            bool isOpenCarmera = false;
            if ((Ui_CameraPreviewSDL::IsInsExist() && Ui_CameraPreviewSDL::IsRunning()))
            {
                isOpenCarmera = true;
                OnBnClickedBtnCamera();
            }
            if (!ModalDlg_SDL::ShowModal_Rai(this, m_Scale))
            {
                //点击了开通会员
                App.m_Dlg_Main.OnBnClickedBtnOpenvip();
                return;
            }
            if (isOpenCarmera)
                OnBnClickedBtnCamera();
        }

        DB(ConsoleHandle, L"OnBnClickedBtnStartrecord开始录制逻辑");
        if (Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning())
        {
            Ui_AreaRecordingSDL::GetInstance()->SetInteractionEnable(false);
            CRect* recordRect = new CRect;
            recordRect->CopyRect(Ui_AreaRecordingSDL::GetInstance()->getRecordRect());
            ::PostMessage(App.m_Dlg_Main, MSG_UIAREARECORD_STARTRECORD,
                NULL, (LPARAM)recordRect);
            m_IsRecording = true;
            return;
        }
        RecordingParams recp = App.m_Dlg_Main.CollectRecordingParams();

        //判断是选区录制还是全屏录制
        ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
        pSRIns->SetRecordSizeCallback([=](double mb) 
            {
                ::PostMessage(
                    m_hWnd,
                    MSG_SCRENC_FILESIZEUPDATE,
                    (WPARAM)(mb * 100.0),
                    (LPARAM)0
                );
            });
        pSRIns->SetRecordSizeCallbackInterval(1000);
        pSRIns->SetSystemAudioVolume(recp.systemAudioVolume);
        pSRIns->SetMicroVolume(recp.microAudioVolume);
        pSRIns->SetVideoEncoder(GlobalFunc::MapUICodecToEncoderName(recp.codecText.c_str()));
        pSRIns->SetAudioCaptureDevice(recp.audioDevice.c_str());
        pSRIns->SetMicroDeviceName(recp.microDevice.c_str());
        pSRIns->SetRecordMouse(recp.RecordMouse);
        if (App.m_Dlg_Main.m_Dlg_Config->m_Bool_IsWaterOn)
            pSRIns->SetVideoTextFilter(L"极速录屏大师", "40", "FFFFFF", recp.logoPath);//设置水印

        // 判断录制模式
        if (m_CRect_RecordRect.IsRectEmpty())
        { //非选区录制
            if (!m_IsOnlyAudioRecord)//是否不是只录制声音
            { //全屏录制
                App.UserScreenRecord(false);
                pSRIns->SetScreenRecordParam(
                    recp.videoResolution,
                    recp.videoQuality,
                    recp.videoFormat,
                    recp.encodePreset,
                    recp.recordMode,
                    recp.audioSampleRate,
                    recp.audioBitRate,
                    recp.fps
                );
            }
            else
            { //只录制声音
                App.UserOnlyAudioRecord(false);
                pSRIns->SetOnlyAudioRecord(true);
                pSRIns->SetScreenRecordParam(
                    recp.videoResolution,
                    recp.videoQuality,
                    recp.videoFormat,
                    recp.encodePreset,
                    recp.recordMode,
                    recp.audioSampleRate,
                    recp.audioBitRate,
                    recp.fps
                );
            }
        }
        else
        { //选区录制
            //判断是否需要跟随鼠标录制
            m_IsMouseAreaRecord ? App.UserFollowMouseRecord(false) : App.UserAreaRecord(false);
            if (m_IsMouseAreaRecord)//是否跟随鼠标录制
            {
                if (m_IsMouseAreaRecord) // 是否跟随鼠标录制
                {
                    DB(ConsoleHandle, L"设置跟随鼠标录制参数");
                    SetTimer(1002, 50, NULL);   // 定时器，检测鼠标位置
                    DB(ConsoleHandle, L"开启鼠标位置检测定时器");
                    POINT currentMousePos;
                    GetCursorPos(&currentMousePos);

                    // 计算录制区域的宽度和高度
                    int width = m_CRect_RecordRect.Width();
                    int height = m_CRect_RecordRect.Height();

                    // 计算以鼠标为中心点的矩形坐标
                    m_CRect_RecordRect.left = currentMousePos.x - width / 2;
                    m_CRect_RecordRect.top = currentMousePos.y - height / 2;
                    m_CRect_RecordRect.right = currentMousePos.x + width / 2;
                    m_CRect_RecordRect.bottom = currentMousePos.y + height / 2;

                    // 确保矩形不超出屏幕边界
                    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                    if (m_CRect_RecordRect.left < 0)
                    {// 左边界超出屏幕，整体右移
                        m_CRect_RecordRect.right += -m_CRect_RecordRect.left;
                        m_CRect_RecordRect.left = 0;
                    }
                    if (m_CRect_RecordRect.top < 0)
                    {// 上边界超出屏幕，整体下移
                        m_CRect_RecordRect.bottom += -m_CRect_RecordRect.top;
                        m_CRect_RecordRect.top = 0;
                    }
                    if (m_CRect_RecordRect.right > screenWidth)
                    {// 右边界超出屏幕，整体左移
                        m_CRect_RecordRect.left -= (m_CRect_RecordRect.right - screenWidth);
                        m_CRect_RecordRect.right = screenWidth;
                    }
                    if (m_CRect_RecordRect.bottom > screenHeight)
                    {// 下边界超出屏幕，整体上移                       
                        m_CRect_RecordRect.top -= (m_CRect_RecordRect.bottom - screenHeight);
                        m_CRect_RecordRect.bottom = screenHeight;
                    }
                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"跟随鼠标录制：鼠标位置(%d,%d)，矩形(%d,%d,%d,%d)，大小%dx%d",
                        currentMousePos.x, currentMousePos.y,
                        m_CRect_RecordRect.left, m_CRect_RecordRect.top,
                        m_CRect_RecordRect.right, m_CRect_RecordRect.bottom,
                        width, height);
                }
            }
            pSRIns->SetAreaRecordParam(
                m_CRect_RecordRect.left, m_CRect_RecordRect.top, m_CRect_RecordRect.right, m_CRect_RecordRect.bottom,
                recp.videoResolution,
                recp.videoQuality,
                recp.videoFormat,
                recp.encodePreset,
                recp.recordMode,
                recp.audioSampleRate,
                recp.audioBitRate,
                recp.fps
            );
        }

        GetParent()->ShowWindow(SW_MINIMIZE);
        App.m_Dlg_Main.HideVideoList();
        ModalDlg_SDL::ShowModal_CountDown(3, "即将开始录制", [=]()
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"倒计时结束");
            }, true);
        if (Ui_CameraPreviewSDL::GetInstance()->IsInsExist() && Ui_CameraPreviewSDL::GetInstance()->IsRunning())
        {
            Ui_CameraPreviewSDL::GetInstance()->SetRaiseWindow();
        }
        //开始录制 
        CT2A outputfileName(recp.outputFilePath.c_str());
        std::string filenameStr(outputfileName);

        //记录输出文件路径并初始化所在盘符剩余空间
        m_lastRecordOutputPath = recp.outputFilePath;
        if (!InitTotalDiskSizeFromPath(m_lastRecordOutputPath))
        {
            m_hasTotalDiskSize = false;
            m_totalDiskSizeGB = 0.0;
        }

        //录制开始前先更新一次大小显示
        UpdateRecordSizeUI(0.0);
        InvalidateRect(CRect(
            (int)m_recordTimerRect.rect_CurRecordSizeArea.X,
            (int)m_recordTimerRect.rect_CurRecordSizeArea.Y,
            (int)(m_recordTimerRect.rect_CurRecordSizeArea.X + m_recordTimerRect.rect_CurRecordSizeArea.Width),
            (int)(m_recordTimerRect.rect_CurRecordSizeArea.Y + m_recordTimerRect.rect_CurRecordSizeArea.Height)
        ), FALSE);
        std::thread([=]() {pSRIns->startRecording(filenameStr.c_str()); }).detach();
        //else
        {
            //if (!Ui_AreaRecordingSDL::IsInstanceExist() && !Ui_AreaRecordingSDL::IsInsRuning())
            ::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(), MSG_CHILDDLG_TIMERCOUNTSTART, NULL, NULL);
            if (Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning())
            {
                Ui_AreaRecordingSDL::GetInstance()->SetUiModeDuringRecord();
            }
            m_IsRecording = true;
            m_Btn_StartRecording.LoadClickPNG(CHILDDLG_PNG_ISRECORDING);
            m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_ISRECORDING);
            DBFMT(ConsoleHandle, L"判断是不是vip，是否登录:vip:%s,login:%s",
                App.m_IsVip ? L"true" : L"false", App.m_isLoginIn ? L"true" : L"false");
            if (!App.m_IsVip && !App.m_IsNonUserPaid)//如果不是VIP，1分钟后触发
                App.m_Dlg_Main.SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, nullptr);
            DBFMT(ConsoleHandle, L"是否超限:%s", App.m_IsOverBinds ? L"true" : L"false");
            if (App.m_IsOverBinds && !App.m_IsNonUserPaid)//如果超限，1分钟后触发
                App.m_Dlg_Main.SetTimer(TIMER_NONEVIP_RECORDTIME, NONEVIP_RECORDTIME * 1000, nullptr);

            //App.m_Dlg_Main.m_closeManager.get()->SetTrayDoubleClickCallback([this]()
            //    {
            //        GetParent()->ShowWindow(SW_RESTORE);
            //        GetParent()->ShowWindow(SW_SHOW);
            //        this->ShowWindow(SW_SHOW);
            //    });
            m_IsRecording = true;

            if (Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning())
            {
                Ui_AreaRecordingSDL::GetInstance()->SetDashedBorder(true);
                Ui_AreaRecordingSDL::GetInstance()->SetInteractionEnable(true);
            }

            //显示录制中的ui,并开启定时器
            initRecordingCtrlPos();
            Invalidate(false);
            SetRecordCountTimer();            // 启动“录制计时”定时器
        }
    }
    else
    {//如果当前正在进行录制
        DB(ConsoleHandle, L"OnBnClickedBtnStartrecord停止录制逻辑");
        ScreenRecorder* pSRIns = ScreenRecorder::GetInstance();
        if (!pSRIns->IsRecording())
            return;
        if (pSRIns->isPausing())//如果处于暂停状态，则退出暂停
            pSRIns->ResumeRecording();

        pSRIns->ReleaseInstance();
        m_IsRecording = false;

        // 停止“录制计时”定时器并复位显示
        KillTimer(CHILDDLG_TIMER_TIMERECORDED);
        m_recordElapsedSec = 0;
        m_recordTimerRect.timerStr = L"00:00:00";
        Invalidate(FALSE);

        if (m_IsMouseAreaRecord)//如果是跟随鼠标录制结束
        {
            KillTimer(1002);//停止鼠标追踪
            m_IsMouseAreaRecord = false; 
        }
        m_Btn_StartRecording.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
        m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_STARTRECORD);
        CloseCarmeraPreview();
        Invalidate();
        if (Ui_AreaRecordingSDL::IsInstanceExist())
            Ui_AreaRecordingSDL::GetInstance()->ReleaseInstance();
        ::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(),
            MSG_CHILDDLG_STOPTIMERCOUNTANDSHOWNORMALUI, NULL, NULL);
        App.m_Dlg_Main.KillTimer(TIMER_NONEVIP_RECORDTIME);
        App.m_Dlg_Main.ShowWindow(SW_SHOW);
        App.m_Dlg_Main.ShowWindow(SW_RESTORE);
        App.m_Dlg_Main.m_Dlg_Child->ShowWindow(SW_SHOW);
        if (m_CRect_RecordRect.IsRectEmpty() || m_IsMouseAreaRecord)
        {
            App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_SHOW);
            App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_RESTORE);
        }

        //显示视频列表对话框到底部
        auto RecordP = App.m_Dlg_Main.GetLastRecordParam();
        App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(RecordP.outputFilePath.c_str());
        App.m_Dlg_Main.ShowVideoList();
        OnBnClickedBtnReturn();

        App.UserRecordSuccess();
        ModalDlg_MFC::ShowModal_Priview(this);
    }
}

void Ui_ChildDlg::SetRecordCountTimer()
{
    // 启动“录制计时”定时器
    m_recordElapsedSec = 0;                           // 重置累计秒数
    m_recordTimerRect.timerStr = L"00:00:00";         // 重置显示文本
    SetTimer(CHILDDLG_TIMER_TIMERECORDED, 1000, NULL);// 开启计时定时器
}

void Ui_ChildDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == TIMER_NONEVIP_RECORDTIME)
    {//非vip用户录制超时
        /*KillTimer(1002);
        KillTimer(TIMER_NONEVIP_RECORDTIME);
        m_IsRecording = false;
        if (ScreenRecorder::GetInstance()->IsRecording())
        {
            ScreenRecorder::ReleaseInstance();
        }
        if (Ui_AreaRecordingSDL::IsInstanceExist())
            Ui_AreaRecordingSDL::GetInstance()->ReleaseInstance();
        ::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(),
            MSG_CHILDDLG_STOPTIMERCOUNTANDSHOWNORMALUI, NULL, NULL);
        GetParent()->ShowWindow(SW_SHOW);
        GetParent()->ShowWindow(SW_RESTORE);
        this->ShowWindow(SW_RESTORE);
        this->ShowWindow(SW_SHOW);
        App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_SHOW);
        App.m_Dlg_Main.m_Dlg_RecTopDlg->ShowWindow(SW_RESTORE);
        m_Btn_StartRecording.LoadClickPNG(CHILDDLG_PNG_STARTRECORD);
        m_Btn_StartRecording.LoadPNG(CHILDDLG_PNG_STARTRECORD);
        auto RecordP = App.m_Dlg_Main.GetLastRecordParam();
        App.m_Dlg_Main.m_Dlg_Videolist->AddVideoToList(RecordP.outputFilePath.c_str());
        App.m_Dlg_Main.ShowVideoList();
        m_Btn_Camera.LoadClickPNG(CHILDDLG_PNG_CAMERA);
        m_Btn_Camera.LoadPNG(CHILDDLG_PNG_CAMERA);
        m_IsCameraRecord = false;
        initCtrlPos();
        Invalidate();
        if (Ui_CameraPreviewSDL::IsInsExist())
        {
            if (Ui_CameraPreviewSDL::IsRunning())Ui_CameraPreviewSDL::GetInstance()->ReleaseInstance();
        }
        this->SetForegroundWindow();
        if (!App.m_isLoginIn)
            ModalDlg_MFC::ShowModal_NeedLogin();
        else if (!App.m_IsVip && !App.m_IsNonUserPaid)
            ModalDlg_MFC::ShowModal_TrialOver(this);
        else if (App.m_IsOverBinds)
            ModalDlg_MFC::ShowModal_OverBindsTips();*/
    }
    else if (nIDEvent == 1002)//跟随鼠标录制，每一段时间检测鼠标是否改变，改变则更新录制区域
    {
        // 获取当前鼠标位置
        POINT currentMousePos;
        GetCursorPos(&currentMousePos);

        // 如果是第一次，初始化上次位置
        if (!m_MousePosInitialized)
        {
            m_LastMousePos = currentMousePos;
            m_MousePosInitialized = true;
            return;
        }

        // 检查鼠标是否移动了
        if (currentMousePos.x != m_LastMousePos.x ||
            currentMousePos.y != m_LastMousePos.y)
        {
            OnMouseMovedAnywhere();
            m_LastMousePos = currentMousePos;
        }
    }
    else if (nIDEvent == CHILDDLG_TIMER_TIMERECORDED)
    {
        // [添加] 录制计时：每秒更新一次计时文本（暂停时不自增）
        auto pSR = ScreenRecorder::GetInstance();
        // 暂停时不更新计时；恢复后继续累加
        if (!pSR->isPausing())
        {
            ++m_recordElapsedSec;

            // 格式化为 HH:MM:SS
            int h = m_recordElapsedSec / 3600;
            int m = (m_recordElapsedSec % 3600) / 60;
            int s = m_recordElapsedSec % 60;
            wchar_t buf[16] = { 0 };
            swprintf_s(buf, L"%02d:%02d:%02d", h, m, s);

            m_recordTimerRect.timerStr = buf;
            InvalidateRect(CRect(
                m_recordTimerRect.rect_timerTextArea.X,
                m_recordTimerRect.rect_timerTextArea.Y,
                m_recordTimerRect.rect_timerTextArea.X + m_recordTimerRect.rect_timerTextArea.Width,
                m_recordTimerRect.rect_timerTextArea.Y + m_recordTimerRect.rect_timerTextArea.Height
            ));
            InvalidateRect(CRect(
                m_recordTimerRect.rect_CurRecordSizeArea.X,
                m_recordTimerRect.rect_CurRecordSizeArea.Y,
                m_recordTimerRect.rect_CurRecordSizeArea.X + m_recordTimerRect.rect_CurRecordSizeArea.Width,
                m_recordTimerRect.rect_CurRecordSizeArea.Y + m_recordTimerRect.rect_CurRecordSizeArea.Height
            ));
        }
    }
    CDialogEx::OnTimer(nIDEvent);
}

void Ui_ChildDlg::OnMouseMovedAnywhere()
{
    // 获取鼠标当前位置
    POINT mousePos;
    GetCursorPos(&mousePos);

    // 计算以鼠标为中心的矩形坐标
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int width = m_CRect_RecordRect.Width();
    int height = m_CRect_RecordRect.Height();
    int left = mousePos.x - width / 2;
    int top = mousePos.y - height / 2;
    int right = left + width;
    int bottom = top + height;

    // 调整位置以确保矩形完全在屏幕内，同时保持大小不变
    if (left < 0)
    { // 左边超出，整体右移
        right -= left; // 加上超出的距离
        left = 0;
    }

    if (top < 0)
    { // 上边超出，整体下移
        bottom -= top; // 加上超出的距离 
        top = 0;
    }

    if (right > screenWidth)
    {// 右边超出，整体左移
        left -= (right - screenWidth);
        right = screenWidth;
    }

    if (bottom > screenHeight)
    { // 下边超出，整体上移
        top -= (bottom - screenHeight);
        bottom = screenHeight;
    }

    // 设置新的矩形坐标
    m_CRect_RecordRect.SetRect(left, top, right, bottom);

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"检测到鼠标移动:posX:%d,posY:%d, 录制区域:(%d,%d,%d,%d)",
        mousePos.x, mousePos.y, left, top, right, bottom);

    // 创建新的矩形副本并发送消息
    CRect* RecordRect = new CRect;
    RecordRect->CopyRect(m_CRect_RecordRect);
    ::PostMessage(this->GetParent()->GetSafeHwnd(), MSG_UIAREARECORD_UPDATE, 0, (LPARAM)RecordRect);
}

void Ui_ChildDlg::HideListBox()
{
    m_ListBoxs.HideListBox();
}

LRESULT Ui_ChildDlg::On_UpdateVolume(WPARAM wParam, LPARAM lParam)
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"[SDL线程]：获取Childdlg设置的音量值");
    MsgParam::VolumeInfo* volumeInfo = reinterpret_cast<MsgParam::VolumeInfo*>(lParam);
    //获取设置的系统音量值
    int Min, Max;
    m_Slider_SystemAudio.GetRange(Min, Max);
    float SystemAudioVolume = float(Max - m_Slider_SystemAudio.GetPos()) / float(Max - Min);

    //获取设置的麦克风音量值
    m_Slider_Micro.GetRange(Min, Max);
    float MicroAudioVolume = float(Max - m_Slider_Micro.GetPos()) / float(Max - Min);

    //设置结构体
    volumeInfo->SystemAudioVolume = SystemAudioVolume;
    volumeInfo->MicroAudioVolume = MicroAudioVolume;
    return 1;
}

LRESULT Ui_ChildDlg::On_SliderMoveOnEdge(WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = (HWND)lParam;
    int newPos = (int)wParam;
    if (m_Slider_Micro.GetSafeHwnd() == hWnd)
    {
        if (newPos == 100)
        {
            m_IsMicSilent = true;
            m_Btn_Micro.LoadPNG(CHILDDLG_PNG_MICRO);
            m_Btn_Micro.LoadClickPNG(CHILDDLG_PNG_MICRO);
            Invalidate();
        }
        else
        {
            if (m_IsMicSilent)
            {
                m_IsMicSilent = false;
                m_Btn_Micro.LoadPNG(GAMINGDLG_PNG_MICROUNABLE);
                m_Btn_Micro.LoadClickPNG(GAMINGDLG_PNG_MICROUNABLE);
                Invalidate();
            }
        }
    }
    else if (m_Slider_SystemAudio.GetSafeHwnd() == hWnd)
    {
        if (newPos == 100)
        {
            m_IsAudioSilent = true;
            m_Btn_SysteamAudio.LoadPNG(GAMINGDLG_PNG_AUDIOUNABLE);
            m_Btn_SysteamAudio.LoadClickPNG(GAMINGDLG_PNG_AUDIOUNABLE);
            Invalidate();
        }
        else
        {
            if (m_IsAudioSilent)
            {
                m_IsAudioSilent = false;
                m_Btn_SysteamAudio.LoadPNG(CHILDDLG_PNG_SYSTEMAUDIO);
                m_Btn_SysteamAudio.LoadClickPNG(CHILDDLG_PNG_SYSTEMAUDIO);
                Invalidate();
            }
        }
    }
    return LRESULT();
}

LRESULT Ui_ChildDlg::On_ScrencFileSizeUpdate(WPARAM wParam, LPARAM lParam)
{
    double sizeMB = (double)wParam / 100.0;
    DBFMT(ConsoleHandle, L"size:%.2f", sizeMB);
    //同步更新 UI 文件大小显示字符串
    UpdateRecordSizeUI(sizeMB);
    InvalidateRect(CRect(
        (int)m_recordTimerRect.rect_CurRecordSizeArea.X,
        (int)m_recordTimerRect.rect_CurRecordSizeArea.Y,
        (int)(m_recordTimerRect.rect_CurRecordSizeArea.X + m_recordTimerRect.rect_CurRecordSizeArea.Width),
        (int)(m_recordTimerRect.rect_CurRecordSizeArea.Y + m_recordTimerRect.rect_CurRecordSizeArea.Height)
    ), FALSE);
    return LRESULT();
}

LRESULT Ui_ChildDlg::On_Ui_TopRecDlg_BnClickStartReocrd(WPARAM wParam, LPARAM lParam)
{
    OnBnClickedBtnStartrecord();
    return 1;
}

LRESULT Ui_ChildDlg::On_Ui_TopRecDlg_BnClickStopReocrd(WPARAM wParam, LPARAM lParam)
{
    OnBnClickedBtnStartrecord();
    return 1;
}

void Ui_ChildDlg::OnBnClickedBtnFollowmouserecord()
{
    m_ListBoxs.HideListBox();
    AreaSelectDlg ModelDlg_AreaSelect;
    if (ModelDlg_AreaSelect.DoModal() == IDOK)
    {
        auto SelectRect = ModelDlg_AreaSelect.GetSelectRect();
        if (!SelectRect.IsRectEmpty())
        {
            m_CRect_RecordRect.CopyRect(SelectRect);
            CString str;
            str.Format(L"区域大小\n%dx%d", SelectRect.Width(), SelectRect.Height());
            str += L"▼";
            m_Btn_MouseRecordAreaPreset.SetWindowTextW(str);
            ModalDlg_MFC::ShowModal_MouseRecordAreaSelectCompelete(SelectRect.Width(), SelectRect.Height());
        }
    }
}

BOOL Ui_ChildDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // TODO: 在此添加专用代码和/或调用基类
    return CDialogEx::OnCommand(wParam, lParam);
}

void Ui_ChildDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialogEx::OnLButtonDown(nFlags, point);
}

void Ui_ChildDlg::OnBnClickedBtnPause()
{
    auto pSrIns = ScreenRecorder::GetInstance();
    if (pSrIns->isRecording() && !pSrIns->isPausing())
    {
        App.m_Dlg_Main.KillTimer(TIMER_NONEVIP_RECORDTIME);
        ScreenRecorder::GetInstance()->PauseRecording();
        m_btn_resume.ShowWindow(SW_SHOW);
        m_btn_pause.ShowWindow(SW_HIDE);
        if (Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning()) 
        {
            Ui_AreaRecordingSDL::GetInstance()->OnPauseRecordingUi();                     
        }
    }
    if (App.m_Dlg_Main.m_Dlg_RecTopDlg && App.m_Dlg_Main.m_Dlg_RecTopDlg->IsWindowVisible())
    {
        App.m_Dlg_Main.m_Dlg_RecTopDlg->SetPauseUIMode();
    }
}

void Ui_ChildDlg::OnBnClickedBtnStoprecordingduringrecord()
{
    OnBnClickedBtnStartrecord();
}

void Ui_ChildDlg::OnBnClickedBtnResume()
{
    auto pSRIns = ScreenRecorder::GetInstance();
    if (pSRIns && pSRIns->IsRecording() && pSRIns->isPausing())
    {
        pSRIns->ResumeRecording();
        m_btn_resume.ShowWindow(SW_HIDE);
        m_btn_pause.ShowWindow(SW_SHOW);
        if (Ui_AreaRecordingSDL::IsInstanceExist() && Ui_AreaRecordingSDL::IsInsRuning())
        {
            Ui_AreaRecordingSDL::GetInstance()->OnResumeRecordingUi();
        }
        if (((!App.m_IsVip && !App.m_IsNonUserPaid) || App.m_IsOverBinds)
            && NONEVIP_RECORDTIME - m_recordElapsedSec > 0)
        {
            App.m_Dlg_Main.SetTimer(TIMER_NONEVIP_RECORDTIME, (NONEVIP_RECORDTIME - m_recordElapsedSec) * 1000, NULL);
        }
    }
    if (App.m_Dlg_Main.m_Dlg_RecTopDlg && App.m_Dlg_Main.m_Dlg_RecTopDlg->IsWindowVisible())
    {
        App.m_Dlg_Main.m_Dlg_RecTopDlg->ResumeFromPauseUiMode();
    }
}

void Ui_ChildDlg::OnBnClickedBtnCombox()
{
    if (m_ListBoxs.IsListBoxVisiable(L"录制选项下拉框"))
    {
        m_ListBoxs.HideListBox();
    }
    else
    {
        CRect rect;
        m_btn_Combox.GetWindowRect(&rect);
        m_ListBoxs.UpdateDroplistXY(L"录制选项下拉框", rect.left, rect.top + rect.Height());
    }
}
