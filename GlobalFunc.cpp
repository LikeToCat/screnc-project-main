#include "stdafx.h"
#include <afxwin.h>
#include <string>
#include <algorithm> 
#include <cctype> 
#include "GlobalFunc.h"
#include "CDebug.h"
#include "LarStringConversion.h"
extern HANDLE ConsoleHandle;
std::string GlobalFunc::MapUICodecToEncoderName(const CString& codecText)
{
    // 转换为标准字符串以便于处理
    std::wstring wstr = codecText.GetString();

    // 根据按钮文本映射编码器名称
    if (wstr.find(L"NVIDIA NVENC") != std::wstring::npos)
        return "h264_nvenc";
    else if (wstr.find(L"AMD AMF") != std::wstring::npos)
        return "h264_amf";
    else if (wstr.find(L"Intel QSV") != std::wstring::npos)
        return "h264_qsv";
    else if (wstr.find(L"NVIDIA CUDA") != std::wstring::npos)
        return "h264_cuvid";

    // 默认使用软件编码器(空字符串)
    return "";
}

int GlobalFunc::ExtractFpsValue(const CString& fpsText)
{
    // 查找fps值的位置
    std::wstring wstr = fpsText.GetString();
    size_t fpsPos = wstr.find(L"fps");
    if (fpsPos == std::wstring::npos)
    {
        return -1;
    }

    // 从字符串的开头到"fps"子串的位置截取子串
    std::wstring fpsValueStr = wstr.substr(0, fpsPos);
    fpsValueStr.erase(std::remove_if(fpsValueStr.begin(), fpsValueStr.end(),
        [](wchar_t c) { return !std::isdigit(c); }), fpsValueStr.end());
    int fpsValue = std::stoi(fpsValueStr);
    return fpsValue;
}

// 提取分辨率值的函数
std::pair<int, int> GlobalFunc::ExtractResolution(const CString& resolutionText)
{
    // 将CString转换为标准字符串以便处理
    std::wstring wstr = resolutionText.GetString();

    // 查找分辨率的分隔符 '*'
    size_t pos = wstr.find(L'*');
    if (pos == std::wstring::npos)
    {
        // 如果没有找到分隔符 '*', 返回 (-1, -1) 表示错误
        return std::make_pair(-1, -1);
    }

    // 从字符串中提取宽度和高度
    int width = std::stoi(wstr.substr(0, pos));
    int height = std::stoi(wstr.substr(pos + 1));

    // 返回提取的分辨率值
    return std::make_pair(width, height);
}

CString GlobalFunc::BuildVideoFilePath(const CString& savePath, const CString& fileFormat)
{

    CTime currentTime = CTime::GetCurrentTime();

    // 创建fileFormat的可修改副本
    CString fileFormatLower = fileFormat;
    fileFormatLower.MakeLower();

    // 使用Format一次性构建文件名，避免多次字符串连接操作，提高性能
    CString fileName;
    fileName.Format(L"%04d%02d%02d_%02d%02d%02d_ScreenRecordMaster.%s",
        currentTime.GetYear(),
        currentTime.GetMonth(),
        currentTime.GetDay(),
        currentTime.GetHour(),
        currentTime.GetMinute(),
        currentTime.GetSecond(),
        fileFormatLower);
    CString fullPath = savePath;
    if (!fullPath.IsEmpty() && fullPath.Right(1) != L"\\" && fullPath.Right(1) != L"/")
    {// 确保路径末尾有分隔符 
        fullPath += L"\\";
    }
    fullPath += fileName; // 添加文件名到路径

    // 检查目录是否存在，不存在则创建 - 使用GetFileAttributes避免额外依赖
    if (!savePath.IsEmpty())
    {
        DWORD attribs = GetFileAttributes(savePath);
        bool pathExists = (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));

        if (!pathExists)
        {
            ::CreateDirectory(savePath, NULL);
        }
    }
    std::wstring SavePathWstr = savePath;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"构建的保存目录:%s", SavePathWstr.c_str());
    return fullPath;
}

