﻿#pragma once
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <vcruntime.h>
#include <thread>

#include "HttpRequest.h"

namespace LLMBasic
{

    template<typename T>
    struct FunctionTraits;

    template<typename C, typename RetType, typename ...ParamTypes>
    struct FunctionTraits<RetType(__cdecl C::*)(ParamTypes...) const>
    {
        typedef RetType ReturnType;
        typedef std::tuple<ParamTypes...> ArgTupleType;
    };
    
    template<typename RetType, typename ...ParamTypes>
    struct FunctionTraits<RetType(*)(ParamTypes...)>
    {
        typedef RetType ReturnType;
        typedef std::tuple<ParamTypes...> ArgTupleType;
    };
    
    template<typename T>
    class SafeQueue
    {
    public:
        SafeQueue() = default;
        SafeQueue(const SafeQueue&) = default;
        SafeQueue(SafeQueue&&) = default;
        SafeQueue& operator=(SafeQueue&) = delete;
        SafeQueue& operator=(SafeQueue&&) = delete;
        ~SafeQueue() = default;

        bool Empty()
        {
            std::unique_lock<std::mutex> Lock(mQueueMtx);
            return mQueue.empty(); 
        }

        int Size()
        {
            std::unique_lock<std::mutex> Lock(mQueueMtx);
            return mQueue.size();
        }
        
        void Enqueue(T& Val)
        {
            std::unique_lock<std::mutex> lock(mQueueMtx);
            mQueue.push(Val);
        }

        bool Dequeue(T& OutVal)
        {
            std::unique_lock<std::mutex> lock(mQueueMtx);
            if (mQueue.empty())
            {
                return false;
            }

            OutVal = std::move(mQueue.front());
            mQueue.pop();
            return true;
        }
        
    private:
        std::mutex mQueueMtx;
        std::queue<T> mQueue;
    };
    
    class ThreadWorkPool
    {
        class ThreadWork
        {
            ThreadWorkPool* ThreadPool;
            int32_t ThreadId;
        public:
            ThreadWork(ThreadWorkPool* InThreadPool, int32_t Id)
                : ThreadPool(InThreadPool), ThreadId(Id)
            {}

            void operator()() const
            {
                std::function<void()> ExecFunc;
                while(ThreadPool != nullptr && !ThreadPool->IsShutdown)
                {
                    std::unique_lock<std::mutex> Lock(ThreadPool->ConditionalVariableMtx);
                    if (ThreadPool->WorkQueue.Empty())
                    {
                        std::cout << "Thread " << ThreadId << " is waiting" << std::endl;
                        ThreadPool->ConditionLock.wait(Lock);
                    }

                    std::cout << "Thread " << ThreadId << " is RUNNING" << std::endl;
                    
                    if (ThreadPool->WorkQueue.Dequeue(ExecFunc))
                        ExecFunc();
                }

            }
        };

        
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

        void Init(int32_t PoolSize)
        {
            WorkThreads = std::vector<std::thread>(PoolSize);
            for (int32_t i = 0; i < PoolSize; ++i)
            {
                WorkThreads[i] = std::thread(ThreadWork(this, i));
            }

            IsInitialized = true;
        }

        void Shutdown()
        {
            if (!IsInitialized) return;

            ConditionLock.notify_all();
            IsShutdown = true;

            for (auto& Thread : WorkThreads)
            {
                if (Thread.joinable())
                    Thread.join();
            }
        }

        template<typename LambdaType>
        auto AddLambdaWork(LambdaType&& Lambda) -> std::future<typename FunctionTraits<decltype(&LambdaType::operator())>::ReturnType>
        {
            typedef typename FunctionTraits<decltype(&LambdaType::operator())>::ReturnType LambdaRetType;
            
            std::function<LambdaRetType()> Func = std::bind(std::forward<LambdaType>(Lambda));
            auto TaskPtr = std::make_shared<std::packaged_task<LambdaRetType()>>(Func);
            std::function<void()> WrapperFunc = [TaskPtr](){ (*TaskPtr)(); };

            WorkQueue.Enqueue(WrapperFunc);

            ConditionLock.notify_one();
            
             return TaskPtr->get_future(); 
        }

        template<typename F, typename... Args>
        auto AddWork(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
        {
            std::function<decltype(f(args...))()> Func = std::bind(std::forward<F>(f), std::forward<Args>(args...));
            auto TaskPtr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(Func);
            std::function<void()> WrapperFunc = [TaskPtr](){ (*TaskPtr)(); };

            WorkQueue.Enqueue(WrapperFunc);
            ConditionLock.notify_one();
            return TaskPtr->get_future();
        }

        bool IsInit() { return IsInitialized; }

    private:
        ThreadWorkPool() = default;

        SafeQueue<std::function<void()> > WorkQueue;
        std::vector<std::thread> WorkThreads;

        std::mutex ConditionalVariableMtx;
        std::condition_variable ConditionLock;

        std::atomic_bool IsInitialized;
        std::atomic_bool IsShutdown;

        
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

