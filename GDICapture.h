#pragma once
#pragma once

#include <windows.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

extern "C" {
#include <libavutil/frame.h>
}

class GDICapture
{
public:
    // 构造函数接收窗口句柄
    GDICapture(int frameRate);
    GDICapture(int left, int top, int right, int bottom, int frameRate);
    GDICapture(int Width, int Height, int frameRate);
    ~GDICapture();

    // 三个主要接口
    bool StartCapture();
    void StopCapture();
    bool CaptureFrame(AVFrame* frame);
    bool UpdateRecordArea(int x, int y);
    void SetRecordCursor(bool IsRecordCursor);
private:
    // 内部实现方法
    void CaptureThreadFunc();        // 捕获线程函数
    bool Initialize();               // 初始化捕获资源
    bool CaptureWindowImage();       // 捕获窗口图像到内部缓冲区
    bool CaptureCursorFrame();       // 捕获鼠标
    bool FillAVFrame(AVFrame* frame);// 填充外部提供的AVFrame
    void CleanupResources();         // 清理资源
    bool DrawCursorOnDC(HDC dc);
    // GDI捕获相关
    HDC m_hMemDC;         // 内存设备上下文
    HBITMAP m_hBitmap;    // 位图句柄
    HBITMAP m_hOldBitmap; // 旧位图句柄(用于恢复)
    HBITMAP m_OriBitmap;  // 原始帧捕获(用于缩放录制)
    int m_width;          // 捕获宽度
    int m_height;         // 捕获高度
    int m_stride;         // 图像行步长
    int m_bitsPerPixel;   // 每像素位数
    int m_left = -1;      // 选区录制left
    int m_top = -1;       // 选区录制top
    int m_right = -1;     // 选区录制right
    int m_bottom = -1;    // 选区录制bottom
    bool m_isAreaReocrd = false;    // 区域录制
    bool m_IsScreenRecord = false;  // 屏幕录制
    bool m_IsRecordCursor = false;  // 是否录制鼠标
    bool m_IsScaleRecord = false;   // 是否缩放录制
    int m_originalWidth;    //原始尺寸w
    int m_originalHeight;   //原始尺寸h
    unsigned char* m_originalBuffer;    //原始帧缓冲区

    // 线程同步相关
    std::thread m_captureThread;
    std::mutex m_frameMutex;
    std::mutex m_frameAvailable;
    std::mutex m_RecordAreaChange;
    std::condition_variable m_frameCV;
    bool m_isCapturing;        // 是否正在捕获
    std::atomic<bool> m_isFrameAvailable = false;   // 是否有新帧可用
    bool m_threadExit;         // 线程退出标志
    int m_frameRate;           // 捕获帧率

    // 内部帧缓冲区
    unsigned char* m_frameBuffer;  // 供线程使用的帧缓冲区
};