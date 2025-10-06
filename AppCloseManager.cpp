#include "stdafx.h"
#include "AppCloseManager.h"
#include "CDebug.h"
#include "theApp.h"
extern HANDLE ConsoleHandle;

// 标准托盘菜单命令ID
#define ID_TRAY_RESTORE 1001
#define ID_TRAY_EXIT    1002

AppCloseManager::AppCloseManager(HWND hWnd)
    : m_hWnd(hWnd)
    , m_closeMode(CloseMode::ExitApplication)
    , m_isInTray(false)
    , m_hTrayMenu(nullptr)
    , m_trayMessageId(WM_TRAYICON)
{
    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = hWnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;

    // 设置默认图标和提示
    HICON hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
    if (hIcon) {
        m_nid.hIcon = hIcon;
    }
    wcscpy_s(m_nid.szTip, L"应用程序");

    // 创建默认托盘菜单 - 添加这部分代码
    m_hTrayMenu = CreatePopupMenu();
    if (m_hTrayMenu) {
        AppendMenu(m_hTrayMenu, MF_STRING, ID_TRAY_RESTORE, L"恢复窗口(&R)");
        AppendMenu(m_hTrayMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(m_hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"退出(&X)");
        DEBUG_CONSOLE_STR(ConsoleHandle, L"托盘右键菜单创建成功");
    }
    else {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"托盘右键菜单创建失败");
    }
}

AppCloseManager::~AppCloseManager()
{
    // 确保托盘图标被移除
    if (m_isInTray) {
        RemoveTrayIcon();
    }

    // 释放菜单
    if (m_hTrayMenu) {
        DestroyMenu(m_hTrayMenu);
        m_hTrayMenu = nullptr;
    }
}

bool AppCloseManager::HandleClose()
{
    // 根据关闭模式处理
    if (m_closeMode == CloseMode::MinimizeToTray) {
        MinimizeToTray();
        return true;  // 返回true表示已处理，阻止窗口关闭
    }

    // 返回false表示继续正常关闭流程
    return false;
}

void AppCloseManager::MinimizeToTray()
{
    // 隐藏窗口
    ShowWindow(m_hWnd, SW_HIDE);
    ShowWindow(App.m_Dlg_Main.m_Dlg_Carmera->GetSafeHwnd(), SW_HIDE);

    // 如果还没有托盘图标，则添加
    if (!m_isInTray) {
        InitTrayIcon();
    }
}

void AppCloseManager::RestoreFromTray()
{
    RestoreAndShow(m_hWnd);
}

void AppCloseManager::RestoreAndShow(HWND hWnd)
{
    if (!IsWindowVisible(hWnd))
    {
        ShowWindow(hWnd, SW_SHOW);
    }
    if (IsIconic(hWnd))
    {
        ShowWindow(hWnd, SW_RESTORE);
    }
    SetForegroundWindow(hWnd);
    BringWindowToTop(hWnd);
    UpdateWindow(hWnd);
}

bool AppCloseManager::HandleTrayMessage(WPARAM wParam, LPARAM lParam)
{
    // 确保是我们的托盘消息
    if (wParam != m_nid.uID) {
        return false;
    }

    // 处理托盘消息
    switch (lParam) {
    case WM_LBUTTONDOWN:
        DEBUG_CONSOLE_STR(ConsoleHandle, L"左键点击托盘图标");
        if (m_trayDoubleClickCallback) {
            m_trayDoubleClickCallback();
        }
        else {
            RestoreFromTray();
        }
        return true;

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_CONTEXTMENU:
        DEBUG_CONSOLE_STR(ConsoleHandle, L"右键点击托盘图标，显示菜单");
        ShowTrayMenu();
        return true;
    }

    return false;
}

void AppCloseManager::SetTrayIconInfo(HICON hIcon, const wchar_t* tipText)
{
    // 更新图标
    if (hIcon) {
        m_nid.hIcon = hIcon;
    }

    // 更新提示文本
    if (tipText) {
        wcscpy_s(m_nid.szTip, tipText);
    }

    // 如果已在托盘中，则更新图标
    if (m_isInTray) {
        Shell_NotifyIcon(NIM_MODIFY, &m_nid);
    }
}

void AppCloseManager::InitTrayIcon()
{
    if (!m_isInTray) {
        Shell_NotifyIcon(NIM_ADD, &m_nid);
        m_isInTray = true;
    }
}

void AppCloseManager::RemoveTrayIcon()
{
    if (m_isInTray) {
        Shell_NotifyIcon(NIM_DELETE, &m_nid);
        m_isInTray = false;
    }
}

void AppCloseManager::ShowTrayMenu()
{
    if (!m_hTrayMenu) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"托盘菜单句柄为空，创建菜单");
        // 如果菜单为空，临时创建一个
        m_hTrayMenu = CreatePopupMenu();
        AppendMenu(m_hTrayMenu, MF_STRING, ID_TRAY_RESTORE, L"恢复窗口(&R)");
        AppendMenu(m_hTrayMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(m_hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"退出(&X)");
    }

    // 获取光标位置
    POINT cursorPos;
    GetCursorPos(&cursorPos);

    // 获取屏幕尺寸
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 基本菜单标志
    UINT flags = TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON;

    // 根据光标位置决定菜单弹出方向
    // 如果光标在屏幕下半部分，菜单向上弹出
    if (cursorPos.y > screenHeight / 2) {
        flags |= TPM_BOTTOMALIGN;  // 菜单底部与光标位置对齐（向上弹出）
        DEBUG_CONSOLE_STR(ConsoleHandle, L"菜单将向上弹出");
    }
    else {
        flags |= TPM_TOPALIGN;     // 菜单顶部与光标位置对齐（向下弹出）
        DEBUG_CONSOLE_STR(ConsoleHandle, L"菜单将向下弹出");
    }

    // 将窗口设为前台，以便菜单正确显示
    SetForegroundWindow(m_hWnd);

    // 显示菜单
    DEBUG_CONSOLE_STR(ConsoleHandle, L"显示托盘右键菜单");
    UINT clicked = TrackPopupMenu(
        m_hTrayMenu,
        flags,
        cursorPos.x,
        cursorPos.y,
        0,
        m_hWnd,
        nullptr
    );

    if (clicked == 0) {
        DWORD error = GetLastError();
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"菜单显示可能有问题，错误: %d", error);
    }

    // 处理菜单命令
    switch (clicked) {
    case ID_TRAY_RESTORE:
        DEBUG_CONSOLE_STR(ConsoleHandle, L"选择：恢复窗口");
        RestoreFromTray();
        break;

    case ID_TRAY_EXIT:
        DEBUG_CONSOLE_STR(ConsoleHandle, L"选择：退出应用程序");
        PostQuitMessage(0);
        break;
    }

    // 发送一个空消息，修复菜单消失后的问题
    PostMessage(m_hWnd, WM_NULL, 0, 0);
}

void AppCloseManager::InitializeTrayIconOnStartup()
{
    // 确保托盘图标被创建
    if (!m_isInTray) {
        // 直接调用现有的函数而不是 InitTrayIcon
        Shell_NotifyIcon(NIM_ADD, &m_nid);
        m_isInTray = true;
    }
}