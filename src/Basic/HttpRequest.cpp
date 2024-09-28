#include "HttpRequest.h"

#include "ThreadWorkPool.h"

#define POST_LARGE_SIZE_THRESHOLD 5242880 // 5MB

namespace LLMBasic
{
    std::shared_ptr<HttpRequest> HttpRequest::Create(const std::string& Url)
    {
        return std::make_shared<HttpRequest>(Url);
    }
    
    void HttpRequest::SendRequest(const json11::Json& RequestData, HttpRequest::ResponseCallback&& Callback)
    {
        Callbacks.push_back(Callback);
        
        ThreadWorkPool::Get().AddLambdaWork([this, RequestData]() -> HttpResponseCode
        {
            auto Code = CurlPostRequest(RequestData);
            for (const auto& Callback : Callbacks)
            {
                Callback(Code, ResponseData);
            }
            return Code;
        });
    }

    json11::Json HttpRequest::AwaitSendRequest(const json11::Json& RequestData)
    {
        std::future<HttpResponseCode> Result = ThreadWorkPool::Get().AddLambdaWork([this, RequestData]() -> HttpResponseCode
        {
            return CurlPostRequest(RequestData);
        });

        auto RespCode = Result.get();
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
        {
            std::string RequestStrData = RequestData.dump();
            CurlData.DataPtr = RequestStrData.c_str();
            CurlData.DataSizeSendLeft = RequestStrData.size();

            curl_easy_setopt(CurlHandle, CURLOPT_READFUNCTION, &CurlRequestSendDataReadCallback);
            curl_easy_setopt(CurlHandle, CURLOPT_READDATA, &CurlData);
        }
        
        // setup response callback
        {
            std::string RespData;
            curl_easy_setopt(CurlHandle, CURLOPT_WRITEFUNCTION, &CurlRequestWriteCallback);
            curl_easy_setopt(CurlHandle, CURLOPT_WRITEDATA, static_cast<void*>(&RespData));
        }
        
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

    size_t HttpRequest::CurlRequestWriteCallback(void* RecvData, size_t DataSize, size_t nMem, void* UserData)
    {
        
    }

    size_t HttpRequest::CurlRequestSendDataReadCallback(void* SendData, size_t DataSize, size_t nMem, void* UserData)
    {
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

    HttpResponseCode HttpRequest::ConvertCurlCodeToHttpErrorCode(const CURLcode& CurlCode)
    {
        return HttpOk;
    }
}
