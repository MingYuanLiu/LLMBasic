#include "ThreadWorkPool.h"

namespace LLMBasic
{
    ThreadWorkPool* ThreadWorkPool::sPoolSingleInstance = nullptr;
    std::once_flag ThreadWorkPool::sOnceFlag = {};
}