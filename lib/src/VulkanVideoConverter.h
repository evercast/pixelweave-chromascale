#pragma once

#include <optional>

#include "VideoConverter.h"
#include "VulkanBase.h"
#include "VulkanDevice.h"

namespace Pixelweave
{

class VulkanVideoConverter : public VideoConverter
{
public:
    VulkanVideoConverter(VulkanDevice* device);
    ~VulkanVideoConverter() override;

    Result Convert(const VideoFrameWrapper& src, VideoFrameWrapper& dst) override;
    ResultValue<BenchmarkResult> ConvertWithBenchmark(const VideoFrameWrapper& src, VideoFrameWrapper& dst) override;

private:
    ResultValue<BenchmarkResult> ConvertInternal(
        const VideoFrameWrapper& src,
        VideoFrameWrapper& dst,
        bool enableBenchmark);

    static Result ValidateInput(const VideoFrameWrapper& src, const VideoFrameWrapper& dst);
    static bool IsInputFormatSupported(PixelFormat format);
    static bool IsOutputFormatSupported(PixelFormat format);

    Result InitResources(const VideoFrameWrapper& src, VideoFrameWrapper& dst);
    void CleanUp();

    VulkanDevice* mDevice;

    VulkanBuffer* mSrcLocalBuffer;
    VulkanBuffer* mSrcDeviceBuffer;

    VulkanBuffer* mDstLocalBuffer;
    VulkanBuffer* mDstDeviceBuffer;

    VulkanDevice::VideoConversionPipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;

    std::optional<VideoFrameWrapper> mPrevSourceFrame, mPrevDstFrame;

    static const uint32_t sTimestampStartIndex = 0;
    static const uint32_t sTimestampSrcTransferDoneIndex = 1;
    static const uint32_t sTimestampConvertIndex = 2;
    static const uint32_t sTimestampDstTransferDoneIndex = 3;
    static const uint32_t sTimemestampQueryCount = 4;

    vk::QueryPool mTimestampQueryPool;
    bool mEnableBenchmark;
};

}  // namespace Pixelweave
