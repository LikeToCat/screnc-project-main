#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>

// FFmpeg头文件包含
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/time.h>
#include <libavutil/buffer.h>
}

// 外部控制台句柄声明
extern HANDLE ConsoleHandle;

// Windows Core Audio捕获单例类
class CoreAudioCapture {
public:
    enum class AudioSampleRate {
        Hz_8000 = 8000,
        Hz_11025 = 11025,
        Hz_22050 = 22050,
        Hz_44100 = 44100,
        Hz_48000 = 48000
    };
    enum class AudioBitRate {
        Kbps_64 = 64000,
        Kbps_128 = 128000,
        Kbps_192 = 192000,
        Kbps_256 = 256000,
        Kbps_320 = 320000
    };
    enum class AudioFmt {
        MP4,
        AVI,
        FLV
    };

    // 静态方法：获取单例实例
    static CoreAudioCapture* GetInstance(
        AudioSampleRate sampleRate = AudioSampleRate::Hz_44100,
        AudioBitRate bitRate = AudioBitRate::Kbps_192,
        AudioFmt audioFmt = AudioFmt::MP4);

    // 静态方法：释放单例实例
    static void ReleaseInstance();

    // 公共方法
    bool Init();                                            // 初始化
    bool StartCapture();                                    // 开始录制
    bool CaptureFrame(void** frameData, int* frameSize);    // 获取音频帧
    bool StopCapture();                                     // 停止录制
    void SetVolume(float volume);                           // 设置音量
    float GetVolume() const;                                // 获取音量
    bool IsCapturing() const;                               // 是否正在捕获

private:
    // 单例模式：私有构造函数、析构函数和赋值运算符
    CoreAudioCapture(
        AudioSampleRate sampleRate = AudioSampleRate::Hz_44100,
        AudioBitRate bitRate = AudioBitRate::Kbps_192,
        AudioFmt audioFmt = AudioFmt::MP4
    );
    ~CoreAudioCapture();
    CoreAudioCapture(const CoreAudioCapture&) = delete;
    CoreAudioCapture& operator=(const CoreAudioCapture&) = delete;

    // 初始化相关方法
    bool InitCoreAudio();                // 初始化Core Audio
    bool InitAudioConvertCtx();          // 初始化音频格式转换器

    // 处理线程函数
    static DWORD WINAPI ProcessingThreadProc(LPVOID param);
    void ProcessingThread();

    // 队列操作函数
    bool EnqueueAudioData(const void* data, DWORD length);
    bool DequeueAudioData(uint8_t* buffer, size_t* size);

    // 辅助方法
    void LogWaveFormat(WAVEFORMATEX* pwfx);
    void CleanupCoreAudio();
    bool InitializeAudioClient();
    HRESULT GetDefaultAudioDevice(IMMDevice** ppDevice);

    // 安全释放COM对象
    template<class T>
    void SafeRelease(T** ppT) {
        if (*ppT) {
            (*ppT)->Release();
            *ppT = nullptr;
        }
    }

    // 预定义常量
    static constexpr size_t RING_BUFFER_SIZE = 1024 * 1024;  // 1MB环形缓冲区
    static constexpr size_t FRAME_SAMPLES = 1024;            // 每帧样本数 (MP4)
    static constexpr size_t AVI_FRAME_SAMPLES = 1012;        // AVI格式帧样本数
    static constexpr size_t MAX_QUEUE_SIZE = 100;            // 最大队列大小

private:
    // 单例实例指针和互斥锁
    static CoreAudioCapture* s_instance;
    static std::mutex s_instanceMutex;

    // Core Audio 相关
    IMMDeviceEnumerator* m_pDeviceEnumerator = nullptr;
    IMMDevice* m_pDevice = nullptr;
    IAudioClient* m_pAudioClient = nullptr;
    IAudioCaptureClient* m_pCaptureClient = nullptr;
    WAVEFORMATEX* m_pwfx = nullptr;
    bool m_coreAudioInitialized = false;

    // 音频处理
    SwrContext* m_pAudioConvertCtx = nullptr;             // 音频格式转换器
    AVFrame* m_pAudioFrame = nullptr;                     // 当前音频帧

    // 参数
    AudioSampleRate m_sampleRate;                         // 采样率
    AudioBitRate m_bitRate;                               // 比特率
    AudioFmt m_audioFmt;                                  // 音频格式
    int m_channels;                                       // 通道数
    float m_volume;                                       // 音量

    // 线程和同步
    HANDLE m_hProcessingThread;                           // 处理线程句柄
    std::atomic<bool> m_isCapturing;                      // 是否正在捕获
    std::atomic<bool> m_isInitialized;                    // 是否初始化成功
    int64_t m_frameCount;                                 // 帧计数

    // 环形缓冲区（避免频繁分配）
    alignas(64) uint8_t m_ringBuffer[RING_BUFFER_SIZE];   // 64字节对齐提高CPU缓存效率
    std::atomic<size_t> m_readPos;                        // 读取位置
    std::atomic<size_t> m_writePos;                       // 写入位置

    // 处理后的帧队列
    struct AudioFrameBuffer {
        AVFrame* frame;
        int size;
    };
    std::vector<AudioFrameBuffer> m_frameQueue;           // 帧队列
    std::mutex m_frameQueueMutex;                         // 帧队列互斥锁

    // 音频测试信号
    bool m_enableTestTone;                                // 启用测试音频
};