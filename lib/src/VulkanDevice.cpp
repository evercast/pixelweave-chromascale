#include "VulkanDevice.h"

#include <limits>

#include "DebugUtils.h"
#include "ResourceLoader.h"
#include "VulkanInstance.h"
#include "VulkanVideoConverter.h"
#include "VideoFrameWrapper.h"

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

    const vk::DeviceCreateInfo deviceCreateInfo =
        vk::DeviceCreateInfo().setQueueCreateInfos(queueCreateInfo).setPNext(&physicalDeviceFeatures);
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

    const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo().setBuffer(bufferHandle).setOffset(0).setRange(size);

    return Buffer{size, bufferHandle, memoryHandle, bufferInfo};
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

VulkanDevice::ComputePipelineResources VulkanDevice::CreateComputePipeline(const Buffer& srcBuffer, const Buffer& dstBuffer)
{
    ComputePipelineResources resources;

    // Create pipeline layout bindings (one for srcBuffer, one for dstBuffer)
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

    // Create pipeline including layout, shader (loaded from file), and pipeline itself
    const vk::PushConstantRange pushConstantInfo =
        vk::PushConstantRange().setOffset(0).setSize(sizeof(InOutPictureInfo)).setStageFlags(vk::ShaderStageFlagBits::eCompute);

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo =
        vk::PipelineLayoutCreateInfo().setSetLayouts(resources.descriptorLayout).setPushConstantRanges(pushConstantInfo);
    resources.pipelineLayout = PW_ASSERT_VK(mLogicalDevice.createPipelineLayout(pipelineLayoutInfo));

    Resource shaderResource = ResourceLoader::Load(Resource::Id::ComputeShader);
    vk::ShaderModuleCreateInfo shaderCreateInfo = vk::ShaderModuleCreateInfo()
                                                      .setCodeSize(shaderResource.size * sizeof(uint8_t))
                                                      .setPCode(reinterpret_cast<const uint32_t*>(shaderResource.buffer));
    resources.shader = PW_ASSERT_VK(mLogicalDevice.createShaderModule(shaderCreateInfo));
    ResourceLoader::Cleanup(shaderResource);

    const vk::PipelineShaderStageCreateInfo stageCreateInfo =
        vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eCompute).setModule(resources.shader).setPName("main");
    const vk::ComputePipelineCreateInfo computePipelineInfo =
        vk::ComputePipelineCreateInfo().setLayout(resources.pipelineLayout).setStage(stageCreateInfo);
    resources.pipeline = PW_ASSERT_VK(mLogicalDevice.createComputePipeline(nullptr, computePipelineInfo));

    // Write descriptor sets for each buffer
    const vk::DescriptorPoolSize poolSize = vk::DescriptorPoolSize().setDescriptorCount(2).setType(vk::DescriptorType::eStorageBuffer);
    const vk::DescriptorPoolCreateInfo poolInfo =
        vk::DescriptorPoolCreateInfo().setPoolSizes(poolSize).setMaxSets(1).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    resources.descriptorPool = PW_ASSERT_VK(mLogicalDevice.createDescriptorPool(poolInfo));

    const vk::DescriptorSetAllocateInfo descriptorAllocInfo = vk::DescriptorSetAllocateInfo()
                                                                  .setDescriptorPool(resources.descriptorPool)
                                                                  .setSetLayouts(resources.descriptorLayout)
                                                                  .setDescriptorSetCount(1);
    resources.descriptorSet = PW_ASSERT_VK(mLogicalDevice.allocateDescriptorSets(descriptorAllocInfo))[0];

    const std::vector<vk::WriteDescriptorSet> bufferWriteDescriptorSet{
        vk::WriteDescriptorSet()
            .setDstSet(resources.descriptorSet)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDstBinding(0)
            .setBufferInfo(srcBuffer.descriptorInfo),
        vk::WriteDescriptorSet()
            .setDstSet(resources.descriptorSet)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDstBinding(1)
            .setBufferInfo(dstBuffer.descriptorInfo)};
    mLogicalDevice.updateDescriptorSets(bufferWriteDescriptorSet, {});

    return resources;
}

void VulkanDevice::DestroyComputePipeline(ComputePipelineResources& pipelineResources)
{
    mLogicalDevice.freeDescriptorSets(pipelineResources.descriptorPool, pipelineResources.descriptorSet);
    mLogicalDevice.destroyDescriptorPool(pipelineResources.descriptorPool);
    mLogicalDevice.destroyPipeline(pipelineResources.pipeline);
    mLogicalDevice.destroyShaderModule(pipelineResources.shader);
    mLogicalDevice.destroyPipelineLayout(pipelineResources.pipelineLayout);
    mLogicalDevice.destroyDescriptorSetLayout(pipelineResources.descriptorLayout);
}

vk::CommandBuffer VulkanDevice::CreateCommandBuffer()
{
    vk::CommandBufferAllocateInfo commandInfo =
        vk::CommandBufferAllocateInfo().setCommandBufferCount(1).setCommandPool(mCommandPool).setLevel(vk::CommandBufferLevel::ePrimary);
    return PW_ASSERT_VK(mLogicalDevice.allocateCommandBuffers(commandInfo))[0];
}

void VulkanDevice::SubmitCommand(const vk::CommandBuffer& commandBuffer, const vk::Fence& fence)
{
    const vk::SubmitInfo submitInfo = vk::SubmitInfo().setCommandBuffers(commandBuffer);
    PW_ASSERT_VK(mComputeQueue.submit(submitInfo, fence));
}

void VulkanDevice::DestroyCommand(vk::CommandBuffer& commandBuffer)
{
    mLogicalDevice.freeCommandBuffers(mCommandPool, commandBuffer);
}

vk::Fence VulkanDevice::CreateFence()
{
    const vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo();
    return PW_ASSERT_VK(mLogicalDevice.createFence(fenceInfo));
}

void VulkanDevice::WaitForFence(vk::Fence& fence)
{
    PW_ASSERT_VK(mLogicalDevice.waitForFences(fence, true, (std::numeric_limits<uint64_t>::max)()));
}

void VulkanDevice::DestroyFence(vk::Fence& fence)
{
    mLogicalDevice.destroyFence(fence);
}

VulkanDevice::~VulkanDevice()
{
    mLogicalDevice.destroyCommandPool(mCommandPool);
    mLogicalDevice.destroy();
    mVulkanInstance = nullptr;
}

}  // namespace PixelWeave
