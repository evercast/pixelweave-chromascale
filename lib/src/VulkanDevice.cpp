#include "VulkanDevice.h"

#include <fstream>

#include "DebugUtils.h"
#include "VulkanInstance.h"
#include "VulkanVideoConverter.h"

namespace PixelWeave
{
ResultValue<Device*> VulkanDevice::Create()
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

    vk::PhysicalDeviceVulkan12Features physicalDeviceFeatures1_2{};
    vk::PhysicalDeviceFeatures2 physicalDeviceFeatures = vk::PhysicalDeviceFeatures2().setPNext(&physicalDeviceFeatures1_2);
    physicalDevice.getFeatures2(&physicalDeviceFeatures);

    const vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo()
                                                      .setQueueCreateInfos(queueCreateInfo)
                                                      .setPNext(&physicalDeviceFeatures);
    mLogicalDevice = PW_ASSERT_VK(mPhysicalDevice.createDevice(deviceCreateInfo));

    mComputeQueue = mLogicalDevice.getQueue(queueFamilyIndex, 0);

    const vk::CommandPoolCreateInfo commandPoolCreateInfo =
        vk::CommandPoolCreateInfo().setQueueFamilyIndex(queueFamilyIndex).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    mCommandPool = PW_ASSERT_VK(mLogicalDevice.createCommandPool(commandPoolCreateInfo));
}

VideoConverter* VulkanDevice::CreateVideoConverter()
{
    return new VulkanVideoConverter(this);
}

VulkanDevice::Buffer VulkanDevice::CreateBuffer(
    const vk::DeviceSize& size,
    const vk::BufferUsageFlags& usageFlags,
    const vk::MemoryPropertyFlags& memoryFlags)
{
    // Create buffer (a memory view)
    const vk::BufferCreateInfo bufferCreateInfo =
        vk::BufferCreateInfo().setSize(size).setUsage(usageFlags).setSharingMode(vk::SharingMode::eExclusive);
    const vk::Buffer bufferHandle = PW_ASSERT_VK(mLogicalDevice.createBuffer(bufferCreateInfo));

    // Find suitable memory to allocate the buffer in
    const vk::MemoryRequirements memoryRequirements = mLogicalDevice.getBufferMemoryRequirements(bufferHandle);
    const vk::PhysicalDeviceMemoryProperties memoryProperties = mPhysicalDevice.getMemoryProperties();
    uint32_t selectedMemoryIndex = 0;
    for (uint32_t memoryIndex = 0; memoryIndex < memoryProperties.memoryTypeCount; ++memoryIndex) {
        if (memoryRequirements.memoryTypeBits & (1 << memoryIndex) &&
            (memoryProperties.memoryTypes[memoryIndex].propertyFlags & memoryFlags)) {
            selectedMemoryIndex = memoryIndex;
            break;
        }
    }

    // Allocate memory for the buffer
    const vk::MemoryAllocateInfo allocateInfo =
        vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(selectedMemoryIndex);
    const vk::DeviceMemory memoryHandle = PW_ASSERT_VK(mLogicalDevice.allocateMemory(allocateInfo));

    PW_ASSERT_VK(mLogicalDevice.bindBufferMemory(bufferHandle, memoryHandle, 0));

    return Buffer{size, bufferHandle, memoryHandle};
}

uint8_t* VulkanDevice::MapBuffer(Buffer& buffer)
{
    return static_cast<uint8_t*>(PW_ASSERT_VK(mLogicalDevice.mapMemory(buffer.memoryHandle, 0, buffer.size)));
}

void VulkanDevice::UnmapBuffer(Buffer& buffer)
{
    mLogicalDevice.unmapMemory(buffer.memoryHandle);
}

void VulkanDevice::DestroyBuffer(Buffer& buffer)
{
    mLogicalDevice.freeMemory(buffer.memoryHandle);
    mLogicalDevice.destroyBuffer(buffer.bufferHandle);
    buffer.size = 0;
}

VulkanDevice::ComputePipelineResources VulkanDevice::CreateComputePipeline()
{
    ComputePipelineResources resources;

    const std::vector<vk::DescriptorSetLayoutBinding> descriptorLayoutBindings{
        vk::DescriptorSetLayoutBinding()
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
            .setDescriptorCount(1),
        vk::DescriptorSetLayoutBinding()
            .setBinding(1)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
            .setDescriptorCount(1)};

    const vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo =
        vk::DescriptorSetLayoutCreateInfo().setBindings(descriptorLayoutBindings);
    resources.descriptorLayout = PW_ASSERT_VK(mLogicalDevice.createDescriptorSetLayout(descriptorLayoutInfo));

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo().setSetLayouts(resources.descriptorLayout);
    resources.pipelineLayout = PW_ASSERT_VK(mLogicalDevice.createPipelineLayout(pipelineLayoutInfo));

    // Load hardcoded path (use ResourceLoader in the future)
    std::vector<uint8_t> rawData;
    {
        std::string shaderPath("../../lib/src/shaders/convert.comp.spv");
        std::ifstream file(shaderPath, std::ios::binary | std::ios::ate);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        rawData.resize(fileSize);
        file.read(reinterpret_cast<char*>(rawData.data()), fileSize);
        file.close();
    }
    vk::ShaderModuleCreateInfo shaderCreateInfo = vk::ShaderModuleCreateInfo()
                                                      .setCodeSize(rawData.size() * sizeof(uint8_t))
                                                      .setPCode(reinterpret_cast<const uint32_t*>(rawData.data()));
    resources.shader = PW_ASSERT_VK(mLogicalDevice.createShaderModule(shaderCreateInfo));

    const vk::PipelineShaderStageCreateInfo stageCreateInfo =
        vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eCompute).setModule(resources.shader).setPName("main");
    const vk::ComputePipelineCreateInfo computePipelineInfo =
        vk::ComputePipelineCreateInfo().setLayout(resources.pipelineLayout).setStage(stageCreateInfo);
    resources.pipeline = PW_ASSERT_VK(mLogicalDevice.createComputePipeline(nullptr, computePipelineInfo));

    return resources;
}

VulkanDevice::~VulkanDevice()
{
    mLogicalDevice.destroyCommandPool(mCommandPool);
    mLogicalDevice.destroy();
    mVulkanInstance = nullptr;
}

}  // namespace PixelWeave
