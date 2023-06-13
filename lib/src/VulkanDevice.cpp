#include "VulkanDevice.h"

#include <array>
#include <limits>

#define VMA_IMPLEMENTATION
#pragma warning(push, 0)
#include "vk_mem_alloc.h"
#pragma warning(pop)
#include "glm/glm.hpp"
#include "shaderc/shaderc.hpp"

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

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorInfo.physicalDevice = mPhysicalDevice;
    allocatorInfo.device = mLogicalDevice;
    allocatorInfo.instance = mVulkanInstance->GetHandle();
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    vmaCreateAllocator(&allocatorInfo, &mAllocator);
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
    const VmaAllocationCreateFlags& memoryFlags)
{
    return VulkanBuffer::Create(this, size, usageFlags, memoryFlags);
}

glm::mat3 GetMatrix(PixelWeave::YUVMatrix matrix)
{
    constexpr float KR709 = 0.2126f;
    constexpr float KB709 = 0.0722f;

    constexpr float KR2020 = 0.2627f;
    constexpr float KB2020 = 0.0593f;
    const auto getMatrix = [](float kr, float kb) -> glm::mat3 {
        float kg = 1.0f - kr - kb;
        return glm::mat3(
            kr,
            -0.5f * (kr / (1.0f - kb)),
            0.5f,
            kg,
            -0.5f * (kg / (1.0f - kb)),
            -0.5f * (kg / (1.0f - kr)),
            kb,
            0.5f,
            -0.5f * (kb / (1.0f - kr)));
    };

    switch (matrix) {
        case YUVMatrix::BT709:
            return getMatrix(KR709, KB709);
        case YUVMatrix::BT2020:
            return getMatrix(KR2020, KB2020);
    }
    return glm::mat3();
}

glm::vec3 GetYUVScale(Range range, [[maybe_unused]] uint32_t bitDepth)
{
    return range == Range::Limited ? glm::vec3((235.0 - 16.0) / 255.0, (240.0 - 16.0) / 255.0, (240.0 - 16.0) / 255.0) : glm::vec3(1.0);
}