CString GlobalFunc::GetDefaultVideoSavePath()
{
    CString videoPath;

    // 获取当前可执行程序的完整路径,提取盘符信息 (例如 "C:")
    TCHAR szModulePath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szModulePath, MAX_PATH);
    CString driveLetter;
    driveLetter.Format(_T("%c:"), szModulePath[0]);

    // 检测是否是C盘，如果是则寻找其他可用盘符
    if (driveLetter.CompareNoCase(_T("C:")) == 0)
    {
        DWORD drives = GetLogicalDrives();// 获取当前可用的所有驱动器
        for (int i = 3; i < 26; i++)// 从D盘开始检查
        {
            if (drives & (1 << i)) // 检查驱动器是否存在
            {
                TCHAR alternateDrive = 'A' + i;
                CString alternatePath;
                alternatePath.Format(_T("%c:\\"), alternateDrive);
                UINT driveType = GetDriveType(alternatePath);
                if (driveType == DRIVE_FIXED) // 检查驱动器是否可用（类型是固定磁盘）
                {
                    driveLetter.Format(_T("%c:"), alternateDrive);
                    DEBUG_CONSOLE_FMT(ConsoleHandle, _T("C盘不用，使用替代盘符: %s\n"), driveLetter);
                    break;
                }
            }
        }
        if (driveLetter.CompareNoCase(_T("C:")) == 0)
        { // 如果没有找到替代盘符，则回退到原始的方案
            DEBUG_CONSOLE_FMT(ConsoleHandle, _T("未找到有效的替代盘符，继续使用默认方案\n"));
        }
    }
    videoPath.Format(_T("%s\\ScreenRecordRec"), driveLetter);

    // 检查目录是否存在，不存在则创建
    if (GetFileAttributes(videoPath) == INVALID_FILE_ATTRIBUTES)
    {
        if (!CreateDirectory(videoPath, NULL))
        {// 目录不存在，创建它
            DWORD error = GetLastError();
            DEBUG_CONSOLE_FMT(ConsoleHandle, _T("Failed to create directory '%s', error code: %d\n"), videoPath, error);

            // 失败回退用户文档目录
            TCHAR szDocumentsPath[MAX_PATH];
            SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szDocumentsPath);
            videoPath.Format(_T("%s\\ScreenRecordRec"), szDocumentsPath);
            if (GetFileAttributes(videoPath) == INVALID_FILE_ATTRIBUTES)
            {// 尝试创建文档目录下的文件夹
                CreateDirectory(videoPath, NULL);
            }
        }
    }
    return videoPath;
}

CString GlobalFunc::GetExecutablePathFolder()

{
    TCHAR szPath[MAX_PATH] = { 0 };
    if (GetModuleFileName(NULL, szPath, MAX_PATH) == 0)
    {
        return _T("");
    }
    CString strPath(szPath);
    int nPos = strPath.ReverseFind(_T('\\'));
    if (nPos != -1)
    {
        strPath = strPath.Left(nPos); // 不包含最后的反斜杠
    }
    return strPath;
}

std::string GlobalFunc::ConvertPathToUtf8(const CString& widePathInput)
{
    std::string utf8Path;
    int utf8Size = WideCharToMultiByte(CP_UTF8, 0, widePathInput, -1, NULL, 0, NULL, NULL);
    if (utf8Size > 0) {
        utf8Path.resize(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, widePathInput, -1, &utf8Path[0], utf8Size, NULL, NULL);
    }
    return utf8Path;
}

std::string GlobalFunc::ConvertToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) {
        return "";
    }
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(),
        nullptr, 0, nullptr, nullptr);
    std::string utf8_str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(),
        &utf8_str[0], size_needed, nullptr, nullptr);
    return utf8_str;
}

std::string GlobalFunc::AnsiToUtf8(const std::string& ansi_str)
{
    if (IsUtf8(ansi_str))
        return ansi_str;

    if (ansi_str.empty()) {
        return "";
    }
    int wide_size = MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), -1, nullptr, 0);
    if (wide_size <= 0) {
        return "";
    }
    std::wstring wide_str(wide_size, 0);
    MultiByteToWideChar(CP_ACP, 0, ansi_str.c_str(), -1, &wide_str[0], wide_size);
    return ConvertToUtf8(wide_str.substr(0, wide_str.length() - 1));
}

