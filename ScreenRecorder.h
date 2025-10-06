#pragma once
#ifdef TARGET_WIN10
#include "DXGICapture.h"
#elif defined TARGET_WIN7
#include "GDICapture.h"
#endif
#include "RecordingStats.h"
#include "MicrophoneCapture.h"
#include "HandleCapture.h"
#include "WasapiCapture.h"
//#include "AudioCapture.h"
#include "CDebug.h"
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <queue> 
#include <memory>
#include <atomic>
#include <map>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/buffer.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

/// <summary>
/// 硬件编码器随着用户电脑驱动的版本发生变化，再开始编码的过程中可能导致无法进行编码
/// 此类用于检测这种情况的发生，以决定是否向硬件编码器到软件编码器的回退
/// </summary>
class EncoderFailureHandler
{
private:
	int m_consecutiveFailures = 0;		  // 当前失败编码的次数
	const int FAILURE_THRESHOLD = 5;	  // 连续失败超过此阈值才回退
	std::atomic<bool> m_isHasFallback = false;	  // 当前是否需要进行回退 
	std::string m_lastErrorMessage = "";  // 获取最近一次的错误信息
public:
	// 检查是否需要回退到软件编码器
	bool ShouldFallbackToSoftware(int errorCode, const char* errorMsg)
	{
		// 清除错误消息后缀中可能的换行符
		std::string cleanErrorMsg = errorMsg ? errorMsg : "";
		if (!cleanErrorMsg.empty())
		{
			size_t pos = cleanErrorMsg.find('\n');
			if (pos != std::string::npos)
			{
				cleanErrorMsg = cleanErrorMsg.substr(0, pos);
			}
		}

		// 立即触发回退的严重错误
		if (cleanErrorMsg.find("Function not implemented") != std::string::npos ||
			cleanErrorMsg.find("map frame to surface failed") != std::string::npos ||
			cleanErrorMsg.find("Error submitting the frame for encoding") != std::string::npos ||
			cleanErrorMsg.find("hardware accelerator failed") != std::string::npos)
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误检测器]:发生了一次视频编码器本身的严重错误!准备暂停编码以回退至软件编码器");
			return true;
		}

		// 连续失败计数
		if (errorCode < 0 && errorCode != AVERROR(EAGAIN))
		{
			DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误检测器]:发生了一次视频编码失败的错误!");
			m_consecutiveFailures++;
			m_lastErrorMessage = cleanErrorMsg;
			if (m_consecutiveFailures >= FAILURE_THRESHOLD)
			{
				DEBUG_CONSOLE_STR(ConsoleHandle, L"[错误检测器]:视频编码失败错误次数计数达到临界值，准备暂停编码以回退至软件编码器");
				return true;
			}
		}
		return false;
	}
	bool IsFallbackActive() const { return m_isHasFallback; }	        // 获取当前回退状态
	void SetFallbackActive(bool active) { m_isHasFallback = active; }	// 设置回退状态
	const std::string& GetLastError() const { return m_lastErrorMessage; }  // 获取上次错误信息
};


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

struct ResolutionRatioParam
{
	int width;
	int height;
};
class ScreenRecorder
{
public:
	static ScreenRecorder* GetInstance();
	static void ReleaseInstance();
	static bool IsRecording() { return m_InsIsRecording; };
	static void Preheat();	//预热整个录制底层
public:
	// 音频录制模式枚举
	enum RecordMode {
		None,           // 不录制音频
		SystemSound,    // 仅录制系统声音
		Microphone,     // 仅录制麦克风
		Both,           // 同时录制系统声音和麦克风
	};
	
