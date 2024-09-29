
#if 0
#define SUCCESS_CODE 0
#define EARLY_TERMINATION_COdE 0x100

class TerminateException : public std::runtime_error
{
public:
    TerminateException(const FString& Message)
        : std::runtime_error(TCHAR_TO_UTF8(*Message))
    {
    }
};

template<typename LambdaType>
struct BindDelegateLambdaWithEvent;

template<typename T, typename RetType, typename ...ParamTypes>
struct BindDelegateLambdaWithEvent<RetType(__cdecl T::*)(ParamTypes...) const>
{
    template<typename DelegateType>
    BindDelegateLambdaWithEvent(DelegateType* Delegate, FEvent* WaitEvent, int* ErrorCode, T LambdaFunc)
    {
        checkSlow(Delegate != nullptr);
        
        Delegate->BindLambda([WaitEvent, LambdaFunc, ErrorCode](ParamTypes... Args)
            {
                try
                {
                    *ErrorCode = SUCCESS_CODE;
                    /*
                    TTuple<ParamTypes...> Tuple = MakeTuple(Forward<ParamTypes>(Args)...);
                    Tuple.ApplyAfter(LambdaFunc);
                    */
                    LambdaFunc(Forward<ParamTypes>(Args)...);
                } catch (const TerminateException& e)
                {
                    *ErrorCode = EARLY_TERMINATION_COdE;
                    UE_LOG(LogTemp, Warning, TEXT("AsyncCaller -- TerminateException: %s"), UTF8_TO_TCHAR(e.what()));
                }
                WaitEvent->Trigger();
            });
    }
};

/**
 * Support asynchronous chain calls
 * example:
 * {
        UDynamicAssetsLoader* Loader = NewObject<UDynamicAssetsLoader>();
   
        AsyncCallerCreator<FOnPakDownloadCompleteNative>::NewAsyncCaller(Loader, &UDynamicAssetsLoader::TestAsyncCaller, TEXT("hello"), TEXT("world"))
        ->Then([](EDynamicLoadResult Result, const UObject* PrivateData)
        {
        
        }, AsyncCallerCreator<FOnPakDownloadCompleteNative>::NewAsyncCaller(Loader, &UDynamicAssetsLoader::TestAsyncCaller, TEXT("hello"), TEXT("world chain1")))
        ->Then([](EDynamicLoadResult Result, const UObject* PrivateData)
        {
            UE_LOG(LogDynamicAssetsLoader, Warning, TEXT("Lambda func with chain1 callers"));

            AsyncTerminate::Terminate(TEXT("Test Terminate")); // terminate caller
        
        }, AsyncCallerCreator<FOnPakDownloadCompleteNative>::NewAsyncCaller(Loader, &UDynamicAssetsLoader::TestAsyncCaller, TEXT("hello"), TEXT("world chain2")))
        ->Then([](EDynamicLoadResult Result, const UObject* PrivateData)
        {
        })
        ->Catch([]()
        {
            UE_LOG(LogDynamicAssetsLoader, Warning, TEXT("Catch func"));
        });
 * }
 *
 * note: Method to be called must has delegate object at the first parameter, like this:
 *       "void UDynamicAssetsLoader::TestAsyncCaller(const FOnPakDownloadCompleteNative& delegate, const FString& Args1, const FString& Args2)"
 * @tparam DelegateType Delegate type used by function caller
 * @tparam UserClass Object type of function caller
 * @tparam FuncType Function type
 * @tparam VarTypes Multiple variable types
 */
template<typename DelegateType, typename UserClass, typename FuncType, typename ...VarTypes>
class AsyncCaller
{
public:
    AsyncCaller(UserClass* InObjectPtr, FuncType InFuncPtr, VarTypes&& ...Vars)
        :UserObjectPtr(InObjectPtr), MethodPtr(InFuncPtr), Args(Forward<VarTypes>(Vars)...)
    {
        check(InObjectPtr != nullptr && InFuncPtr != nullptr);
    }

    ~AsyncCaller() {}

