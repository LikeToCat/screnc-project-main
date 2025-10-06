#include "stdafx.h"
#include "HttpRequestHandler.h"
#include "GlobalFunc.h"
#include "CDebug.h"
#include "LarStringConversion.h"
extern HANDLE ConsoleHandle;
static size_t HttpRequestHandlerWriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
	size_t totalSize = size * nmemb;
	userp->append((char*)contents, totalSize);
	return totalSize;
}

HttpRequestHandler::HttpRequestHandler(RequestType requestType, ValueEncodeType valueEncodeType, std::string url)
{
	m_RequestType = requestType;
	m_RequestState = UNSENDED;
	m_ValueEncodeType = valueEncodeType;
	m_str_url = url;
	m_str_Json = "NULL";
    m_headers = nullptr;
	m_ResponeData = "NULL";
    m_errorMsg = "";
    m_ostream_requestHeaderInfo << "Request Url:" << url << "\n" << "Header:\n";
    m_ostream_requestBodyInfo << "Body:\n";
    AddHeader("Content-Type", "application/json");
}

void HttpRequestHandler::AddHeader(std::string Header, std::string Value)
{
    Header = Header + ": " + Value;
    if (m_ValueEncodeType == UTF8)
    {
        Header = GlobalFunc::AnsiToUtf8(Header);
    }
    m_ostream_requestHeaderInfo << Header << "\n";
    m_headers = curl_slist_append(m_headers, Header.c_str());
}

void HttpRequestHandler::AddJsonBody(Json::Value JsonValue)
{
    std::string jsonOutput = m_JsonWriter.write(JsonValue);
    //根据编码预设，进行编码转换
    if (m_ValueEncodeType == ANSI)
    {
        m_str_Json = jsonOutput;
    }
    else if (m_ValueEncodeType == UTF8)
    {
        m_str_Json = jsonOutput;
        m_str_Json = GlobalFunc::AnsiToUtf8(m_str_Json);
    }
    m_ostream_requestBodyInfo << m_str_Json << "\n";
}

bool HttpRequestHandler::PerformRequest()
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        m_RequestState = HTMLINVALIDEDATA;
        return false;
    }
    CURLcode res;
    long httpCode = 0;
    m_ResponeData.clear();
    m_str_url = GlobalFunc::AnsiToUtf8(m_str_url);

    // 设置请求头
    if (m_headers)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_headers);
    }
    curl_easy_setopt(curl, CURLOPT_URL, m_str_url.c_str());// 设置URL
    if (m_RequestType == POST)                             // 设置请求类型 (POST 或 GET)
    {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (!m_str_Json.empty() && m_str_Json != "NULL")
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_str_Json.c_str());
        }
    }
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // HTTP 4xx/5xx 时直接返回错误，不再走写回调
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);    // 设置超时时间
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpRequestHandlerWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &m_ResponeData);
    std::ostringstream requestInfo;
    requestInfo
        << "=====Http Request Start======================================\n"
        << "=====Request Url============\n"
        << m_str_url
        << "\n=====Request Header=====\n"
        << m_ostream_requestHeaderInfo.str()
        << "=====Request Body=======\n"
        << m_ostream_requestBodyInfo.str();
    DEBUG_CONSOLE_STR(ConsoleHandle, LARSC::s2ws(requestInfo.str()).c_str());

    // 执行请求
    res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode); // 获取HTTP响应码
    if (httpCode == 500 || httpCode == 404)
    {   
        m_ResponeData.clear();
        m_errorMsg = "Server returned HTTP 500 Internal Server Error.";
        m_RequestState = HTMLINVALIDEDATA;
    }

    if (res == CURLE_OPERATION_TIMEDOUT)
    {
        m_ResponeData.clear();
        m_errorMsg = "Operation timed out.";
        m_RequestState = TIMEOUT;
    }
    else if (res != CURLE_OK)
    {
        m_ResponeData.clear();
        m_errorMsg = curl_easy_strerror(res);
        m_RequestState = FAILED; 
    }
    else
    {
        m_RequestState = NONE; 
    }

    if (m_RequestState == NONE)
    {
        std::string ResponeDataAnsi = GlobalFunc::Utf8ToAnsi(m_ResponeData);
        DEBUG_CONSOLE_STR(ConsoleHandle, L"=====Responese Data=====");
        DEBUG_CONSOLE_STR(ConsoleHandle, LARSC::s2ws(ResponeDataAnsi).c_str());
        DEBUG_CONSOLE_STR(ConsoleHandle, L"=====Http Request Over======================================");
    }
    if (m_errorMsg != "")
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, LARSC::s2ws(m_errorMsg).c_str());
    }

    // 清理
    curl_easy_cleanup(curl);
    if (m_headers)
    {
        curl_slist_free_all(m_headers);
    }
    return (res == CURLE_OK);
}

Json::Value HttpRequestHandler::GetResponeData(ValueEncodeType type)
{
    if (type == UTF8)
    {
       
    }
    else if (type == ANSI)
    {
        m_ResponeData = GlobalFunc::Utf8ToAnsi(m_ResponeData);
    }

    Json::Reader reader;
    Json::Value Root;
    if (!reader.parse(m_ResponeData, Root))
    {
        DEBUG_CONSOLE_STR(ConsoleHandle, L"错误！响应数据解析失败!");
        return Json::Value();
    }
    return Root;
}

std::wstring HttpRequestHandler::GetDebugInfoString()
{

    return std::wstring();
}
