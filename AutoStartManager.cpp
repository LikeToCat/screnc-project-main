// AutoStartManager.cpp
#include "stdafx.h"
#include "AutoStartManager.h"
#include <Windows.h>

// 返回注册表运行键路径
const wchar_t* AutoStartManager::GetRegistryRunPath()
{
    static const wchar_t* path = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    return path;
}

// 返回应用程序在注册表中的值名称
const wchar_t* AutoStartManager::GetAppValueName()
{
    static const wchar_t* name = L"ScreenRec";
    return name;
}

// 设置应用程序是否开机自启动
bool AutoStartManager::SetAutoStart(bool enable)
{
    // 打开注册表键
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,  // 当前用户注册表（不需要管理员权限）
        GetRegistryRunPath(),
        0,
        KEY_WRITE,
        &hKey
    );

    if (result != ERROR_SUCCESS)
    {
        // 打开注册表键失败
        return false;
    }

    bool success = false;

    if (enable)
    {
        // 获取应用程序路径
        std::wstring appPath = GetAppPath();

        // 添加自启动项
        result = RegSetValueExW(
            hKey,
            GetAppValueName(),
            0,
            REG_SZ,
            reinterpret_cast<const BYTE*>(appPath.c_str()),
            static_cast<DWORD>((appPath.length() + 1) * sizeof(wchar_t))
        );

        success = (result == ERROR_SUCCESS);
    }
    else
    {
        // 删除自启动项
        result = RegDeleteValueW(hKey, GetAppValueName());

        // 如果值不存在，也视为成功
        success = (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND);
    }

    // 关闭注册表键
    RegCloseKey(hKey);

    return success;
}

// 检查应用程序是否已设置为开机自启动
bool AutoStartManager::IsAutoStartEnabled()
{
    // 打开注册表键
    HKEY hKey;
    LONG result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        GetRegistryRunPath(),
        0,
        KEY_READ,
        &hKey
    );

    if (result != ERROR_SUCCESS)
    {
        // 打开注册表键失败
        return false;
    }

    // 查询值是否存在
    DWORD type;
    DWORD dataSize = 0;
    result = RegQueryValueExW(
        hKey,
        GetAppValueName(),
        nullptr,
        &type,
        nullptr,
        &dataSize
    );

    // 关闭注册表键
    RegCloseKey(hKey);

    // 如果值存在且类型为REG_SZ，则表示已启用自启动
    return (result == ERROR_SUCCESS && type == REG_SZ);
}

// 获取应用程序的完整路径
std::wstring AutoStartManager::GetAppPath()
{
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::wstring(path);
}