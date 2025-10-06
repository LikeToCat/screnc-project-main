#ifndef LAR_PAYMENT_POLLING_H
#define LAR_PAYMENT_POLLING_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

// 支付轮询管理类
class LarPaymentPolling
{
public:
    using PaymentSuccessCallback = std::function<void()>;   // 支付成功时的回调函数类型
    using TimeoutCallback = std::function<void()>;          // 超时回调函数类型

    LarPaymentPolling();
    ~LarPaymentPolling();

    void startPolling(
        const std::string& preOrderNo,
        PaymentSuccessCallback onSuccess,
        TimeoutCallback onTimeout);             // 启动轮询操作：传入订单号和对应的回调函数
    void stopPolling();                         // 停止轮询和定时器线程
    void refresh(const std::string& preOrderNo);// 刷新轮询 (用于超时后重新开始)
    void pausePolling();
    void resumePolling();
    inline bool isRunning() const { return m_pollingActive; }
private:
    void pollingLoop();                 // 轮询线程主循环
    void timerLoop();                   // 定时器线程主循环
    bool checkPaymentStatus();          // 发送支付状态检查请求

    std::atomic<bool> m_pollingActive;   // 轮询状态
    std::atomic<bool> m_stopRequested;   // 是否请求停止
    std::atomic<bool> m_paused{ false };

    std::thread m_pollingThread;         // 轮询线程
    std::thread m_timerThread;           // 定时器线程
    std::mutex m_mutex;
    std::condition_variable m_cv;

    std::string m_preOrderNo;            // 预订单号
    std::string m_apiToken;              // API Token

    PaymentSuccessCallback m_onSuccess;
    TimeoutCallback m_onTimeout;
};

#endif // LAR_PAYMENT_POLLING_H