#pragma once

#include <Windows.h>
#include <shellapi.h>
#include <memory>
#include <functional>
#include "CMessage.h"
// 应用程序关闭管理器 - 处理最小化到托盘和直接关闭的行为
class AppCloseManager
{
public:
    // 关闭模式枚举
    enum class CloseMode
    {
        MinimizeToTray,   // 最小化到系统托盘
        ExitApplication   // 完全退出应用程序
    };

    // 构造函数
    AppCloseManager(HWND hWnd);

    // 析构函数 - 清理资源
    ~AppCloseManager();

    // 设置关闭模式
    void SetCloseMode(CloseMode mode) { m_closeMode = mode; }

    // 获取当前关闭模式
    CloseMode GetCloseMode() const { return m_closeMode; }

    // 处理窗口关闭消息 - 在WM_CLOSE消息处理中调用
    // 返回true表示已处理，窗口不应关闭；返回false表示应继续默认关闭处理
    bool HandleClose();

    // 处理系统托盘消息 - 在WM_NOTIFYICON消息处理中调用
    bool HandleTrayMessage(WPARAM wParam, LPARAM lParam);

    //初始化系统托盘
    void InitializeTrayIconOnStartup();

    // 最小化到托盘
    void MinimizeToTray();

    // 从托盘还原
    void RestoreFromTray();

    void RestoreAndShow(HWND hWnd);

    // 设置托盘图标信息
    void SetTrayIconInfo(HICON hIcon, const wchar_t* tipText);

    // 设置托盘菜单项 - 如果需要自定义托盘菜单
    void SetTrayContextMenu(HMENU hMenu) { m_hTrayMenu = hMenu; }

    // 设置托盘双击回调函数
    void SetTrayDoubleClickCallback(std::function<void()> callback) {
        m_trayDoubleClickCallback = callback;
    }

private:
    // 初始化系统托盘图标
    void InitTrayIcon();

    // 移除系统托盘图标
    void RemoveTrayIcon();

    // 显示托盘菜单
    void ShowTrayMenu();

private:
    HWND m_hWnd;               // 主窗口句柄
    CloseMode m_closeMode;     // 关闭模式
    NOTIFYICONDATA m_nid;      // 托盘图标数据
    HMENU m_hTrayMenu;         // 托盘菜单句柄
    bool m_isInTray;           // 是否已在托盘中
    UINT m_trayMessageId;      // 托盘消息ID

    std::function<void()> m_trayDoubleClickCallback;  // 托盘双击回调
};