    template<typename RetCallerType, typename LambdaFuncType>
    RetCallerType Then(LambdaFuncType&& LambdaFunc, RetCallerType NextCaller)
    {
        if (bEarlyTerminate)
        {
            if(NextCaller)
            {
                NextCaller->SetEarlyTerminate();
            }

            bEarlyTerminate = false;
            return NextCaller;
        }
        
        FEvent* WaitEvent = FPlatformProcess::GetSynchEventFromPool();

        int ErrorCode = SUCCESS_CODE;
        
        BindDelegateLambdaWithEvent<decltype(&LambdaFuncType::operator())> BindDelegateObject(&this->CallbackDelegate,
            WaitEvent, &ErrorCode, LambdaFunc);
    
        using MutableUserClass = std::remove_const_t<UserClass>;

        // Safely remove const to work around a compiler issue with instantiating template permutations for 
        // overloaded functions that take a function pointer typedef as a member of a templated class.  In
        // all cases where this code is actually invoked, the UserClass will already be a const pointer.
        MutableUserClass* MutableUserObject = const_cast<MutableUserClass*>(UserObjectPtr);
        checkSlow(MethodPtr != nullptr);
    
        (void)this->Args.ApplyAfter(MethodPtr, MutableUserObject, CallbackDelegate);
        
        // wait exec finished
        WaitEvent->Wait(MaxCallTimeOut);
        FPlatformProcess::ReturnSynchEventToPool(WaitEvent);

        if (ErrorCode == EARLY_TERMINATION_COdE)
        {
            if(NextCaller)
            {
                NextCaller->SetEarlyTerminate();
            }
        }

        return NextCaller;
    }

    template<typename LambdaFuncType>
    AsyncCaller* Then(LambdaFuncType&& LambdaFunc)
    {
        if (bEarlyTerminate)
        {
            bEarlyTerminate = false;
            return this;
        }
        
        FEvent* WaitEvent = FPlatformProcess::GetSynchEventFromPool();
        
        int ErrorCode = SUCCESS_CODE;
        BindDelegateLambdaWithEvent<decltype(&LambdaFuncType::operator())> BindDelegateObject(&this->CallbackDelegate, WaitEvent, &ErrorCode, LambdaFunc);

        using MutableUserClass = std::remove_const_t<UserClass>;
        MutableUserClass* MutableUserObject = const_cast<MutableUserClass*>(UserObjectPtr);

        checkSlow(MethodPtr != nullptr);
        
        (void)this->Args.ApplyAfter(MethodPtr, MutableUserObject, CallbackDelegate);

        // wait exec finished
        WaitEvent->Wait(MaxCallTimeOut);
        FPlatformProcess::ReturnSynchEventToPool(WaitEvent);
        
        return this;
    }

    template<typename LambdaType>
    AsyncCaller* Catch(LambdaType LambdaFunc)
    {
        return this;
    }

    template<typename LambdaType>
    AsyncCaller* Finally(LambdaType LambdaFunc)
    {
        return this;
    }

    void SetEarlyTerminate()
    {
        bEarlyTerminate = true;
    }
    
    static constexpr uint64 MaxCallTimeOut = 10 * 1000; // 10s

protected:
    UserClass* UserObjectPtr;
    FuncType MethodPtr;
    TTuple<VarTypes...> Args;

    DelegateType CallbackDelegate;
    bool bEarlyTerminate = false;
};

template<typename DelegateType>
struct AsyncCallerCreator
{
    template<typename UserClassToNew, typename FuncTypeToNew, typename ...VarTypesToNew>
    static TSharedPtr<AsyncCaller<DelegateType, UserClassToNew, FuncTypeToNew, VarTypesToNew...>> NewAsyncCaller(UserClassToNew* ObjectPtr, FuncTypeToNew FuncPtr, VarTypesToNew&& ...Vars)
    {
        return MakeShared<AsyncCaller<DelegateType, UserClassToNew, FuncTypeToNew, VarTypesToNew...> >(ObjectPtr, FuncPtr, Forward<VarTypesToNew>(Vars)...);
    }
};

struct AsyncTerminate
{
    static void Terminate(const FString& Message)
    {
        throw TerminateException(Message);
    }
};

#endif