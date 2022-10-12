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
    void InitResources(const ProtoVideoFrame& src, ProtoVideoFrame& dst);
    void Cleanup();

    VulkanDevice* mDevice;

    VulkanDevice::Buffer mSrcBuffer;
    VulkanDevice::Buffer mDstBuffer;

    VulkanDevice::ComputePipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;

    bool mIsInitialized;
};
}  // namespace PixelWeave
