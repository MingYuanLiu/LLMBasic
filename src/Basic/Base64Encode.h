#pragma once

#include <string>

namespace LLMBasic
{
    class Base64Encoder
    {
    public:
        std::string Encode(const std::string& Input);

        std::string Decode(const std::string& Input);

    private:
        bool IsBase64(const char code);
        
        static std::string Base64EncodeTable;
    };

}

