#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <mutex>
#include <map>

class WindowHandleManager
{
public:
    // 获取单例实例
    static WindowHandleManager& GetInstance();

    // 获取所有窗口句柄
    const std::vector<HWND>& GetWindowHandles() const;

    // 获取所有窗口句柄及其标题（用于调试）
    std::map<HWND, std::wstring> GetWindowHandlesWithTitles() const;

    // 刷新窗口句柄列表
    void RefreshWindowHandles();

    // 获取窗口标题（辅助函数）
    static std::wstring GetWindowTitle(HWND hwnd);

    // 获取窗口类名（辅助函数）
    static std::wstring GetWindowClassName(HWND hwnd);

    // 是否是Alt+Tab窗口的判断
    static bool IsAltTabWindow(HWND hwnd);
private:
    // 私有构造函数和析构函数（单例模式）
    WindowHandleManager();
    ~WindowHandleManager();

    // 禁止拷贝和赋值（单例模式）
    WindowHandleManager(const WindowHandleManager&) = delete;
    WindowHandleManager& operator=(const WindowHandleManager&) = delete;

    // 窗口枚举回调函数
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

    // 存储窗口句柄的容器
    std::vector<HWND> m_windowHandles;

    // 互斥锁保护多线程访问
    mutable std::mutex m_mutex;
};