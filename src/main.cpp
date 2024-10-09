#include <iostream>
#include <curl/curl.h>
#include "Basic/HttpRequest.h"
#include "Basic/ThreadWorkPool.h"
#include "LLMOpenAPI/ClaudeClient.h"

int main() {
    /**
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://curl.se/docs/caextract.html");

#ifdef CA_Certificate_Path
        curl_easy_setopt(curl, CURLOPT_CAINFO, CA_Certificate_Path);
#endif
        
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    */

    /*
   

    auto Request = LLMBasic::HttpRequest::Create("https://api.gptsapi.net/v1/messages");
    Request->AddHeader("content-type:application/json");
    Request->AddHeader("anthropic-version:2023-06-01");
    Request->AddHeader("x-api-key:sk-H2p1b94d796bf78fe015e24fca6fa99857e527e0187qtmvi");

    json11::Json::array MessageArray = json11::Json::array {
        json11::Json::object {
            {"role", "user"},
            {"content", "What do UFSFiles refer to in the build products of the UE game engine?"}
        },
    };
    json11::Json SendData =  json11::Json::object {
        {"model", "claude-3-5-sonnet-20240620"},
        {"max_tokens", 1024},
        {"messages", MessageArray}
    };
    
    json11::Json Data = Request->AwaitSendRequest(SendData);
    std::cout << Data.dump() << std::endl;*/
    LLMBasic::ThreadWorkPool::Get().Init(4);
    
    LLMBasic::ClientInitOptions Opt;
    Opt.ApiKey = "sk-H2p1b94d796bf78fe015e24fca6fa99857e527e0187qtmvi";
    Opt.BaseUrl = "https://api.gptsapi.net";
    Opt.SystemPrompt = "You are a house seller";
    Opt.ModelType = LLMBasic::Claude_3_5_Sonnect;
    Opt.MaxTokens = 4096;

    std::string ImagePath = "H:\\Workspace\\AI_UI\\LLMBasic\\build\\Debug\\images.jpg";
    LLMBasic::ClaudeClient Client(Opt);
    auto Resp = Client.SendPictureAndMessages({ImagePath}, {"Convert the UI in the image into UMG code"});

    if (Resp.size() > 0)
        std::cout << Resp[0] << std::endl;

    LLMBasic::ThreadWorkPool::Get().Shutdown();
    return 0;
}