	enum VideoQuality
	{
		Origin = 10,//原画
		SuperDefinition = 8,//超清
		HighDefinition = 7,//高清
		StandardDefinition = 2//标清
	};
	enum ResolutionRatio
	{
		Rs_SameAsScreen,//和屏幕一样
		Rs_360P = 130000,
		Rs_480P = 140000,
		Rs_720P = 150000,
		Rs_1080P = 300000,
		Rs_2K = 480000,
		Rs_4k = 810000
	};
	enum VideoFormat
	{
		MP4,
		AVI,
		FLV
	};
	enum EncodingPreset
	{
		Slow,//视频品质优先
		Medium,//平衡
		Fast,//视频帧率优先
	};
	struct VideoPacket {
		AVPacket pkt;
		int64_t pts;
		int64_t dts;
	};
	struct AudioPacket {
		AVPacket pkt;
		int64_t pts;
		int64_t dts;
	};

	//回调函数类型
	enum RecordCallBack
	{
		WindowRecord_WindowMinimalAndClose//应用窗口录制时，窗口最小化，或者窗口关闭时回调函数类型
	};
private:
	static ScreenRecorder* instance;
	static bool m_InsIsRecording;
	ScreenRecorder();
	~ScreenRecorder();
public://外部调用
	//应用窗口录制
	void SetWindowRecordParam(HWND hWnd,
		ResolutionRatio resolutionRatio = Rs_SameAsScreen,
		VideoQuality videoQuality = StandardDefinition,
		VideoFormat videoFmt = MP4,
		EncodingPreset EncodingPreset = EncodingPreset::Medium,
		RecordMode audioCaptureMode = RecordMode::SystemSound,
		AudioSampleRate audioSampleRate = AudioSampleRate::Hz_44100,
		AudioBitRate audioBitrate = AudioBitRate::Kbps_128,
		int frameRate = 24);

	//选区录制
	void SetAreaRecordParam(int left, int top, int right, int bottom,
		ResolutionRatio resolutionRatio = Rs_SameAsScreen,
		VideoQuality videoQuality = StandardDefinition,
		VideoFormat videoFmt = MP4,
		EncodingPreset EncodingPreset = EncodingPreset::Medium,
		RecordMode audioCaptureMode = RecordMode::SystemSound,
		AudioSampleRate audioSampleRate = AudioSampleRate::Hz_44100,
		AudioBitRate audioBitrate = AudioBitRate::Kbps_128,
		int frameRate = 24);

	//全屏录制
	void SetScreenRecordParam(ResolutionRatio resolutionRatio = Rs_SameAsScreen,
		VideoQuality videoQuality = StandardDefinition,
		VideoFormat videoFmt = MP4,
		EncodingPreset EncodingPreset = EncodingPreset::Medium,
		RecordMode audioCaptureMode = RecordMode::SystemSound,
		AudioSampleRate audioSampleRate = AudioSampleRate::Hz_44100,
		AudioBitRate audioBitrate = AudioBitRate::Kbps_128,
		int frameRate = 24);

	//录制参数
	void SetSystemAudioVolume(float Volume);//设置系统音量值(0-1)
	void SetMicroVolume(float Volume);//设置麦克风音量值(0-1)
	void SetVideoEncoder(const std::string& encoderName) {
		m_VideoEncoder = encoderName;

		// 打印编码器信息
		if (!m_VideoEncoder.empty()) {
			DEBUG_CONSOLE_FMT(ConsoleHandle, L"设置视频编码器: %hs", m_VideoEncoder.c_str());
		}
		else {
			DEBUG_CONSOLE_STR(ConsoleHandle, L"使用默认软件编码器");
		}
	}
	void SetVideoFps(int fps);
	void SetVideoResolution(int width, int height);
	void SetVideoQuality(bool isQualityPriority);
	void SetAudioSampleRate(AudioSampleRate audioSampleRate);
	void SetAudioBitrate(AudioBitRate audioBitrate);
	void SetAudioCaptureDevice(CString deviceName);
	void SetMicroDeviceName(CString deviceName);
	void SetOnlyAudioRecord(bool IsOnlyAudioRecord);//是否只录制声音
	void SetRecordMouse(bool IsRecordMouse);
	void SetVideoTextFilter(
		std::wstring textInfo,
		std::string  textSize,
		std::string  fontColor,
		std::wstring logoPathUtf8);
	void PauseRecording();
	void ResumeRecording();
	void SetRecordSizeCallback(std::function<void(double)> cb);
	void SetRecordSizeCallbackInterval(int intervalMs);

