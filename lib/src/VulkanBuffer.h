#pragma once

#include "RefCountPtr.h"
#include "VulkanBase.h"

namespace PixelWeave
{
class VulkanDevice;

class VulkanBuffer : public RefCountPtr
{
public:
    static VulkanBuffer* Create(
        VulkanDevice* device,
        const vk::DeviceSize& size,
        const vk::BufferUsageFlags& usageFlags,
        const VmaAllocationCreateFlags& memoryFlags);

    const vk::DeviceSize& GetBufferSize() const { return mSize; }
    const vk::Buffer& GetBufferHandle() const { return mBufferHandle; }
    const vk::DescriptorBufferInfo& GetDescriptorInfo() const { return mDescriptorInfo; }

    uint8_t* MapBuffer();
    void UnmapBuffer();

private:
    VulkanBuffer(
        VulkanDevice* device,
        vk::DeviceSize size,
        vk::Buffer bufferHandle,
        VmaAllocation allocation,
        vk::DescriptorBufferInfo descriptorInfo);

    ~VulkanBuffer() override;

    VulkanDevice* mDevice;
    vk::DeviceSize mSize;
    vk::Buffer mBufferHandle;
    VmaAllocation mAllocation;
    vk::DescriptorBufferInfo mDescriptorInfo;
};
}  // namespace PixelWeave
