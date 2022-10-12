#include "VulkanVideoConverter.h"

#include <algorithm>

#include "DebugUtils.h"

namespace PixelWeave
{
VulkanVideoConverter::VulkanVideoConverter(VulkanDevice* device) : mDevice(nullptr)
{
    device->AddRef();
    mDevice = device;
}

void VulkanVideoConverter::Convert(const ProtoVideoFrame& src, ProtoVideoFrame& dst)
{
    // Create source buffer and copy CPU memory into it
    const vk::DeviceSize srcBufferSize = src.stride * src.height;
    mSrcBuffer = mDevice->CreateBuffer(
        srcBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    uint8_t* mappedSrcBuffer = mDevice->MapBuffer(mSrcBuffer);
    std::copy_n(src.buffer, srcBufferSize, mappedSrcBuffer);
    mDevice->UnmapBuffer(mSrcBuffer);

    // Create CPU readable dest buffer to do conversions in
    const vk::DeviceSize dstBufferSize = dst.stride * dst.height + dst.height * ((dst.width + 1) / 2) + dst.height * ((dst.width + 1) / 2);
    mDstBuffer = mDevice->CreateBuffer(
        dstBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    // Create compute pipeline and bindings
    mPipelineResources = mDevice->CreateComputePipeline(mSrcBuffer, mDstBuffer);
    mCommand = mDevice->CreateCommandBuffer();

    // Record command buffer
    {
        const vk::CommandBufferBeginInfo commandBeginInfo = vk::CommandBufferBeginInfo();
        PW_ASSERT_VK(mCommand.begin(commandBeginInfo));
        mCommand.bindPipeline(vk::PipelineBindPoint::eCompute, mPipelineResources.pipeline);
        mCommand.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            mPipelineResources.pipelineLayout,
            0,
            mPipelineResources.descriptorSet,
            {});

        // Hardcoded to 422 for now
        const uint32_t srcChromaStride = (src.width + 1) / 2;
        const uint32_t srcChromaHeight = src.height;
        const uint32_t srcChromaWidth = (src.width + 1) / 2;

        const uint32_t dstChromaStride = (dst.width + 1) / 2;
        const uint32_t dstChromaHeight = dst.height;
        const uint32_t dstChromaWidth = (dst.width + 1) / 2;

        const InOutPictureInfo pictureInfo{
            {src.width, src.height, src.stride, srcChromaWidth, srcChromaHeight, srcChromaStride},
            {dst.width, dst.height, dst.stride, dstChromaWidth, dstChromaHeight, dstChromaStride}};
        mCommand
            .pushConstants(mPipelineResources.pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(InOutPictureInfo), &pictureInfo);

        const uint32_t groupCountX = dstChromaWidth / 16;
        const uint32_t groupCountY = dstChromaHeight / 16;
        mCommand.dispatch(groupCountX, groupCountY, 1);

        PW_ASSERT_VK(mCommand.end());
    }

    // Dispatch command in compute queue
    vk::Fence computeFence = mDevice->CreateFence();
    mDevice->SubmitCommand(mCommand, computeFence);
    mDevice->WaitForFence(computeFence);
    mDevice->DestroyFence(computeFence);

    // Copy contents into CPU buffer
    uint8_t* mappedDstBuffer = mDevice->MapBuffer(mDstBuffer);
    std::copy_n(mappedDstBuffer, dstBufferSize, dst.buffer);
    mDevice->UnmapBuffer(mDstBuffer);

    // Cleanup
    mDevice->DestroyCommand(mCommand);
    mDevice->DestroyComputePipeline(mPipelineResources);
    mDevice->DestroyBuffer(mSrcBuffer);
    mDevice->DestroyBuffer(mDstBuffer);
}

VulkanVideoConverter::~VulkanVideoConverter()
{
    mDevice->Release();
}

}  // namespace PixelWeave
