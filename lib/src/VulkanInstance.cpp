#include "VulkanInstance.h"

#include "DebugUtils.h"
#include "VulkanDevice.h"

namespace Pixelweave
{

#if defined(PIXELWEAVE_DEBUG)
static VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*)
{
    PIXELWEAVE_LOG(callbackData->pMessage);
    PIXELWEAVE_ASSERT_MSG(
        messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT &&
            messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        callbackData->pMessage);
    return VK_FALSE;
}
#endif

ResultValue<std::shared_ptr<VulkanInstance>> VulkanInstance::Create()
{
    const vk::ApplicationInfo appInfo = vk::ApplicationInfo()
                                            .setPApplicationName("Pixelweave")
                                            .setApplicationVersion(VK_MAKE_VERSION(0, 0, 1))
                                            .setPEngineName("Pixelweave")
                                            .setEngineVersion(VK_MAKE_VERSION(0, 0, 1))
                                            .setApiVersion(VK_API_VERSION_1_2);

    std::vector<const char*> layersToEnable
    {
#if defined(PIXELWEAVE_DEBUG)
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    std::vector<const char*> extensionsToEnable
    {
#if defined(PIXELWEAVE_PLATFORM_MACOS)
        VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
#endif
    };

#if defined(PIXELWEAVE_DEBUG)
    {
        const std::vector<const char*> debugExtensions{VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        extensionsToEnable.insert(extensionsToEnable.end(), debugExtensions.begin(), debugExtensions.end());
    }
#endif

    std::vector<vk::ExtensionProperties> supportedExtensions = PIXELWEAVE_ASSERT_VK(vk::enumerateInstanceExtensionProperties());

    auto supportedLayers = PIXELWEAVE_ASSERT_VK(vk::enumerateInstanceLayerProperties());

    bool allExtensionsSupported = true;
    for (const char* extensionName : extensionsToEnable) {
        const bool isExtensionSupported = std::find_if(
                                              supportedExtensions.begin(),
                                              supportedExtensions.end(),
                                              [&extensionName](const vk::ExtensionProperties& presentExtension) {
                                                  return std::string(extensionName) == std::string(presentExtension.extensionName.data());
                                              }) != supportedExtensions.end();
        allExtensionsSupported = allExtensionsSupported && isExtensionSupported;
    }
    if (!allExtensionsSupported) {
        return {Result::DriverNotFoundError, nullptr};
    }

    vk::InstanceCreateFlags createFlags;
#ifdef PIXELWEAVE_PLATFORM_MACOS
    createFlags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    const vk::InstanceCreateInfo instanceInfo = vk::InstanceCreateInfo()
                                                    .setFlags(createFlags)
                                                    .setPApplicationInfo(&appInfo)
                                                    .setPEnabledExtensionNames(extensionsToEnable)
                                                    .setPEnabledLayerNames(layersToEnable);

    auto [instanceResult, instanceHandle] = vk::createInstance(instanceInfo);
    if (instanceResult == vk::Result::eSuccess) {
        return {Result::Success, std::make_shared<VulkanInstance>(instanceHandle)};
    }
    return {Result::DriverNotFoundError, nullptr};
}

VulkanInstance::VulkanInstance(const vk::Instance& instance) : mInstanceHandle(instance)
{
#if VK_HEADER_VERSION >= 301
    mDynamicDispatcher = vk::detail::DispatchLoaderDynamic(mInstanceHandle, vkGetInstanceProcAddr);
#else
    mDynamicDispatcher = vk::DispatchLoaderDynamic(mInstanceHandle, vkGetInstanceProcAddr);
#endif
    mDynamicDispatcher.init(mInstanceHandle, vkGetInstanceProcAddr);

#if defined(PIXELWEAVE_DEBUG)
    const vk::DebugUtilsMessengerCreateInfoEXT debugCallbackCreateInfo =
        vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
#pragma warning(suppress : 4996)  // MSVC: suppress deprecation warning for `setPfnUserCallback()`
            .setPfnUserCallback(vkDebugCallback);

    mDebugMessenger = PIXELWEAVE_ASSERT_VK(mInstanceHandle.createDebugUtilsMessengerEXT(debugCallbackCreateInfo, nullptr, mDynamicDispatcher));
#endif
}

ResultValue<Device*> VulkanInstance::CreateDevice(const std::shared_ptr<VulkanInstance>& instance)
{
    auto [result, physicalDevice] = instance->FindSuitablePhysicalDevice();
    if (result == Result::Success) {
        return {Result::Success, new VulkanDevice(instance, physicalDevice)};
    }
    return {Result::InvalidDeviceError, nullptr};
}

ResultValue<vk::PhysicalDevice> VulkanInstance::FindSuitablePhysicalDevice()
{
    const std::vector<vk::PhysicalDevice> physicalDevices = PIXELWEAVE_ASSERT_VK(mInstanceHandle.enumeratePhysicalDevices());
    vk::PhysicalDevice chosenDevice = nullptr;
    uint32_t chosenDeviceScore = 0;
    for (const vk::PhysicalDevice& physicalDevice : physicalDevices) {
        const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        uint32_t currentDeviceScore = 0;
        {
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                currentDeviceScore += 1000;
            }
            if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
                currentDeviceScore += 100;
            }

            vk::PhysicalDeviceVulkan11Features physicalDeviceFeatures1_1{};
            vk::PhysicalDeviceVulkan12Features physicalDeviceFeatures1_2{};
            physicalDeviceFeatures1_1.setPNext(&physicalDeviceFeatures1_2);
            vk::PhysicalDeviceFeatures2 physicalDeviceFeatures = vk::PhysicalDeviceFeatures2().setPNext(&physicalDeviceFeatures1_1);
            physicalDevice.getFeatures2(&physicalDeviceFeatures);

            if (!physicalDeviceFeatures1_2.uniformAndStorageBuffer8BitAccess) {
                currentDeviceScore = 0;
            }

            const std::vector<vk::QueueFamilyProperties> queueFamiliesProperties = physicalDevice.getQueueFamilyProperties();
            const bool hasComputeQueue = std::any_of(
                queueFamiliesProperties.begin(),
                queueFamiliesProperties.end(),
                [](const vk::QueueFamilyProperties& properties) -> bool {
                    return static_cast<bool>(properties.queueFlags & vk::QueueFlagBits::eCompute);
                });
            if (!hasComputeQueue) {
                currentDeviceScore = 0;
            }
        }
        if (chosenDeviceScore < currentDeviceScore) {
            chosenDevice = physicalDevice;
            chosenDeviceScore = currentDeviceScore;
        }
    }
    return {chosenDeviceScore > 0 ? Result::Success : Result::NoSuitableDeviceError, chosenDevice};
}

VulkanInstance::~VulkanInstance()
{
#if defined(PIXELWEAVE_DEBUG)
    mInstanceHandle.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr, mDynamicDispatcher);
#endif
    mInstanceHandle.destroy();
}

}  // namespace Pixelweave
