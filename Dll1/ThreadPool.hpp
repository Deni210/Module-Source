#pragma once

#include <Windows.h>
#include <functional>
#include <vector>
#include <memory>
#include <thread>
#include <iostream>

template <typename T>
struct STraits;

template <typename R, typename... Args>
struct STraits<R(Args...)> {
    using RetType = R;
};

struct SInfo {
    std::function<void()> Task;
};

class CThreadPool {
public:
    template<typename T, typename... Args>
    bool Run(T Callback, Args&&... Arguments) {
        auto ThreadInfo = std::make_unique<SInfo>(SInfo{ [Callback, Arguments...]() { Callback(Arguments...); } });

        auto Function = [](PTP_CALLBACK_INSTANCE Instance, PVOID DataInfo, PTP_WORK Work) -> void {
            auto Data = static_cast<SInfo*>(DataInfo);
            try {
                Data->Task();
            }
            catch (const std::exception) {}

            CloseThreadpoolWork(Work);
            delete Data;
            };

        const auto Work = CreateThreadpoolWork(Function, ThreadInfo.release(), nullptr);
        if (!Work) return false;

        SubmitThreadpoolWork(Work);
        return true;
    }
};

inline auto ThreadPool = std::make_unique<CThreadPool>();