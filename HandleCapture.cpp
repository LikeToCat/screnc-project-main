#include "stdafx.h"
#include "CDebug.h"
#include "HandleCapture.h"
#include "LarStringConversion.h"
#include "WindowHandleManager.h"
#include "theApp.h"
#include <stdexcept>
#include <chrono>

#if (WINVER < 0x0602) 
#ifndef PW_RENDERFULLCONTENT
#define PW_RENDERFULLCONTENT  0x00000000
#endif
#endif
extern HANDLE ConsoleHandle;
// 构造函数初始化所有成员
HandleCapture::HandleCapture(HWND targetWindowl, int frameRate)
    : m_hTargetWindow(targetWindowl)
    , m_hMemDC(NULL)
    , m_hBitmap(NULL)
    , m_hOldBitmap(NULL)
    , m_pBitmapData(nullptr)
    , m_width(0)
    , m_height(0)
    , m_stride(0)
    , m_bitsPerPixel(32) // 默认使用32位(RGBA)
    , m_isCapturing(false)
    , m_isFrameAvailable(false)
    , m_threadExit(false)
    , m_frameRate(frameRate)
    , m_frameBuffer(nullptr)
    , m_lastFrame(nullptr)
{
    if (!m_hTargetWindow || !IsWindow(m_hTargetWindow)) {
        throw std::runtime_error("Invalid window handle");
    }
    m_lastFrame = av_frame_alloc();
    if (App.IsWin7Over())
    {
        m_CaptureFlag = 2;
    }
    else
    {
        m_CaptureFlag = 0;
    }
}

HandleCapture::~HandleCapture()
{
    StopCapture();
    CleanupResources();
}

void HandleCapture::CleanupResources()
{
    // 释放位图资源
    if (m_hMemDC && m_hOldBitmap) {
        SelectObject(m_hMemDC, m_hOldBitmap);
        m_hOldBitmap = NULL;
    }

    if (m_hBitmap) {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }

    if (m_hMemDC) {
        DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
    }

    // 释放内部帧缓冲区
    if (m_frameBuffer) {
        delete[] m_frameBuffer;
        m_frameBuffer = nullptr;
    }
    if (m_lastFrame) {
        av_frame_free(&m_lastFrame);
        m_lastFrame = nullptr;
    }

    // 重置尺寸信息
    m_width = 0;
    m_height = 0;
    m_stride = 0;
}

void HandleCapture::GetHwndSize(_In_ HWND hWnd, _In_ int oldWidth, _In_ int oldHeight,
    _Out_ int* width, _Out_ int* height, _Out_ bool* isChanged)
{
    // 检查窗口尺寸是否变化，如有必要重新初始化
    CRect WindowRect;
    int oldwidth = oldWidth;
    int oldheight = oldHeight;
    *isChanged = false;
    if (::IsIconic(hWnd))// 对于最小化窗口，获取其原始尺寸
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if (::GetWindowPlacement(hWnd, &wp))
        {
            // rcNormalPosition包含窗口在正常状态下的位置和尺寸
            RECT rcNormal = wp.rcNormalPosition;
            *width = rcNormal.right - rcNormal.left;
            *height = rcNormal.bottom - rcNormal.top;
        }
    }
    else
    {
        GetWindowRect(m_hTargetWindow, WindowRect);
        *width = WindowRect.Width();
        *height = WindowRect.Height();
    }

    //保证为偶数
    if ((*width) % 2 != 0)
    {
        (*width)--;
    }
    if ((*height) % 2 != 0)
    {
        (*height)--;
    }

    //比较是否一样
    if (oldwidth != (*width) || oldheight != (*height))
    {
        *isChanged = true;
    }
}

bool HandleCapture::CaptureWindowImage()
{
    if (!m_hTargetWindow || !m_hMemDC || !m_frameBuffer) {
        return false;
    }

    // 检查窗口句柄是否有效
    if (!IsWindow(m_hTargetWindow)) {
        return false;
    }

    //判断录制的窗口大小是否发生了改变
    int oldWidth, oldHeight, newWidth, newHeight;
    oldHeight = m_height;
    oldWidth = m_width;
    bool isChanged;
    GetHwndSize(m_hTargetWindow, oldWidth, oldHeight, &newWidth, &newHeight, &isChanged);
    if (isChanged)//停止捕获，设置另一个编码线程的暂停标志，直到尺寸稳定下来
    {
        int Cnt = 0;//连续检查不变的次数
        while (isChanged || Cnt <= 5)//只有当连续5次检查，窗口大小都没有发生改变，则退出循环
        {
            GetHwndSize(m_hTargetWindow, newWidth, newHeight, &newWidth, &newHeight, &isChanged);
            if (!isChanged)Cnt++;
            else Cnt = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        CleanupResources();
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            if (!Initialize()) {//重新初始化以适配新的窗口大小
                return false;
            }
        }
    }

    //捕获窗口
    bool captureSuccess = false;
    HDC hWindowDC = GetDC(m_hTargetWindow);
    if (hWindowDC)
    {
        if (PrintWindow(m_hTargetWindow, m_hMemDC, m_CaptureFlag)) 
        { // 先尝试PrintWindow，它通常可以捕获到更完整的窗口内容
            captureSuccess = true;
        }
        else if (BitBlt(m_hMemDC, 0, 0, m_width, m_height, hWindowDC, 0, 0, SRCCOPY | CAPTUREBLT)) 
        {// 如果PrintWindow失败，再尝试BitBlt
            captureSuccess = true;
        }
        ReleaseDC(m_hTargetWindow, hWindowDC);
    }
    if (!captureSuccess) {
        return false;
    }

    // 如果需要捕获鼠标内容
    if (m_IsRecordCursor)
    {
        RECT windowRect;   // 更新窗口位置
        if (GetWindowRect(m_hTargetWindow, &windowRect)) 
        {
            m_left = windowRect.left;
            m_top = windowRect.top;
        }
        if (!CaptureCursorFrame())
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"[应用窗口捕获器]:捕获鼠标失败");
        }
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
            return false;
        }
    }
    return true;
}

