#pragma once
#include <mutex>
#include <vcruntime.h>


namespace LLMBasic
{

    class IThreadWork
    {
    public:
        virtual ~IThreadWork() {}

        virtual void Run() = 0;
    };
    
    class ThreadWorkPool
    {
    public:
        static ThreadWorkPool& Get()
        {
            std::call_once(sOnceFlag, &ThreadWorkPool::InitSingleInstance);
            return *sPoolSingleInstance;
        }

        ~ThreadWorkPool()
        {
            if (sPoolSingleInstance)
            {
                delete sPoolSingleInstance;
                sPoolSingleInstance = nullptr;
            }
        }

        void EnqueueWorks();

    private:
        ThreadWorkPool() = default;

        static void InitSingleInstance()
        {
            if (!sPoolSingleInstance)
            {
                sPoolSingleInstance = new ThreadWorkPool();
            }
        }
        
        static ThreadWorkPool* sPoolSingleInstance;
        static std::once_flag sOnceFlag;
    };

}

