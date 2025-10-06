#ifndef STRINGCONVERSION_H_
#define STRINGCONVERSION_H_
#include <string>
#include <afxwin.h>
namespace LARSC//×Ö·û´®×ª»»
{
	//std::string->std::wstring
	std::wstring s2ws(const std::string& str);
	//std::wstring->std::string
	std::string ws2s(const std::wstring& wstr);
	//const char*->wchar_t
	wchar_t* c2w(const char* str);
	//const wchar_t*->char*
	char* w2c(const wchar_t* wstr);
	//UTF-8 ±àÂë×ª»»
	std::wstring Utf8ToWideString(const std::string& utf8Str);
	// CString ×ª»»Îª const char*
	const char* CStringToCharPtr(const CString& str);
	//CString->std::string
	std::string CStringToStdString(CString string);
}

#endif	//!STRINGCONVERSION_H_