std::string GlobalFunc::Utf8ToAnsi(const std::string& utf8_str)
{
    if (utf8_str.empty()) {
        return "";
    }
    int wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
    if (wide_size <= 0) {
        return "";
    }
    std::wstring wide_str(wide_size, 0);
    if (!MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wide_str[0], wide_size)) {
        return "";
    }
    int ansi_size = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ansi_size <= 0) {
        return "";
    }
    std::string ansi_str(ansi_size, 0);
    if (!WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), -1, &ansi_str[0], ansi_size, nullptr, nullptr)) {
        return "";
    }
    return ansi_str.substr(0, ansi_str.length() - 1);
}

SIZE GlobalFunc::MeasureTextSize(const std::wstring& text, const std::wstring& fontName,
    int fontSizePt, int fontWeight)
{
    SIZE size = { 0, 0 };
    HDC hdc = GetDC(NULL);
    if (!hdc) return size;
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    int lfHeight = -MulDiv(fontSizePt, dpiY, 72);

    // 创建并选入字体
    HFONT hFont = CreateFontW(
        lfHeight, 0, 0, 0,
        fontWeight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE,
        fontName.c_str()
    );
    if (hFont)
    {
        HGDIOBJ hOld = SelectObject(hdc, hFont);
        // 测量文本
        GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &size);
        SelectObject(hdc, hOld);
        DeleteObject(hFont);
    }

    ReleaseDC(NULL, hdc);
    return size;
}

bool GlobalFunc::GetPngSize(const std::wstring& path, int& w, int& h)
{
    using namespace Gdiplus;
    static ULONG_PTR token = 0;
    if (!token) {
        GdiplusStartupInput si;
        GdiplusStartup(&token, &si, NULL);
    }
    Bitmap bmp(path.c_str());
    if (bmp.GetLastStatus() != Ok) return false;
    w = bmp.GetWidth();
    h = bmp.GetHeight();
    return true;
}

