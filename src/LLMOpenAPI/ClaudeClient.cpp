#include "ClaudeClient.h"

#include "json11.hpp"
#include "../Basic/HttpRequest.h"

namespace LLMBasic
{
    const std::string ClaudeClient::API_MESSAGES = "/v1/message";

     std::vector<std::string> ClaudeClient::SendMessages(const std::vector<std::string>& Messages)
    {
        json11::Json::array ChatContextMessageArray = json11::Json::array {};
         if (InitOptions.IsEnableContext)
         {
             ChatContextMessageArray.emplace_back(ChatContext);
         }

         json11::Json::array MessageArray = json11::Json::array {};
        for (const auto& Message : Messages)
        {
            json11::Json MessageText = json11::Json::object {
                {"type", "text"},
                {"text", Message}
            };
            
            MessageArray.push_back(MessageText);
        }
         
        json11::Json JsonMessage = json11::Json::object {
                {"role", "user"},
                {"content", MessageArray}
        };

         ChatContextMessageArray.emplace_back(JsonMessage);
         
        json11::Json PostData = GenerateMessageTemplate(ChatContextMessageArray);

         LLMResponse.clear();
         SendMessagesInternal(PostData);
         
         return LLMResponse;
    }

    std::vector<std::string> ClaudeClient::SendPictureAndMessages(const std::string& ImagePath, const std::vector<std::string>& Messages)
    {
        
    }

    void ClaudeClient::SendMessagesInternal(const json11::Json& Message)
     {
         if (InitOptions.BaseUrl.empty())
         {
             return;
         }

         std::string RequestUrl = InitOptions.BaseUrl + API_MESSAGES;
         auto Request = HttpRequest::Create(RequestUrl);
         Request->AddHeader("Content-Type", "application/json");
         Request->AddHeader("anthropic-version", "2023-06-01");
         Request->AddHeader("x-api-key", InitOptions.ApiKey);
         auto Response = Request->AwaitPostRequest(Message);

         // parse response data
         if (Request->GetResponseCode() != HttpOk)
         {
             return;
         }

         auto Contents = Response["contents"].array_items();
         if (Contents.empty())
         {
             return;
         }

         auto Role = Response["role"].string_value();

         for (const auto& Content : Contents)
         {
             auto ContentType = Content["type"].string_value();
             if (ContentType != "text")
             {
                 continue;
             }

             auto ContentText = Content["text"].string_value();
             if (ContentText.empty())
             {
                 continue;
             }

             LLMResponse.push_back(ContentText);
             AppendToChatContext(Role, ContentText);
         }
     }

    std::string ClaudeClient::ConvertModelTypeToModelName(ModelType Model)
    {
        switch (Model)
        {
        case ModelType::Claude_3_opus:
            return "claude-3-opus-20240229";
        case ModelType::Claude_3_haiku:
            return "claude-3-haiku-20240307";
        case ModelType::Claude_3_5_Sonnect:
            return "claude-3-5-sonnet-20240620";
        }
    }

    json11::Json ClaudeClient::GenerateMessageTemplate(const json11::Json::array& Messages)
    {
        auto ModelName = ConvertModelTypeToModelName(InitOptions.ModelType);
        auto MaxTokens = InitOptions.MaxTokens > 0 ? InitOptions.MaxTokens : 1024;
        
        json11::Json DataTemplate = json11::Json::object {
            {"model", ModelName},
            {"max_tokens", MaxTokens},
            {"messages", Messages}
        };

        return DataTemplate;
    }

    void ClaudeClient::AppendToChatContext(const std::string& Role, const std::string& Message)
     {
         if (ChatContext.size() > static_cast<size_t>(InitOptions.MaxHistoryMessageNums))
         {
             ChatContext.erase(ChatContext.begin());
         }
         
         json11::Json JsonMessage = json11::Json::object {
                 {"role", Role},
                 {"content", Message}
         };
         
         ChatContext.push_back(JsonMessage);
     }

    std::string ClaudeClient::ReadBinaryImageFileWithBase64(const std::string& ImagePath)
    {
         
    }
}
