#pragma once
#include <string>
#include <mutex>
#include <afxwin.h>

// 单例：用户行为日志记录器
class UserActionLogger
{
public:
    // 获取单例
    static UserActionLogger* GetInstance();
    void Init(const std::string& machineGuid, const std::wstring& logFilePath = L"", size_t maxFileSizeKB = 0);
    void Log(const std::wstring& action);   //记录一条行为
    void Flush();                           //立即刷新
    void UploadLogToServer();               //上传到服务器

private:
    UserActionLogger() ;
    ~UserActionLogger() = default;
    UserActionLogger(const UserActionLogger&) = delete;
    UserActionLogger& operator=(const UserActionLogger&) = delete;

    void WriteHeaderIfNeeded(FILE* fp, bool isNewFile);
    std::wstring BuildDefaultPath() const;
    bool EnsureDirectory(const std::wstring& fullFilePath);
private:
    std::mutex    m_mutex;
    bool          m_initialized;
    bool          m_headerWritten;
    std::wstring  m_logFilePath;     // 完整路径
    std::string   m_machineGuid;     // 原始 MachineGuid
    size_t       m_maxFileSizeBytes;

private:
    static UserActionLogger* s_UserActionLogger;
    void TrimIfNeeded(size_t newEntryBytes);
};