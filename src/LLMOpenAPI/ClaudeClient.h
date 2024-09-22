#pragma once

#include <string>

namespace LLMBasic
{
    class ClaudeClient
    {
    public:
        explicit ClaudeClient(const std::string& ApiKey);

        // domain url: 
        void SwitchDomainUrl(const std::string& DomainUrl);

        
    };
}

