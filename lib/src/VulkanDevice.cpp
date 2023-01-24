#include "VulkanDevice.h"

#include <array>
#include <limits>

#include "DebugUtils.h"
#include "ResourceLoader.h"
#include "VideoFrameWrapper.h"
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

    vk::PhysicalDeviceVulkan11Features physicalDeviceFeatures1_1{};
    vk::PhysicalDeviceVulkan12Features physicalDeviceFeatures1_2{};
    physicalDeviceFeatures1_1.setPNext(&physicalDeviceFeatures1_2);
    vk::PhysicalDeviceFeatures2 physicalDeviceFeatures = vk::PhysicalDeviceFeatures2().setPNext(&physicalDeviceFeatures1_1);
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

vk::DeviceMemory VulkanDevice::AllocateMemory(const vk::MemoryPropertyFlags& memoryFlags, const vk::MemoryRequirements memoryRequirements)
{
    // Find suitable memory to allocate the buffer in
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
    return PW_ASSERT_VK(mLogicalDevice.allocateMemory(allocateInfo));
}

VulkanBuffer* VulkanDevice::CreateBuffer(
    const vk::DeviceSize& size,
    const vk::BufferUsageFlags& usageFlags,
    const vk::MemoryPropertyFlags& memoryFlags)
{
    return VulkanBuffer::Create(this, size, usageFlags, memoryFlags);
}

struct SpecializationEntries {
    uint32_t srcPictureWidth;
    uint32_t srcPictureHeight;
    uint32_t srcPictureStride;
    uint32_t srcPictureChromaWidth;
    uint32_t srcPictureChromaHeight;
    uint32_t srcPictureChromaStride;
    uint32_t srcPictureFormat;
    uint32_t srcPictureSubsampleType;
    uint32_t srcPictureUOffset;
    uint32_t srcPictureVOffset;
    uint32_t srcPictureBitDepth;
    uint32_t srcPictureByteDepth;
    uint32_t srcPictureRange;
    uint32_t srcPictureYUVMatrix;

    uint32_t dstPictureWidth;
    uint32_t dstPictureHeight;
    uint32_t dstPictureStride;
    uint32_t dstPictureChromaWidth;
    uint32_t dstPictureChromaHeight;
    uint32_t dstPictureChromaStride;
    uint32_t dstPictureFormat;
    uint32_t dstPictureSubsampleType;
    uint32_t dstPictureUOffset;
    uint32_t dstPictureVOffset;
    uint32_t dstPictureBitDepth;
    uint32_t dstPictureByteDepth;
    uint32_t dstPictureRange;
    uint32_t dstPictureYUVMatrix;
};

struct SpecializationData {
    using SpecializationBindings = std::array<vk::SpecializationMapEntry, 28>;
    SpecializationBindings bindings;
    SpecializationEntries data;
};

