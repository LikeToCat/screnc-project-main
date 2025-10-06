#pragma once

// 包括 SDKDDKVer.h 将定义最高版本的可用 Windows 平台。

// 如果要为以前的 Windows 平台生成应用程序，请包括 WinSDKVer.h，并将
// WIN32_WINNT 宏设置为要支持的平台，然后再包括 SDKDDKVer.h。
#include <WinSDKVer.h>
#define WINVER 0x0601        // Windows 7
#define _WIN32_WINNT 0x0601  // Windows 7
#define _WIN32_IE 0x0800     // IE 8.0

#include <SDKDDKVer.h>
