#include "stdafx.h"
#include "LarPaymentPolling.h"
#include "json.h"
#include <iostream>
#include <chrono>
#include <curl.h>
#include "CDebug.h"
#include "theApp.h"
#include "LarStringConversion.h"
#include "GlobalFunc.h"

extern HANDLE ConsoleHandle;  // 假设全局控制台句柄

// curl 的回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* recvBuffer = reinterpret_cast<std::string*>(userp);
    size_t totalSize = size * nmemb;
    recvBuffer->append(reinterpret_cast<char*>(contents), totalSize);
    return totalSize;
}

LarPaymentPolling::LarPaymentPolling() : m_pollingActive(false), m_stopRequested(false) {
    // 初始化
    m_paused.store(false);
}

LarPaymentPolling::~LarPaymentPolling() {
    stopPolling();
}

void LarPaymentPolling::startPolling(
    const std::string& preOrderNo,
    PaymentSuccessCallback onSuccess,
    TimeoutCallback onTimeout) {

    // 如果之前的轮询还未结束，则先停止
    stopPolling();

    // 设置参数
    m_preOrderNo = preOrderNo;
    m_onSuccess = onSuccess;
    m_onTimeout = onTimeout;

    // 设置 API Token
#if RELEASE_CODE == 1
    m_apiToken = App.m_appToken;
#else
    m_apiToken = "oKqdT4g_7O8yLxKtdUeOsoMr85qE22LfzKlotOfAydbcUIVnNklYe-XYmqvzkRORJxlvpHqVor0TEtB6ErJJbWQhMBGdVwCQFg-jxMQR_eY";
#endif

    m_stopRequested = false;
    m_pollingActive = true;
    m_paused.store(false);

    // 开启轮询线程
    m_pollingThread = std::thread(&LarPaymentPolling::pollingLoop, this);
    // 开启定时器线程
    m_timerThread = std::thread(&LarPaymentPolling::timerLoop, this);

    DEBUG_CONSOLE_STR(ConsoleHandle, L"支付状态轮询已启动");
}

void LarPaymentPolling::stopPolling()
{
    m_stopRequested = true;
    m_pollingActive = false;
    m_paused.store(false);
    m_cv.notify_all();  // 通知所有等待线程

    if (m_pollingThread.joinable())
        m_pollingThread.join();

    // 防止线程在内部调用 stopPolling 后 join 自己
    if (std::this_thread::get_id() != m_timerThread.get_id() && m_timerThread.joinable())
        m_timerThread.join();

    DEBUG_CONSOLE_STR(ConsoleHandle, L"支付状态轮询已停止");
}

void LarPaymentPolling::refresh(const std::string& preOrderNo) {
    // 更新预订单号并重新启动轮询
    DEBUG_CONSOLE_STR(ConsoleHandle, L"刷新支付状态轮询");
    startPolling(preOrderNo, m_onSuccess, m_onTimeout);
}

void LarPaymentPolling::pausePolling()
{
    if (!m_pollingActive) return;
    m_paused.store(true);
    m_cv.notify_all(); 
    DEBUG_CONSOLE_STR(ConsoleHandle, L"支付状态轮询已暂停");
}

void LarPaymentPolling::resumePolling()
{
    if (!m_pollingActive) return;
    m_paused.store(false);
    m_cv.notify_all(); 
    DEBUG_CONSOLE_STR(ConsoleHandle, L"支付状态轮询已恢复");
}

void LarPaymentPolling::pollingLoop()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"支付轮询线程已启动");

    while (!m_stopRequested) 
    {
        //如果处于暂停，则在此阻塞，直到恢复或被请求停止
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]()
                {
                    return m_stopRequested.load() || !m_paused.load();
                });
        }
        if (m_stopRequested) break;

        // 发起一次状态查询
        bool paymentSuccess = checkPaymentStatus();
        if (paymentSuccess) 
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"检测到支付成功");
            if (m_onSuccess) m_onSuccess();
            break;
        }
        else 
        {
            DEBUG_CONSOLE_STR(ConsoleHandle, L"未支付，等待下一次轮询...");
        }

        // 间隔等待3秒；如果暂停/停止则提前唤醒
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait_for(lock, std::chrono::seconds(3), [this]()
            {
                return m_stopRequested.load() || m_paused.load();
            });
    }

    m_pollingActive = false;
}

