#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <windows.h>
#include <chrono>
#include "DeviceManager.h"

// FFmpeg头文件包含
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/time.h>
#include <libavutil/buffer.h>
}

// 外部控制台句柄声明
extern HANDLE ConsoleHandle;
// 常用采样率枚举
enum class AudioSampleRate {
    Hz_8000 = 8000,
    Hz_11025 = 11025,
    Hz_22050 = 22050,
    Hz_44100 = 44100,
    Hz_48000 = 48000
};

// 常用音频比特率枚举(单位为kbps)
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
// 音频录制类
class AudioCapture {
public:
    // 构造函数
    AudioCapture(
        AudioSampleRate sampleRate = AudioSampleRate::Hz_44100,
        AudioBitRate bitRate = AudioBitRate::Kbps_192,
        AudioFmt audioFmt = AudioFmt::MP4
    );
    ~AudioCapture();
    bool Init();//初始化
    bool StartCapture();//开始录制 
    bool CaptureFrame(void** frameData, int* frameSize);//存储重采样后的音频数据的缓冲区
    bool StopCapture();//停止录制
private:
    //初始化
    bool InitAudioCapture();//初始化音频捕获器
    bool InitAudioInnerResampleCtx();//初始化音频采样率转换器
    bool InitAudioConvertCtx();//初始化音频格式转换器 

    //线程函数
    static DWORD WINAPI AudioInnerCaptureProc(LPVOID);//音频捕获线程函数
    static DWORD WINAPI AudioInnerResampleProc(LPVOID);//音频重采样线程函数
    void AudioInnerCaptureThread();//音频捕获线程函数
    void AudioResampleThread();//音频重采样线程函数

    //外部接口转接
    bool QueryFrame(void** frameData, int* captureSize);//拿取一帧音频数据
private:
    //ffmpeg
    AVFormatContext* m_pFormatCtx_AudioInner;//编码器上下文
    AVCodecContext* m_pReadCodecCtx_AudioInner;//ffmpeg上下文
    SwrContext* m_pAudioInnerResampleCtx;//音频采样率转换器
    SwrContext* m_pAudioConvertCtx;//音频格式转换器 
    AVAudioFifo* m_pAudioInnerFifo;//存储原始音频数据的缓冲区
    AVAudioFifo* m_pAudioInnerResampleFifo;//存储重采样后的音频数据的缓冲区
    AVFrame* m_pAudioFrame;//存储重采样后返回给的每一帧音频数据

    //参数
    AudioSampleRate m_sampleRate;//采样率
    AudioBitRate m_bitRate;//比特率
    int m_dst_nb_samples;//重采样后每一帧的样本数
    int64_t m_frameCount;  // 用于设置音频帧的pts
    AudioFmt m_audioFmt; //音频格式

    //线程句柄
    HANDLE m_hAudioInnerCapture;//音频捕获线程
    HANDLE m_hAudioInnerResample;//音频重采样线程
    std::atomic<bool> m_IsRecord;//是否正在捕获音频
    bool m_InitSuccess;//初始化是否成功

    //线程同步
    std::mutex m_csAudioInnerMutex;
};