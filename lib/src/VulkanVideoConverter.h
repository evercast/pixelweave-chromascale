#pragma once

#include <optional>

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

    VulkanDevice::Buffer mSrcLocalBuffer;
    VulkanDevice::Buffer mSrcDeviceBuffer;
    VulkanTexture* mSrcTexture;

    VulkanDevice::Buffer mDstLocalBuffer;
    VulkanDevice::Buffer mDstDeviceBuffer;

    VulkanDevice::ComputePipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;

    std::optional<ProtoVideoFrame> mPrevSourceFrame, mPrevDstFrame;
};
}  // namespace PixelWeave
