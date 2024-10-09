#pragma once

#include <string>
#include <vector>

namespace LLMBasic
{
    class Base64Encoder
    {
    public:
        static std::string Encode(const std::vector<char>& Input);

        static std::string Decode(const std::vector<char>& Input);

    private:
        static bool IsBase64(const char code);
        
        static std::string Base64EncodeTable;
    };

}

