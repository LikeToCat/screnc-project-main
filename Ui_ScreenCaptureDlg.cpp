#include "stdafx.h"
#include "Ui_ScreenCaptureDlg.h"
#include "theApp.h"
#include "Ui_ConfigDlg.h"
#include "Ui_MainDlg.h"
#include <ShlObj.h>

IMPLEMENT_DYNAMIC(Ui_ScreenCaptureDlg, CDialogEx)

BEGIN_MESSAGE_MAP(Ui_ScreenCaptureDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_TIMER()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

Ui_ScreenCaptureDlg::Ui_ScreenCaptureDlg(HBITMAP hBmp, CWnd* pParent)
    : CDialogEx(IDD_DIALOG_SCREENCAPTURE, pParent)
    , m_hScreenBmp(hBmp)
    , m_borderThickness(10)
{
    // 默认保存到“我的图片\Screenshots”
    m_saveFolder = App.m_Dlg_Main.m_Dlg_Config->getRecSavePath();
    ::CreateDirectory(m_saveFolder, nullptr);
}

Ui_ScreenCaptureDlg::~Ui_ScreenCaptureDlg()
{
    if (m_hScreenBmp)
        ::DeleteObject(m_hScreenBmp);
}

BOOL Ui_ScreenCaptureDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 无边框全屏置顶
    ::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
    ::SetWindowPos(m_hWnd, HWND_TOPMOST,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        SWP_SHOWWINDOW);

    // 1.5 秒后保存 & 关闭
    SetTimer(1, 1500, nullptr);
    return TRUE;
}

void Ui_ScreenCaptureDlg::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    // 绘制整屏位图
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap bmpScreen;
    bmpScreen.Attach(m_hScreenBmp);
    CBitmap* pOld = memDC.SelectObject(&bmpScreen);
    dc.BitBlt(0, 0, rc.Width(), rc.Height(),
        &memDC, 0, 0, SRCCOPY);
    memDC.SelectObject(pOld);
    bmpScreen.Detach();

    // 绘制蓝色边框
    CPen pen(PS_SOLID, m_borderThickness, RGB(0, 120, 215));
    CPen* pOldPen = dc.SelectObject(&pen);
    dc.SelectStockObject(NULL_BRUSH);
    CRect inner = rc;
    inner.DeflateRect(m_borderThickness / 2, m_borderThickness / 2);
    dc.Rectangle(inner);
    dc.SelectObject(pOldPen);

    DB(ConsoleHandle, L"Ui_ScreenCaptureDlg:repaint..");
}

void Ui_ScreenCaptureDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1)
    {
        KillTimer(1);
        SaveCroppedImage();
        // 改为 PostMessage，让框架先把剩余消息处理完再关闭
        PostMessage(WM_CLOSE);
        ::PostMessage(App.m_Dlg_Main.m_Dlg_RecTopDlg->GetSafeHwnd(), MSG_SCREENSHOTDLG_SHOTCOMPELETE, NULL, NULL);
    }
    CDialogEx::OnTimer(nIDEvent);
}

void Ui_ScreenCaptureDlg::OnClose()
{
    DestroyWindow();
}

void Ui_ScreenCaptureDlg::SaveCroppedImage()
{
    CRect rc;
    GetClientRect(&rc);
    int w = rc.Width(), h = rc.Height(), t = m_borderThickness;

    // 屏幕 DC
    HDC hdcScreen = ::GetDC(NULL);
    CDC screenDC;
    screenDC.Attach(hdcScreen);

    // 裁剪后位图
    CDC cropDC;
    cropDC.CreateCompatibleDC(&screenDC);
    CBitmap bmpCropped;
    bmpCropped.CreateCompatibleBitmap(&screenDC, w - 2 * t, h - 2 * t);

    CBitmap* pOldSrc = screenDC.SelectObject(CBitmap::FromHandle(m_hScreenBmp));
    CBitmap* pOldDst = cropDC.SelectObject(&bmpCropped);

    // 拷贝去除边框区域
    cropDC.BitBlt(0, 0, w - 2 * t, h - 2 * t, &screenDC, t, t, SRCCOPY);

    // 组保存路径
    CString path;
    path.Format(_T("%s\\Capture_%04d.png"),
        (LPCTSTR)m_saveFolder,
        GetTickCount() % 10000);

    // 用 CImage 保存
    CImage img;
    img.Attach((HBITMAP)bmpCropped.Detach());
    img.Save(path);
    img.Destroy();

    // 恢复、释放 DC
    screenDC.SelectObject(pOldSrc);
    cropDC.SelectObject(pOldDst);
    screenDC.Detach();
    ::ReleaseDC(NULL, hdcScreen);

    //ShowTrayTip(L"截屏完成", L"极速录屏大师");
}

void Ui_ScreenCaptureDlg::PostNcDestroy()
{
    CDialogEx::PostNcDestroy();
    // 最后 delete this
    delete this;
}

void Ui_ScreenCaptureDlg::ShowTrayTip(LPCTSTR szInfo, LPCTSTR szTitle)
{
    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = m_hWnd;
    nid.uID = 1001;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    nid.uCallbackMessage = WM_USER + 200; 

    // 标题和内容
    _tcsncpy_s(nid.szInfoTitle, szTitle, _TRUNCATE);
    _tcsncpy_s(nid.szInfo, szInfo, _TRUNCATE);

    nid.dwInfoFlags = NIIF_INFO;

    Shell_NotifyIcon(NIM_ADD, &nid);
    Shell_NotifyIcon(NIM_MODIFY, &nid);
    Shell_NotifyIcon(NIM_DELETE, &nid);
}