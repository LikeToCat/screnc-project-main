#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <dshow.h>
#pragma comment(lib, "strmiids.lib")
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/log.h>
}

// 摄像头能力描述结构
struct CameraCapability {
    int width;             // 宽度
    int height;            // 高度
    double fps;            // 帧率
    std::string vcodec;    // 视频编码格式
};

// 屏幕分辨率描述结构
struct ScreenResolution {
    int width;             // 宽度
    int height;            // 高度
    int colorDepth;        // 颜色深度
    int refreshRate;       // 刷新率
    bool isCurrent;        // 是否为当前分辨率
};

// 设备类型枚举
enum class DeviceType {
    MICROPHONE = 1,  // 麦克风设备
    AUDIO_CAPTURE = 2, // 系统音频捕获设备
    CAMERA = 3       // 摄像头设备
};

// 设备信息结构体
struct DeviceInfo {
    std::wstring nameW;         // 设备名称 (宽字符)
    std::string nameA;          // 设备名称 (窄字符)
    std::string alternateName;  // 设备替代名称
    DeviceType type;            // 设备类型
    std::vector<CameraCapability> capabilities; // 摄像头能力列表
};

// 显卡类型枚举
enum class GPUVendor {
    UNKNOWN = 0,
    NVIDIA = 1,
    AMD = 2,
    INTEL = 3
};

// 编码器类型枚举
enum class VideoEncoderType {
    SOFTWARE = 0,     // 软件编码 (cpu)
    NVIDIA_NVENC = 1, // NVIDIA NVENC
    AMD_AMF = 2,      // AMD AMF/VCE
    INTEL_QSV = 3,    // Intel Quick Sync Video
    NVIDIA_CUDA = 4   // NVIDIA CUDA (某些场景下使用)
};

class DeviceManager {
public:
    static DeviceManager& GetInstance();

    // 核心功能
    bool RefreshDeviceList();
    std::vector<CameraCapability> GetCameraCapabilities(const std::string& cameraName);

    // 获取设备列表
    const std::vector<DeviceInfo>& GetMicrophoneDevices() const;
    const std::vector<DeviceInfo>& GetCameraDevices() const;

    // 屏幕分辨率相关接口
    bool RefreshScreenResolutions();
    ScreenResolution GetCurrentScreenResolution() const;
    // 该接口已经启用（在构造函数中放开枚举分辨率代码即可启用）
    const std::vector<ScreenResolution>& GetScreenResolutions() const;

    // 显卡相关
    GPUVendor GetGPUVendor();
    VideoEncoderType GetSupportedHWEncoder();
    std::string GetGPUInfo();

private:
    //功能函数
    //（在枚举DShow设备时，x86的ffmpegAPI无法准确的识别枚举到的设备是否是摄像头设备，所以要在这里进行过滤）
    void FilterCameraDevices();
    void FilterMicrophoneDevices();
private:
    DeviceManager();
    ~DeviceManager();
    static DeviceManager* Instance;

    // 禁止复制和移动
    DeviceManager(const DeviceManager&) = delete;
    DeviceManager& operator=(const DeviceManager&) = delete;
    DeviceManager(DeviceManager&&) = delete;
    DeviceManager& operator=(DeviceManager&&) = delete;

    // 核心功能
    bool EnumerateDirectShowDevices();
    bool QueryCameraCapabilities(const std::string& cameraName);
    void AddPredefinedDevices();

    // 屏幕分辨率枚举
    bool EnumerateScreenResolutions();

    // 设备存储
    std::vector<DeviceInfo> m_microphoneDevices;
    std::vector<DeviceInfo> m_cameraDevices;

    // 屏幕分辨率存储
    std::vector<ScreenResolution> m_screenResolutions;

    // 互斥锁
    static std::mutex s_mutex;
};