	//控制录制关键方法
	bool startRecording(const char* outputfilePath);//开始录制
	bool stopRecording();							//停止录制
	inline bool isRecording()const { return m_IsRecording; };	//是否正在录制
	inline int GetFrameRate()const { return m_frameRate; };		//获取当前帧率
	inline bool isPausing() { return m_isPause; }				//获取暂停状态
	void UpdateRecordArea(const CRect& RecordRect);				//更新录制区域
	void SetRecordCallBack(RecordCallBack CallBackType, std::function<void()> funcCallBack);

private://内部封装
	bool InitFFmpeg(const char* outputfilePath);	//初始化ffmpeg
	bool InitTextImageFilter(const std::string& logoPathUtf8);//初始化视频水印
	void CleanUpFFmpeg();							//清理ffmpeg
	void CleanCaptureInterface();					//清理各个捕获器
	bool TestEncoderAvailability(const char* encoderName);						//测试编码器是否可行
	void SetEncodeParam(EncodingPreset encodePreset);	//设置硬件编码参数

	//视频参数计算 
	ResolutionRatioParam GetRsParamByEnum(ResolutionRatio resolutionRatio);					//根据枚举构造分辨率参数
	double GetVideoQualityMbps(ResolutionRatio resolutionRatio, VideoQuality videoQuality); //根据枚举构造视频码率参数
	double GetVideoQualityMbps(ResolutionRatioParam rsParam, VideoQuality videoQuality);
	double GetOriginQualityByFullScreen();//根据当前用户设置的桌面分辨率获取原画下的标准mbps

	//音视频处理
	void VideoCaptureThreadFunc();         // 视频捕获线程函数
	void AudioProcessThreadFunc();         // 音频处理线程函数
	void FlushEncoder(AVCodecContext* enc_ctx, int stream_index);//刷新编码器

	//音视频同步
	void MixingThreadFunc();               // 音视频混合同步写文件线程函数
	int64_t GetCurrentPtsUsec();

	//音视频频编码
#ifdef TARGET_WIN10
	void DXGIVideoFrameEncode();//编码DXGI捕获的视频帧(全屏录制)
#elif defined TARGET_WIN7
	void GDIScreenVideoFrameEncode();//编码GDI捕获的视频帧(全屏录制)
#endif 
	void GdiVideoFrameEncode(); //编码gdi捕获的视频帧(窗口录制)
	void AudioFrameEncode();    //编码音频帧
	void MicroFrameEncode();    //编码麦克风音频帧
	void AudioMicroMixEncode(); //混合编码音频帧和麦克风音频帧

	//混合编码算法
	void MixMicroAudioS16(void** dstFrameData, void** srcAudioData, void** srcMicroData); //S16格式混合编码算法
	void MixMicroAudioFLTP(void** dstFrameData, void** srcAudioData, void** srcMicroData);//FLTP格式混合编码算法

	//录制重启到软件编码器（当硬件编码器在编码过程中发生错误时）
	bool RestartRecordThread();	//重新初始化ffmpeg到软件编码器(线程函数)
	bool StopRecordToRestart(); //停止当前所有的录制并重启动(重启线程函数调用)

