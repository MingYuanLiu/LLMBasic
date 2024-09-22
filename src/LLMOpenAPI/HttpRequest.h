#pragma once
#include <memory>

namespace LLMBasic
{
    class HttpRequest
    {
    public:
        static std::shared_ptr<HttpRequest> CreateHttpRequest();
    };
}


