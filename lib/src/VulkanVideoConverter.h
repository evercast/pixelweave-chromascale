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
    Result Convert(const VideoFrameWrapper& src, VideoFrameWrapper& dst) override;
    ~VulkanVideoConverter() override;

private:
    Result ValidateInput(const VideoFrameWrapper& src, const VideoFrameWrapper& dst);

    void InitResources(const VideoFrameWrapper& src, VideoFrameWrapper& dst);
    void CleanUp();

    VulkanDevice* mDevice;

    VulkanBuffer* mSrcLocalBuffer;
    VulkanBuffer* mSrcDeviceBuffer;

    VulkanBuffer* mDstLocalBuffer;
    VulkanBuffer* mDstDeviceBuffer;

    VulkanDevice::VideoConversionPipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;

    std::optional<VideoFrameWrapper> mPrevSourceFrame, mPrevDstFrame;
};
}  // namespace PixelWeave