	//是否为静音数据
	inline bool IsAudioFrameSilent(const AVFrame* frame)
	{
		if (!frame || !frame->data[0])
			return true;

		static const uint64_t zero64 = 0;

		if (av_sample_fmt_is_planar((AVSampleFormat)frame->format))
		{
			if (frame->data[0] && *(uint64_t*)frame->data[0] != zero64)
				return false;
		}
		else
		{
			if (*(uint64_t*)frame->data[0] != zero64)
				return false;
		}
		return true;
	}
	//函数调试
	void MyTest();
private:
	//帧捕获对象
#ifdef TARGET_WIN10
	DXGICapture* m_dxgiCapture = nullptr;		//DXGI桌面捕获(对象)(高帧率屏幕录制核心(win8以上))
#elif defined TARGET_WIN7
	GDICapture* m_ScreenCapture = nullptr;      //GDI桌面捕获(接口)(屏幕录制核心)
#endif
	WasapiCapture* m_audioCapture = nullptr;    //WASAPI音频捕获(接口)(系统音频捕获核心)
	MicrophoneCapture* m_microCapture = nullptr;//FFmpeg麦克风音频捕获器(对象)(麦克风捕获核心)
	HandleCapture* m_gdiCapture = nullptr;      //GDI应用程序窗口句柄捕获(对象)(句柄录制捕获核心)
	//AudioCapture* m_audioCapture = nullptr;   //FFmpeg系统音频捕获(对象)(现已弃用,如果用户电脑有虚拟音频捕获驱动程序,可启用)

	//预设录制参数选项
	bool m_isInit;                        //是否已经初始化 
	int m_width;                          //屏幕尺寸 
	int m_height;                         //屏幕尺寸 
	int m_frameRate;                      //帧率
	double m_bitRate;                     //比特率
	int m_offsetx;                        //屏幕录制左上角x
	int m_offsety;                        //屏幕录制右上角y
	float m_MicroVolume = 0.45f;          //麦克风音量值(0-1)
	float m_SystemAudioVolume = 0.75f;    //系统音频音量值(0-1)
	float m_Left = 0.0f;	              //录制区域left
	float m_Top = 0.0f;		              //录制区域top
	float m_Right = 0.0f;	              //录制区域right
	float m_Bottom = 0.0f;	              //录制区域botom
	HWND m_Hwnd;                          //录制的某个应用程序的句柄
	VideoFormat m_VideoFmt;               //录制格式
	ResolutionRatio m_Rs;                 //分辨率
	VideoQuality m_VideoQulity;           //视频质量
	EncodingPreset m_EncodingPreset;      //编码质量(速度)预设
	AudioSampleRate m_AudioSampleRate;    //音频采样率
	AudioBitRate m_AudioBitRate;          //音频比特率
	RecordMode m_RecordMode;              //录制模式
	std::string m_VideoEncoder = "";      //编码器名称
	bool m_IsQualityPriority = false;     //画质优先
	std::wstring m_wstr_Audio = L"";	  //音频设备
	std::wstring m_wstr_MicroDevice = L"";//麦克风设备
	std::string m_str_outputfile = "";	  //输出文件路径
	bool m_IsRecordMouse = false;		  //是否录制到鼠标

	//录制标志
	bool m_Bool_OnlyAudioRecord = false;
	bool m_Bool_AreaRecord = false;
	bool m_Bool_ScreenRecord = false;
	bool m_Bool_WindowRecord = false;

	//ffmpeg
	AVFormatContext* m_formatCtx = nullptr;
	AVCodecContext* m_CodecCtx = nullptr;
	AVCodecContext* m_pCodecCtx_Audio = nullptr;
	AVStream* m_VideosStream = nullptr;
	AVStream* m_AudioStream = nullptr;
	AVFrame* m_frame = nullptr;
	SwsContext* m_swsContext = nullptr;
	int m_frameSize;

	// 文字水印相关
	AVFilterGraph* m_AVFGraph_Text = nullptr;
	AVFilterContext* m_AVFCtx_TextIn = nullptr;
	AVFilterContext* m_AVFCtx_TextOut = nullptr;
	std::atomic<bool>  m_bool_IsTextFilterActive{ false };
	std::string   m_str_fontColor;
	std::wstring  m_str_textInfo;
	std::string   m_str_textSize;
	std::string   m_str_textX;
	std::string   m_str_textY;
	std::string   m_str_pathUtf8;

