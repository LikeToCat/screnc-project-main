#include "stdafx.h"
#include "DeviceManager.h"
#include "CDebug.h"
#include "LarStringConversion.h"
#include "GlobalFunc.h"
#include <iostream>
#include <windows.h>
#include <sstream>
#include <future>
#include <thread>
#include <chrono>
#include <regex>
#include <set> 
#include <algorithm>
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

extern HANDLE ConsoleHandle;
// 初始化静态成员
// 将设备路径转换为FFmpeg期望的编码格式

// 字符编码转换工具
std::wstring UTF8ToWide(const std::string& utf8);
std::wstring AnsiToWide(const std::string& ansi);
std::string WideToAnsi(const std::wstring& wide);

std::string ConvertToFFmpegEncoding(const std::wstring& devicePath)
{
    if (devicePath.empty())
        return "";

    // 使用CP_UTF8确保生成的是UTF-8编码，这是FFmpeg期望的格式
    int size = WideCharToMultiByte(CP_UTF8, 0, devicePath.c_str(), -1, NULL, 0, NULL, NULL);
    std::vector<char> buffer(size);
    WideCharToMultiByte(CP_UTF8, 0, devicePath.c_str(), -1, &buffer[0], size, NULL, NULL);

    // 创建string时去掉末尾的null字符
    std::string result(buffer.data(), size - 1);

    // 将所有冒号(":") 替换为下划线("_")
    size_t pos = 0;
    while ((pos = result.find(":", pos)) != std::string::npos)
    {
        result.replace(pos, 1, "_");
        pos++; // 移动到下一个可能的位置
    }

    return result;
}

std::mutex DeviceManager::s_mutex;

struct CameraCapabilitiesCapture 
{
    std::vector<CameraCapability> capabilities;
    std::string cameraName;
    std::string logBuffer;
    bool capabilitiesFound;
};

// 全局捕获对象

static CameraCapabilitiesCapture g_cameraCapabilities;

// 单例模式获取实例
DeviceManager& DeviceManager::GetInstance()
{
    static DeviceManager instance;
    return instance;
}

// 构造函数
DeviceManager::DeviceManager()
{
    avdevice_register_all();
    RefreshDeviceList();
    //RefreshScreenResolutions();
}

// 析构函数
DeviceManager::~DeviceManager()
{
    // 无需特殊清理
}

// 设备列表刷新
bool DeviceManager::RefreshDeviceList()
{
    // 清空现有设备列表
    m_microphoneDevices.clear();
    m_cameraDevices.clear();

    // 枚举设备
    bool result = EnumerateDirectShowDevices();

    // 如果没有检测到设备，添加预定义的常用设备
    if (m_microphoneDevices.empty() && m_cameraDevices.empty()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"没有检测到任何麦克风设备和音频设备");
    }

    // 输出设备信息
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"检测到麦克风: %d个", m_microphoneDevices.size());
    for (const auto& device : m_microphoneDevices) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"  - %s", device.nameW.c_str());
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"检测到摄像头: %d个", m_cameraDevices.size());
    for (const auto& device : m_cameraDevices) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"  - %s", device.nameW.c_str());

        // 获取摄像头能力参数
        if (device.capabilities.empty()) {
            QueryCameraCapabilities(device.nameA);
        }
    }

    return !m_microphoneDevices.empty() || !m_cameraDevices.empty();
}

// 获取设备列表
const std::vector<DeviceInfo>& DeviceManager::GetMicrophoneDevices() const
{
    return m_microphoneDevices;
}

const std::vector<DeviceInfo>& DeviceManager::GetCameraDevices() const
{
    return m_cameraDevices;
}

// 摄像头能力日志回调函数
static void camera_capabilities_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
    if (level > AV_LOG_DEBUG) return; // 增加日志级别为DEBUG，捕获更多信息

   // 格式化日志消息
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, vl);
    std::string line(buffer);

    // 记录到日志缓冲区
    g_cameraCapabilities.logBuffer += line;

    // 打印原始日志内容以便调试
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[RAW] %hs", line.c_str());

    // 标记是否找到能力信息
    if (line.find("DirectShow video device options") != std::string::npos) {
        g_cameraCapabilities.capabilitiesFound = true;
        DEBUG_CONSOLE_STR(ConsoleHandle, L"找到摄像头选项标头");
    }

    // 尝试解析能力信息 - 更灵活的匹配
    // 分别查找编码格式、分辨率和帧率
    std::string vcodec;
    int width = 0, height = 0;
    double fps = 0;

    // 提取编码格式
    if (line.find("vcodec=") != std::string::npos) {
        size_t pos = line.find("vcodec=") + 7;
        size_t end = line.find(" ", pos);
        if (end != std::string::npos) {
            vcodec = line.substr(pos, end - pos);
        }
    }
    else if (line.find("pixel_format=") != std::string::npos) {
        size_t pos = line.find("pixel_format=") + 13;
        size_t end = line.find(" ", pos);
        if (end != std::string::npos) {
            vcodec = line.substr(pos, end - pos);
        }
    }

    // 提取分辨率
    std::regex resRegex("(\\d+)x(\\d+)");
    std::smatch resMatch;
    if (std::regex_search(line, resMatch, resRegex)) {
        width = std::stoi(resMatch[1].str());
        height = std::stoi(resMatch[2].str());
    }

    // 提取帧率
    std::regex fpsRegex("fps=(\\d+(\\.\\d+)?)");
    std::smatch fpsMatch;
    if (std::regex_search(line, fpsMatch, fpsRegex)) {
        fps = std::stod(fpsMatch[1].str());
    }

    // 如果找到了所有必要信息，创建能力对象
    if (!vcodec.empty() && width > 0 && height > 0 && fps > 0) {
        CameraCapability cap;
        cap.vcodec = vcodec;
        cap.width = width;
        cap.height = height;
        cap.fps = fps;

        g_cameraCapabilities.capabilities.push_back(cap);
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"解析摄像头参数: %dx%d @%.0ffps, %hs",
            cap.width, cap.height, cap.fps, cap.vcodec.c_str());
    }
}

