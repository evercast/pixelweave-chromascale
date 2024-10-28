#pragma once

#include "Macros.h"
#include "RefCountPtr.h"
#include "Result.h"
#include "VideoFrameWrapper.h"

namespace Pixelweave
{

struct BenchmarkResult {
    uint64_t copyToDeviceVisibleTimeMicros = 0;
    uint64_t transferDeviceVisibleToDeviceLocalTimeMicros = 0;
    uint64_t computeConversionTimeMicros = 0;
    uint64_t transferDeviceLocalToHostVisibleTimeMicros = 0;
    uint64_t gpuConversionTimeMicros = 0;
    uint64_t copyDeviceVisibleToHostLocalTimeMicros = 0;
};

class PIXELWEAVE_LIB_CLASS VideoConverter : public RefCountPtr
{
public:
    VideoConverter() = default;

    virtual Result Convert(const VideoFrameWrapper& src, VideoFrameWrapper& dst) = 0;
    virtual ResultValue<BenchmarkResult> ConvertWithBenchmark(const VideoFrameWrapper& src, VideoFrameWrapper& dst) = 0;
    virtual ~VideoConverter() override = default;
};
}  // namespace Pixelweave