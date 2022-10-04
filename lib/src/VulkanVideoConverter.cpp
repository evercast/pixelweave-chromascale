#include "VulkanVideoConverter.h"

namespace PixelWeave
{
VulkanVideoConverter::VulkanVideoConverter(VulkanDevice* device)
{
    device->AddRef();
    mDevice = device;
}

void VulkanVideoConverter::Convert(const ProtoVideoFrame&, ProtoVideoFrame&) {}

void VulkanVideoConverter::CreateBuffer(
    const vk::DeviceSize&,
    vk::BufferUsageFlags,
    vk::MemoryPropertyFlags,
    vk::Buffer&,
    vk::DeviceMemory&)
{
}

VulkanVideoConverter::~VulkanVideoConverter()
{
    mDevice->Release();
}

}  // namespace PixelWeave