static void GetCameraCapabilitiesThread(const std::string& cameraName)
{
    // 初始化
    g_cameraCapabilities.capabilities.clear();
    g_cameraCapabilities.cameraName = cameraName;
    g_cameraCapabilities.logBuffer.clear();
    g_cameraCapabilities.capabilitiesFound = false;

#ifdef _WIN64
    // ===== x64 平台实现 =====
    av_log_set_callback(camera_capabilities_log_callback);

    // 准备查询
    AVInputFormat* dshow_format = av_find_input_format("dshow");
    if (!dshow_format)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"无法找到DirectShow输入格式");
        av_log_set_callback(av_log_default_callback);
        return;
    }

    // 创建设备路径和选项
    std::string device_path = "video=" + cameraName;
    AVDictionary* options = nullptr;
    av_dict_set(&options, "list_options", "true", 0);
    av_dict_set(&options, "timeout", "5000000", 0);  // 5秒超时

    // 执行查询
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"开始查询摄像头 %hs 参数...", cameraName.c_str());
    AVFormatContext* ctx = nullptr;
    int ret = avformat_open_input(&ctx, device_path.c_str(), dshow_format, &options);

    // 清理资源
    if (ctx) avformat_close_input(&ctx);
    av_dict_free(&options);
    av_log_set_callback(av_log_default_callback);
#else
    // ===== x86 平台实现 ===== 
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"开始查询摄像头 %hs 参数...", cameraName.c_str());

    // 使用 DirectShow API 查询摄像头能力
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    try {
        ICreateDevEnum* pDevEnum = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
            CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
            (void**)&pDevEnum);

        if (SUCCEEDED(hr)) {
            // 创建视频捕获设备枚举器
            IEnumMoniker* pEnum = NULL;
            hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

            // 查找指定名称的摄像头
            if (SUCCEEDED(hr) && pEnum != NULL) {
                IMoniker* pMoniker = nullptr;
                ULONG fetched;

                while (pEnum->Next(1, &pMoniker, &fetched) == S_OK) {
                    IPropertyBag* pPropBag = nullptr;
                    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

                    if (SUCCEEDED(hr)) {
                        VARIANT varName;
                        VariantInit(&varName);
                        hr = pPropBag->Read(L"FriendlyName", &varName, 0);

                        if (SUCCEEDED(hr)) {
                            // 转换设备名称以匹配
                            // 使用 DeviceManager 实例的 WideToAnsi 方法
                            std::string deviceName = WideToAnsi(varName.bstrVal);

                            // 如果找到匹配的摄像头
                            if (deviceName == cameraName) {
                                // 获取摄像头的媒体类型能力
                                IBaseFilter* pCap = nullptr;
                                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap);

                                if (SUCCEEDED(hr)) {
                                    // 枚举引脚
                                    IEnumPins* pEnumPins = nullptr;
                                    hr = pCap->EnumPins(&pEnumPins);

                                    if (SUCCEEDED(hr)) {
                                        IPin* pPin = nullptr;
                                        while (pEnumPins->Next(1, &pPin, NULL) == S_OK) {
                                            // 获取引脚方向
                                            PIN_DIRECTION pinDir;
                                            hr = pPin->QueryDirection(&pinDir);

                                            if (SUCCEEDED(hr) && pinDir == PINDIR_OUTPUT) {
                                                // 枚举媒体类型
                                                IEnumMediaTypes* pEnumMediaTypes = nullptr;
                                                hr = pPin->EnumMediaTypes(&pEnumMediaTypes);

                                                if (SUCCEEDED(hr)) {
                                                    AM_MEDIA_TYPE* pmt = nullptr;
                                                    while (pEnumMediaTypes->Next(1, &pmt, NULL) == S_OK) {
                                                        if (pmt->majortype == MEDIATYPE_Video &&
                                                            pmt->formattype == FORMAT_VideoInfo) {

                                                            VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)pmt->pbFormat;

                                                            // 创建能力对象
                                                            CameraCapability cap;
                                                            cap.width = pvih->bmiHeader.biWidth;
                                                            cap.height = abs(pvih->bmiHeader.biHeight);  // 可能为负
                                                            cap.fps = 10000000.0 / pvih->AvgTimePerFrame;

                                                            // 设置编码格式 (简化处理)
                                                            if (pmt->subtype == MEDIASUBTYPE_RGB24)
                                                                cap.vcodec = "rgb24";
                                                            else if (pmt->subtype == MEDIASUBTYPE_MJPG)
                                                                cap.vcodec = "mjpeg";
                                                            else if (pmt->subtype == MEDIASUBTYPE_YUY2)
                                                                cap.vcodec = "yuyv422";
                                                            else
                                                                cap.vcodec = "unknown";

                                                            // 添加到能力列表
                                                            g_cameraCapabilities.capabilities.push_back(cap);
                                                            g_cameraCapabilities.capabilitiesFound = true;

                                                            DEBUG_CONSOLE_FMT(ConsoleHandle,
                                                                L"解析到摄像头参数: %dx%d @%.0ffps, %hs",
                                                                cap.width, cap.height, cap.fps, cap.vcodec.c_str());
                                                        }

                                                        // 释放媒体类型
                                                        if (pmt->cbFormat != 0)
                                                            CoTaskMemFree(pmt->pbFormat);
                                                        if (pmt)
                                                            CoTaskMemFree(pmt);
                                                    }
                                                    pEnumMediaTypes->Release();
                                                }
                                            }
                                            pPin->Release();
                                        }
                                        pEnumPins->Release();
                                    }
                                    pCap->Release();
                                }
                                break;  // 找到匹配的摄像头后退出循环
                            }
                        }

                        VariantClear(&varName);
                        pPropBag->Release();
                    }
                    pMoniker->Release();
                }
                pEnum->Release();
            }
            pDevEnum->Release();
        }
    }
    catch (...) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"查询摄像头能力时发生异常");
    }

    CoUninitialize();
