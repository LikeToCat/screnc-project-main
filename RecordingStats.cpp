#include "stdafx.h"
#include "RecordingStats.h"
#include "LarStringConversion.h"
#include <iomanip>
#include <sstream>
#include <fstream>

RecordingStats::RecordingStats()
    : m_width(0)
    , m_height(0)
    , m_bitRate(0)
    , m_targetFrameRate(0)
    , m_frameCount(0)
    , m_isRecording(false)
{
}

void RecordingStats::StartRecording() {
    m_frameCount = 0;
    m_startTime = std::chrono::steady_clock::now();
    m_isRecording = true;
}

void RecordingStats::StopRecording() {
    if (m_isRecording) {
        m_endTime = std::chrono::steady_clock::now();
        m_isRecording = false;
    }
}

void RecordingStats::IncrementFrameCount() {
    m_frameCount++;
}

void RecordingStats::SetVideoParameters(int width, int height, double bitRate, int frameRate, const char* codecName) {
    m_width = width;
    m_height = height;
    m_bitRate = bitRate;
    m_targetFrameRate = frameRate;
    m_codecName = codecName ? codecName : "unknown";
}

void RecordingStats::SetOutputPath(const std::string& path) {
    m_outputPath = path;
}

double RecordingStats::GetDurationSeconds() const {
    if (m_isRecording) return 0.0;

    return std::chrono::duration<double>(m_endTime - m_startTime).count();
}

double RecordingStats::GetActualBitrate() const {
    double durationSec = GetDurationSeconds();
    if (durationSec <= 0) return 0.0;

    // 获取文件大小，计算实际比特率
    std::ifstream file(m_outputPath, std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open()) return 0.0;

    std::streampos fileSize = file.tellg();
    return (fileSize * 8.0) / durationSec; // 比特/秒
}

std::pair<double, std::string> RecordingStats::FormatFileSize() const {
    // 获取文件大小
    std::ifstream file(m_outputPath, std::ifstream::ate | std::ifstream::binary);
    if (!file.is_open()) {
        return std::make_pair(0.0, std::string("bytes"));
    }

    double size = static_cast<double>(file.tellg());
    std::string unit = "bytes";

    // 确定最佳单位
    if (size >= 1024.0 * 1024.0 * 1024.0) {
        size /= (1024.0 * 1024.0 * 1024.0);
        unit = "GB";
    }
    else if (size >= 1024.0 * 1024.0) {
        size /= (1024.0 * 1024.0);
        unit = "MB";
    }
    else if (size >= 1024.0) {
        size /= 1024.0;
        unit = "KB";
    }

    return std::make_pair(size, unit);
}

std::wstring RecordingStats::FormatTime(double seconds) const {
    int hours = static_cast<int>(seconds) / 3600;
    int minutes = (static_cast<int>(seconds) % 3600) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int milliseconds = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

    std::wstringstream ss;
    ss << std::setfill(L'0') << std::setw(2) << hours << L":"
        << std::setfill(L'0') << std::setw(2) << minutes << L":"
        << std::setfill(L'0') << std::setw(2) << secs << L"."
        << std::setfill(L'0') << std::setw(3) << milliseconds;
    return ss.str();
}

std::wstring RecordingStats::GenerateSummary() const {
    double durationSec = GetDurationSeconds();
    double actualBitrate = GetActualBitrate();

    // 替换结构化绑定
    std::pair<double, std::string> fileSizeInfo = FormatFileSize();
    double fileSize = fileSizeInfo.first;
    std::string sizeUnit = fileSizeInfo.second;

    double averageFps = (durationSec > 0) ? (m_frameCount / durationSec) : 0;

    std::ifstream file(m_outputPath, std::ifstream::ate | std::ifstream::binary);
    long rawFileSize = file.is_open() ? static_cast<long>(file.tellg()) : 0;

    std::wstringstream ss;
    ss << L"\n========== 录制完成 - 视频信息摘要 ==========\n"
        << L"文件路径: " << std::wstring(LARSC::s2ws(m_outputPath)) << L"\n"
        << L"时长: " << FormatTime(durationSec) << L" (" << std::fixed << std::setprecision(2) << durationSec << L"秒)\n"
        << L"分辨率: " << m_width << L"x" << m_height << L"\n"
        << L"编码器: " << std::wstring(LARSC::s2ws(m_codecName)) << L"\n"
        << L"总帧数: " << m_frameCount << L"\n"
        << L"设置帧率: " << m_targetFrameRate << L" FPS\n"
        << L"实际平均I帧数: " << std::fixed << std::setprecision(2) << averageFps
        << L"设置码率: " << std::fixed << std::setprecision(2) << (m_bitRate / 1000000.0) << L" Mbps\n"
        << L"实际码率: " << std::fixed << std::setprecision(2) << (actualBitrate / 1000000.0) << L" Mbps\n"
        << L"文件大小: " << std::fixed << std::setprecision(2) << static_cast<double>(fileSize) << L" "
        << std::wstring(sizeUnit.begin(), sizeUnit.end()) << L" (" << rawFileSize << L" 字节)\n";

    if (durationSec > 0) {
        ss << L"数据效率: " << std::fixed << std::setprecision(2) << ((rawFileSize / 1024.0) / durationSec) << L" KB/秒\n";
    }

    ss << L"============================================";
    return ss.str();
}