#pragma once

#include <vulkan/vulkan.hpp>

#include "VideoConverter.h"
#include "VulkanDevice.h"

namespace PixelWeave
{
class VulkanVideoConverter : public VideoConverter
{
public:
    VulkanVideoConverter(VulkanDevice* device);
    void Convert(const ProtoVideoFrame& src, ProtoVideoFrame& dst) override;
    ~VulkanVideoConverter() override;

private:
    VulkanDevice* mDevice;

    VulkanDevice::Buffer mSrcBuffer;
    VulkanDevice::Buffer mDstBuffer;

    VulkanDevice::ComputePipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;
    vk::Fence mWaitComputeFence;
};
}  // namespace PixelWeave
