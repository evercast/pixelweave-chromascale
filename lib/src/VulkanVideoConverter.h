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
    void Convert(const ProtoVideoFrame& source, ProtoVideoFrame& dst) override;
    ~VulkanVideoConverter() override;

private:
    void CreateBuffer(
        const vk::DeviceSize& size,
        vk::BufferUsageFlags usageFlags,
        vk::MemoryPropertyFlags memoryFlags,
        vk::Buffer& bufferHandle,
        vk::DeviceMemory& memoryHandle);

    VulkanDevice* mDevice;

    vk::Buffer mSrcBuffer;
    vk::DeviceMemory mSrcBufferMemory;

    vk::Buffer mDstBuffer;
    vk::DeviceMemory mDstBufferMemory;
};
}  // namespace PixelWeave
