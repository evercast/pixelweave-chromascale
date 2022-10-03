#include "VulkanDevice.h"

#include "DebugUtils.h"
#include "VulkanInstance.h"

namespace PixelWeave
{
ResultValue<std::shared_ptr<Device>> VulkanDevice::Create()
{
    const auto instanceResult = VulkanInstance::Create();
    if (instanceResult.result == Result::Success) {
        const auto instance = instanceResult.value;
        return VulkanInstance::CreateDevice(instance);
    }
    return {instanceResult.result, nullptr};
}

VulkanDevice::VulkanDevice(const std::shared_ptr<VulkanInstance>& instance, vk::PhysicalDevice physicalDevice)
    : mVulkanInstance(instance), mPhysicalDevice(physicalDevice)
{
    const std::vector<vk::QueueFamilyProperties> queueFamiliesProperties = mPhysicalDevice.getQueueFamilyProperties();
    uint32_t queueFamilyIndex = 0;
    for (const auto& properties : queueFamiliesProperties) {
        if (properties.queueFlags & vk::QueueFlagBits::eCompute) {
            break;
        }
        queueFamilyIndex += 1;
    }
    PW_ASSERT(queueFamilyIndex < static_cast<uint32_t>(queueFamiliesProperties.size()));
    const std::vector<float> queuePriorities{1.0f};
    vk::DeviceQueueCreateInfo queueCreateInfo =
        vk::DeviceQueueCreateInfo().setQueueFamilyIndex(queueFamilyIndex).setQueuePriorities(queuePriorities);
    const vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo().setQueueCreateInfos(queueCreateInfo);
    mLogicalDevice = PW_ASSERT_VK(mPhysicalDevice.createDevice(deviceCreateInfo));
    mComputeQueue = mLogicalDevice.getQueue(queueFamilyIndex, 0);
}

VulkanDevice::~VulkanDevice()
{
    mLogicalDevice.destroy();
    mVulkanInstance = nullptr;
}

}  // namespace PixelWeave
