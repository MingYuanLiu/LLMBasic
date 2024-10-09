#pragma once
#include <functional>
#include <memory>
#include <string>

#include "json11.hpp"
#include "curl/curl.h"

namespace LLMBasic
{
    enum HttpResponseCode : uint8_t
    {
        HttpOk,
        Http404,
        Http400,
        CurlInitError,
    };
    
    class HttpRequest
    {
    public:
        using ResponseCallback = std::function<void(HttpResponseCode, const json11::Json&)>;
        
        static std::shared_ptr<HttpRequest> Create(const std::string& Url)
        {
            return std::make_shared<HttpRequest>(Url);
        }

        void AddHeader(const std::string& HttpHeader)
        {
            Headers.push_back(HttpHeader);
        }

        void AddHeader(std::string HeaderKey, std::string HeaderValue)
        {
            std::string HttpHeader = HeaderKey + ":" + HeaderValue;
            Headers.push_back(HttpHeader);
        }
        
        void PostRequest(const json11::Json& RequestData, ResponseCallback&& Callback);

        json11::Json GetResponse() { return ResponseData; }

        HttpResponseCode GetResponseCode() { return ResponseCode; }

        std::string GetErrorMessage() { return ErrorMsg; }

        json11::Json AwaitPostRequest(const json11::Json& RequestData);

        void AddResponseCallback(ResponseCallback&& Callback) { Callbacks.push_back(std::forward<ResponseCallback>(Callback)); }

        void SetResponseWithHeaderData(bool WithHeader) { IsResponseWithHeaderData = WithHeader;  }

    private:
        explicit HttpRequest(const std::string& Url)
            : Headers(std::vector<std::string>()),
            ResponseData(json11::Json()),
            RequestUrl(Url),
            IsResponseWithHeaderData(false),
            Callbacks(std::vector<ResponseCallback>())
        {}
        
        HttpResponseCode CurlPostRequest(const json11::Json& RequestData);

        HttpResponseCode ConvertCurlCodeToHttpErrorCode(const CURLcode& CurlCode);
        
        std::vector<std::string> Headers;

        HttpResponseCode ResponseCode;
        
        json11::Json ResponseData;

        std::string RequestUrl;

        std::string ErrorMsg;

        bool IsResponseWithHeaderData;

        std::vector<ResponseCallback> Callbacks;
    };
}