#endif

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"摄像头 %hs 参数查询完成，找到 %zu 个参数",
        cameraName.c_str(), g_cameraCapabilities.capabilities.size());
}

// 查询摄像头能力
bool DeviceManager::QueryCameraCapabilities(const std::string& cameraName)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    // 参数检查
    if (cameraName.empty()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误：摄像头名称为空");
        return false;
    }

    // 查找对应的摄像头
    DeviceInfo* targetDevice = nullptr;
    for (auto& camera : m_cameraDevices) {
        if (camera.nameA == cameraName) {
            targetDevice = &camera;
            break;
        }
    }

    if (!targetDevice) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"未找到摄像头: %hs", cameraName.c_str());
        return false;
    }

    // 清空现有能力列表
    targetDevice->capabilities.clear();

    // 执行异步查询
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"开始查询摄像头 %hs 能力参数...", cameraName.c_str());
    auto future = std::async(std::launch::async, GetCameraCapabilitiesThread, cameraName);

    // 等待查询完成，设置超时
    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"摄像头能力查询超时!");

        // 尝试从已捕获日志中解析
        if (!g_cameraCapabilities.logBuffer.empty() && g_cameraCapabilities.capabilitiesFound) {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"尝试从部分日志解析摄像头信息...");
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"未找到摄像头能力信息");
            // 不返回false，而是继续添加预设
        }
    }

    // 拷贝捕获的能力信息
    targetDevice->capabilities = g_cameraCapabilities.capabilities;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"获取到 %zu 个摄像头能力参数",
        targetDevice->capabilities.size());

    // 如果捕获的参数太少，尝试从累积的日志中解析
    if (targetDevice->capabilities.size() < 5 && g_cameraCapabilities.capabilitiesFound) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"从累积日志中重新解析摄像头能力信息...");

        // 清空现有能力，重新解析
        targetDevice->capabilities.clear();

        // 对完整的日志进行行分割
        std::istringstream logStream(g_cameraCapabilities.logBuffer);
        std::string line;

        while (std::getline(logStream, line)) {
            // 查找包含编码格式和分辨率的行
            if ((line.find("vcodec=") != std::string::npos ||
                line.find("pixel_format=") != std::string::npos) &&
                line.find("min s=") != std::string::npos) {

                // 提取信息
                std::string vcodec;
                if (line.find("vcodec=") != std::string::npos) {
                    size_t pos = line.find("vcodec=") + 7;
                    size_t end = line.find(" ", pos);
                    if (end != std::string::npos) {
                        vcodec = line.substr(pos, end - pos);
                    }
                }
                else if (line.find("pixel_format=") != std::string::npos) {
                    size_t pos = line.find("pixel_format=") + 13;
                    size_t end = line.find(" ", pos);
                    if (end != std::string::npos) {
                        vcodec = line.substr(pos, end - pos);
                    }
                }

                // 提取分辨率
                int width = 0, height = 0;
                size_t minPos = line.find("min s=");
                if (minPos != std::string::npos) {
                    minPos += 6;
                    size_t xPos = line.find("x", minPos);
                    if (xPos != std::string::npos) {
                        width = std::stoi(line.substr(minPos, xPos - minPos));

                        size_t fpsPos = line.find(" fps=", xPos);
                        if (fpsPos != std::string::npos) {
                            height = std::stoi(line.substr(xPos + 1, fpsPos - xPos - 1));

                            // 提取帧率
                            double fps = 0;
                            fpsPos += 5;
                            size_t maxPos = line.find(" max", fpsPos);
                            if (maxPos != std::string::npos) {
                                fps = std::stod(line.substr(fpsPos, maxPos - fpsPos));

                                // 创建能力对象
                                if (!vcodec.empty() && width > 0 && height > 0 && fps > 0) {
                                    CameraCapability cap;
                                    cap.vcodec = vcodec;
                                    cap.width = width;
                                    cap.height = height;
                                    cap.fps = fps;

                                    targetDevice->capabilities.push_back(cap);
                                    DEBUG_CONSOLE_FMT(ConsoleHandle, L"从日志解析摄像头参数: %dx%d @%.0ffps, %hs",
                                        cap.width, cap.height, cap.fps, cap.vcodec.c_str());
                                }
                            }
                        }
                    }
                }
            }
        }

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"从日志中解析到 %zu 个摄像头能力参数",
            targetDevice->capabilities.size());
    }

    // 如果仍然没有能力信息，添加预设
    if (targetDevice->capabilities.empty()) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"未能获取摄像头能力参数，添加常见预设");

        // 1080p
        CameraCapability cap1080p;
        cap1080p.width = 1920;
        cap1080p.height = 1080;
        cap1080p.fps = 30;
        cap1080p.vcodec = "mjpeg";
        targetDevice->capabilities.push_back(cap1080p);

        // 720p
        CameraCapability cap720p;
        cap720p.width = 1280;
        cap720p.height = 720;
        cap720p.fps = 30;
        cap720p.vcodec = "mjpeg";
        targetDevice->capabilities.push_back(cap720p);

        // 480p
        CameraCapability cap480p;
        cap480p.width = 640;
        cap480p.height = 480;
        cap480p.fps = 30;
        cap480p.vcodec = "mjpeg";
        targetDevice->capabilities.push_back(cap480p);
    }

    return !targetDevice->capabilities.empty();
}

