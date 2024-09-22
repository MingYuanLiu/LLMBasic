#pragma once

#include <functional>
#include <string>

namespace LLMBasic
{

    struct ClientInitOptions
    {
        std::string ApiKey;

        std::string BaseUrl;

        std::string SystemPrompt;

        std::string CustomHeader;
    };
    
    class ClaudeClient
    {
    public:
        static constexpr std::string API_MESSAGE = "/v1/message";
        

        using ResponseCallback = std::function<bool(std::string& /* response string data */)>;
        
        explicit ClaudeClient(const ClientInitOptions& ApiKey);

        void SendMessage(const std::string& Message, ResponseCallback&& ResponseAsyncCallback);
    };
}

