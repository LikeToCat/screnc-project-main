#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
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
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/buffer.h>
}
struct CameraOptions
{
	std::string deviceName;//设备名
	std::string deviceDesc;//设备描述
	std::string vcodec;//格式选择
	int fps;//帧率
	int pixelX;//分辨率
	int pixelY;//分辨率
};

//摄像头画面捕获接口
class CameraCapture
{
public:
	static CameraCapture* GetInstance();
	static void ReleaseInstance();
public:
	void StartCapture();
	void StopCapture();
	bool Init(CameraOptions cameraOptions);
	bool CaptureFrame(AVFrame* frame);
	void PauseCapture();
	void ResumeCapture();
	inline const CameraOptions& getOptionParam() { return m_OptionParam; }
	inline AVPixelFormat getVideoPixFmt() { return m_pDecode_CodecCtx->pix_fmt; }
private:
	bool InitInputDevice();
	void DecodeFrame();
private:
	//ffmpeg
	AVFormatContext* m_pInput_Ctx = NULL;//输入设备上下文
	AVCodecContext* m_pDecode_CodecCtx = NULL;//解码器上下文
	AVFrame* m_VideoFrame = NULL;//帧缓冲区
	int m_VideoStreamIndex = -1;//视频流下标

	//线程相关
	std::thread m_Decode_Thread;			//图像解码线程
	std::mutex m_FrameBuf_Mutex;			//帧缓冲区互斥锁
	std::condition_variable m_FrameBuf_CV;	//条件变量
	std::mutex m_Pausing_Mutex;				//暂停锁
	std::condition_variable m_Pasuing_CV;	//暂停条件变量
	std::atomic<bool> m_IsCapture = false;	//是否继续捕获
	std::atomic<bool> m_IsPausing = false;	//是否暂停捕获
	bool m_IsInitSuccess;					//是否成功初始化
	bool m_IsFrameAvailable;				//是否有可用帧
	bool m_HasFrameData;					//是否曾经有过帧数据

	CameraOptions m_OptionParam;//摄像头录制参数 

	//调试
	char m_Buf[256];

private:
	CameraCapture(CameraOptions cameraOptions);
	CameraCapture();
	~CameraCapture();
	static CameraCapture* instance;
};

