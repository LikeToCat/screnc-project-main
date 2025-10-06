#pragma once

#include <windows.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

extern "C" 
{
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class HandleCapture
{
public:
    // 构造函数接收窗口句柄
    HandleCapture(HWND targetWindowl, int frameRate);
    ~HandleCapture();

    // 三个主要接口
    bool StartCapture();
    void StopCapture();
    bool CaptureFrame(AVFrame* frame);
    void SetRecordCursor(bool IsRecordCursor);
private:
    // 内部实现方法
    void CaptureThreadFunc();           // 捕获线程函数
    bool Initialize();                  // 初始化捕获资源
    bool CaptureWindowImage();          // 捕获窗口图像到内部缓冲区
    bool FillAVFrame(AVFrame* frame);   // 填充外部提供的AVFrame
    void CleanupResources();            // 清理资源
    void GetHwndSize(_In_ HWND hWnd, _In_ int oldWidth, _In_ int oldHeight,
        _Out_ int* width, _Out_ int* height, _Out_ bool* isChanged);//判断窗口大小是否改变
    bool CaptureCursorFrame();
    // 窗口相关
    HWND m_hTargetWindow;      // 目标窗口句柄

    // GDI捕获相关
    HDC m_hMemDC;              // 内存设备上下文
    HBITMAP m_hBitmap;         // 位图句柄
    HBITMAP m_hOldBitmap;      // 旧位图句柄(用于恢复)
    unsigned char* m_pBitmapData;  // 位图数据
    int m_width;               // 捕获宽度
    int m_height;              // 捕获高度
    int m_stride;              // 图像行步长
    int m_bitsPerPixel;        // 每像素位数

    //鼠标录制相关
    bool m_IsRecordCursor = false;  //是否录制鼠标
    int m_left = 0;                 // 窗口在屏幕上的左边界
    int m_top = 0;                  // 窗口在屏幕上的上边界

    // 线程同步相关
    std::thread m_captureThread;
    std::mutex m_frameMutex;
    std::condition_variable m_frameCV;
    bool m_isCapturing;        // 是否正在捕获
    std::atomic<bool> m_isFrameAvailable = false;   // 是否有新帧可用
    bool m_threadExit;         // 线程退出标志
    int m_frameRate;           // 捕获帧率

    // 窗口尺寸调整检测相关
    std::atomic<bool> m_bool_IsResizing = false;


    // 内部帧缓冲区
    unsigned char* m_frameBuffer;  // 供线程使用的帧缓冲区
    AVFrame* m_lastFrame;//上一帧
    int m_CaptureFlag;
};