// 获取摄像头能力
std::vector<CameraCapability> DeviceManager::GetCameraCapabilities(const std::string& cameraName)
{
    // 查找对应的摄像头
    for (auto& camera : m_cameraDevices) {
        if (camera.nameA == cameraName) {
            // 如果还没有查询过能力，现在查询
            if (camera.capabilities.empty()) {
                QueryCameraCapabilities(cameraName);
            }
            return camera.capabilities;
        }
    }

    // 找不到设备，返回空列表
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"未找到摄像头: %hs", cameraName.c_str());
    return std::vector<CameraCapability>();
}

//过滤枚举到的摄像头设备
void DeviceManager::FilterCameraDevices()
{
    // 没有设备时快速返回
    if (m_cameraDevices.empty()) {
        return;
    }

    // 预分配向量容量，避免重新分配内存
    std::vector<DeviceInfo> filteredDevices;
    filteredDevices.reserve(m_cameraDevices.size());

    DEBUG_CONSOLE_STR(ConsoleHandle, L"开始过滤非真实摄像头设备...");

    // 性能优化：使用一次遍历
    for (const auto& device : m_cameraDevices)
    {
        // 临时存储字符串，避免重复转换开销
        const std::string deviceNameA = WideToAnsi(device.nameW);
        const std::string& deviceId = device.alternateName;

        // 检查是否具有真实摄像头的特征 (USB/PnP设备路径)
        bool isRealCamera = (deviceId.find("usb#") != std::string::npos ||
            deviceId.find("vid_") != std::string::npos ||
            deviceId.find("pnp") != std::string::npos);

        // 检查是否包含常见摄像头关键词
        bool hasCameraKeyword = (deviceNameA.find("camera") != std::string::npos ||
            deviceNameA.find("webcam") != std::string::npos ||
            deviceNameA.find("摄像头") != std::string::npos ||
            deviceNameA.find("相机") != std::string::npos);

        // 检查是否为已知的虚拟摄像头软件
        bool isVirtualDevice = (deviceNameA.find("virtual") != std::string::npos ||
            deviceNameA.find("obs") != std::string::npos ||
            deviceNameA.find("xsplit") != std::string::npos ||
            deviceNameA.find("manycam") != std::string::npos);

        // 决策逻辑：保留真实摄像头
        if ((isRealCamera || hasCameraKeyword) && !isVirtualDevice)
        {
            filteredDevices.push_back(device);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"保留真实摄像头: %s", device.nameW.c_str());
        }
        else
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"过滤掉非摄像头设备: %s", device.nameW.c_str());
        }
    }
    m_cameraDevices.swap(filteredDevices);
}

