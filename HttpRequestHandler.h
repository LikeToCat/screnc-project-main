#pragma once
#include <string>
#include <curl.h>
#include <functional>
#include <json.h>
#include <sstream>
class HttpRequestHandler
{
public:
	enum ErrorState
	{
		UNSENDED,
		NONE,
		TIMEOUT,
		FAILED,
		HTMLINVALIDEDATA,
	};
	enum RequestType
	{
		POST,
		GET
	};
	enum ValueEncodeType
	{
		ANSI,
		UTF8
	};
public:
	HttpRequestHandler(RequestType requestType, ValueEncodeType valueEncodeType, std::string url);
	void AddHeader(std::string Header, std::string Value);
	void AddJsonBody(Json::Value JsonValue);
	
	bool PerformRequest();

	ErrorState GetRequestState() { return m_RequestState; }
	Json::Value GetResponeData(ValueEncodeType type);
	std::string GetErrorMessage() { return m_errorMsg; }
	std::wstring GetDebugInfoString();
private:
	RequestType m_RequestType;
	ErrorState m_RequestState;
	ValueEncodeType m_ValueEncodeType;

	std::string m_ResponeData;
	std::string m_errorMsg;

	std::string m_str_url;
	std::string m_str_Json;
	curl_slist* m_headers = nullptr;

	std::ostringstream m_ostream_requestHeaderInfo;
	std::ostringstream m_ostream_requestBodyInfo;
	std::ostringstream m_ostream_responseInfo;

	Json::StyledWriter m_JsonWriter;
};