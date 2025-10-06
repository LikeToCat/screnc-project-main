#include "stdafx.h" //根据项目是否启动预编译头功能，来确认是否包含该头文件
#include "LarStringConversion.h"

std::wstring LARSC::s2ws(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
	std::wstring wstr(sizeNeeded, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstr[0], sizeNeeded);
	return wstr;
}

wchar_t* LARSC::c2w(const char* str)
{
	if (str == nullptr)
		return nullptr;

	int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	// 分配内存，注意调用者需要释放返回的内存
	wchar_t* wstr = new wchar_t[sizeNeeded];
	MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, sizeNeeded);
	return wstr;
}

// 将 wchar_t* 转换为 char* 的函数
char* LARSC::w2c(const wchar_t* wstr)
{
	if (wstr == nullptr)
		return nullptr;

	int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	// 分配内存，注意调用者需要释放返回的内存
	char* str = new char[sizeNeeded];
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, sizeNeeded, NULL, NULL);
	return str;
}

// 将 std::wstring 转换为 std::string 的函数
std::string LARSC::ws2s(const std::wstring& wstr)
{
	if (wstr.empty())
		return std::string();

	int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
	std::string str(sizeNeeded, 0);
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], sizeNeeded, NULL, NULL);
	return str;
}

std::wstring LARSC::Utf8ToWideString(const std::string& utf8Str)
{
	if (utf8Str.empty()) return std::wstring();

	// 计算宽字符所需的长度
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), NULL, 0);
	if (sizeNeeded <= 0) return std::wstring();

	// 分配空间并转换
	std::wstring wstr(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wstr[0], sizeNeeded);

	return wstr;
}

const char* LARSC::CStringToCharPtr(const CString& str)
{
	// 当使用Unicode编译时（通常为默认情况）
#ifdef _UNICODE
	// 将Unicode CString转换为ANSI
	CStringA strA(str);
	// 返回ANSI字符指针
	return strA.GetString();
#else
	// 非Unicode环境下直接返回
	return str.GetString();
#endif
}

std::string LARSC::CStringToStdString(CString string)
{
	CT2CA pszConvertedAnsiString(string);
	return std::string(pszConvertedAnsiString);
}