//过滤枚举到的麦克风设备
void DeviceManager::FilterMicrophoneDevices()
{
    // 没有设备时快速返回
    if (m_microphoneDevices.empty()) {
        return;
    }

    // 预分配向量容量，避免重新分配内存
    std::vector<DeviceInfo> filteredDevices;
    filteredDevices.reserve(m_microphoneDevices.size());

    DEBUG_CONSOLE_STR(ConsoleHandle, L"开始过滤非真实麦克风设备...");

    // 性能优化：使用一次遍历
    for (const auto& device : m_microphoneDevices)
    {
        // 临时存储字符串，避免重复转换开销
        const std::string deviceNameA = WideToAnsi(device.nameW);
        const std::string& deviceId = device.alternateName;

        // 检查是否具有真实麦克风的特征 (USB/PnP设备路径)
        bool isRealMicrophone = (deviceId.find("usb#") != std::string::npos ||
            deviceId.find("vid_") != std::string::npos ||
            deviceId.find("pnp") != std::string::npos);

        // 检查是否包含常见麦克风关键词
        bool hasMicrophoneKeyword = (deviceNameA.find("microphone") != std::string::npos ||
            deviceNameA.find("mic") != std::string::npos ||
            deviceNameA.find("麦克风") != std::string::npos ||
            deviceNameA.find("话筒") != std::string::npos ||
            deviceNameA.find("headset") != std::string::npos);

        // 检查是否为已知的虚拟麦克风软件
        bool isVirtualDevice = (deviceNameA.find("virtual") != std::string::npos ||
            deviceNameA.find("mixer") != std::string::npos ||
            deviceNameA.find("voicemeeter") != std::string::npos ||
            deviceNameA.find("vac") != std::string::npos ||
            deviceNameA.find("cable output") != std::string::npos ||
            deviceNameA.find("捕获") != std::string::npos ||
            deviceNameA.find("立体声混音") != std::string::npos);

        // 检查是否为扬声器或其他输出设备（应该排除）
        bool isSpeakerOrOutput = (deviceNameA.find("speaker") != std::string::npos ||
            deviceNameA.find("headphone") != std::string::npos ||
            deviceNameA.find("扬声器") != std::string::npos ||
            deviceNameA.find("耳机") != std::string::npos ||
            deviceNameA.find("输出") != std::string::npos ||
            deviceNameA.find("output") != std::string::npos);

        // 决策逻辑：保留真实麦克风
        if ((isRealMicrophone || hasMicrophoneKeyword) && !isVirtualDevice && !isSpeakerOrOutput)
        {
            filteredDevices.push_back(device);
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"保留真实麦克风: %s", device.nameW.c_str());
        }
        else
        {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"过滤掉非麦克风设备: %s", device.nameW.c_str());
        }
    }
    m_microphoneDevices.swap(filteredDevices);
}

// 枚举DirectShow设备
bool DeviceManager::EnumerateDirectShowDevices()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"开始使用Windows DirectShow API枚举设备");

    // 清空设备列表
    m_microphoneDevices.clear();
    m_cameraDevices.clear();

    // 确保COM已初始化
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    bool needUninitialize = SUCCEEDED(hr);

    // 如果COM已经初始化，不需要再次释放
    if (hr == S_FALSE)
    {
        needUninitialize = false;
    }
    else if (FAILED(hr))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"COM初始化失败");
        return false;
    }

    bool deviceFound = false;
    int deviceCount = 0;

    try
    {
        // 创建系统设备枚举器
        ICreateDevEnum* pDevEnum = nullptr;
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void**)&pDevEnum);
        if (FAILED(hr))
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"无法创建系统设备枚举器");
            if (needUninitialize) CoUninitialize();
            return false;
        }

        // -----枚举获取到摄像头设备----
        IEnumMoniker* pVideoEnum = nullptr;
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pVideoEnum, 0);
        if (hr == S_OK && pVideoEnum)
        {
            pVideoEnum->Reset();

            IMoniker* pMoniker = nullptr;
            ULONG fetched;
            while (pVideoEnum->Next(1, &pMoniker, &fetched) == S_OK)
            {
                // 获取设备路径
                LPOLESTR oleDisplayName = NULL;
                hr = pMoniker->GetDisplayName(NULL, NULL, &oleDisplayName);
                std::wstring devicePath;
                if (SUCCEEDED(hr) && oleDisplayName)
                {
                    devicePath = oleDisplayName;
                    CoTaskMemFree(oleDisplayName);
                }

                IPropertyBag* pPropBag = nullptr;
                hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
                if (SUCCEEDED(hr))
                {
                    // 获取友好名称
                    VARIANT varName;
                    VariantInit(&varName);
                    hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                    if (SUCCEEDED(hr))
                    {
                        // 设备名称
                        std::string deviceName = WideToAnsi(varName.bstrVal);

                        // 创建设备信息
                        DeviceInfo device;
                        device.nameW = varName.bstrVal;
                        device.nameA = deviceName;
                        device.type = DeviceType::CAMERA;  // 显式设置为摄像头类型

                        // 设置替代名称（用于FFmpeg调用）
                        if (!devicePath.empty())
                        {
                            std::string pathStr = ConvertToFFmpegEncoding(devicePath);
                            device.alternateName = pathStr;
                        }

                        // 添加到摄像头设备列表
                        m_cameraDevices.push_back(device);
                        deviceCount++;

                        DEBUG_CONSOLE_FMT(ConsoleHandle, L"发现摄像头设备: %s",
                            device.nameW.c_str());
                    }
                    VariantClear(&varName);
                    pPropBag->Release();
                }
                pMoniker->Release();
            }
            pVideoEnum->Release();
        }



        // ----- 枚举音频捕获设备 -----
        IEnumMoniker* pAudioEnum = nullptr;
        hr = pDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pAudioEnum, 0);
        if (hr == S_OK && pAudioEnum)
        {
            pAudioEnum->Reset();

            IMoniker* pMoniker = nullptr;
            ULONG fetched;
            while (pAudioEnum->Next(1, &pMoniker, &fetched) == S_OK)
            {
                // 获取设备路径
                LPOLESTR oleDisplayName = NULL;
                hr = pMoniker->GetDisplayName(NULL, NULL, &oleDisplayName);
                std::wstring devicePath;
                if (SUCCEEDED(hr) && oleDisplayName)
                {
                    devicePath = oleDisplayName;
                    CoTaskMemFree(oleDisplayName);
                }

                IPropertyBag* pPropBag = nullptr;
                hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
                if (SUCCEEDED(hr))
                {
                    // 获取友好名称
                    VARIANT varName;
                    VariantInit(&varName);
                    hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                    if (SUCCEEDED(hr))
                    {
                        // 设备名称，用于检查是否为麦克风
                        std::string deviceName = WideToAnsi(varName.bstrVal);

                        // 创建设备信息
                        DeviceInfo device;
                        device.nameW = varName.bstrVal;
                        device.nameA = deviceName;

                        std::string formattedAlternateName;
                        if (!devicePath.empty())
                        {
                            std::string pathStr = ConvertToFFmpegEncoding(devicePath);
                            device.alternateName = pathStr;
                        }
                        m_microphoneDevices.push_back(device);
                        deviceCount++;
                    }
                    VariantClear(&varName);
                    pPropBag->Release();
                }
                pMoniker->Release();
            }
            pAudioEnum->Release();
        }

        pDevEnum->Release();
        deviceFound = deviceCount > 0;

        if (deviceFound) {
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"使用Windows API枚举到 %d 个设备", deviceCount);
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"Windows API枚举未找到任何设备");
        }
    }
    catch (...) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"枚举设备时发生异常");
        deviceFound = false;
    }

    // 释放COM
    if (needUninitialize) {
        CoUninitialize();
    }

    // 过滤设备列表
    FilterCameraDevices();
    FilterMicrophoneDevices();

    return !m_microphoneDevices.empty() || !m_cameraDevices.empty();
}

