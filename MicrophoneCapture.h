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
}
extern HANDLE ConsoleHandle;
//麦克风录制类
enum class MircroSampleRate
{
	Hz_8000 = 8000,
	Hz_11025 = 11025,
	Hz_22050 = 22050,
	Hz_44100 = 44100,
	Hz_48000 = 48000
};
enum class  MircroBitRate {
	Kbps_64 = 64000,
	Kbps_128 = 128000,
	Kbps_192 = 192000,
	Kbps_256 = 256000,
	Kbps_320 = 320000
};
enum class MircroFmt {
	MP4,
	AVI,
	FLV
};

class MicrophoneCapture
{
public:
	MicrophoneCapture(
		MircroSampleRate sampleRate = MircroSampleRate::Hz_44100,
		MircroBitRate bitRate = MircroBitRate::Kbps_128,
		MircroFmt microFmt = MircroFmt::MP4);
	~MicrophoneCapture();
	bool Init(std::wstring MicroDevice);//初始化
	bool StartCapture();//开始捕获麦克风数据
	bool StopCapture();//停止捕获麦克风数据
	bool CaptureMicroFrame(void** microFrame, int* captureSize);//获取一帧捕获的麦克风数据
private:
	//初始化
	bool InitMicroCapture(std::wstring MicroDevice);//初始化麦克风捕获
	bool InitMicroResample();//初始化采样率转换器
	bool InitMicroResampleInner();//初始化格式转换器

	//外部接口转接
	bool QueryFrame(void** microFrame, int* captureSize);//获取一帧捕获的麦克风数据

	//线程函数
	void DecodeMicroFrameThread();//麦克风音频解码线程
	void MicroSampleRateThread();//麦克风音频采样率转换器线程
	void MircroFormatThread();//麦克风音频格式转换器线程
private:
	//捕获的麦克风设备
	std::wstring m_wstr_MicroDevice = L"";

	//FFmpeg
	AVFormatContext* m_pMicroDecode_Ctx = nullptr;//设备上下文
	AVCodecContext* m_pMicroDecode_CodecCtx = nullptr;//麦克风解码器
	AVFrame* m_pMicro_Frame = nullptr;//最终重采样后的麦克风数据缓冲区
	SwrContext* m_pMicroSampleRate_SwrCtx = nullptr;//采样率转换器上下文
	SwrContext* m_pMicroFormat_SwrCtx = nullptr;//格式转换器上下文
	AVAudioFifo* m_pBuffer1_Fifo = nullptr;//存储从捕获的原始麦克风数据
	AVAudioFifo* m_pBuffer2_Fifo = nullptr;//存储采样率转换后音频数据缓冲区
	AVAudioFifo* m_pBuffer3_Fifo = nullptr;//存储格式转换后的最终重采样的音频数据缓冲区

	//麦克风录制重采样音频数据预设
	MircroSampleRate m_sampleRate;
	MircroBitRate m_birRate;
	MircroFmt m_microFmt;

	//线程同步
	std::mutex m_Buffer1_Mutex;
	std::mutex m_Buffer2_Mutex;
	std::mutex m_Buffer3_Mutex;

	//线程句柄
	std::thread m_MircroDecode_Thread;//麦克风数据解码线程
	std::thread m_MircroResample1_Thread;//采样率转换线程
	std::thread m_MircroResample2_Thread;//格式转换线程

	//线程状态
	std::atomic<bool> m_IsRecording = false;
	bool m_isInitSuccess = false;

	//错误处理
	char* m_errBuf = nullptr;
	int m_errNum = 0;
};