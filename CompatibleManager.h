#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <windows.h>

// FFmpeg头文件包含
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

extern HANDLE ConsoleHandle;

// 编码预设
enum class EncoderPreset
{
    Slow,   // 编码最慢，质量最高(高CPU占用)
    Medium, // 中等
    Fast    // 编码最快，质量最低（低CPU占用）
};

// 编码器类型枚举
enum class EncoderType
{
    NVENC,  // NVIDIA硬件编码器
    AMF,    // AMD硬件编码器
    QSV,    // Intel硬件编码器
    X264,   // 软件编码器
    UNKNOWN // 未知编码器
};

// 容器格式枚举
enum class ContainerFormat
{
    MP4,
    AVI,
    FLV
};

// 编码器参数集 - 存储一组编码参数
struct EncoderParamSet {
    std::string name;                               // 参数集名称
    std::unordered_map<std::string, std::string> params;  // 参数名-值对
};

//编码器兼容性管理单例类
class CompatibleManager
{
public:
    // 获取单例实例
    static CompatibleManager& GetInstance();

    // 禁止拷贝和赋值
    CompatibleManager(const CompatibleManager&) = delete;
    CompatibleManager& operator=(const CompatibleManager&) = delete;

    // 根据预设和格式获取最佳编码器及其参数
    AVDictionary* GetBestEncoder(const AVCodec** codec, EncoderPreset preset, ContainerFormat format);

private:
    CompatibleManager();
    ~CompatibleManager();
    // 获取编码器类型
    EncoderType GetEncoderType(const AVCodec* codec);
    // 测试编码器基本可用性
    bool TestEncoderAvailability(const char* encoderName);
    // 测试特定编码器参数集是否兼容
    bool TestEncoderParamSet(const AVCodec* codec, const EncoderParamSet& paramSet, int width = 640, int height = 480);
    // 查找最佳兼容的参数集
    EncoderParamSet FindBestCompatibleParams(const AVCodec* codec, EncoderPreset preset, ContainerFormat format);
    // 获取特定编码器类型、预设和格式的所有可能参数集
    std::vector<EncoderParamSet> GetEncoderParamSets(EncoderType encoderType, EncoderPreset preset, ContainerFormat format);
    // 创建编码器参数字典
    AVDictionary* CreateOptionDictionary(const EncoderParamSet& paramSet);
    // 将容器格式特定参数添加到参数集
    void AddContainerFormatParams(EncoderParamSet& paramSet, ContainerFormat format);
    // 生成NVENC编码器的参数集列表
    std::vector<EncoderParamSet> GenerateNVENCParamSets(EncoderPreset preset);
    // 生成AMF编码器的参数集列表
    std::vector<EncoderParamSet> GenerateAMFParamSets(EncoderPreset preset);
    // 生成QSV编码器的参数集列表
    std::vector<EncoderParamSet> GenerateQSVParamSets(EncoderPreset preset);
    // 生成X264编码器的参数集列表
    std::vector<EncoderParamSet> GenerateX264ParamSets(EncoderPreset preset);
    // 缓存 - 存储已知兼容的参数集
    std::unordered_map<std::string, EncoderParamSet> m_compatibilityCache;
};