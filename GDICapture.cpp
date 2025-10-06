#include "stdafx.h"
#include "CDebug.h"
#include "GDICapture.h"
#include "LarStringConversion.h"
#include "WindowHandleManager.h"
#include <stdexcept>
#include <chrono>


extern HANDLE ConsoleHandle;
// 构造函数初始化所有成员
GDICapture::GDICapture(int frameRate)
    : m_hMemDC(NULL)
    , m_hBitmap(NULL)
    , m_hOldBitmap(NULL)
    , m_width(0)
    , m_height(0)
    , m_stride(0)
    , m_bitsPerPixel(32) // 默认使用32位(RGBA)
    , m_isCapturing(false)
    , m_isFrameAvailable(false)
    , m_threadExit(false)
    , m_frameRate(frameRate)
    , m_frameBuffer(nullptr)
{
    m_isAreaReocrd = false;
    m_IsScreenRecord = true;
}

GDICapture::GDICapture(int left, int top, int right, int bottom, int frameRate)
    : m_hMemDC(NULL)
    , m_hBitmap(NULL)
    , m_hOldBitmap(NULL)
    , m_width(0)
    , m_height(0)
    , m_stride(0)
    , m_bitsPerPixel(32) 
    , m_isCapturing(false)
    , m_isFrameAvailable(false)
    , m_threadExit(false)
    , m_frameRate(frameRate)
    , m_frameBuffer(nullptr)
    , m_left(left)
    , m_top(top)
    , m_right(right)
    , m_bottom(bottom)
{
    m_isAreaReocrd = true;
    m_IsScreenRecord = false;
}

GDICapture::GDICapture(int Width, int Height,int frameRate)
    : m_hMemDC(NULL)
    , m_hBitmap(NULL)
    , m_hOldBitmap(NULL)
    , m_width(0)
    , m_height(0)
    , m_stride(0)
    , m_bitsPerPixel(32) // 默认使用32位(RGBA)
    , m_isCapturing(false)
    , m_isFrameAvailable(false)
    , m_threadExit(false)
    , m_frameRate(frameRate)
    , m_frameBuffer(nullptr)
{
    m_width = Width;
    m_height = Height;
    m_frameRate = frameRate;
    m_IsScaleRecord = true;
    m_IsScreenRecord = true;
    m_isAreaReocrd = false;
}

GDICapture::~GDICapture()
{
    StopCapture();
    CleanupResources();
}

void GDICapture::CleanupResources()
{
    // 释放位图资源
    if (m_hMemDC && m_hOldBitmap) 
    {
        SelectObject(m_hMemDC, m_hOldBitmap);
        m_hOldBitmap = NULL;
    }

    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }
    if (m_OriBitmap)
    {
        DeleteObject(m_OriBitmap);
        m_OriBitmap = NULL;
    }

    if (m_hMemDC) 
    {
        DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
    }

    // 释放内部帧缓冲区
    if (m_frameBuffer)
    {
        delete[] m_frameBuffer;
        m_frameBuffer = nullptr;
    }

    // 重置尺寸信息
    m_width = 0;
    m_height = 0;
    m_stride = 0;
}

