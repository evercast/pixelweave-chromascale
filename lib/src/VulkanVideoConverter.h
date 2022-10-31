#pragma once

#include <optional>

#include "VideoConverter.h"
#include "VulkanBase.h"
#include "VulkanDevice.h"

namespace PixelWeave
{
class VulkanVideoConverter : public VideoConverter
{
public:
    VulkanVideoConverter(VulkanDevice* device);
    void Convert(const VideoFrameWrapper& src, VideoFrameWrapper& dst) override;
    ~VulkanVideoConverter() override;

private:
    void InitResources(const VideoFrameWrapper& src, VideoFrameWrapper& dst);
    void Cleanup();

    VulkanDevice* mDevice;

    VulkanBuffer* mSrcLocalBuffer;
    VulkanBuffer* mSrcDeviceBuffer;

    VulkanBuffer* mDstLocalBuffer;
    VulkanBuffer* mDstDeviceBuffer;

    VulkanDevice::ComputePipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;

    std::optional<VideoFrameWrapper> mPrevSourceFrame, mPrevDstFrame;
};
}  // namespace PixelWeave
