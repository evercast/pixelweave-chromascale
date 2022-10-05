#pragma once

#include "Device.h"

#include <vulkan/vulkan.hpp>

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
    struct Buffer {
        vk::DeviceSize size;
        vk::Buffer bufferHandle;
        vk::DeviceMemory memoryHandle;
    };

    Buffer CreateBuffer(
        const vk::DeviceSize& size,
        const vk::BufferUsageFlags& usageFlags,
        const vk::MemoryPropertyFlags& memoryFlags);

    uint8_t* MapBuffer(Buffer& buffer);
    void UnmapBuffer(Buffer& buffer);

    void DestroyBuffer(Buffer& buffer);

    // Pipeline handling

    struct ComputePipelineResources {
        vk::DescriptorSetLayout descriptorLayout;
        vk::PipelineLayout pipelineLayout;
        vk::ShaderModule shader;
        vk::Pipeline pipeline;
    };
    ComputePipelineResources CreateComputePipeline();

    ~VulkanDevice() override;

private:
    std::shared_ptr<VulkanInstance> mVulkanInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device mLogicalDevice;
    vk::Queue mComputeQueue;
    vk::CommandPool mCommandPool;
};
}  // namespace PixelWeave