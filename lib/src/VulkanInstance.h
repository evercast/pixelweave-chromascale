#pragma once

#include <memory>

#include "Device.h"
#include "Result.h"
#include "VulkanBase.h"

namespace Pixelweave
{
class VulkanInstance
{
public:
    static ResultValue<std::shared_ptr<VulkanInstance>> Create();
    static ResultValue<Device*> CreateDevice(const std::shared_ptr<VulkanInstance>& instance);

    VulkanInstance(const vk::Instance& instance);
    ~VulkanInstance();

    vk::Instance GetHandle() { return mInstanceHandle; }

private:
    ResultValue<vk::PhysicalDevice> FindSuitablePhysicalDevice();

    vk::Instance mInstanceHandle;
#if VK_HEADER_VERSION >= 301
    vk::detail::DispatchLoaderDynamic mDynamicDispatcher;
#else
    vk::DispatchLoaderDynamic mDynamicDispatcher;
#endif
#if defined(PIXELWEAVE_DEBUG)
    vk::DebugUtilsMessengerEXT mDebugMessenger;
#endif
};
}  // namespace Pixelweave
