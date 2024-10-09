#pragma once

#include <functional>
#include <string>
#include <future>
#include <json11.hpp>

namespace LLMBasic
{

    enum ModelType : uint8_t
    {
        Claude_3_5_Sonnect = 0,
        Claude_3_haiku,
        Claude_3_opus
    };
    
    struct ClientInitOptions
    {
        std::string ApiKey;

        std::string BaseUrl;

        std::string SystemPrompt;

        std::string CustomHeader;
        
        ModelType ModelType = Claude_3_5_Sonnect;

        int32_t MaxTokens = 1024;

        int32_t MaxHistoryMessageNums = 5;
        
        bool IsEnableContext = true; // Whether to enable context, if enabled, it will use all ask&question messages as context
    };
    
    class ClaudeClient
    {
    public:
        static const std::string API_MESSAGES;
        
        using ResponseCallback = std::function<bool(std::string& /* response string data */)>;
        
        explicit ClaudeClient(const ClientInitOptions& InitOptions)
            : InitOptions(InitOptions)
        {
        }

         std::vector<std::string> SendMessages(const std::vector<std::string>& Message);

        std::vector<std::string> SendPictureAndMessages(const std::string& ImagePath, const std::vector<std::string>& Messages);

        const std::vector<std::string>& GetLLMResponse() { return LLMResponse; }

        std::string ConvertModelTypeToModelName(ModelType Model);

        json11::Json GenerateMessageTemplate(const json11::Json::array& Messages);

        std::string ReadBinaryImageFileWithBase64(const std::string& ImagePath);
    private:
        void AppendToChatContext(const std::string& Role, const std::string& Message);

        void SendMessagesInternal(const json11::Json&Message);
        
        ClientInitOptions InitOptions;
        std::vector<std::string> LLMResponse;
        json11::Json::array ChatContext;
    };
}

