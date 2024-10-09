#include "Base64Encode.h"
    
#include <iostream>

#include <cstring>
#include <fstream>
#include <malloc.h>

namespace LLMBasic
{
    std::string Base64Encoder::Base64EncodeTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    bool Base64Encoder::IsBase64(const char code)
    {
        return (isalnum(code) || (code == '+') || (code == '/'));
    }
 
    std::string Base64Encoder::Encode(const std::vector<char>& Input)
    {
        std::string Result;
        size_t InputLen = Input.size();
        int i = 0;
        int j = 0;
        uint8_t ArrayWithLengthThree[3];
        uint8_t ArrayWithLengthFour[4];
 
        while (InputLen--)
        {
            ArrayWithLengthThree[i++] = Input[InputLen];
            if(i == 3)
            {
                ArrayWithLengthFour[0] = (ArrayWithLengthThree[0] & 0xfc) >> 2;
                ArrayWithLengthFour[1] = static_cast<uint8_t>((ArrayWithLengthThree[0] & 0x03) << 4) + static_cast<uint8_t>((ArrayWithLengthThree[1] & 0xf0) >> 4);
                ArrayWithLengthFour[2] = static_cast<uint8_t>((ArrayWithLengthThree[1] & 0x0f) << 2) + static_cast<uint8_t>((ArrayWithLengthThree[2] & 0xc0) >> 6);
                ArrayWithLengthFour[3] = ArrayWithLengthThree[2] & 0x3f;
                for(i = 0; i <4 ; i++)
                {
                    Result += Base64EncodeTable[ArrayWithLengthFour[i]];
                }
                
                i = 0;
            }
        }
        
        if(i)
        {
            for(j = i; j < 3; j++)
            {
                ArrayWithLengthThree[j] = '\0';
            }
 
            ArrayWithLengthFour[0] = (ArrayWithLengthThree[0] & 0xfc) >> 2;
            ArrayWithLengthFour[1] = static_cast<uint8_t>((ArrayWithLengthThree[0] & 0x03) << 4) + ((ArrayWithLengthThree[1] & 0xf0) >> 4);
            ArrayWithLengthFour[2] = static_cast<uint8_t>((ArrayWithLengthThree[1] & 0x0f) << 2) + ((ArrayWithLengthThree[2] & 0xc0) >> 6);
            ArrayWithLengthFour[3] = ArrayWithLengthThree[2] & 0x3f;
 
            for(j = 0; j < i + 1; j++)
            {
                Result += Base64EncodeTable[ArrayWithLengthFour[j]];
            }
 
            while(i++ < 3)
            {
                Result += '=';
            }
        }
        
        return Result;
    }
        
    std::string Base64Encoder::Decode(const std::vector<char>& Input)
    {
        int InputLen = static_cast<int>(Input.size());
        int i = 0;
        int j = 0;
        int Index = 0;
        uint8_t ArrayWithLengthFour[4], ArrayWithLengthThree[3];
        std::string DecodeRes;
     
        while (InputLen-- &&  Input[Index] != '=' && IsBase64(Input[Index]))
        {
            ArrayWithLengthFour[i++] = Input[Index++];
            if (i ==4)
            {
                for (i = 0; i <4; i++)
                    ArrayWithLengthFour[i] = Base64EncodeTable[static_cast<char>(ArrayWithLengthFour[i])];
     
                ArrayWithLengthThree[0] = static_cast<uint8_t>(ArrayWithLengthFour[0] << 2) + ((ArrayWithLengthFour[1] & 0x30) >> 4);
                ArrayWithLengthThree[1] = static_cast<uint8_t>((ArrayWithLengthFour[1] & 0xf) << 4) + ((ArrayWithLengthFour[2] & 0x3c) >> 2);
                ArrayWithLengthThree[2] = static_cast<uint8_t>((ArrayWithLengthFour[2] & 0x3) << 6) + ArrayWithLengthFour[3];
     
                for (i = 0; i < 3; i++)
                    DecodeRes += static_cast<char>(ArrayWithLengthThree[i]);
                
                i = 0;
            }
        }
        if (i)
        {
            for (j = i; j <4; j++)
                ArrayWithLengthFour[j] = 0;
     
            for (j = 0; j <4; j++)
                ArrayWithLengthFour[j] = Base64EncodeTable[static_cast<char>(ArrayWithLengthFour[j])];
     
            ArrayWithLengthThree[0] = static_cast<uint8_t>(ArrayWithLengthFour[0] << 2) + ((ArrayWithLengthFour[1] & 0x30) >> 4);
            ArrayWithLengthThree[1] = static_cast<uint8_t>((ArrayWithLengthFour[1] & 0xf) << 4) + ((ArrayWithLengthFour[2] & 0x3c) >> 2);
            ArrayWithLengthThree[2] = static_cast<uint8_t>((ArrayWithLengthFour[2] & 0x3) << 6) + ArrayWithLengthFour[3];
     
            for (j = 0; (j < i - 1); j++) DecodeRes += static_cast<char>(ArrayWithLengthThree[j]);
        }
     
        return DecodeRes;
    }
}
