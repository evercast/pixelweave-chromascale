#pragma once

#include <cstdint>
#include <chrono>

#include "Macros.h"

namespace Pixelweave
{
class Timer
{
public:
    void Start() { beginTime = std::chrono::steady_clock::now(); }
    template <typename M>
    uint64_t Elapsed()
    {
        return std::chrono::duration_cast<M>(std::chrono::steady_clock::now() - beginTime).count();
    }
    uint64_t ElapsedMillis() { return Elapsed<std::chrono::milliseconds>(); }
    uint64_t ElapsedMicros() { return Elapsed<std::chrono::microseconds>(); }

private:
    std::chrono::steady_clock::time_point beginTime;
};
}  // namespace Pixelweave