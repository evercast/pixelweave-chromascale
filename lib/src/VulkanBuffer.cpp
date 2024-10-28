#include "VulkanBuffer.h"

#include "DebugUtils.h"
#include "VulkanDevice.h"

namespace Pixelweave
{

ResultValue<VulkanBuffer*> VulkanBuffer::Create(
    VulkanDevice* device,
    const vk::DeviceSize& size,
    const vk::BufferUsageFlags& usageFlags,
    const VmaAllocationCreateFlags& memoryFlags)
{
    VmaAllocator allocator = device->GetAllocator();
    const vk::BufferCreateInfo bufferCreateInfo =
        vk::BufferCreateInfo().setSize(size).setUsage(usageFlags).setSharingMode(vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocationInfo{};
    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationInfo.flags = memoryFlags;
    VmaAllocationInfo allocInfo{};

    vk::Buffer bufferHandle;
    VmaAllocation allocation;
    VkResult result = vmaCreateBuffer(
        allocator,
        (VkBufferCreateInfo*)&bufferCreateInfo,
        &allocationInfo,
        (VkBuffer*)&bufferHandle,
        &allocation,
        &allocInfo);
    if (result != VK_SUCCESS) {
        return {Result::AllocationFailed, nullptr};
    }

    const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo().setBuffer(bufferHandle).setOffset(0).setRange(size);

    return {Result::Success, new VulkanBuffer(device, size, bufferHandle, allocation, bufferInfo)};
}

VulkanBuffer::VulkanBuffer(
    VulkanDevice* device,
    vk::DeviceSize size,
    vk::Buffer bufferHandle,
    VmaAllocation allocation,
    vk::DescriptorBufferInfo descriptorInfo)
    : mDevice(device), mSize(size), mBufferHandle(bufferHandle), mAllocation(allocation), mDescriptorInfo(descriptorInfo)
{
    mDevice->AddRef();
}

uint8_t* VulkanBuffer::MapBuffer()
{
    VmaAllocator allocator = mDevice->GetAllocator();
    uint8_t* mappedResult = nullptr;
    PIXELWEAVE_ASSERT_VK(vmaMapMemory(allocator, mAllocation, (void**)&mappedResult));
    return mappedResult;
}

void VulkanBuffer::UnmapBuffer()
{
    VmaAllocator allocator = mDevice->GetAllocator();
    vmaUnmapMemory(allocator, mAllocation);
}

VulkanBuffer::~VulkanBuffer()
{
    VmaAllocator allocator = mDevice->GetAllocator();
    vmaDestroyBuffer(allocator, mBufferHandle, mAllocation);
    mDevice->Release();
}

}  // namespace Pixelweave