// 枚举屏幕分辨率
bool DeviceManager::EnumerateScreenResolutions()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    m_screenResolutions.clear();

    // 获取当前分辨率作为参考
    int currentWidth = GetSystemMetrics(SM_CXSCREEN);
    int currentHeight = GetSystemMetrics(SM_CYSCREEN);
    int currentBpp = 0;
    int currentRefreshRate = 0;

    // 获取当前的颜色深度和刷新率
    HDC hdc = GetDC(NULL);
    if (hdc) {
        currentBpp = GetDeviceCaps(hdc, BITSPIXEL);
        currentRefreshRate = GetDeviceCaps(hdc, VREFRESH);
        ReleaseDC(NULL, hdc);
    }

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"当前屏幕分辨率: %dx%d, %d位色深, %dHz",
        currentWidth, currentHeight, currentBpp, currentRefreshRate);

    // 使用集合来去除重复的分辨率
    std::set<std::pair<int, int>> uniqueResolutions;

    // 枚举所有支持的显示模式
    DEVMODE dm = { 0 };
    dm.dmSize = sizeof(DEVMODE);

    for (int i = 0; EnumDisplaySettings(NULL, i, &dm); i++) {
        // 只考虑32位色深的分辨率(通常是最佳的)
        if (dm.dmBitsPerPel >= 24) {
            auto resPair = std::make_pair(dm.dmPelsWidth, dm.dmPelsHeight);
            uniqueResolutions.insert(resPair);

            // 检查是否是当前分辨率
            bool isCurrent = (dm.dmPelsWidth == currentWidth &&
                dm.dmPelsHeight == currentHeight &&
                dm.dmBitsPerPel == currentBpp &&
                dm.dmDisplayFrequency == currentRefreshRate);

            // 只为当前分辨率添加详细信息
            if (isCurrent) {
                ScreenResolution res;
                res.width = dm.dmPelsWidth;
                res.height = dm.dmPelsHeight;
                res.colorDepth = dm.dmBitsPerPel;
                res.refreshRate = dm.dmDisplayFrequency;
                res.isCurrent = true;

                // 添加到列表开头
                m_screenResolutions.insert(m_screenResolutions.begin(), res);

                DEBUG_CONSOLE_FMT(ConsoleHandle, L"添加当前分辨率: %dx%d, %d位色深, %dHz",
                    res.width, res.height, res.colorDepth, res.refreshRate);
            }
        }
    }

    // 添加所有唯一分辨率
    for (const auto& resPair : uniqueResolutions) {
        // 跳过已经添加的当前分辨率
        if (resPair.first == currentWidth && resPair.second == currentHeight)
            continue;

        ScreenResolution res;
        res.width = resPair.first;
        res.height = resPair.second;
        res.colorDepth = 32; // 默认32位色深
        res.refreshRate = 60; // 默认60Hz刷新率
        res.isCurrent = false;

        m_screenResolutions.push_back(res);

        DEBUG_CONSOLE_FMT(ConsoleHandle, L"添加可用分辨率: %dx%d",
            res.width, res.height);
    }

    // 按分辨率从高到低排序
    std::sort(m_screenResolutions.begin() + 1, m_screenResolutions.end(),
        [](const ScreenResolution& a, const ScreenResolution& b) {
            return (a.width * a.height) > (b.width * b.height);
        });

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"共枚举到 %zu 种屏幕分辨率", m_screenResolutions.size());

    return !m_screenResolutions.empty();
}

