#include <iostream>
#include <curl/curl.h>
#include "Basic/HttpRequest.h"
#include "Basic/ThreadWorkPool.h"

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

    // test thread pool
    LLMBasic::ThreadWorkPool::Get().Init(4);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
        std::cout << "hello world 1" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
        std::cout << "hello world 2" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    
    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
        std::cout << "hello world 3" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "hello world 4" << std::endl;
    });
    
    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "hello world 5" << std::endl;
    });
    
    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "hello world 6" << std::endl;
    });
    
    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "hello world 7" << std::endl;
    });
    
    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "hello world 8" << std::endl;
    });

    LLMBasic::ThreadWorkPool::Get().AddLambdaWork([]()
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "hello world 9" << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(10));

    LLMBasic::ThreadWorkPool::Get().Shutdown();
    return 0;
}
