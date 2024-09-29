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
        
        static std::shared_ptr<HttpRequest> Create(const std::string& Url);

        HttpRequest& AddHeader(const std::string& HttpHeader)
        {
            Headers.push_back(HttpHeader);
            return *this;
        }
        
        void SendRequest(const json11::Json& RequestData, ResponseCallback&& Callback);

        json11::Json GetResponse() { return ResponseData; }

        std::string GetErrorMessage() { return ErrorMsg; }

        json11::Json AwaitSendRequest(const json11::Json& RequestData);

        void AddResponseCallback(ResponseCallback&& Callback) { Callbacks.push_back(std::forward<ResponseCallback>(Callback)); }

        void SetResponseWithHeaderData(bool WithHeader) { IsResponseWithHeaderData = WithHeader;  }
        

        explicit HttpRequest(const std::string& Url)
            : Headers(std::vector<std::string>()),
            ResponseData(json11::Json()),
            RequestUrl(Url),
            IsResponseWithHeaderData(false),
            Callbacks(std::vector<ResponseCallback>())
        {}
    private:
        
        HttpResponseCode CurlPostRequest(const json11::Json& RequestData);

        /*
        size_t CurlRequestWriteCallback(void* RecvData, size_t DataSize, size_t nMem, void* UserData);

        // read callback of sending data
        size_t CurlRequestSendDataReadCallback(void* SendData, size_t DataSize, size_t nMem, void* UserData);*/

        HttpResponseCode ConvertCurlCodeToHttpErrorCode(const CURLcode& CurlCode);
        
        std::vector<std::string> Headers;
        
        json11::Json ResponseData;

        std::string RequestUrl;

        std::string ErrorMsg;

        bool IsResponseWithHeaderData;

        std::vector<ResponseCallback> Callbacks;
    };
}