bool GDICapture::CaptureWindowImage()
{
    if (!m_IsScaleRecord)
    {
        //捕获一帧屏幕
        bool captureSuccess = false;
        HDC hWindowDC = GetDC(NULL);
        if (hWindowDC)
        {
            if (m_IsScreenRecord)
            {
                std::lock_guard<std::mutex> lock(m_RecordAreaChange);
                captureSuccess = BitBlt(
                    m_hMemDC,
                    0, 0, m_width, m_height, 
                    hWindowDC, 0, 0, 
                    SRCCOPY | CAPTUREBLT
                );
            }
            else
            {
                std::lock_guard<std::mutex> lock(m_RecordAreaChange);
                captureSuccess = BitBlt(
                    m_hMemDC, 
                    0, 0, m_width, m_height, 
                    hWindowDC, m_left, m_top, 
                    SRCCOPY | CAPTUREBLT
                );
            }
        }
        if (!captureSuccess)
            return false;

        if (m_IsRecordCursor)//如果需要捕获鼠标
        {
            if (!CaptureCursorFrame())
                DEBUG_CONSOLE_STR(ConsoleHandle, L"捕获一帧鼠标失败");
        }

        // 获取位图数据
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_width;
        bmi.bmiHeader.biHeight = -m_height;  // 负值表示自顶向下的DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = m_bitsPerPixel;
        bmi.bmiHeader.biCompression = BI_RGB;
        {//临界区保护m_frameBuffer
            std::lock_guard<std::mutex> lock(m_frameMutex);
            if (!GetDIBits(m_hMemDC, m_hBitmap, 0, m_height, m_frameBuffer, &bmi, DIB_RGB_COLORS)) {
                ReleaseDC(NULL, hWindowDC);
                return false;
            }
        }
        ReleaseDC(NULL, hWindowDC);
    }
    else
    {//缩放录制
         // 缩放录制
        bool captureSuccess = false;
        HDC hWindowDC = GetDC(NULL);
        if (!hWindowDC)
            return false;

        // 创建临时DC和位图，用于捕获原始画面
        HDC tempDC = CreateCompatibleDC(hWindowDC);
        if (!tempDC)
        {
            ReleaseDC(NULL, hWindowDC);
            return false;
        }

        // 选择原始大小的位图到临时DC
        HBITMAP oldBitmap = (HBITMAP)SelectObject(tempDC, m_OriBitmap);

        // 捕获原始画面到临时DC中的位图
        captureSuccess = BitBlt(tempDC, 0, 0, m_originalWidth, m_originalHeight,
            hWindowDC, 0, 0, SRCCOPY | CAPTUREBLT);
        if (!captureSuccess)
        {
            SelectObject(tempDC, oldBitmap);
            DeleteDC(tempDC);
            ReleaseDC(NULL, hWindowDC);
            return false;
        }

        // 如果需要捕获鼠标，在原始大小画面上绘制
        if (m_IsRecordCursor) {
            DrawCursorOnDC(tempDC); // 这里需要修改为适合临时DC的鼠标捕获方法
        }

        // 设置缩放模式为高质量
        SetStretchBltMode(m_hMemDC, HALFTONE);
        SetBrushOrgEx(m_hMemDC, 0, 0, NULL);

        // 将原始画面缩放到目标大小
        StretchBlt(m_hMemDC, 0, 0, m_width, m_height,
            tempDC, 0, 0, m_originalWidth, m_originalHeight, SRCCOPY);

        // 清理临时资源
        SelectObject(tempDC, oldBitmap);
        DeleteDC(tempDC);

        // 获取缩放后的位图数据到m_frameBuffer
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = m_width;
        bmi.bmiHeader.biHeight = -m_height;  // 负值表示自顶向下的DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = m_bitsPerPixel;
        bmi.bmiHeader.biCompression = BI_RGB;

        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (!GetDIBits(m_hMemDC, m_hBitmap, 0, m_height, m_frameBuffer, &bmi, DIB_RGB_COLORS)) {
            ReleaseDC(NULL, hWindowDC);
            return false;
        }

        ReleaseDC(NULL, hWindowDC);
    }
    
    return true;
}

bool GDICapture::CaptureCursorFrame()
{
    // 获取光标信息,检查光标是否可见
    CURSORINFO cursorInfo = { sizeof(CURSORINFO) };
    if (!GetCursorInfo(&cursorInfo)) {
        return false;
    }
    if (!(cursorInfo.flags & CURSOR_SHOWING)) {
        return false;  
    }

    // 计算光标相对于捕获区域的位置
    POINT cursorPos = cursorInfo.ptScreenPos;
    int cursorX, cursorY;
    if (m_IsScreenRecord)
    {// 全屏录制模式，直接使用光标坐标
        cursorX = cursorPos.x;
        cursorY = cursorPos.y;
    }
    else 
    {  // 区域录制模式，需要调整坐标
        cursorX = cursorPos.x - m_left;
        cursorY = cursorPos.y - m_top;
    }
    if (cursorX < 0 || cursorX >= m_width || cursorY < 0 || cursorY >= m_height)// 检查光标是否在捕获区域内 
        return false; // 光标不在捕获区域内

    // 获取光标图标的句柄
    HICON hCursor = CopyIcon(cursorInfo.hCursor);
    if (!hCursor) 
        return false;
    
    // 获取光标大小,计算光标的热点(hotspot)相对位置
    ICONINFO iconInfo;
    if (!GetIconInfo(hCursor, &iconInfo)) 
    {
        DestroyIcon(hCursor);
        return false;
    }
    cursorX -= iconInfo.xHotspot;
    cursorY -= iconInfo.yHotspot;

    // 释放图标信息中的位图资源,// 绘制光标
    if (iconInfo.hbmColor) DeleteObject(iconInfo.hbmColor);
    if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
    DrawIconEx(m_hMemDC, cursorX, cursorY, hCursor, 0, 0, 0, NULL, DI_NORMAL);
    DestroyIcon(hCursor);
}

