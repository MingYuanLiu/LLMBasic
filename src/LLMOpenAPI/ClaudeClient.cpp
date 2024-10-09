#include "ClaudeClient.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "json11.hpp"
#include "../Basic/Base64Encode.h"
#include "../Basic/HttpRequest.h"

namespace LLMBasic
{
    namespace fs = std::filesystem;
    
    const std::string ClaudeClient::API_MESSAGES = "/v1/messages";

     std::vector<std::string> ClaudeClient::SendMessages(const std::vector<std::string>& Messages)
    {
        json11::Json::array ChatContextMessageArray = json11::Json::array {};
        if (InitOptions.IsEnableContext)
        {
            for (const auto& Item : ChatContext)
            {
                ChatContextMessageArray.push_back(Item);
            }
        }

        json11::Json::array MessageArray = json11::Json::array {};
        for (const auto& Message : Messages)
        {
            json11::Json MessageText = json11::Json::object
            {
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

         AppendToChatContext(JsonMessage);
         
        json11::Json PostData = GenerateMessageTemplate(ChatContextMessageArray);

         LLMResponse.clear();
         SendMessagesInternal(PostData);
         
         return LLMResponse;
    } 

    std::vector<std::string> ClaudeClient::SendPictureAndMessages(const std::vector<std::string>& ImagePaths, const std::vector<std::string>& Messages)
    {
         json11::Json::array ChatContextMessageArray = json11::Json::array {};
         if (InitOptions.IsEnableContext) 
         {
             for (const auto& Item : ChatContext)
             {
                 ChatContextMessageArray.push_back(Item);
             }
         }
         
         json11::Json::array MessageArray = json11::Json::array {};
         for (const auto& Path : ImagePaths)
         {
             std::string Base64ImageContent = ReadBinaryImageFileWithBase64(Path);
             std::string ImageType = GetImageFileType(Path);
             if (ImageType == "unknown")
             {
                 // do not support image type
                 continue;
             }
             
             if (!Base64ImageContent.empty())
             {
                 json11::Json MessageImage = json11::Json::object
                 {
                     {"type", "image"},
                     {"source", json11::Json::object {
                         {"type", "base64"},
                         {"media_type", ImageType},
                         {"data", Base64ImageContent}
                     }}
                 };

                 MessageArray.push_back(MessageImage);
             }
         }
         
         for (const auto& Message : Messages)
         {
             json11::Json MessageText = json11::Json::object
             {
                {"type", "text"},
                {"text", Message}
             };

             MessageArray.push_back(MessageText);
         }

         json11::Json JsonMessage = json11::Json::object {
                {"role", "user"},
                {"content", MessageArray}
         };

         ChatContextMessageArray.push_back(JsonMessage);
         
         AppendToChatContext(JsonMessage);
         
         json11::Json PostData = GenerateMessageTemplate(ChatContextMessageArray);

         auto StrData = PostData.dump();
         std::cout << StrData << std::endl;
         
         LLMResponse.clear();
         SendMessagesInternal(PostData);
         
         return LLMResponse;
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

         auto Contents = Response["content"].array_items();
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
        case Claude_3_opus:
            return "claude-3-opus-20240229";
        case Claude_3_haiku:
            return "claude-3-haiku-20240307";
        case Claude_3_5_Sonnect:
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
            {"system", InitOptions.SystemPrompt},
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

    void ClaudeClient::AppendToChatContext(const json11::Json& JsonMessage)
    {
         if (ChatContext.size() > static_cast<size_t>(InitOptions.MaxHistoryMessageNums))
         {
             ChatContext.erase(ChatContext.begin());
         }

         ChatContext.push_back(JsonMessage);
    }

    std::string ClaudeClient::ReadBinaryImageFileWithBase64(const std::string& ImagePath)
    {
         fs::path ImageFilePath = ImagePath;
         if (!fs::exists(ImageFilePath))
         {
             return "";
         }

         std::ifstream FileStream(ImageFilePath, std::ifstream::binary);
         
         if (!FileStream.is_open())
         {
             return "";
         }

         FileStream.seekg(0, FileStream.end);
         int FileLen = FileStream.tellg();
         FileStream.seekg(0, FileStream.beg);

         std::vector<char> ReadBuffer(FileLen);
         FileStream.read(ReadBuffer.data(), FileLen);
         
         FileStream.close();
         
         return Base64Encoder::Encode(ReadBuffer);
    }

    bool ClaudeClient::CompareStringWithIgnoreCase(const std::string& str1, const std::string& str2)
    {
         if (str1.size() != str2.size())
             return false;

         for(size_t i = 0; i < str1.size(); ++i)
         {
             if (std::tolower(str1[i]) != std::tolower(str2[i]))
             {
                 return false;
             }
         }

         return true;
    }

    std::string ClaudeClient::GetImageFileType(const std::string& ImagePath)
    {
         fs::path FilePath = ImagePath;
         std::string extension = FilePath.extension().string();
         if (CompareStringWithIgnoreCase(extension, ".png"))
             return "image/png";

         if (CompareStringWithIgnoreCase(extension, ".jpeg"))
             return "image/jpeg";

         if (CompareStringWithIgnoreCase(extension, ".jpg"))
             return "image/jpeg";

         if (CompareStringWithIgnoreCase(extension, ".gif"))
             return "image/gif";

         if (CompareStringWithIgnoreCase(extension, ".webp"))
             return "image/webp";

         return "unknown";
    }
}