glm::vec3 GetYUVOffset(Range range, uint32_t bitDepth)
{
    return glm::vec3(
               (range == Range::Limited) ? float(1 << (bitDepth - 4)) : 0.0f,
               float(1 << (bitDepth - 1)),
               float(1 << (bitDepth - 1))) /
           glm::vec3(float((1 << bitDepth) - 1));
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

    std::vector<uint32_t> compiledShader;
    {
        Resource shaderResource = ResourceLoader::Load(Resource::Id::ComputeShader);

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_performance);

        const auto encodeMatrix = [](const glm::mat3& matrix) -> std::string {
            std::ostringstream stringStream;
            stringStream << "mat3(";
            for (uint32_t i = 0; i < 3 * 3; ++i) {
                stringStream << matrix[i % 3][i / 3];
                if (i < 3 * 3 - 1) {
                    stringStream << ",";
                }
            }
            stringStream << ")";
            return stringStream.str();
        };

        const auto encodeVector = [](const glm::vec3& vector) -> std::string {
            std::ostringstream stringStream;
            stringStream << "vec3(";
            for (uint32_t i = 0; i < 3; ++i) {
                stringStream << vector[i];
                if (i < 2) {
                    stringStream << ",";
                }
            }
            stringStream << ")";
            return stringStream.str();
        };

        const glm::mat3 srcMatrix = GetMatrix(src.yuvMatrix);
        const glm::mat3 srcMatrixInv = glm::inverse(srcMatrix);

        const glm::mat3 dstMatrix = GetMatrix(dst.yuvMatrix);
        const glm::mat3 dstMatrixInv = glm::inverse(dstMatrix);

        options.AddMacroDefinition("SRC_PICTURE_WIDTH", std::to_string(src.width));
        options.AddMacroDefinition("SRC_PICTURE_HEIGHT", std::to_string(src.height));
        options.AddMacroDefinition("SRC_PICTURE_STRIDE", std::to_string(src.stride));
        options.AddMacroDefinition("SRC_PICTURE_CHROMA_WIDTH", std::to_string(src.GetChromaWidth()));
        options.AddMacroDefinition("SRC_PICTURE_CHROMA_HEIGHT", std::to_string(src.GetChromaHeight()));
        options.AddMacroDefinition("SRC_PICTURE_CHROMA_STRIDE", std::to_string(src.GetChromaStride()));
        options.AddMacroDefinition("SRC_PICTURE_FORMAT", std::to_string(static_cast<uint32_t>(src.pixelFormat)));
        options.AddMacroDefinition("SRC_PICTURE_SUBSAMPLE_TYPE", std::to_string(static_cast<uint32_t>(src.GetSubsampleType())));
        options.AddMacroDefinition("SRC_PICTURE_U_OFFSET", std::to_string(src.GetUOffset()));
        options.AddMacroDefinition("SRC_PICTURE_V_OFFSET", std::to_string(src.GetVOffset()));
        options.AddMacroDefinition("SRC_PICTURE_BIT_DEPTH", std::to_string(src.GetBitDepth()));
        options.AddMacroDefinition("SRC_PICTURE_BYTE_DEPTH", std::to_string(src.GetByteDepth()));
        options.AddMacroDefinition("SRC_PICTURE_RANGE", std::to_string(static_cast<uint32_t>(src.range)));
        options.AddMacroDefinition("SRC_PICTURE_YUV_MATRIX", std::to_string(static_cast<uint32_t>(src.yuvMatrix)));
        options.AddMacroDefinition("SRC_PICTURE_RGB_TO_YUV_MATRIX", encodeMatrix(srcMatrix));
        options.AddMacroDefinition("SRC_PICTURE_YUV_TO_RGB_MATRIX", encodeMatrix(srcMatrixInv));
        options.AddMacroDefinition("SRC_PICTURE_YUV_OFFSET", encodeVector(GetYUVOffset(src.range, src.GetBitDepth())));
        options.AddMacroDefinition("SRC_PICTURE_YUV_OFFSET_FULL", encodeVector(GetYUVOffset(Range::Full, src.GetBitDepth())));
        options.AddMacroDefinition("SRC_PICTURE_YUV_SCALE", encodeVector(GetYUVScale(src.range, src.GetBitDepth())));

        options.AddMacroDefinition("DST_PICTURE_WIDTH", std::to_string(dst.width));
        options.AddMacroDefinition("DST_PICTURE_HEIGHT", std::to_string(dst.height));
        options.AddMacroDefinition("DST_PICTURE_STRIDE", std::to_string(dst.stride));
        options.AddMacroDefinition("DST_PICTURE_CHROMA_WIDTH", std::to_string(dst.GetChromaWidth()));
        options.AddMacroDefinition("DST_PICTURE_CHROMA_HEIGHT", std::to_string(dst.GetChromaHeight()));
        options.AddMacroDefinition("DST_PICTURE_CHROMA_STRIDE", std::to_string(dst.GetChromaStride()));
        options.AddMacroDefinition("DST_PICTURE_FORMAT", std::to_string(static_cast<uint32_t>(dst.pixelFormat)));
        options.AddMacroDefinition("DST_PICTURE_SUBSAMPLE_TYPE", std::to_string(static_cast<uint32_t>(dst.GetSubsampleType())));
        options.AddMacroDefinition("DST_PICTURE_U_OFFSET", std::to_string(dst.GetUOffset()));
        options.AddMacroDefinition("DST_PICTURE_V_OFFSET", std::to_string(dst.GetVOffset()));
        options.AddMacroDefinition("DST_PICTURE_BIT_DEPTH", std::to_string(dst.GetBitDepth()));
        options.AddMacroDefinition("DST_PICTURE_BYTE_DEPTH", std::to_string(dst.GetByteDepth()));
        options.AddMacroDefinition("DST_PICTURE_RANGE", std::to_string(static_cast<uint32_t>(dst.range)));
        options.AddMacroDefinition("DST_PICTURE_YUV_MATRIX", std::to_string(static_cast<uint32_t>(dst.yuvMatrix)));
        options.AddMacroDefinition("DST_PICTURE_RGB_TO_YUV_MATRIX", encodeMatrix(dstMatrix));
        options.AddMacroDefinition("DST_PICTURE_YUV_TO_RGB_MATRIX", encodeMatrix(dstMatrixInv));
        options.AddMacroDefinition("DST_PICTURE_YUV_OFFSET", encodeVector(GetYUVOffset(dst.range, dst.GetBitDepth())));
        options.AddMacroDefinition("DST_PICTURE_YUV_OFFSET_FULL", encodeVector(GetYUVOffset(Range::Full, dst.GetBitDepth())));
        options.AddMacroDefinition("DST_PICTURE_YUV_SCALE", encodeVector(GetYUVScale(dst.range, dst.GetBitDepth())));

        /* {
                    shaderc::PreprocessedSourceCompilationResult proprocessResult = compiler.PreprocessGlsl(
                        reinterpret_cast<const char*>(shaderResource.buffer),
                        shaderResource.size,
                        shaderc_shader_kind::shaderc_glsl_compute_shader,
                        "convert.comp",
                        options);
                    if (proprocessResult.GetCompilationStatus() != shaderc_compilation_status_success) {
                        std::cout << "Error building shader: \n" << proprocessResult.GetErrorMessage() << std::endl;
                    } else {
                        std::vector<char> preprocessedCode = std::vector<char>(proprocessResult.cbegin(), proprocessResult.cend());
                        preprocessedCode.push_back('\0');
                        std::cout << preprocessedCode.data() << std::endl;
                    }
                }
        */

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
            reinterpret_cast<const char*>(shaderResource.buffer),
            shaderResource.size,
            shaderc_shader_kind::shaderc_glsl_compute_shader,
            "convert.comp",
            options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::cerr << module.GetErrorMessage();
        }

        compiledShader = std::vector<uint32_t>(module.cbegin(), module.cend());
        ResourceLoader::CleanUp(shaderResource);
    }
    vk::ShaderModuleCreateInfo shaderCreateInfo =
        vk::ShaderModuleCreateInfo().setCodeSize(compiledShader.size() * sizeof(uint32_t)).setPCode(compiledShader.data());
    resources.shader = PW_ASSERT_VK(mLogicalDevice.createShaderModule(shaderCreateInfo));

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

