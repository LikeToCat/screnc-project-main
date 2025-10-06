// AutoStartManager.h
#pragma once

#include <string>

class AutoStartManager
{
public:
    // 设置应用程序是否开机自启动
    // @param enable: true表示启用开机自启动，false表示禁用
    // @return: 操作是否成功
    static bool SetAutoStart(bool enable);

    // 检查应用程序是否已设置为开机自启动
    // @return: 是否已设置开机自启动
    static bool IsAutoStartEnabled();

private:
    // 获取应用程序的完整路径
    static std::wstring GetAppPath();

    // 获取注册表键路径
    static const wchar_t* GetRegistryRunPath();

    // 获取程序在注册表中的值名称
    static const wchar_t* GetAppValueName();
};