bool GDICapture::Initialize()
{
    // 设置屏幕尺寸
    if (m_IsScaleRecord) {
        m_originalWidth = GetSystemMetrics(SM_CXSCREEN);
        m_originalHeight = GetSystemMetrics(SM_CYSCREEN);
        // m_width和m_height已在构造函数中设置为目标尺寸
    }
    else {
        // 非缩放模式代码保持不变
        if (m_left != -1 || m_right != -1 || m_top != -1 || m_bottom != -1) {
            m_width = m_right - m_left;
            m_height = m_bottom - m_top;
        }
        else {
            m_width = GetSystemMetrics(SM_CXSCREEN);
            m_height = GetSystemMetrics(SM_CYSCREEN);
        }
    }

    // 获取屏幕DC
    HDC hWindowDC = GetDC(NULL);
    if (!hWindowDC) return false;

    // 创建内存DC
    m_hMemDC = CreateCompatibleDC(hWindowDC);
    if (!m_hMemDC) {
        ReleaseDC(NULL, hWindowDC);
        return false;
    }

    // 对于缩放模式，创建两个位图
    if (m_IsScaleRecord) {
        // 创建原始大小的位图
        m_OriBitmap = CreateCompatibleBitmap(hWindowDC, m_originalWidth, m_originalHeight);
        if (!m_OriBitmap) {
            DeleteDC(m_hMemDC);
            ReleaseDC(NULL, hWindowDC);
            return false;
        }
    }

    // 创建目标大小的位图
    m_hBitmap = CreateCompatibleBitmap(hWindowDC, m_width, m_height);
    if (!m_hBitmap) {
        if (m_IsScaleRecord) DeleteObject(m_OriBitmap);
        DeleteDC(m_hMemDC);
        ReleaseDC(NULL, hWindowDC);
        return false;
    }

    // 选择目标位图到内存DC
    m_hOldBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hBitmap);

    // 分配帧缓冲区
    m_stride = ((m_width * m_bitsPerPixel + 31) / 32) * 4;
    m_frameBuffer = new unsigned char[m_stride * m_height];
    if (!m_frameBuffer) {
        CleanupResources();
        ReleaseDC(NULL, hWindowDC);
        return false;
    }

    // 释放屏幕DC
    ReleaseDC(NULL, hWindowDC);

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"[屏幕录制] 初始化完成: %s, 尺寸=%dx%d",
        m_IsScaleRecord ? L"缩放模式" : L"标准模式",
        m_width, m_height);

    return true;
}

bool GDICapture::FillAVFrame(AVFrame* frame)
{
    if (!frame || !m_frameBuffer)
    {
        return false;
    }
    av_frame_unref(frame);
    frame->width = m_width;
    frame->height = m_height;
    frame->format = AV_PIX_FMT_BGRA;
    int ret = av_frame_get_buffer(frame, 32);  // 32字节对齐
    if (ret < 0)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[视频编码线程]:分配转换帧缓冲区失败，错误码GDICapture::FillAVFrame");
        return false;
    }

    //将帧数据填充为FFmpeg的AVFame
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        for (int y = 0; y < m_height; y++)
        {
            memcpy(frame->data[0] + y * frame->linesize[0],
                m_frameBuffer + y * m_stride,
                m_width * m_bitsPerPixel / 8);
        }
    }
    return true;
}

