#pragma once
#include <string>
#include <chrono>

class RecordingStats {
public:
    RecordingStats();

    // 记录开始/结束录制
    void StartRecording();
    void StopRecording();

    // 帧计数器
    void IncrementFrameCount();

    // 设置录制参数
    void SetVideoParameters(int width, int height, double bitRate, int frameRate, const char* codecName);
    void SetOutputPath(const std::string& path);

    // 生成格式化摘要
    std::wstring GenerateSummary() const;
private:
    // 记录参数
    int m_width;
    int m_height;
    double m_bitRate;
    int m_targetFrameRate;
    std::string m_codecName;
    std::string m_outputPath;

    // 统计数据
    std::chrono::steady_clock::time_point m_startTime;
    std::chrono::steady_clock::time_point m_endTime;
    int m_frameCount;
    bool m_isRecording;

    // 辅助方法
    double GetDurationSeconds() const;
    double GetActualBitrate() const;
    std::pair<double, std::string> FormatFileSize() const;
    std::wstring FormatTime(double seconds) const;
};