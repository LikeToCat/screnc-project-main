#pragma once
////---------------视频播放器接口类
#include <SDL.h>
#include <SDL_syswm.h>     // 系统窗口信息头文件
#include <string>
#include <map>
#include <memory>
#include <atomic>
#include <thread>
#include <queue>
#include "CameraCapture.h"      // 摄像头捕获帧接口
#include "MicrophoneCapture.h"  // 麦克风捕获帧接口
#include "CDebug.h"

// SDL帧渲染窗口(用于播放各种多媒体视频，目前这个类主要被我实现为专门渲染摄像头的捕获帧)
class Ui_CameraDisplaySDL 
{
public:
    static Ui_CameraDisplaySDL* GetInstance();
    static void ReleaseInstance();

    // 初始化
    bool Initialize(const CRect& recordArea,
        std::string deviceName,
        std::string desc,
        const int dstframeRate = 30,
        std::string srcfmt = "yuyv422",
        int32_t srcwidth = 640,
        int32_t srcheight = 480,
        int32_t SampleRate = 44100,
        int32_t BirRate = 128,
        std::string videoEncoder = ""
    ); // 初始化SDL窗口和渲染器，设置渲染区域

    //录制模式参数相关
    enum RecordMode
    {
        VideoOnly,
        Both
    };
    //音视频同步相关
    struct VideoPacket 
    {
        AVPacket pkt;
        int64_t pts;
        int64_t dts;
    };
    struct AudioPacket
    {
        AVPacket pkt;
        int64_t pts;
        int64_t dts;
    };

    //线程相关
    struct InterFaceState//接口状态结构体
    {
        std::atomic<bool> m_Bool_UiWindowRunning = false;    // 窗口是否正在运行
        std::atomic<bool> m_Bool_Renderering = false;        // 停止帧渲染线程
        std::atomic<bool> m_Bool_SDLInitialized = false;     // SDL是否已初始化
        std::atomic<bool> m_Bool_IsPauseRendering = false;   // 是否暂停帧渲染线程
        std::atomic<bool> m_Bool_IsRecording = false;        // 是否正在录制
        std::atomic<bool> m_Bool_IsEncodeFrameAvailable = false; // 编码帧是否可用
    };
    struct InterFaceThread//接口所有线程
    {
        std::thread m_Thread_RenderFrame;   //帧渲染线程
        std::thread m_Thread_UiWindow;      //窗口Ui事件循环
        std::thread m_Thread_Recording;     //录制线程
        std::thread m_Thread_AudioRecording;//音频录制线程
    };
    struct InterFaceThreadProtect//线程保护变量
    {
        std::condition_variable m_CV_WaitUiInital;           // 条件变量（等待SDL窗口创建完成）
        std::condition_variable m_CV_RenderPasuing;          // 条件变量（是否暂停帧渲染）
        std::condition_variable m_CV_RenderingStarted;       // 条件变量（渲染线程已开始）
        std::condition_variable m_CV_IsEncodeFrameAvailable; // 条件变量（编码帧是否可用）
        std::mutex m_Mutex_WaitUiInital;                     // 条件变量锁(等待SDL窗口创建完成)
        std::mutex m_Mutex_RenderPausing;                    // 条件变量锁（是否暂停帧渲染）
        std::mutex m_Mutex_RenderingStarted;                 // 条件变量锁（渲染线程已开始）
        std::mutex m_Mutex_IsEncodeFrameAvailable;           // 条件变量锁（编码帧是否可用）
    };
    struct RecordingContext // 录制上下文
    {
        std::string outputFilePath;
        RecordMode mode;
        AVFormatContext* formatContext = nullptr;
        AVCodecContext* videoCodecContext = nullptr;
        AVCodecContext* audioCodecContext = nullptr;
        int videoStreamIndex = -1;
        int audioStreamIndex = -1;
        std::queue<VideoPacket> videoPacketQueue;
        std::queue<AudioPacket> audioPacketQueue;
        std::mutex videoQueueMutex;
        std::mutex audioQueueMutex;
        std::mutex formatMutex;                              
        std::condition_variable packetQueueCV;
        std::atomic<bool> isActive = false;
    } ;

    //窗口控制
    void RunSDLWindow();      // 运行SDL窗口
    void HideSDLWindow();     // 隐藏SDL窗口
    void ShowSDLWindow();     // 从隐藏状态SDL窗口解除
    void MinimalSDLWindow();  // 最小化SDL窗口
    void RestoreSDLWindow();  // 从最小化状态中恢复
    void UpdateSDLWindowPos(const CRect& newPosition);// 更新SDL窗口位置
    void CloseWindow();       // 关闭SDL并清理窗口
    void ResumeRendering();   // 暂停摄像头渲染
    void PauseRendering();    // 恢复摄像头渲染