void GDICapture::CaptureThreadFunc()
{
    using namespace std::chrono;
    const milliseconds frameInterval(1000 / m_frameRate);// 计算帧间隔时间（毫秒）
    while (!m_threadExit) 
    {
        auto startTime = steady_clock::now();
        m_isFrameAvailable = CaptureWindowImage(); // 捕获当前窗口内容
        if (m_isFrameAvailable)//如果新帧可用，则通知编码线程进行编码
            m_frameCV.notify_all();

        // 若还未到下一帧，则睡眠剩余事件(防止过快的捕获帧，在低帧率模式下显著降低CPU占用)
        auto endTime = steady_clock::now();
        auto elapsedTime = duration_cast<milliseconds>(endTime - startTime);
        if (elapsedTime < frameInterval) {
            std::this_thread::sleep_for(frameInterval - elapsedTime);  // 睡眠剩余时间，保持帧率
        }
    }
}

bool GDICapture::StartCapture()
{
    // 防止重复启动
    if (m_isCapturing) 
        return true;
    
    // 初始化捕获资源
    if (!Initialize()) 
        return false;
    
    // 初始化线程状态
    m_isCapturing = true;
    m_threadExit = false;
    m_isFrameAvailable = false;

    // 启动捕获线程
    try 
    {
        m_captureThread = std::thread(&GDICapture::CaptureThreadFunc, this);
    }
    catch (const std::exception&)
    {
        m_isCapturing = false;
        CleanupResources();
        return false;
    }
    return true;
}

void GDICapture::StopCapture()
{
    if (m_isCapturing)
    {  // 停止捕获线程
        m_isFrameAvailable.store(false);
        m_threadExit = true;
        m_isCapturing = false;

        // 等待线程结束
        if (m_captureThread.joinable()) 
            m_captureThread.join();
    }
    {// 清理资源
        std::lock_guard<std::mutex> lock(m_frameMutex);//这里加锁，是为了让编码线程可能的正在编码的帧进行保护
        CleanupResources();
    }
}

bool GDICapture::CaptureFrame(AVFrame* frame)
{
    if (!frame || !m_isCapturing) {
        return false;
    }
    // 等待新帧可用
    {
        std::unique_lock<std::mutex> lock(m_frameAvailable);
        if (m_frameCV.wait_for(lock, std::chrono::milliseconds(1000),
            [this]
            {
                return m_isFrameAvailable || m_threadExit;
            }))
        {
            if (m_threadExit && !m_isFrameAvailable) // 检查是否因线程退出而被唤醒
            {
                return false;
            }
            if (!FillAVFrame(frame))// 填充AVFrame
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[屏幕录制编码线程]：等待到了新帧，但是填充AVFrame失败!");
                m_isFrameAvailable.store(false);
                return false;
            }
        }
        else
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[屏幕录制编码线程]：等待新帧超时");
            return false;       
        }
    }
    m_isFrameAvailable.store(false);
    return true;
}

bool GDICapture::UpdateRecordArea(int x, int y)
{
    //改变录制区域
    {
        std::lock_guard<std::mutex> lock(m_RecordAreaChange);
        m_left = x;
        m_top = y;
    }
    return true;
}

void GDICapture::SetRecordCursor(bool IsRecordCursor)
{
    m_IsRecordCursor = IsRecordCursor;
}

bool GDICapture::DrawCursorOnDC(HDC dc)
{
    // 获取光标信息
    CURSORINFO cursorInfo = { sizeof(CURSORINFO) };
    if (!GetCursorInfo(&cursorInfo) || !(cursorInfo.flags & CURSOR_SHOWING)) {
        return false;
    }

    // 计算光标位置（在原始坐标系中）
    POINT cursorPos = cursorInfo.ptScreenPos;

    // 获取光标图标
    HICON hCursor = CopyIcon(cursorInfo.hCursor);
    if (!hCursor) return false;

    // 获取光标信息
    ICONINFO iconInfo;
    if (!GetIconInfo(hCursor, &iconInfo)) {
        DestroyIcon(hCursor);
        return false;
    }

    // 调整光标位置
    int cursorX = cursorPos.x - iconInfo.xHotspot;
    int cursorY = cursorPos.y - iconInfo.yHotspot;

    // 释放图标资源
    if (iconInfo.hbmColor) DeleteObject(iconInfo.hbmColor);
    if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);

    // 绘制光标
    DrawIconEx(dc, cursorX, cursorY, hCursor, 0, 0, 0, NULL, DI_NORMAL);
    DestroyIcon(hCursor);

    return true;
}
