#include "stdafx.h"
#include "WindowHandleManager.h"
#include <dwmapi.h>
#include <TlHelp32.h>
#include <psapi.h>
#include <algorithm>
#include <set>
#pragma comment(lib, "dwmapi.lib")
#include "CDebug.h"
extern HANDLE ConsoleHandle;
// 单例获取函数
WindowHandleManager& WindowHandleManager::GetInstance()
{
    static WindowHandleManager instance;
    return instance;
}

// 构造函数
WindowHandleManager::WindowHandleManager()
{
    RefreshWindowHandles();
}

// 析构函数
WindowHandleManager::~WindowHandleManager()
{
}

// 获取窗口标题
std::wstring WindowHandleManager::GetWindowTitle(HWND hwnd)
{
    if (!hwnd) return L"";

    int length = GetWindowTextLengthW(hwnd);
    if (length == 0) return L"";

    std::vector<wchar_t> buffer(length + 1);
    GetWindowTextW(hwnd, buffer.data(), length + 1);

    return std::wstring(buffer.data());
}

// 获取窗口类名
std::wstring WindowHandleManager::GetWindowClassName(HWND hwnd)
{
    if (!hwnd) return L"";

    wchar_t className[256] = { 0 };
    GetClassNameW(hwnd, className, 256);

    return std::wstring(className);
}

// 判断窗口是否是Alt+Tab窗口
bool WindowHandleManager::IsAltTabWindow(HWND hwnd)
{
    // 检查窗口是否可见
    if (!IsWindowVisible(hwnd))
        return false;

    // 检查窗口是否有标题
    if (GetWindowTextLengthW(hwnd) == 0)
        return false;

    // 获取窗口扩展样式
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    // 获取窗口样式
    LONG style = GetWindowLong(hwnd, GWL_STYLE);

    // 获取窗口的所有者
    HWND ownerWindow = GetWindow(hwnd, GW_OWNER);

    // 检查是否是普通窗口
    if (!(style & WS_OVERLAPPEDWINDOW) && !(exStyle & WS_EX_APPWINDOW))
    {
        // 如果既不是普通窗口也不是应用窗口，则检查是否是工具窗口或有所有者
        if ((exStyle & WS_EX_TOOLWINDOW) || ownerWindow != NULL)
            return false;
    }

    // 获取窗口类名
    std::wstring className = GetWindowClassName(hwnd);

    // 排除常见的系统窗口
    if (className == L"Shell_TrayWnd" ||
        className == L"Progman" ||
        className == L"DV2ControlHost" ||
        className == L"Windows.UI.Core.CoreWindow" ||
        className == L"SysListView32" ||
        className == L"WorkerW" ||
        className == L"Shell_SecondaryTrayWnd")
        return false;

    // 检查窗口是否有有效尺寸
    RECT rcWnd;
    if (!GetWindowRect(hwnd, &rcWnd))
        return false;

    int width = rcWnd.right - rcWnd.left;
    int height = rcWnd.bottom - rcWnd.top;

    // 排除太小的窗口
    if (width < 10 || height < 10)
        return false;

    // 使用DWM API检查窗口是否可见在任务栏中
    BOOL cloaked = FALSE;
    HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (SUCCEEDED(hr) && cloaked)
        return false;

    // 额外检查UWP应用窗口
    if (className == L"ApplicationFrameWindow")
    {
        // 查找UWP内容窗口，确保它是有效的
        HWND childWindow = FindWindowEx(hwnd, NULL, L"Windows.UI.Core.CoreWindow", NULL);
        if (childWindow != NULL && GetWindowTextLengthW(childWindow) > 0)
            return true;
    }

    // 检查窗口是否有WS_EX_NOACTIVATE扩展样式，这些通常不在Alt+Tab中显示
    if (exStyle & WS_EX_NOACTIVATE)
        return false;

    return true;
}

// 窗口枚举回调函数
BOOL CALLBACK WindowHandleManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto handleVector = reinterpret_cast<std::vector<HWND>*>(lParam);

    if (IsAltTabWindow(hwnd))
    {
        DBFMT(ConsoleHandle, L"WindowHandleManager当前AltTabWindow的枚举的hwnd:%p，窗口标题:%s",
            (void*)hwnd, GetWindowTitle(hwnd).c_str());
        handleVector->push_back(hwnd);
    }

    // 继续枚举
    return TRUE;
}

// 刷新窗口句柄列表
void WindowHandleManager::RefreshWindowHandles()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // 清空当前列表
    m_windowHandles.clear();

    // 枚举所有顶层窗口
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&m_windowHandles));
}

// 获取所有窗口句柄
const std::vector<HWND>& WindowHandleManager::GetWindowHandles() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_windowHandles;
}

// 获取所有窗口句柄及其标题（用于调试）
std::map<HWND, std::wstring> WindowHandleManager::GetWindowHandlesWithTitles() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::map<HWND, std::wstring> result;

    for (const auto& hwnd : m_windowHandles)
    {
        result[hwnd] = GetWindowTitle(hwnd);
    }

    return result;
}