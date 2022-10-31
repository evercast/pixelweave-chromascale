#pragma once

#include "Macros.h"

#include "Device.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

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
        const vk::MemoryPropertyFlags& memoryFlags);

    VulkanImage* CreateImage(PixelFormat pixelFormat, uint32_t width, uint32_t height, vk::ImageUsageFlags usage);

    // Pipeline handling

    struct ComputePipelineResources {
        vk::DescriptorSetLayout descriptorLayout;
        vk::PipelineLayout pipelineLayout;
        vk::ShaderModule shader;
        vk::Pipeline pipeline;
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
    };
    ComputePipelineResources CreateComputePipeline(const VideoFrameWrapper& src, const VulkanBuffer* srcBuffer, const VideoFrameWrapper& dst, const VulkanBuffer* dstBuffer);
    void DestroyComputePipeline(ComputePipelineResources& pipelineResources);

    vk::CommandBuffer CreateCommandBuffer();
    void SubmitCommand(const vk::CommandBuffer& commandBuffer, const vk::Fence& fence);
    void DestroyCommand(vk::CommandBuffer& commandBuffer);

    vk::Fence CreateFence();
    void WaitForFence(vk::Fence& fence);
    void DestroyFence(vk::Fence& fence);

    vk::Device& GetLogicalDevice() { return mLogicalDevice; }

    ~VulkanDevice() override;

private:
    std::shared_ptr<VulkanInstance> mVulkanInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device mLogicalDevice;
    vk::Queue mComputeQueue;
    vk::CommandPool mCommandPool;
};
}  // namespace PixelWeave
