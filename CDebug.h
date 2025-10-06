#pragma once
#include <afxwin.h>  
#include <cstdio>       
#include <cwchar>      

// 根据编译模式选择宏实现
#ifdef _DEBUG  // Debug模式

//MFC应用程序调试宏
#define ALLOC_CONSOLE_AND_GET_HANDLE(hConsole)				    \
do {\
	AllocConsole();												\
	(hConsole) = GetStdHandle(STD_OUTPUT_HANDLE);				\
} while (0)

#define DEBUG_MESSAGE_INT(value, title)							\
do {\
	CString debugStr;											\
	debugStr.Format(L"%d", (value));							\
	AfxMessageBox(debugStr, MB_ICONINFORMATION, 0);				\
} while (0)

#define DEBUG_MESSAGE_STR(str, title)							\
do {\
	AfxMessageBox(CString(str), MB_ICONINFORMATION, 0);			\
} while (0)

#define DEBUG_MESSAGE_FMT(format, ...)                          \
do {\
	CString debugStr;                                           \
	debugStr.Format((format), __VA_ARGS__);                     \
	AfxMessageBox(debugStr, MB_ICONINFORMATION, 0);             \
} while (0)

#define DEBUG_CONSOLE_INT(hConsole, value)                      \
do {\
	CString debugStr;                                           \
	debugStr.Format(L"%d", (value));                            \
	wchar_t buf[100] = L"";                                     \
	swprintf(buf, 100, L"%ls\n", debugStr.GetString());         \
	WriteConsoleW((hConsole), buf, wcslen(buf), NULL, NULL);    \
} while (0)

#define DEBUG_CONSOLE_STR(hConsole, str)                        \
do {\
    wchar_t buf[4096] = L"";                                     \
    swprintf(buf, 4096, L"[线程ID:%lu]: %ls\n", GetCurrentThreadId(), (LPCTSTR)(str)); \
    WriteConsoleW((hConsole), buf, wcslen(buf), NULL, NULL);    \
} while (0)

#define DEBUG_CONSOLE_FMT(hConsole, format, ...)                \
do {\
    wchar_t buf[4096] = L"";                                     \
    swprintf(buf, 4096, L"[线程ID:%lu]: " format, GetCurrentThreadId(), __VA_ARGS__); \
    wcscat(buf, L"\n");                                          \
    WriteConsoleW((hConsole), buf, wcslen(buf), NULL, NULL);    \
} while (0)

#define DEBUG_CONSOLE_STR_LONGBUFER(hConsole, str)                        \
do {\
	wchar_t buf[4096] = L"";                                     \
	swprintf(buf, 4096, L"%ls\n", (LPCTSTR)(str));               \
	WriteConsoleW((hConsole), buf, wcslen(buf), NULL, NULL);    \
} while (0)

//ffmpeg调试宏
#define INIT_ERROR_HANDLING(console) \
    g_ConsoleHandle = (console)

#define CHECK_CONDITION_ERROR(nullRef,message) \
    do { \
		if(!nullRef){\
			DEBUG_CONSOLE_STR(g_ConsoleHandle, message); \
			return false; \
		}\
    } while (0)

#define CHECK_FFMPEG_ERROR(errorRet,message) \
    do { \
		int rets = errorRet;\
        char tempBuf[256] = {0}; \
        av_strerror(rets, tempBuf, sizeof(tempBuf) - 1); \
        DEBUG_CONSOLE_FMT(g_ConsoleHandle, L"%s: %s\n", message, LARSC::c2w(tempBuf)); \
        return false; \
    } while (0)

#else  // Release模式 - 所有调试宏定义为空操作

#define ALLOC_CONSOLE_AND_GET_HANDLE(hConsole)                 ((void)0)
#define DEBUG_MESSAGE_INT(value, title)                         ((void)0)
#define DEBUG_MESSAGE_STR(str, title)                           ((void)0)
#define DEBUG_MESSAGE_FMT(format, ...)                          ((void)0)
#define DEBUG_CONSOLE_INT(hConsole, value)                      ((void)0)
#define DEBUG_CONSOLE_STR(hConsole, str)                        ((void)0)
#define DEBUG_CONSOLE_FMT(hConsole, format, ...)                ((void)0)
#define DEBUG_CONSOLE_STR_LONGBUFER(hConsole, str)              ((void)0)

// FFmpeg错误处理宏需要保留基本功能但去除调试输出
#define INIT_ERROR_HANDLING(console)                            ((void)0)
#define CHECK_CONDITION_ERROR(nullRef,message) \
    do { \
        if(!(nullRef)) { \
            return false; \
        } \
    } while (0)
#define CHECK_FFMPEG_ERROR(errorRet,message) \
    do { \
        int rets = errorRet; \
        if (rets < 0) { \
            return false; \
        } \
    } while (0)

#endif // _DEBUG

// 定义通用简写宏 - 这些在Debug和Release模式下都可用
#define DB(hConsole, str) DEBUG_CONSOLE_STR(hConsole, str)
#define DBFMT(hConsole, format, ...) DEBUG_CONSOLE_FMT(hConsole, format, __VA_ARGS__)

// 声明全局控制台句柄
#ifdef _DEBUG
static HANDLE g_ConsoleHandle = NULL;
#endif