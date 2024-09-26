#pragma once
#include <functional>
#include <memory>
#include <string>
#include "json11.hpp"

namespace LLMBasic
{
    enum HttpResponseCode : uint8_t
    {
        HttpOk,
        Http404,
        Http400
    };
    
    class HttpRequest
    {
    public:
        using ResponseCallback = std::function<void(HttpResponseCode, const json11::Json&)>;
        
        static std::shared_ptr<HttpRequest> Create();

        void SetHeader(const std::string& HttpHeader);
        
        HttpResponseCode SendMessage(const json11::Json& RequestData, ResponseCallback&& Callback);

        json11::Json GetResponse();

        // 
        json11::Json AwaitSendMessage(const json11::Json& RequestData);

        void AddResponseCallback(ResponseCallback&& Callback);
        
    private:
        json11::Json ResponseData;

        std::vector<ResponseCallback> Callbacks;
    };
}