bool VulkanDevice::SupportsTimestamps() const
{
    const vk::PhysicalDeviceProperties deviceProperties = mPhysicalDevice.getProperties();
    return deviceProperties.limits.timestampComputeAndGraphics;
}

vk::QueryPool VulkanDevice::CreateTimestampQueryPool(const uint32_t queryCount)
{
    vk::QueryPoolCreateInfo queryPoolCreateInfo =
        vk::QueryPoolCreateInfo().setQueryType(vk::QueryType::eTimestamp).setQueryCount(queryCount);
    vk::QueryPool queryPool = PW_ASSERT_VK(mLogicalDevice.createQueryPool(queryPoolCreateInfo));
    mLogicalDevice.resetQueryPool(queryPool, 0, queryCount);
    return queryPool;
}

void VulkanDevice::ResetQueryPool(vk::QueryPool& queryPool, const uint32_t queryCount)
{
    mLogicalDevice.resetQueryPool(queryPool, 0, queryCount);
}

std::vector<uint64_t> VulkanDevice::GetTimestampQueryResults(vk::QueryPool queryPool, const uint32_t queryCount)
{
    std::vector<uint64_t> results = PW_ASSERT_VK(mLogicalDevice.getQueryPoolResults<uint64_t>(
        queryPool,
        0,
        queryCount,
        size_t(queryCount * sizeof(uint64_t)),
        vk::DeviceSize(sizeof(uint64_t)),
        vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait));
    const double timePeriod = mPhysicalDevice.getProperties().limits.timestampPeriod;
    for (uint64_t& result : results) {
        result = static_cast<uint64_t>(static_cast<double>(result) * timePeriod / 1000.0);
    }
    return results;
}

void VulkanDevice::DestroyQueryPool(vk::QueryPool& queryPool)
{
    mLogicalDevice.destroyQueryPool(queryPool);
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
    vmaDestroyAllocator(mAllocator);
    mLogicalDevice.destroyCommandPool(mCommandPool);
    mLogicalDevice.destroy();
    mVulkanInstance = nullptr;
}

}  // namespace PixelWeave
