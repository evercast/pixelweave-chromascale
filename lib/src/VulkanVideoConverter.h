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

    VulkanBuffer* mSrcLocalBuffer;
    VulkanBuffer* mSrcDeviceBuffer;

    VulkanBuffer* mDstLocalBuffer;
    VulkanBuffer* mDstDeviceBuffer;

    VulkanDevice::ComputePipelineResources mPipelineResources;
    vk::CommandBuffer mCommand;

    std::optional<ProtoVideoFrame> mPrevSourceFrame, mPrevDstFrame;
};
}  // namespace PixelWeave
