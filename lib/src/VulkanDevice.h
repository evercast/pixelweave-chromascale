#pragma once

#include <memory>

#include "Device.h"
#include "VulkanBase.h"
#include "VulkanBuffer.h"

namespace PixelWeave
{

class VulkanInstance;

class VulkanDevice : public Device
{
public:
    static ResultValue<Device*> Create();

    VulkanDevice(const std::shared_ptr<VulkanInstance>& instance, vk::PhysicalDevice physicalDevice);

    VideoConverter* CreateVideoConverter() override;

    // Memory handling
    vk::DeviceMemory AllocateMemory(const vk::MemoryPropertyFlags& memoryFlags, const vk::MemoryRequirements memoryRequirements);

    VulkanBuffer* CreateBuffer(
        const vk::DeviceSize& size,
        const vk::BufferUsageFlags& usageFlags,
        const VmaAllocationCreateFlags& memoryFlags);

    // Pipeline handling
    struct VideoConversionPipelineResources {
        vk::DescriptorSetLayout descriptorLayout;
        vk::PipelineLayout pipelineLayout;
        vk::ShaderModule shader;
        vk::Pipeline pipeline;
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
    };
    VideoConversionPipelineResources CreateVideoConversionPipeline(
        const VideoFrameWrapper& src,
        const VulkanBuffer* srcBuffer,
        const VideoFrameWrapper& dst,
        const VulkanBuffer* dstBuffer);
    void DestroyVideoConversionPipeline(VideoConversionPipelineResources& pipelineResources);

    vk::CommandBuffer CreateCommandBuffer();
    void SubmitCommand(const vk::CommandBuffer& commandBuffer, const vk::Fence& fence);
    void DestroyCommand(vk::CommandBuffer& commandBuffer);

    bool SupportsTimestamps() const;
    vk::QueryPool CreateTimestampQueryPool(const uint32_t queryCount);
    void ResetQueryPool(vk::QueryPool& queryPool, const uint32_t queryCount);
    std::vector<uint64_t> GetTimestampQueryResults(vk::QueryPool queryPool, const uint32_t queryCount);
    void DestroyQueryPool(vk::QueryPool& queryPool);

    vk::Fence CreateFence();
    void WaitForFence(vk::Fence& fence);
    void DestroyFence(vk::Fence& fence);

    vk::Device& GetLogicalDevice() { return mLogicalDevice; }

    VmaAllocator& GetAllocator() { return mAllocator; }

    ~VulkanDevice() override;

private:
    std::shared_ptr<VulkanInstance> mVulkanInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device mLogicalDevice;
    vk::Queue mComputeQueue;
    vk::CommandPool mCommandPool;
    VmaAllocator mAllocator;
};

}  // namespace PixelWeave
