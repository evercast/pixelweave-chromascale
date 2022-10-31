#pragma once

#include "PixelFormat.h"
#include "RefCountPtr.h"
#include "VulkanBase.h"

namespace PixelWeave
{
class VulkanDevice;

class VulkanImage : public RefCountPtr
{
public:
    static VulkanImage* Create(VulkanDevice* device, PixelFormat pixelFormat, uint32_t width, uint32_t height, vk::ImageUsageFlags usage);

    const vk::DeviceSize& GetBufferSize() const { return mMemorySize; }
    const vk::Sampler& GetSampler() const { return mSampler; }
    const vk::ImageView& GetImageView() const { return mImageView; }

private:
    VulkanImage(
        VulkanDevice* device,
        vk::Image image,
        vk::DeviceMemory memory,
        vk::DeviceSize memorySize,
        vk::SamplerYcbcrConversion samplerConversion,
        vk::ImageView imageView,
        vk::Sampler sampler);

    ~VulkanImage() override;

    VulkanDevice* mDevice;
    vk::Image mImage;
    vk::DeviceMemory mMemory;
    vk::DeviceSize mMemorySize;
    vk::SamplerYcbcrConversion mSamplerConversion;
    vk::ImageView mImageView;
    vk::Sampler mSampler;
};
}  // namespace PixelWeave
