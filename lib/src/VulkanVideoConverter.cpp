#include "VulkanVideoConverter.h"

#include <algorithm>

namespace PixelWeave
{
VulkanVideoConverter::VulkanVideoConverter(VulkanDevice* device) : mDevice(nullptr)
{
    device->AddRef();
    mDevice = device;
}

void VulkanVideoConverter::Convert(const ProtoVideoFrame& src, ProtoVideoFrame& dst)
{
    // Create source buffer and copy CPU memory into it
    const vk::DeviceSize srcBufferSize = src.stride * src.height;
    mSrcBuffer = mDevice->CreateBuffer(
        srcBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    uint8_t* mappedSrcBuffer = mDevice->MapBuffer(mSrcBuffer);
    std::copy_n(src.buffer, srcBufferSize, mappedSrcBuffer);
    mDevice->UnmapBuffer(mSrcBuffer);

    // Create CPU readable dest buffer to do conversions in
    const vk::DeviceSize dstBufferSize = dst.stride * dst.height;
    mDstBuffer = mDevice->CreateBuffer(
        dstBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // Create compute pipeline and bindings
    mDevice->CreateComputePipeline();

    // Copy contents into CPU buffer
    uint8_t* mappedDstBuffer = mDevice->MapBuffer(mDstBuffer);
    std::copy_n(mappedDstBuffer, dstBufferSize, dst.buffer);
    mDevice->UnmapBuffer(mDstBuffer);

    mDevice->DestroyBuffer(mSrcBuffer);
    mDevice->DestroyBuffer(mDstBuffer);
}

VulkanVideoConverter::~VulkanVideoConverter()
{
    mDevice->Release();
}

}  // namespace PixelWeave
