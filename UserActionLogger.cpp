#include "stdafx.h"
#include "UserActionLogger.h"
#include "GlobalFunc.h"
#include "theApp.h"
#include "CDebug.h"
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <windows.h>

extern HANDLE ConsoleHandle; 
UserActionLogger* UserActionLogger::s_UserActionLogger = nullptr;
UserActionLogger* UserActionLogger::GetInstance()
{
    if (!s_UserActionLogger)
    {
        s_UserActionLogger = new UserActionLogger;
    }
    return s_UserActionLogger;
}

UserActionLogger::UserActionLogger() 
    : m_initialized(false),
    m_headerWritten(false),
    m_maxFileSizeBytes(0)
{
}

std::wstring UserActionLogger::BuildDefaultPath() const
{
    // <exe_dir>\logs\user_action.log
    wchar_t exePath[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring p = exePath;
    size_t pos = p.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        p = p.substr(0, pos);
    p += L"\\logs\\user_action.log";
    return p;
}

bool UserActionLogger::EnsureDirectory(const std::wstring& fullFilePath)
{
    // 提取目录部分
    size_t pos = fullFilePath.find_last_of(L"\\/");
    if (pos == std::wstring::npos) return true;
    std::wstring dir = fullFilePath.substr(0, pos);
    if (dir.empty()) return true;

    DWORD attr = GetFileAttributesW(dir.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
        return true;

    // 递归创建
    std::wstring partial;
    for (size_t i = 0; i < dir.size(); ++i)
    {
        wchar_t c = dir[i];
        if (c == L'\\' || c == L'/')
        {
            if (!partial.empty())
            {
                if (GetFileAttributesW(partial.c_str()) == INVALID_FILE_ATTRIBUTES)
                {
                    if (!CreateDirectoryW(partial.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
                    {
                        DBFMT(ConsoleHandle, L"[UserActionLogger] 创建目录失败: %s", partial.c_str());
                        return false;
                    }
                }
            }
        }
        partial.push_back(c);
    }
    if (!partial.empty() &&
        GetFileAttributesW(partial.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        if (!CreateDirectoryW(partial.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            DBFMT(ConsoleHandle, L"[UserActionLogger] 创建目录失败: %s", partial.c_str());
            return false;
        }
    }
    return true;
}

void UserActionLogger::Init(const std::string& machineGuid, const std::wstring& logFilePath, size_t maxFileSizeKB)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_initialized)
        return;

    m_machineGuid = machineGuid.empty() ? "NULL" : machineGuid;
    m_logFilePath = logFilePath.empty() ? BuildDefaultPath() : logFilePath;
    m_maxFileSizeBytes = (maxFileSizeKB > 0) ? (static_cast<size_t>(maxFileSizeKB) * 1024) : 0; 

    if (!EnsureDirectory(m_logFilePath))
    {
        DB(ConsoleHandle, L"[UserActionLogger] 目录创建失败，日志初始化中止");
        m_initialized = false;
        return;
    }

    // 判断文件是否存在/是否为空
    bool isNewFile = false;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(m_logFilePath.c_str(), GetFileExInfoStandard, &fad))
    {
        isNewFile = true;
    }
    else
    {
        // 文件存在但大小为 0 也视为新文件
        if (fad.nFileSizeHigh == 0 && fad.nFileSizeLow == 0)
            isNewFile = true;
    }

    FILE* fp = nullptr;
    _wfopen_s(&fp, m_logFilePath.c_str(), L"ab"); // 追加模式（二进制）
    if (!fp)
    {
        DBFMT(ConsoleHandle, L"[UserActionLogger] 打开日志文件失败: %s", m_logFilePath.c_str());
        return;
    }

    if (isNewFile)
    {
        WriteHeaderIfNeeded(fp, true);
    }
    else
    {
        m_headerWritten = true; // 既然已有内容，视为头已写
    }

    fclose(fp);
    m_initialized = true;
    DBFMT(ConsoleHandle, L"[UserActionLogger] 初始化成功，路径: %s", m_logFilePath.c_str());
}

void UserActionLogger::WriteHeaderIfNeeded(FILE* fp, bool isNewFile)
{
    if (m_headerWritten)
        return;

    if (isNewFile)
    {
        // UTF-8 BOM
        const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
        fwrite(bom, 1, 3, fp);
    }

    // MachineGuid 行
    std::wstring header = L"MachineGuid: " + std::wstring(m_machineGuid.begin(), m_machineGuid.end());
    std::string  utf8Header = GlobalFunc::ConvertToUtf8(header) + "\r\n";
    fwrite(utf8Header.data(), 1, utf8Header.size(), fp);
    fflush(fp);

    m_headerWritten = true;
    DBFMT(ConsoleHandle, L"[UserActionLogger] 写入头部 MachineGuid=%s", header.c_str());
}

void UserActionLogger::TrimIfNeeded(size_t newEntryBytes)
{
    // 未初始化或未限制直接返回
    if (!m_initialized || m_maxFileSizeBytes == 0)
        return;

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(m_logFilePath.c_str(), GetFileExInfoStandard, &fad))
        return;

    unsigned long long currentSize =
        (static_cast<unsigned long long>(fad.nFileSizeHigh) << 32) |
        static_cast<unsigned long long>(fad.nFileSizeLow);

    // 如果加上新行后不超过限制，直接返回
    if (currentSize + newEntryBytes <= m_maxFileSizeBytes)
        return;

    // 读取整个文件内容
    FILE* fp = nullptr;
    _wfopen_s(&fp, m_logFilePath.c_str(), L"rb");
    if (!fp)
        return;

    std::vector<unsigned char> buf;
    buf.resize(static_cast<size_t>(currentSize));
    size_t readBytes = fread(buf.data(), 1, buf.size(), fp);
    fclose(fp);
    if (readBytes != buf.size())
        return; // 读取异常放弃裁剪

    bool hasBom = false;
    size_t offset = 0;
    if (buf.size() >= 3 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
    {
        hasBom = true;
        offset = 3;
    }

    // 找到第一行(机器ID行)的结束位置（包含换行）
    size_t headerEnd = offset;
    while (headerEnd < buf.size())
    {
        if (buf[headerEnd] == '\n')
        {
            headerEnd += 1; // 包含 '\n'
            break;
        }
        headerEnd++;
    }
    if (headerEnd > buf.size())
        headerEnd = buf.size();

    size_t headerBytes = headerEnd; // 头部字节(含BOM+MachineGuid行)
    size_t payloadStart = headerEnd;
    size_t payloadBytes = (payloadStart < buf.size()) ? (buf.size() - payloadStart) : 0;

    // 可用于旧内容的空间
    size_t availableForOld = 0;
    if (m_maxFileSizeBytes > (headerBytes + newEntryBytes))
        availableForOld = static_cast<size_t>(m_maxFileSizeBytes - headerBytes - newEntryBytes);

    size_t keepTailBytes = 0;
    size_t tailStart = payloadStart;

    if (availableForOld > 0 && payloadBytes > 0)
    {
        if (payloadBytes <= availableForOld)
        {
            // 全部旧内容可保留
            keepTailBytes = payloadBytes;
            tailStart = payloadStart;
        }
        else
        {
            // 截取尾部 availableForOld 字节
            tailStart = payloadStart + (payloadBytes - availableForOld);
            keepTailBytes = availableForOld;

            // 尝试对齐到下一行开头（不保留截断半行）
            size_t scan = tailStart;
            while (scan < buf.size() && buf[scan] != '\n')
                scan++;
            if (scan < buf.size())
            {
                // 跳过这一行的换行符
                scan++;
                size_t adjust = (scan > buf.size()) ? buf.size() : scan;
                if (adjust > tailStart && adjust <= buf.size())
                {
                    // 调整长度
                    size_t removed = adjust - tailStart;
                    if (removed < keepTailBytes)
                    {
                        keepTailBytes -= removed;
                        tailStart = adjust;
                    }
                }
            }
        }
    }
    else
    {
        // 无法保留任何旧内容
        keepTailBytes = 0;
    }

    // 重新写文件：BOM + 头部(重新生成) + 保留尾部
    fp = nullptr;
    _wfopen_s(&fp, m_logFilePath.c_str(), L"wb");
    if (!fp)
        return;

    if (hasBom)
    {
        const unsigned char bom[3] = { 0xEF,0xBB,0xBF };
        fwrite(bom, 1, 3, fp);
    }

    // 重写头部（与初始写法一致，保证 MachineGuid 行存在且最新）
    std::wstring header = L"MachineGuid: " + std::wstring(m_machineGuid.begin(), m_machineGuid.end());
    std::string  utf8Header = GlobalFunc::ConvertToUtf8(header) + "\r\n";
    fwrite(utf8Header.data(), 1, utf8Header.size(), fp);

    if (keepTailBytes > 0)
    {
        fwrite(buf.data() + tailStart, 1, keepTailBytes, fp);
    }
    fflush(fp);
    fclose(fp);

    // 头部已写
    m_headerWritten = true;

    DBFMT(ConsoleHandle, L"[UserActionLogger] 日志裁剪完成，旧大小=%llu，新保留=%zu 字节", currentSize, keepTailBytes);
}

void UserActionLogger::Log(const std::wstring& action)
{
    if (action.empty()) return;

    std::lock_guard<std::mutex> lk(m_mutex);
    if (!m_initialized)
    {
        // 兜底：若用户忘记 Init，则以 NULL 初始化
        Init("NULL");
        if (!m_initialized)
            return;
    }

    // 时间戳
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm tmLocal;
    localtime_s(&tmLocal, &t);

    wchar_t timeBuf[32] = { 0 };
    swprintf(timeBuf, 32, L"[%04d-%02d-%02d %02d:%02d:%02d]",
        tmLocal.tm_year + 1900,
        tmLocal.tm_mon + 1,
        tmLocal.tm_mday,
        tmLocal.tm_hour,
        tmLocal.tm_min,
        tmLocal.tm_sec);

    std::wstring line = std::wstring(timeBuf) + L" " + action;
    std::string utf8 = GlobalFunc::ConvertToUtf8(line) + "\r\n";

    // 如果设置了最大大小，裁剪旧内容
    if (m_maxFileSizeBytes > 0)
    {
        TrimIfNeeded(utf8.size());
    }

    FILE* fp = nullptr;
    _wfopen_s(&fp, m_logFilePath.c_str(), L"ab");
    if (!fp)
    {
        DB(ConsoleHandle, L"[UserActionLogger] 打开日志文件失败(Log阶段)");
        return;
    }
    if (!m_headerWritten)
        WriteHeaderIfNeeded(fp, false);

    fwrite(utf8.data(), 1, utf8.size(), fp);
    fflush(fp);
    fclose(fp);

    DBFMT(ConsoleHandle, L"[UserActionLogger] 写入日志: %s", line.c_str());
}

void UserActionLogger::Flush()
{
    DB(ConsoleHandle, L"[UserActionLogger] Flush 刷新");

}

void UserActionLogger::UploadLogToServer()
{
    DB(ConsoleHandle, L"[UserActionLogger] UploadLogToServer() 开始上传用户行为日志记录");

}