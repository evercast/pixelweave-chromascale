#pragma once

#include <vulkan/vulkan.hpp>

#include "PixelFormat.h"
#include "RefCountPtr.h"
#include "Result.h"

namespace PixelWeave
{
class VulkanDevice;

class VulkanTexture : RefCountPtr
{
public:
    static ResultValue<VulkanTexture*> Create(
        VulkanDevice* device,
        PixelFormat pixelFormat,
        uint32_t width,
        uint32_t height,
        vk::ImageUsageFlags usage);

private:
    VulkanTexture(
        VulkanDevice* device,
        vk::Image image,
        vk::DeviceMemory memory,
        vk::SamplerYcbcrConversion samplerConversion,
        vk::ImageView imageView,
        vk::Sampler sampler);

    ~VulkanTexture() override;

    VulkanDevice* mDevice;
    vk::Image mImage;
    vk::DeviceMemory mMemory;
    vk::SamplerYcbcrConversion mSamplerConversion;
    vk::ImageView mImageView;
    vk::Sampler mSampler;
};
}  // namespace PixelWeave