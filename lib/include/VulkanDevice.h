#pragma once

#include "Device.h"

#include <vulkan/vulkan.hpp>

namespace PixelWeave
{

class VulkanInstance;

class VulkanDevice : public Device
{
public:
    static ResultValue<std::shared_ptr<Device>> Create();

    VulkanDevice(const std::shared_ptr<VulkanInstance>& instance, vk::PhysicalDevice physicalDevice);

    virtual ~VulkanDevice();

private:
    std::shared_ptr<VulkanInstance> mVulkanInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device mLogicalDevice;
    vk::Queue mComputeQueue;
};
}  // namespace PixelWeave