// 刷新屏幕分辨率列表
bool DeviceManager::RefreshScreenResolutions()
{
    return EnumerateScreenResolutions();
}

// 获取所有支持的屏幕分辨率
const std::vector<ScreenResolution>& DeviceManager::GetScreenResolutions() const
{
    return m_screenResolutions;
}

// 获取当前的屏幕分辨率
ScreenResolution DeviceManager::GetCurrentScreenResolution() const
{
    if (!m_screenResolutions.empty() && m_screenResolutions[0].isCurrent) {
        return m_screenResolutions[0];
    }

    // 如果没有缓存当前分辨率，直接获取系统信息
    ScreenResolution res;
    res.width = GetSystemMetrics(SM_CXSCREEN);
    res.height = GetSystemMetrics(SM_CYSCREEN);
    res.isCurrent = true;

    HDC hdc = GetDC(NULL);
    if (hdc) {
        res.colorDepth = GetDeviceCaps(hdc, BITSPIXEL);
        res.refreshRate = GetDeviceCaps(hdc, VREFRESH);
        ReleaseDC(NULL, hdc);
    }
    else {
        res.colorDepth = 32; // 默认值
        res.refreshRate = 60; // 默认值
    }

    return res;
}

// 获取GPU供应商
GPUVendor DeviceManager::GetGPUVendor()
{
    // 存储结果
    GPUVendor result = GPUVendor::UNKNOWN;
    std::wstring gpuName;

    // 初始化COM
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"COM初始化失败");
        return result;
    }

    // 初始化安全
    hr = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL);

    // 安全初始化失败可以继续
    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"COM安全初始化失败: 0x%08x", hr);
        CoUninitialize();
        return result;
    }

    // 创建WMI对象
    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(
        CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hr)) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"创建WMI定位器失败: 0x%08x", hr);
        CoUninitialize();
        return result;
    }

    // 连接到WMI
    IWbemServices* pSvc = NULL;
    hr = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (FAILED(hr)) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"连接WMI服务失败: 0x%08x", hr);
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    // 设置安全级别
    hr = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    if (FAILED(hr)) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"设置代理安全级别失败: 0x%08x", hr);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    // 创建查询
    IEnumWbemClassObject* pEnumerator = NULL;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hr)) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"执行WMI查询失败: 0x%08x", hr);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return result;
    }

    // 获取查询结果
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn) {
            break;
        }

        VARIANT vtName;
        VariantInit(&vtName);
        hr = pclsObj->Get(L"Name", 0, &vtName, 0, 0);

        if (SUCCEEDED(hr) && vtName.vt == VT_BSTR) {
            gpuName = vtName.bstrVal;
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"检测到显卡: %s", gpuName.c_str());

            // 转换为小写以便不区分大小写比较
            std::wstring lowerName = gpuName;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            // 判断显卡类型
            if (lowerName.find(L"nvidia") != std::wstring::npos) {
                result = GPUVendor::NVIDIA;
                DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到NVIDIA显卡");
            }
            else if (lowerName.find(L"amd") != std::wstring::npos ||
                lowerName.find(L"radeon") != std::wstring::npos ||
                lowerName.find(L"ati ") != std::wstring::npos) {
                result = GPUVendor::AMD;
                DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到AMD显卡");
            }
            else if (lowerName.find(L"intel") != std::wstring::npos) {
                result = GPUVendor::INTEL;
                DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到Intel显卡");
            }
        }

        VariantClear(&vtName);
        pclsObj->Release();
    }

    // 清理资源
    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    // 如果WMI方法失败，尝试DLL检测方法
    if (result == GPUVendor::UNKNOWN) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"通过DLL检测显卡类型");

        // 检查NVIDIA DLL
        HMODULE nvModule = LoadLibrary(L"nvapi64.dll");
        if (nvModule == NULL) {
            nvModule = LoadLibrary(L"nvapi.dll");
        }

        if (nvModule != NULL) {
            result = GPUVendor::NVIDIA;
            DEBUG_CONSOLE_STR(ConsoleHandle, L"通过DLL检测到NVIDIA显卡");
            FreeLibrary(nvModule);
        }
        else {
            // 检查AMD DLL
            HMODULE amdModule = LoadLibrary(L"atiadlxx.dll");
            if (amdModule == NULL) {
                amdModule = LoadLibrary(L"atiadlxy.dll");
            }

            if (amdModule != NULL) {
                result = GPUVendor::AMD;
                DEBUG_CONSOLE_STR(ConsoleHandle, L"通过DLL检测到AMD显卡");
                FreeLibrary(amdModule);
            }
            else {
                // 检查Intel DLL
                HMODULE intelModule = LoadLibrary(L"igdumd64.dll");
                if (intelModule == NULL) {
                    intelModule = LoadLibrary(L"igdumd32.dll");
                    if (intelModule == NULL) {
                        intelModule = LoadLibrary(L"igd10umd64.dll");
                        if (intelModule == NULL) {
                            intelModule = LoadLibrary(L"igd10umd32.dll");
                        }
                    }
                }

                if (intelModule != NULL) {
                    result = GPUVendor::INTEL;
                    DEBUG_CONSOLE_STR(ConsoleHandle, L"通过DLL检测到Intel显卡");
                    FreeLibrary(intelModule);
                }
            }
        }
    }

    return result;
}

