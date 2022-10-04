#pragma once

#include <memory>

#include <vulkan/vulkan.hpp>

#include "Device.h"
#include "Result.h"

namespace PixelWeave
{
class VulkanInstance
{
public:
    static ResultValue<std::shared_ptr<VulkanInstance>> Create();
    static ResultValue<Device*> CreateDevice(const std::shared_ptr<VulkanInstance>& instance);

    VulkanInstance(const vk::Instance& instance);

    ~VulkanInstance();

private:
    ResultValue<vk::PhysicalDevice> FindSuitablePhysicalDevice();

    vk::Instance mInstanceHandle;
    vk::DispatchLoaderDynamic mDynamicDispatcher;
#if defined(_DEBUG)
    vk::DebugUtilsMessengerEXT mDebugMessenger;
#endif
};
}  // namespace PixelWeave