	//Debug所用对象
	RecordingStats m_stats;//录制统计信息
	char* errorBuf = nullptr;//错误信息缓冲区
	EncoderFailureHandler m_EncoderFailureHandler;

	//媒体流下标
	int m_VideoStreamIndex;//视频流下标
	int m_AudioStreamIndex;//音频流下标 

	//录制重启到软件编码器相关
	std::thread m_Thread_RestartRecord;
	std::atomic<bool> m_Bool_RestartRecord = false;
	std::mutex m_Mutex_RestartRecord;
	std::condition_variable m_CV_RestartRecord;
	bool m_Bool_IsHasRestartSince = false;	//之前是否重启过

	//外部回调设置相关
	std::map<RecordCallBack, std::function<void()>> m_Map_RecordCallBackFunc;

	//音视频同步
	std::queue<VideoPacket> m_videoQueue;   // 视频帧队列
	std::queue<AudioPacket> m_audioQueue;   // 音频帧队列
	int64_t m_baseTime;					    // 录制开始的时间点
	int64_t m_lastAudioPts;				    // 最后一个音频帧的PTS
	int64_t m_lastVideoPts;				    // 最后一个视频帧的PTS
	int64_t m_frameCount;				    // 当前帧计数

	// 录制大小回调相关
	std::function<void(double)> m_RecordSizeCallback;   // 录制大小回调
	std::atomic<int64_t> m_bytesWritten{ 0 };           // 已写入字节统计
	int m_sizeCallbackIntervalMs = 1000;                // 回调最小间隔(毫秒)
	std::chrono::steady_clock::time_point m_lastSizeCallbackTime; // 上次回调时间


	// 线程同步
	std::mutex m_videoMutex;              // 视频缓冲区互斥锁
	std::mutex m_audioMutex;              // 音频缓冲区互斥锁
	std::mutex m_UpdateRecordAreaMutex;	  // 更新录制区域时互斥锁

	//线程状态
	std::thread m_recordThread;			  //录制线程
	std::atomic<bool> m_IsRecording;	  //是否正在录制

	// 线程
	std::thread m_videoCapureThread;       // 视频捕获线程
	std::thread m_audioProcessThread;      // 音频处理线程
	std::thread m_mixingThread;            // 混合写文件线程

	//初始化状态
	bool m_bool_isAudioInitSuccess = false;
	bool m_bool_isMicroInitSuccess = false;
	bool m_bool_isScreenCaptureSuccess = false;
	bool m_bool_isGdiCaptureSuccess = false;

	//暂停相关
	int64_t m_pauseBeginUsec = 0;
	std::atomic<bool> m_isPause;			//是否暂停
	std::condition_variable m_cvPause;		//暂停的条件变量
	std::mutex m_muxPause;					//暂停的条件变量锁

	//预热状态与中止标记
	static std::atomic<bool> s_isPreheating;				// 是否正在预热
	static std::atomic<bool> s_abortPreheat;				// 是否请求中止预热
	static std::atomic<bool> s_isInsCreateByPreheat;		// 是否是预热自己在创建实例
};

struct RecordingParams
{
	std::wstring codecText;
	int fps;

	ScreenRecorder::VideoFormat     videoFormat;
	ScreenRecorder::VideoQuality    videoQuality;
	AudioSampleRate                 audioSampleRate;
	AudioBitRate                    audioBitRate;
	float                           microAudioVolume;
	float                           systemAudioVolume;
	ScreenRecorder::RecordMode     recordMode;
	ScreenRecorder::ResolutionRatio videoResolution;
	ScreenRecorder::EncodingPreset  encodePreset;

	std::wstring savePath;
	std::wstring fileFormat;
	std::wstring audioDevice;
	std::wstring microDevice;


	std::wstring outputFilePath;
	std::wstring logoPath;

	bool RecordMouse;
};