// 获取支持的硬件编码器
VideoEncoderType DeviceManager::GetSupportedHWEncoder()
{
    GPUVendor vendor = GetGPUVendor();

    switch (vendor) {
    case GPUVendor::NVIDIA:
        // 检查NVENC库是否可用
    {
        HMODULE nvencModule = LoadLibrary(L"nvEncodeAPI64.dll");
        if (nvencModule == NULL) {
            nvencModule = LoadLibrary(L"nvEncodeAPI.dll");
        }

        if (nvencModule != NULL) {
            FreeLibrary(nvencModule);
            DEBUG_CONSOLE_STR(ConsoleHandle, L"支持NVIDIA NVENC编码器");
            return VideoEncoderType::NVIDIA_NVENC;
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"NVIDIA显卡但不支持NVENC，回退到软件编码");
            return VideoEncoderType::SOFTWARE;
        }
    }

    case GPUVendor::AMD:
        // 检查AMF库是否可用
    {
        HMODULE amfModule = LoadLibrary(L"amfrt64.dll");
        if (amfModule == NULL) {
            amfModule = LoadLibrary(L"amfrt32.dll");
        }

        if (amfModule != NULL) {
            FreeLibrary(amfModule);
            DEBUG_CONSOLE_STR(ConsoleHandle, L"支持AMD AMF编码器");
            return VideoEncoderType::AMD_AMF;
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"AMD显卡但不支持AMF，回退到软件编码");
            return VideoEncoderType::SOFTWARE;
        }
    }

    case GPUVendor::INTEL:
        // 检查QSV库是否可用
    {
        HMODULE qsvModule = LoadLibrary(L"libmfxhw64.dll");
        if (qsvModule == NULL) {
            qsvModule = LoadLibrary(L"libmfxhw32.dll");
        }

        if (qsvModule != NULL) {
            FreeLibrary(qsvModule);
            DEBUG_CONSOLE_STR(ConsoleHandle, L"支持Intel QSV编码器");
            return VideoEncoderType::INTEL_QSV;
        }
        else {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"Intel显卡但不支持QSV，回退到软件编码");
            return VideoEncoderType::SOFTWARE;
        }
    }

    default:
        DEBUG_CONSOLE_STR(ConsoleHandle, L"未知显卡类型，使用软件编码");
        return VideoEncoderType::SOFTWARE;
    }
}

// 获取GPU信息
std::string DeviceManager::GetGPUInfo()
{
    std::string info;
    GPUVendor vendor = GetGPUVendor();
    VideoEncoderType encoder = GetSupportedHWEncoder();

    // 供应商信息
    switch (vendor) {
    case GPUVendor::NVIDIA:
        info = "NVIDIA";
        break;
    case GPUVendor::AMD:
        info = "AMD";
        break;
    case GPUVendor::INTEL:
        info = "Intel";
        break;
    default:
        info = "Unknown";
        break;
    }

    // 编码器信息
    switch (encoder) {
    case VideoEncoderType::NVIDIA_NVENC:
        info += " (NVENC)";
        break;
    case VideoEncoderType::AMD_AMF:
        info += " (AMF/VCE)";
        break;
    case VideoEncoderType::INTEL_QSV:
        info += " (QSV)";
        break;
    case VideoEncoderType::NVIDIA_CUDA:
        info += " (CUDA)";
        break;
    case VideoEncoderType::SOFTWARE:
        info += " (Software)";
        break;
    }

    return info;
}


// UTF8转宽字符串
std::wstring UTF8ToWide(const std::string& utf8)
{
    if (utf8.empty()) return std::wstring();

    // 计算需要的宽字符数量
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (size <= 0) return std::wstring();

    // 执行转换
    std::vector<wchar_t> buffer(size);
    if (MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buffer.data(), size) == 0) {
        return std::wstring();
    }

    return std::wstring(buffer.data());
}

// 宽字符串转ANSI
std::string WideToAnsi(const std::wstring& wide)
{
    if (wide.empty()) return std::string();

    // 计算需要的多字节字符数量
    int size = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 0) return std::string();

    // 执行转换
    std::vector<char> buffer(size);
    if (WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, buffer.data(), size, NULL, NULL) == 0) {
        return std::string();
    }

    return std::string(buffer.data());
}

// ANSI转宽字符
std::wstring AnsiToWide(const std::string& ansi)
{
    if (ansi.empty()) return std::wstring();

    // 计算所需的宽字符数组大小
    int size = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
    if (size <= 0) return std::wstring();

    // 执行转换
    std::vector<wchar_t> buffer(size);
    MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, buffer.data(), size);

    return std::wstring(buffer.data());
}