bool GlobalFunc::IsUtf8(const std::string& json)
{
    size_t i = 0;
    while (i < json.size())
    {
        unsigned char c = static_cast<unsigned char>(json[i]);
        if (c < 0x80) // 单字节字符 (ASCII)
        {
            ++i;
        }
        else if ((c & 0xE0) == 0xC0) // 两字节字符
        {
            if (i + 1 >= json.size() ||
                (static_cast<unsigned char>(json[i + 1]) & 0xC0) != 0x80)
            {
                return false; // 非合法 UTF-8 两字节字符
            }
            i += 2;
        }
        else if ((c & 0xF0) == 0xE0) // 三字节字符
        {
            if (i + 2 >= json.size() ||
                (static_cast<unsigned char>(json[i + 1]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(json[i + 2]) & 0xC0) != 0x80)
            {
                return false; // 非合法 UTF-8 三字节字符
            }
            i += 3;
        }
        else if ((c & 0xF8) == 0xF0) // 四字节字符
        {
            if (i + 3 >= json.size() ||
                (static_cast<unsigned char>(json[i + 1]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(json[i + 2]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(json[i + 3]) & 0xC0) != 0x80)
            {
                return false; // 非合法 UTF-8 四字节字符
            }
            i += 4;
        }
        else
        {
            return false; // 非合法 UTF-8 字符
        }
    }
    return true;
}

float GlobalFunc::GetUserDPI()
{
    // 获取系统 DPI
    HDC screen = ::GetDC(NULL);
    if (screen == NULL) {
        AfxMessageBox(L"无法获取屏幕 DC。");
        return 1.0f;
    }
    int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
    int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
    ::ReleaseDC(NULL, screen);

    // 计算缩放因子（基准 DPI 为 96）
    double scaleX = static_cast<double>(dpiX) / 96.0;
    double scaleY = static_cast<double>(dpiY) / 96.0;
    return scaleX;
}

std::wstring GlobalFunc::GetDefaultDownloadFolder()
{
    PWSTR pszPath = nullptr;
    // FOLDERID_Downloads: 下载文件夹的 KnownFolder ID
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Downloads, KF_FLAG_DEFAULT, nullptr, &pszPath);
    std::wstring downloadDir;
    if (SUCCEEDED(hr) && pszPath)
    {
        downloadDir = pszPath;
    }
    if (pszPath)
    {
        CoTaskMemFree(pszPath);
    }
    return downloadDir;
}

std::wstring GlobalFunc::AnsiToUtf8W(const std::wstring& ansiStr)
{
    if (ansiStr.empty()) {
        return std::wstring();
    }

    // 第一步：将宽字符串转换为ANSI多字节字符串
    int ansiLen = WideCharToMultiByte(CP_ACP, 0, ansiStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ansiLen <= 0) {
        return std::wstring();
    }

    std::string ansiString(ansiLen - 1, 0); // -1 因为不需要包含null终止符
    WideCharToMultiByte(CP_ACP, 0, ansiStr.c_str(), -1, &ansiString[0], ansiLen, nullptr, nullptr);

    // 第二步：将ANSI多字节字符串转换为UTF-8宽字符串
    int utf8Len = MultiByteToWideChar(CP_UTF8, 0, ansiString.c_str(), -1, nullptr, 0);
    if (utf8Len <= 0) {
        return std::wstring();
    }

    std::wstring utf8String(utf8Len - 1, 0); // -1 因为不需要包含null终止符
    MultiByteToWideChar(CP_UTF8, 0, ansiString.c_str(), -1, &utf8String[0], utf8Len);

    return utf8String;
}

std::wstring GlobalFunc::Utf8ToAnsiW(const std::wstring& utf8Str)
{
    if (utf8Str.empty()) {
        return std::wstring();
    }

    // 第一步：将UTF-8宽字符串转换为UTF-8多字节字符串
    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len <= 0) {
        return std::wstring();
    }

    std::string utf8String(utf8Len - 1, 0); // -1 因为不需要包含null终止符
    WideCharToMultiByte(CP_UTF8, 0, utf8Str.c_str(), -1, &utf8String[0], utf8Len, nullptr, nullptr);

    // 第二步：将UTF-8多字节字符串转换为ANSI宽字符串
    int ansiLen = MultiByteToWideChar(CP_ACP, 0, utf8String.c_str(), -1, nullptr, 0);
    if (ansiLen <= 0) {
        return std::wstring();
    }

    std::wstring ansiString(ansiLen - 1, 0); // -1 因为不需要包含null终止符
    MultiByteToWideChar(CP_ACP, 0, utf8String.c_str(), -1, &ansiString[0], ansiLen);

    return ansiString;
}

CString GlobalFunc::Utf8ToCString(const std::string& utf8)
{
    if (utf8.empty()) return L"";
    int  wlen = ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
    CStringW tmp;
    ::MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), tmp.GetBuffer(wlen), wlen);
    tmp.ReleaseBuffer(wlen);
    return tmp;
}

CString GlobalFunc::AnsiToCString(const std::string& ansi)
{
    if (ansi.empty())
        return CString();

    // 先计算需要的宽字符长度
    int wideLength = ::MultiByteToWideChar(
        CP_ACP,            // 源字符串使用当前 ANSI 代码页
        0,                 // 默认转换标志
        ansi.data(),       // 源缓冲区
        (int)ansi.length(),// 源字节数
        nullptr,           // 不输出到目标，先获取长度
        0
    );
    if (wideLength <= 0)
        return CString();

    // 分配缓冲区并执行转换
    CStringW tmp;
    tmp.GetBufferSetLength(wideLength);
    ::MultiByteToWideChar(
        CP_ACP,
        0,
        ansi.data(),
        (int)ansi.length(),
        tmp.GetBuffer(),
        wideLength
    );
    tmp.ReleaseBuffer(wideLength);

    // 在 Unicode build 下，CString == CStringW
    return tmp;
}