    //录制控制
    bool StartRecording(const std::string& outputFilePath, RecordMode mode = RecordMode::Both);
    void StopRecording();
    bool IsRecording() const { return m_InterFaceState_State.m_Bool_IsRecording.load(); }

    // 硬件编码相关方法
    void SetVideoEncoder(const std::string& encoderName)
    {
        m_string_videoEncoder = encoderName;
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"设置视频编码器: %hs",
            encoderName.empty() ? "软件编码器(默认)" : encoderName.c_str());
    }

    //状态等待
    bool WaitForRenderingToStart(int timeoutMs = 5000); // 等待渲染开始，超时时间默认5秒

    //Get方法
    inline const InterFaceState& GetState() { return m_InterFaceState_State; }
private:
    // 初始化
    bool InitializeSDL();     // 初始化SDL
    bool InitialSwsCtx();     // 初始化FFmpeg图像格式转换器
    bool CreateSDLWindow();   // 创建SDL窗口和渲染器

    //线程函数
    void RunUiEventThread();      // 运行SDL窗口事件的主循环
    void RunFrameRendererThread();// 运行SDL窗口渲染循环
    void Render();                // 运行SDL窗口渲染循环线程函数
    void UiLoop();                // 运行SDL窗口事件的主循环线程函数
    void AudioRecordingThread();  // 音频录制线程函数

    // 录制相关
    bool SetupRecording(const std::string& outputFilePath, RecordMode mode);
    void RecordingThread();
    void CleanupRecording();
    bool SetupVideoStream();  
    bool SetupAudioStream();  
    void ReceiveAndWritePackets(AVCodecContext* codecContext, int streamIndex, bool flush);

    //编码器相关
    bool SetupVideoStreamFallback();                       // 回退到软件编码器
    bool TestEncoderAvailability(const char* encoderName); // 测试编码器是否可用

    //调试函数
    void LogSDLError(const wchar_t* prefix);
private:
    // SDL核心对象
    std::shared_ptr<SDL_Window> m_SDL_Window;       // SDL窗口句柄
    std::shared_ptr<SDL_Renderer> m_SDL_Renderer;   // SDL渲染器句柄
    std::shared_ptr<SDL_Texture> m_SDL_Texture;     // SDL摄像头画面纹理数据

    //渲染帧相关(除帧率外，必须被设定成与捕获帧一样的格式)
    uint16_t m_Int_FrameWidth = 640;        //帧宽
    uint16_t m_Int_FrameHeight = 480;       //帧高
    uint8_t m_Int_frameRate = 30;           //帧渲染帧率
    int32_t m_Int_SampleRate = 44100;       //音频采样率
    int32_t m_Int_BitRate = 128;            //音频比特率
    std::string m_string_fmt = "yuyv422";   //格式
    std::string m_string_device = "Integrated Camera";//摄像头捕获设备
    std::string m_string_desc = "";         //摄像头捕获设备唯一描述
    std::string m_string_videoEncoder = ""; //存储指定的编码器名称

    //ffmpeg
    std::map<AVPixelFormat, std::shared_ptr<SwsContext>> m_Map_SwsCtx;//不同的格式转换器
    std::shared_ptr<AVFrame> m_AVFrame_DisplayFrame;                  //单帧缓冲区
    AVFrame* m_AVFrame_EncodeFrame = nullptr;;                   //编码帧缓冲区

    InterFaceState m_InterFaceState_State;                  //类的原子状态
    InterFaceThread m_InterFaceThread_Thread;               //类的所有线程
    InterFaceThreadProtect m_InterFaceThreadProtect_Protect;//线程同步保护
    RecordingContext m_Recording_Context;
    // 界面属性
    CRect m_Rect_CameraDisplayArea;   // 摄像头画面显示区域的位置和大小

    int m_LastSrcPixelFormat = -1;          // 上一次记录的源帧像素格式
    bool m_DisplayFrameBufferInited = false;// 显示帧缓冲是否已一次性分配
private:
    Ui_CameraDisplaySDL();
    ~Ui_CameraDisplaySDL();
    static Ui_CameraDisplaySDL* s_instance;                        //单一接口
    static std::mutex s_Mutex_Instance;                            //单例锁
};