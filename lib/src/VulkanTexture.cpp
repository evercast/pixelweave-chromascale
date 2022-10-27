#include "VulkanTexture.h"

#include "DebugUtils.h"
#include "VulkanDevice.h"

namespace PixelWeave
{

vk::Format PixelFormatToVkImageFormat(const PixelFormat& pixelFormat)
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
            return vk::Format::eG8B8G8R8422Unorm;
        case PixelFormat::Planar8Bit422:
            return vk::Format::eG8B8R83Plane422Unorm;
    }
    PW_ASSERT_MSG(false, "Failed to convert pixel format to valid Vulkan format");
    return vk::Format::eUndefined;
}

ResultValue<VulkanTexture*> VulkanTexture::Create(
    VulkanDevice* device,
    PixelFormat pixelFormat,
    uint32_t width,
    uint32_t height,
    vk::ImageUsageFlags usage)
{
    // Create image
    vk::Device& logicalDevice = device->GetLogicalDevice();
    const vk::Format textureFormat = PixelFormatToVkImageFormat(pixelFormat);
    vk::ImageCreateInfo imageInfo = vk::ImageCreateInfo()
                                        .setImageType(vk::ImageType::e2D)
                                        .setFormat(textureFormat)
                                        .setMipLevels(1)
                                        .setArrayLayers(1)
                                        .setSamples(vk::SampleCountFlagBits::e1)
                                        .setTiling(vk::ImageTiling::eOptimal)
                                        .setSharingMode(vk::SharingMode::eExclusive)
                                        .setInitialLayout(vk::ImageLayout::eUndefined)
                                        .setExtent(vk::Extent3D{width, height, 1})
                                        .setUsage(usage);
    vk::Image image = PW_ASSERT_VK(logicalDevice.createImage(imageInfo));

    // Allocate image memory
    vk::MemoryRequirements memoryRequirements;
    logicalDevice.getImageMemoryRequirements(image, &memoryRequirements);
    const vk::DeviceMemory imageBufferMemory = device->AllocateMemory(vk::MemoryPropertyFlagBits::eDeviceLocal, memoryRequirements);
    PW_ASSERT_VK(logicalDevice.bindImageMemory(image, imageBufferMemory, 0));

    // Create yuv conversion resources and image view
    const vk::SamplerYcbcrConversionCreateInfo samplerConversionInfo = vk::SamplerYcbcrConversionCreateInfo()
                                                                           .setFormat(PixelFormatToVkImageFormat(pixelFormat))
                                                                           .setYcbcrModel(vk::SamplerYcbcrModelConversion::eYcbcr709)
                                                                           .setYcbcrRange(vk::SamplerYcbcrRange::eItuFull)
                                                                           .setComponents(vk::ComponentMapping{
                                                                               vk::ComponentSwizzle::eIdentity,
                                                                               vk::ComponentSwizzle::eIdentity,
                                                                               vk::ComponentSwizzle::eIdentity,
                                                                               vk::ComponentSwizzle::eIdentity})
                                                                           .setXChromaOffset(vk::ChromaLocation::eMidpoint)
                                                                           .setYChromaOffset(vk::ChromaLocation::eMidpoint)
                                                                           .setChromaFilter(vk::Filter::eLinear);
    const vk::SamplerYcbcrConversion samplerConversion = PW_ASSERT_VK(logicalDevice.createSamplerYcbcrConversion(samplerConversionInfo));
    const vk::SamplerYcbcrConversionInfo samplerConversionInfoExt = vk::SamplerYcbcrConversionInfo().setConversion(samplerConversion);

    const vk::ImageViewCreateInfo imageViewInfo = vk::ImageViewCreateInfo()
                                                      .setImage(image)
                                                      .setViewType(vk::ImageViewType::e2D)
                                                      .setFormat(textureFormat)
                                                      .setComponents(vk::ComponentMapping{
                                                          vk::ComponentSwizzle::eIdentity,
                                                          vk::ComponentSwizzle::eIdentity,
                                                          vk::ComponentSwizzle::eIdentity,
                                                          vk::ComponentSwizzle::eIdentity})
                                                      .setSubresourceRange(vk::ImageSubresourceRange()
                                                                               .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                                                               .setBaseArrayLayer(0)
                                                                               .setLayerCount(1)
                                                                               .setBaseMipLevel(0)
                                                                               .setLevelCount(1))
                                                      .setPNext(&samplerConversionInfoExt);
    const vk::ImageView imageView = PW_ASSERT_VK(logicalDevice.createImageView(imageViewInfo));

    const vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo()
                                                  .setMagFilter(vk::Filter::eLinear)
                                                  .setMinFilter(vk::Filter::eLinear)
                                                  .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                                                  .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
                                                  .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
                                                  .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
                                                  .setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
                                                  .setAnisotropyEnable(false)
                                                  .setCompareEnable(false)
                                                  .setMinLod(0.0f)
                                                  .setMaxLod(1.0f)
                                                  .setUnnormalizedCoordinates(false)
                                                  .setPNext(&samplerConversionInfoExt);
    const vk::Sampler sampler = PW_ASSERT_VK(logicalDevice.createSampler(samplerInfo));
    VulkanTexture* texture = new VulkanTexture(device, image, imageBufferMemory, samplerConversion, imageView, sampler);

    return {Result::Success, texture};
}

VulkanTexture::VulkanTexture(
    VulkanDevice* device,
    vk::Image image,
    vk::DeviceMemory memory,
    vk::SamplerYcbcrConversion samplerConversion,
    vk::ImageView imageView,
    vk::Sampler sampler)
    : mDevice(device), mImage(image), mMemory(memory), mSamplerConversion(samplerConversion), mImageView(imageView), mSampler(sampler)
{
    mDevice->AddRef();
}

VulkanTexture::~VulkanTexture()
{
    vk::Device& logicalDevice = mDevice->GetLogicalDevice();
    logicalDevice.destroySampler(mSampler);
    logicalDevice.destroyImageView(mImageView);
    logicalDevice.destroySamplerYcbcrConversion(mSamplerConversion);
    logicalDevice.freeMemory(mMemory);
    logicalDevice.destroyImage(mImage);
    mDevice->Release();
}

}  // namespace PixelWeave