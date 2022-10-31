#pragma once

#include <memory>

#include "Device.h"
#include "Result.h"
#include "VulkanBase.h"

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
#if defined(PW_DEBUG)
    vk::DebugUtilsMessengerEXT mDebugMessenger;
#endif
};
}  // namespace PixelWeave