bool HandleCapture::Initialize()
{
    // 获取窗口尺寸（包含非客户区）
    CRect WindowRect;
    if (::IsIconic(m_hTargetWindow))// 对于最小化窗口，获取其原始尺寸
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if (::GetWindowPlacement(m_hTargetWindow, &wp))
        {
            // rcNormalPosition包含窗口在正常状态下的位置和尺寸
            RECT rcNormal = wp.rcNormalPosition;
            m_width = rcNormal.right - rcNormal.left;
            m_height = rcNormal.bottom - rcNormal.top;
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"窗口句柄捕获器初始化：获取最小化窗口 \"%s\" 的原始分辨率: %d x %d",
                WindowHandleManager::GetWindowTitle(m_hTargetWindow).c_str(), m_width, m_height);
        }
    }
    else
    {
        GetWindowRect(m_hTargetWindow, WindowRect);
        m_width = WindowRect.Width();
        m_height = WindowRect.Height();
    }
    if (m_width <= 0 || m_height <= 0) {// 确保窗口尺寸有效
        return false;
    }
    bool IsAdjust = false;
    // 确保宽度和高度是偶数 (YUV420P格式要求)
    if (m_width % 2 != 0) {
        IsAdjust = true;
        m_width--;  // 减1使宽度变为偶数
    }
    if (m_height % 2 != 0) {
        IsAdjust = true;
        m_height--;  // 减1使高度变为偶数
    }
    if (IsAdjust == true)
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"调整后的捕获分辨率: %d x %d (确保为偶数)", m_width, m_height);

    // 创建兼容的DC
    HDC hWindowDC = GetDC(NULL);  // 获取屏幕DC
    if (!hWindowDC) {
        return false;
    }
    m_hMemDC = CreateCompatibleDC(hWindowDC);
    if (!m_hMemDC) {
        ReleaseDC(NULL, hWindowDC);
        return false;
    }

    // 创建兼容位图
    m_hBitmap = CreateCompatibleBitmap(hWindowDC, m_width, m_height);
    if (!m_hBitmap) {
        DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
        ReleaseDC(NULL, hWindowDC);
        return false;
    }
    m_hOldBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hBitmap);

    //申请新的帧缓冲区（确保总是大于实际捕获的帧，防止缓冲区溢出问题）
    m_stride = max(((m_width * m_bitsPerPixel + 31) / 32) * 4, m_width * m_bitsPerPixel / 8);
    m_frameBuffer = new unsigned char[m_stride * m_height];
    if (!m_frameBuffer) {
        CleanupResources();
        ReleaseDC(NULL, hWindowDC);
        return false;
    }
    // 释放屏幕DC
    ReleaseDC(NULL, hWindowDC);
    return true;
}

bool HandleCapture::FillAVFrame(AVFrame* frame)
{
    if (!frame || !m_frameBuffer)
    {
        return false;
    }
    //设置转换后的帧结构
    frame->width = m_width;
    frame->height = m_height;
    frame->format = AV_PIX_FMT_BGRA;
    int ret = av_frame_get_buffer(frame, 32);  // 32字节对齐
    if (ret < 0) 
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"[窗口句柄录制编码线程]:分配转换帧缓冲区失败，错误码handleCapture23");
        return false;
    }
    for (int y = 0; y < m_height; y++)
    {
        memcpy(frame->data[0] + y * frame->linesize[0],
            m_frameBuffer + y * m_stride,
            m_width * m_bitsPerPixel / 8);
    }
    //更新上一帧
    if (m_lastFrame) 
    {
        av_frame_unref(m_lastFrame);
        av_frame_ref(m_lastFrame, frame);
    }
    return true;
}