SpecializationData CreateSpecializationInfo(const VideoFrameWrapper& src, const VideoFrameWrapper& dst)
{
    // Load specialization data and write based on current buffer properties
    SpecializationData specializationInfo;
    specializationInfo.data = SpecializationEntries{
        src.width,
        src.height,
        src.stride,
        src.GetChromaWidth(),
        src.GetChromaHeight(),
        src.GetChromaStride(),
        static_cast<uint32_t>(src.pixelFormat),
        static_cast<uint32_t>(src.GetSubsampleType()),
        src.GetUOffset(),
        src.GetVOffset(),
        src.GetBitDepth(),
        src.GetByteDepth(),
        static_cast<uint32_t>(src.range),
        static_cast<uint32_t>(src.yuvMatrix),

        dst.width,
        dst.height,
        dst.stride,
        dst.GetChromaWidth(),
        dst.GetChromaHeight(),
        dst.GetChromaStride(),
        static_cast<uint32_t>(dst.pixelFormat),
        static_cast<uint32_t>(dst.GetSubsampleType()),
        dst.GetUOffset(),
        dst.GetVOffset(),
        dst.GetBitDepth(),
        dst.GetByteDepth(),
        static_cast<uint32_t>(dst.range),
        static_cast<uint32_t>(dst.yuvMatrix),
    };

    specializationInfo.bindings = SpecializationData::SpecializationBindings{
        vk::SpecializationMapEntry(0, offsetof(SpecializationEntries, srcPictureWidth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(1, offsetof(SpecializationEntries, srcPictureHeight), sizeof(uint32_t)),
        vk::SpecializationMapEntry(2, offsetof(SpecializationEntries, srcPictureStride), sizeof(uint32_t)),
        vk::SpecializationMapEntry(3, offsetof(SpecializationEntries, srcPictureChromaWidth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(4, offsetof(SpecializationEntries, srcPictureChromaHeight), sizeof(uint32_t)),
        vk::SpecializationMapEntry(5, offsetof(SpecializationEntries, srcPictureChromaStride), sizeof(uint32_t)),
        vk::SpecializationMapEntry(6, offsetof(SpecializationEntries, srcPictureFormat), sizeof(uint32_t)),
        vk::SpecializationMapEntry(7, offsetof(SpecializationEntries, srcPictureSubsampleType), sizeof(uint32_t)),
        vk::SpecializationMapEntry(8, offsetof(SpecializationEntries, srcPictureUOffset), sizeof(uint32_t)),
        vk::SpecializationMapEntry(9, offsetof(SpecializationEntries, srcPictureVOffset), sizeof(uint32_t)),
        vk::SpecializationMapEntry(10, offsetof(SpecializationEntries, srcPictureBitDepth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(11, offsetof(SpecializationEntries, srcPictureByteDepth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(12, offsetof(SpecializationEntries, srcPictureRange), sizeof(uint32_t)),
        vk::SpecializationMapEntry(13, offsetof(SpecializationEntries, srcPictureYUVMatrix), sizeof(uint32_t)),

        vk::SpecializationMapEntry(14, offsetof(SpecializationEntries, dstPictureWidth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(15, offsetof(SpecializationEntries, dstPictureHeight), sizeof(uint32_t)),
        vk::SpecializationMapEntry(16, offsetof(SpecializationEntries, dstPictureStride), sizeof(uint32_t)),
        vk::SpecializationMapEntry(17, offsetof(SpecializationEntries, dstPictureChromaWidth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(18, offsetof(SpecializationEntries, dstPictureChromaHeight), sizeof(uint32_t)),
        vk::SpecializationMapEntry(19, offsetof(SpecializationEntries, dstPictureChromaStride), sizeof(uint32_t)),
        vk::SpecializationMapEntry(20, offsetof(SpecializationEntries, dstPictureFormat), sizeof(uint32_t)),
        vk::SpecializationMapEntry(21, offsetof(SpecializationEntries, dstPictureSubsampleType), sizeof(uint32_t)),
        vk::SpecializationMapEntry(22, offsetof(SpecializationEntries, dstPictureUOffset), sizeof(uint32_t)),
        vk::SpecializationMapEntry(23, offsetof(SpecializationEntries, dstPictureVOffset), sizeof(uint32_t)),
        vk::SpecializationMapEntry(24, offsetof(SpecializationEntries, dstPictureBitDepth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(25, offsetof(SpecializationEntries, dstPictureByteDepth), sizeof(uint32_t)),
        vk::SpecializationMapEntry(26, offsetof(SpecializationEntries, dstPictureRange), sizeof(uint32_t)),
        vk::SpecializationMapEntry(27, offsetof(SpecializationEntries, dstPictureYUVMatrix), sizeof(uint32_t)),
    };

    return specializationInfo;
}

VulkanDevice::VideoConversionPipelineResources VulkanDevice::CreateVideoConversionPipeline(
    const VideoFrameWrapper& src,
    const VulkanBuffer* srcBuffer,
    const VideoFrameWrapper& dst,
    const VulkanBuffer* dstBuffer)
{
    VideoConversionPipelineResources resources;

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
    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo().setSetLayouts(resources.descriptorLayout);
    resources.pipelineLayout = PW_ASSERT_VK(mLogicalDevice.createPipelineLayout(pipelineLayoutInfo));

    Resource shaderResource = ResourceLoader::Load(Resource::Id::ComputeShader);
    vk::ShaderModuleCreateInfo shaderCreateInfo = vk::ShaderModuleCreateInfo()
                                                      .setCodeSize(shaderResource.size * sizeof(uint8_t))
                                                      .setPCode(reinterpret_cast<const uint32_t*>(shaderResource.buffer));
    resources.shader = PW_ASSERT_VK(mLogicalDevice.createShaderModule(shaderCreateInfo));
    ResourceLoader::CleanUp(shaderResource);

    const SpecializationData specializationData = CreateSpecializationInfo(src, dst);
    const vk::SpecializationInfo specializationInfo = vk::SpecializationInfo()
                                                          .setDataSize(sizeof(SpecializationEntries))
                                                          .setPData(&specializationData.data)
                                                          .setMapEntryCount(static_cast<uint32_t>(specializationData.bindings.size()))
                                                          .setPMapEntries(specializationData.bindings.data());

    const vk::PipelineShaderStageCreateInfo stageCreateInfo = vk::PipelineShaderStageCreateInfo()
                                                                  .setStage(vk::ShaderStageFlagBits::eCompute)
                                                                  .setModule(resources.shader)
                                                                  .setPName("main")
                                                                  .setPSpecializationInfo(&specializationInfo);
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

    const std::vector<vk::WriteDescriptorSet> imageWriteDescriptorSet{
        vk::WriteDescriptorSet()
            .setDstSet(resources.descriptorSet)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDstBinding(0)
            .setBufferInfo(srcBuffer->GetDescriptorInfo()),
        vk::WriteDescriptorSet()
            .setDstSet(resources.descriptorSet)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDstBinding(1)
            .setBufferInfo(dstBuffer->GetDescriptorInfo())};
    mLogicalDevice.updateDescriptorSets(imageWriteDescriptorSet, {});

    return resources;
}

void VulkanDevice::DestroyVideoConversionPipeline(VideoConversionPipelineResources& pipelineResources)
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
