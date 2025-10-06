#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>
#include "CDebug.h"

extern HANDLE ConsoleHandle;

class GameFocusDetector
{
public:
    // 定义回调函数类型 (参数: 窗口句柄, 是否是游戏窗口)
    using FocusChangedCallback = std::function<void(HWND, bool)>;

    // 获取单例实例
    static GameFocusDetector& GetInstance();

    // 判断指定窗口是否是游戏窗口
    bool IsGameWindow(HWND hwnd);

    // 判断当前焦点窗口是否是游戏窗口
    bool IsFocusedWindowGame();

    // 获取当前焦点窗口句柄
    HWND GetFocusedWindow();

    // 启动焦点窗口监控
    bool StartMonitoring();

    // 停止焦点窗口监控
    void StopMonitoring();

    // 设置焦点变化回调
    void SetFocusChangedCallback(FocusChangedCallback callback) { m_focusChangedCallback = callback; }

    // 设置要排除的进程ID（通常是当前程序）
    void SetExcludedProcessId(DWORD processId) { m_excludedProcessId = processId; }

    // 自动排除当前进程
    void ExcludeCurrentProcess() { m_excludedProcessId = GetCurrentProcessId(); }

    // 析构函数
    ~GameFocusDetector();

private:
    // 私有构造函数 (单例模式)
    GameFocusDetector();

    // 禁止拷贝和赋值 (单例模式)
    GameFocusDetector(const GameFocusDetector&) = delete;
    GameFocusDetector& operator=(const GameFocusDetector&) = delete;

    // 判断窗口标题是否匹配已知游戏标题
    bool IsGameWindowTitle(const std::wstring& windowTitle, DWORD processId);

    // 判断进程是否加载了游戏相关的DLL
    bool HasGameRelatedDependencies(DWORD processId);

    // 获取进程路径
    bool GetProcessPath(DWORD processId, std::wstring& processPath);

    // 判断路径是否是游戏相关路径
    bool IsGamePath(const std::wstring& path);

    // 判断进程是否是会改变标题的应用
    bool IsTitleChangingApp(DWORD processId, std::wstring& exeName);

    // 处理焦点窗口变化
    void HandleFocusChange(HWND newFocusedWindow);

    // 静态回调函数 (用于WinEventHook)
    static void CALLBACK WinEventProc(
        HWINEVENTHOOK hWinEventHook,
        DWORD event,
        HWND hwnd,
        LONG idObject,
        LONG idChild,
        DWORD idEventThread,
        DWORD dwmsEventTime
    );

    // 单例实例
    static GameFocusDetector* s_instance;

    // 焦点窗口监控成员变量
    HWINEVENTHOOK m_eventHook;
    HWND m_lastFocusedWindow;
    FocusChangedCallback m_focusChangedCallback;
    bool m_isMonitoring;

    // 要排除的进程ID（通常是当前程序）
    DWORD m_excludedProcessId = 0;
};