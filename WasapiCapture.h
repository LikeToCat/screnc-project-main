#pragma once
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <condition_variable>
// FFmpeg头文件包含
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/time.h>
}

// 外部控制台句柄声明
extern HANDLE ConsoleHandle;
struct CaptureDeviceInfo {
    std::wstring deviceId;     // 设备ID
    std::wstring deviceName;   // 设备名称
    std::wstring deviceDesc;   // 设备描述
    bool isDefault;            // 是否为默认设备
};

// 系统音频捕获类 - 使用WASAPI，与AudioCapture接口兼容
class WasapiCapture {
public:
    // 最终音频数据块参数
    enum class AudioSampleRate {
        Hz_8000 = 8000,
        Hz_11025 = 11025,
        Hz_22050 = 22050,
        Hz_44100 = 44100,
        Hz_48000 = 48000
    };
    enum class AudioFmt {
        MP4,
        AVI,
        FLV
    };

    // 原始捕获帧数据参数
    enum class OriAudioFmt
    {
        AV_SAMPLE_FMT_S16 = 1,
        AV_SAMPLE_FMT_S32 = 2,
        AV_SAMPLE_FMT_FLT = 3
    };

    // 单例模式
    static WasapiCapture* GetInstance();
    static void ReleaseInstance();

    // 主要接口函数
    bool Init(AudioSampleRate sampleRate = AudioSampleRate::Hz_44100,
        AudioFmt audioFmt = AudioFmt::MP4, const std::wstring& deviceName = L"");

    // 捕获接口
    bool StartCapture();
    bool StopCapture();

    //外部调用获取重采样后的编码帧
    bool CaptureFrame(void** frameData, int* frameSize) {
        return QueryFrame(frameData, frameSize);
    }

    //枚举音频捕获设备接口
    bool getCaptureDevicesInfo(_Out_ std::vector<CaptureDeviceInfo>* vecDevicesInfo);
private:
    //辅助函数
    bool SetCaptureCParam(WAVEFORMATEX* pwfx);//设置捕获帧参数

    // 初始化相关函数
    bool InitWASAPI(const std::wstring& deviceId = L"");
    bool InitSampleRateConvertCtx();
    bool InitFormatConvertCtx();

    //帧拉取
    bool QueryFrame(void** frameData, int* frameSize);

    // 线程函数
    void Capture();
    void SampleRateConvert();

    // 音频流重置函数
    void ResetAudioPipeline();
    void CompleteAudioReset();

private:
    //ffmpeg
    AVAudioFifo* m_AudioFifo_fifo1 = nullptr; //存储WASAPI捕获的音频数据缓冲区
    AVAudioFifo* m_AudioFifo_fifo2 = nullptr; //存储采样率转换后的音频数据缓冲区
    SwrContext* m_SwrCtx_SampleRate = nullptr;//音频采样率转换器
    SwrContext* m_SwrCtx_Format = nullptr;    //音频格式转换器 
    AVFrame* m_AVFrame_buffer = nullptr;      //存储最终编码帧的缓冲区

    //线程相关
    std::thread m_Thread_Capture;           //wasapi捕获线程
    std::thread m_Thread_SampleRateConvert; //原始帧采样率转换线程
    std::mutex m_Mutex_fifo1;               //存储采样率转换后的音频数据缓冲区的互斥锁
    std::mutex m_Mutex_fifo2;               //存储最终编码帧的缓冲区互斥锁
    std::atomic<bool> m_bool_IsCapturing = false;   //是否正在捕获
    std::mutex m_Mutex_Frame;           //音频数据库互斥锁

    //编码音频参数
    AudioSampleRate m_Enum_SampleRate;      //编码帧采样率
    AudioFmt m_Enum_format;                 //编码帧格式

    //捕获音频参数
    int64_t m_Int_RawSampleRate;            //原始捕获帧采样率
    uint16_t m_Int_RawChannels;             //原始捕获帧通道数
    uint8_t m_Int_bytePerSample;            //单位样本所占字节数
    OriAudioFmt m_Enum_oriFormat;           //原始帧格式

    //wasapi
    IAudioClient* m_Interface_IAudioClient = NULL;               //WASAPI的核心接口
    IAudioCaptureClient* m_Interface_IAudioCaptureClient = NULL; //音频捕获的接口
    IMMDevice* m_Interface_IMMDevice = NULL;                     //特定音频设备接口
    IMMDeviceEnumerator* m_Interface_IMMDeviceEnumerator = NULL; //设备枚举器
    WAVEFORMATEX* m_WaveFormatEx_CaptureFmt = NULL;              //捕获音频格式结构体
    std::atomic<bool> m_bool_WasapiInitialized = false;          //WASAPI是否初始化成功

    //音频数据就绪事件
    HANDLE m_hAudioEvent;

    // 音频流不连续性处理
    std::atomic<bool> m_bool_NeedReset = false;      // 是否需要重置音频处理管线
    std::atomic<bool> m_bool_IsInResetState = false; // 当前音频捕获是否处于重置状态
    std::mutex m_Mutex_Reset;                        // 重置过程的互斥锁
    std::condition_variable m_CV_ResetComplete;      // 重置完成的条件变量

    //设备成员
    std::vector<CaptureDeviceInfo> m_Vec_AudioDevices;  // 存储可用音频设备
    std::wstring m_Str_SelectedDeviceId;                // 当前选择的设备ID
private:
    // 私有构造、析构函数
    WasapiCapture();
    ~WasapiCapture();

    // 禁止拷贝和赋值
    WasapiCapture(const WasapiCapture&) = delete;
    WasapiCapture& operator=(const WasapiCapture&) = delete;
    void CleanUp();//资源释放

    static WasapiCapture* m_Interfance_Ins;//接口实例
};