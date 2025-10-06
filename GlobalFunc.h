#pragma once
#include <afxwin.h>
namespace GlobalFunc
{
    // 将ConfigDlg设置对话框中编码器选项映射到FFmpeg编码器名称
    std::string MapUICodecToEncoderName(const CString& codecText);
    // 将ConfigDlg设置对话框中帧率值提取
    int ExtractFpsValue(const CString& fpsText);
    // 将ConfigDlg设置对话框中分辨率值提取
    std::pair<int, int> ExtractResolution(const CString& resolutionText);
    // 构建视频文件的完整路径（时间_极速录屏大师.格式）
    CString BuildVideoFilePath(const CString& savePath, const CString& fileFormat);
    // 获取用户当前电脑下标准的保存视频目录的位置,支持中文路径
    CString GetDefaultVideoSavePath();
    // 获取当前可执行程序所在目录（不包括当前程序）
    CString GetExecutablePathFolder();
    // 将CString路径转变为SDL可识别的UTF8编码的路径
    std::string ConvertPathToUtf8(const CString& widePathInput);
    // 将宽字符串转换为UTF-8编码的字符串
    std::string ConvertToUtf8(const std::wstring& wstr);
    // 将ANSI字符串转换为UTF-8编码
    std::string AnsiToUtf8(const std::string& ansi_str);
    // UTF8编码转ANSI编码
    std::string Utf8ToAnsi(const std::string& utf8_str);
    // 获取字体宽高
    SIZE MeasureTextSize(const std::wstring& text, const std::wstring& fontName,
        int fontSizePt, int fontWeight = FW_NORMAL);
    // 获取图片宽度/高度
    bool GetPngSize(const std::wstring& path, int& w, int& h);
    //判断字符是否为utf8编码 
    bool IsUtf8(const std::string& json);
    //获取DPI
    float GetUserDPI();
    //获取用户默认下载目录（utf16编码）
    std::wstring GetDefaultDownloadFolder();
    // 将ANSI编码的std::wstring转换为UTF-8编码的std::wstring
    std::wstring AnsiToUtf8W(const std::wstring& ansiStr);
    // 将UTF-8编码的std::wstring转换为ANSI编码的std::wstring
    std::wstring Utf8ToAnsiW(const std::wstring& utf8Str);
    // 将utf8编码的string转为CString
    CString Utf8ToCString(const std::string& utf8);
    // 将ansi编码的string转为CString
    CString AnsiToCString(const std::string& ansi);
}