void LarPaymentPolling::timerLoop()
{
    DEBUG_CONSOLE_STR(ConsoleHandle, L"支付轮询计时器线程已启动");

    int remaining = 300; 
    std::unique_lock<std::mutex> lock(m_mutex);
    while (remaining > 0 && !m_stopRequested) 
    {
        // 若处于暂停，不消耗剩余时间
        if (m_paused.load())
        {
            m_cv.wait(lock, [this]()
                {
                    return m_stopRequested.load() || !m_paused.load();
                });
            if (m_stopRequested) return;
        }

        // 正常计时
        m_cv.wait_for(lock, std::chrono::seconds(1), [this]()
            {
                return m_stopRequested.load() || m_paused.load();
            });
        if (m_stopRequested) return;
        if (!m_paused.load()) 
        {
            --remaining; 
        }
    }
    lock.unlock();

    if (remaining <= 0 && m_pollingActive)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"支付超时，停止轮询");
        stopPolling();
        if (m_onTimeout) m_onTimeout();
    }
}

bool LarPaymentPolling::checkPaymentStatus() {
    // 构造请求体 JSON
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64)
    Json::Value requestJson;
    requestJson["pre_order_no"] = m_preOrderNo;

    Json::StreamWriterBuilder writer;
    std::string jsonBody = Json::writeString(writer, requestJson);
#else
    // x86 平台使用旧版 Json API
    Json::Value requestJson;
    requestJson["pre_order_no"] = m_preOrderNo;

    Json::FastWriter writer;
    std::string jsonBody = writer.write(requestJson);
#endif

    // 初始化curl
    CURL* curl = curl_easy_init();
    if (!curl) {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"初始化 CURL 失败");
        return false;
    }

    // 设置请求头
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // 添加 API Token
    std::string tokenHeader = "x-api-token: " + m_apiToken;
    headers = curl_slist_append(headers, GlobalFunc::AnsiToUtf8(tokenHeader).c_str());

    DEBUG_CONSOLE_FMT(ConsoleHandle, L"轮询请求发送:Token:%s", LARSC::s2ws(tokenHeader).c_str());

    // 存储响应数据
    std::string responseBuffer;

    // 配置 curl 选项
    curl_easy_setopt(curl, CURLOPT_URL, "http://scrnrec.appapr.com/paid");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    // 获取HTTP状态码
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode == 500)
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"数据请求失败，错误代码500");

        Ui_MessageModalDlg messageBox;
        messageBox.SetModal(L"极速录屏大师", L"Ops！出错了", L"无法正常拉取支付信息", L"确认");
        messageBox.DoModal();
        ::PostMessage(App.m_Dlg_Main.GetSafeHwnd(), MSG_OPENVIPDLG_PAYMENTLOOPERROR, NULL, NULL);
        // 清理资源
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return false;
    }

    // 清理资源
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        DEBUG_CONSOLE_FMT(ConsoleHandle, L"请求失败: %hs", curl_easy_strerror(res));
        return false;
    }

    // 解析响应 JSON
    bool isPaid = false;
    Json::Reader reader;
    Json::Value responseJson;
    DEBUG_CONSOLE_FMT(ConsoleHandle, L"返回响应:%s", LARSC::s2ws(GlobalFunc::Utf8ToAnsi(responseBuffer)).c_str());
    if (reader.parse(responseBuffer, responseJson)) {
        // 检查响应状态
        if (responseJson.get("success", false).asBool()) {
            // 获取支付状态
            if (responseJson.isMember("data") && responseJson["data"].isObject() &&
                responseJson["data"].isMember("is_paid")) {
                isPaid = responseJson["data"]["is_paid"].asBool();
                DEBUG_CONSOLE_FMT(ConsoleHandle, L"支付状态: %s", isPaid ? L"已支付" : L"未支付");
            }
        }
        else {
            std::string message = responseJson.get("message", "unknown error").asString();
            DEBUG_CONSOLE_FMT(ConsoleHandle, L"支付状态查询失败: %hs", message.c_str());
        }
    }
    else {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"解析响应 JSON 失败");
    }

    return isPaid;
}