#include "HttpRequest.h"

#include "ThreadWorkPool.h"

#define POST_LARGE_SIZE_THRESHOLD 5242880 // 5MB

namespace LLMBasic
{
    struct CurlSendData
    {
        const char* DataPtr;
        size_t DataSizeSendLeft;
    };
    
    size_t CurlRequestWriteCallback(void* RecvData, size_t DataSize, size_t nMem, void* UserData)
    {
        if (UserData == nullptr || RecvData == nullptr || DataSize == 0)
            return 0;

        size_t BufferSize = DataSize * nMem;
        std::string *Buffer = (std::string*)UserData;
        if (Buffer != nullptr)
        {
            Buffer->append(static_cast<const char*>(RecvData), BufferSize);
        }
        
        return BufferSize;
    }

    static size_t CurlRequestSendDataReadCallback(void* SendData, size_t DataSize, size_t nMem, void* UserData)
    {
        if (UserData == nullptr || SendData == nullptr || DataSize == 0)
            return 0;
        
        CurlSendData* DataPtr = static_cast<CurlSendData*>(UserData);
        size_t BufferSize = DataSize * nMem;

        if (DataPtr->DataSizeSendLeft)
        {
            size_t ThisTimeCopySize = DataPtr->DataSizeSendLeft;
            if (ThisTimeCopySize > BufferSize)
            {
                ThisTimeCopySize = BufferSize;
            }

            memcpy(SendData, DataPtr->DataPtr, ThisTimeCopySize);
            DataPtr->DataPtr += ThisTimeCopySize;
            DataPtr->DataSizeSendLeft -= ThisTimeCopySize;

            return ThisTimeCopySize;
        }

        return 0;
    }

    void HttpRequest::PostRequest(const json11::Json& RequestData, HttpRequest::ResponseCallback&& Callback)
    {
        Callbacks.push_back(Callback);
        
        ThreadWorkPool::Get().AddLambdaWork([this, RequestData]()
        {
            auto Code = CurlPostRequest(RequestData);
            for (const auto& Callback : Callbacks)
            {
                Callback(Code, ResponseData);
            }
        });
    }

    json11::Json HttpRequest::AwaitPostRequest(const json11::Json& RequestData)
    {
        std::future<HttpResponseCode> Result = ThreadWorkPool::Get().AddLambdaWork([this, RequestData]() -> HttpResponseCode
        {
            return CurlPostRequest(RequestData);
        });

        ResponseCode = Result.get();
        return ResponseData;
    }

    HttpResponseCode HttpRequest::CurlPostRequest(const json11::Json& RequestData)
    {
        CURL* CurlHandle = curl_easy_init();
        if (!CurlHandle)
        {
            return CurlInitError;
        }

#ifdef CA_Certificate_Path
        curl_easy_setopt(CurlHandle, CURLOPT_CAINFO, CA_Certificate_Path);
#endif
        
        curl_easy_setopt(CurlHandle, CURLOPT_POST, 1);
        curl_easy_setopt(CurlHandle, CURLOPT_URL, RequestUrl.c_str());

        // setup header
        curl_slist* HeaderList = nullptr;
        {
            for (auto& H : Headers)
            {
                HeaderList = curl_slist_append(HeaderList, H.c_str());
            }
            if (HeaderList)
            {
                curl_easy_setopt(CurlHandle, CURLOPT_HTTPHEADER, HeaderList);
            }
        }
        
        // setup send data read callback
        CurlSendData CurlData;
        std::string RequestStrData = RequestData.dump();
        CurlData.DataPtr = RequestStrData.c_str();
        CurlData.DataSizeSendLeft = RequestStrData.size();
        std::cout << "request data: " << RequestStrData << std::endl;

        curl_easy_setopt(CurlHandle, CURLOPT_READFUNCTION, CurlRequestSendDataReadCallback);
        curl_easy_setopt(CurlHandle, CURLOPT_READDATA, static_cast<void*>(&CurlData));
        
        // setup response callback
        std::string RespData;
        curl_easy_setopt(CurlHandle, CURLOPT_WRITEFUNCTION, CurlRequestWriteCallback);
        curl_easy_setopt(CurlHandle, CURLOPT_WRITEDATA, static_cast<void*>(&RespData));
        
        
        if (IsResponseWithHeaderData)
        {
            curl_easy_setopt(CurlHandle, CURLOPT_HEADER, 1);
        }

        // for debug
        curl_easy_setopt(CurlHandle, CURLOPT_VERBOSE, 1L);

        // set post size
        if (CurlData.DataSizeSendLeft > POST_LARGE_SIZE_THRESHOLD/* 5MB */)
        {
            curl_easy_setopt(CurlHandle, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<long>(CurlData.DataSizeSendLeft));
        }
        else
        {
            curl_easy_setopt(CurlHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(CurlData.DataSizeSendLeft));
        }
        
        CURLcode RespCode = curl_easy_perform(CurlHandle);
        std::string ParseError;
        ResponseData = json11::Json::parse(RespData, ParseError);
        if (!ParseError.empty())
        {
            std::cout << "parse response data failed: " << ParseError << std::endl;
        }
        
        if (RespCode != CURLE_OK)
        {
            ErrorMsg = curl_easy_strerror(RespCode);
        }
        
        HttpResponseCode RetCode = ConvertCurlCodeToHttpErrorCode(RespCode);
        
        if (HeaderList)
        {
            curl_slist_free_all(HeaderList);
        }

        curl_easy_cleanup(CurlHandle);
            
        return RetCode;
    }

    HttpResponseCode HttpRequest::ConvertCurlCodeToHttpErrorCode(const CURLcode& CurlCode)
    {
        return HttpOk;
    }
}