void HandleCapture::CaptureThreadFunc()
{
    using namespace std::chrono;
    const milliseconds frameInterval(1000 / m_frameRate);// 计算帧间隔时间（毫秒）
    while (!m_threadExit)
    {
        auto startTime = steady_clock::now();
        // 捕获当前窗口内容
        m_isFrameAvailable = CaptureWindowImage();
        if (m_isFrameAvailable)//如果新帧可用，则通知编码线程进行编码
            m_frameCV.notify_all();

        // 若还未到下一帧，则睡眠剩余事件
        auto endTime = steady_clock::now();
        auto elapsedTime = duration_cast<milliseconds>(endTime - startTime);
        if (elapsedTime < frameInterval) {
            std::this_thread::sleep_for(frameInterval - elapsedTime);  // 睡眠剩余时间，保持帧率
        }
    }
}

bool HandleCapture::StartCapture()
{
    // 防止重复启动或无效窗口的录制启动
    if (m_isCapturing || !IsWindow(m_hTargetWindow)) 
    {
        return true;
    }

    if (!Initialize()) 
    {// 初始化捕获资源
        return false;
    }

    // 初始化线程状态
    m_isCapturing = true;
    m_threadExit = false;
    m_isFrameAvailable = false;

    // 启动捕获线程
    try 
    {
        m_captureThread = std::thread(&HandleCapture::CaptureThreadFunc, this);
    }
    catch (const std::exception&)
    {
        m_isCapturing = false;
        CleanupResources();
        return false;
    }
    return true;
}

void HandleCapture::StopCapture()
{
    // 停止捕获线程
    if (m_isCapturing) 
    {
        m_isFrameAvailable.store(false);
        m_threadExit = true;
        m_isCapturing = false;

        // 等待线程结束
        if (m_captureThread.joinable()) {
            m_captureThread.join();
        }
    }
    {// 清理资源
        std::lock_guard<std::mutex> lock(m_frameMutex);//这里加锁，是为了让编码线程可能的正在编码的帧进行保护
        CleanupResources();
    }

}

bool HandleCapture::CaptureFrame(AVFrame* frame)
{
    if (!frame || !m_isCapturing) {
        return false;
    }
    // 等待新帧可用
    {
        std::unique_lock<std::mutex> lock(m_frameMutex);
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
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[窗口句柄录制编码线程]：等待到了新帧，但是填充失败!");
                m_isFrameAvailable.store(false);
                return false;
            }
        }
        else
        {
            if (m_lastFrame)
            {
                av_frame_ref(frame, m_lastFrame);
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[窗口句柄录制编码线程]：等待新帧超时，使用旧帧");
            }
            else
            {
                DEBUG_CONSOLE_STR(ConsoleHandle, L"[窗口句柄录制编码线程]：等待新帧超时");
                return false;
            }
        }
    }
    m_isFrameAvailable.store(false);
    return true;
}

void HandleCapture::SetRecordCursor(bool IsRecordCursor)
{
    m_IsRecordCursor = IsRecordCursor;
}

bool HandleCapture::CaptureCursorFrame()
{
    // 获取光标信息
    CURSORINFO cursorInfo = { sizeof(CURSORINFO) };
    if (!GetCursorInfo(&cursorInfo)) {
        return false; // 获取光标信息失败
    }
    if (!(cursorInfo.flags & CURSOR_SHOWING)) {
        return false; // 光标不可见，不需要绘制
    }

    // 获取窗口在屏幕上的位置
    RECT windowRect;
    if (!GetWindowRect(m_hTargetWindow, &windowRect)) {
        return false; // 获取窗口位置失败
    }
    m_left = windowRect.left;
    m_top = windowRect.top;

    // 计算光标相对于窗口的位置
    POINT cursorPos = cursorInfo.ptScreenPos;
    int cursorX = cursorPos.x - m_left;
    int cursorY = cursorPos.y - m_top;
    if (cursorX < 0 || cursorX >= m_width || cursorY < 0 || cursorY >= m_height) {
        return false; // 光标不在捕获区域内
    }

    // 获取光标图标的句柄
    HICON hCursor = CopyIcon(cursorInfo.hCursor);
    if (!hCursor) {
        return false; // 获取光标图标失败
    }

    // 获取光标大小,计算光标的热点(hotspot)相对位置
    ICONINFO iconInfo;
    if (!GetIconInfo(hCursor, &iconInfo)) {
        DestroyIcon(hCursor);
        return false; // 获取图标信息失败
    }
    cursorX -= iconInfo.xHotspot;
    cursorY -= iconInfo.yHotspot;

    // 释放图标信息中的位图资源,绘制光标
    if (iconInfo.hbmColor) DeleteObject(iconInfo.hbmColor);
    if (iconInfo.hbmMask) DeleteObject(iconInfo.hbmMask);
    DrawIconEx(m_hMemDC, cursorX, cursorY, hCursor, 0, 0, 0, NULL, DI_NORMAL);
    DestroyIcon(hCursor);

    return true; 
}
