#include "VulkanVideoConverter.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include "DebugUtils.h"

namespace PixelWeave
{
VulkanVideoConverter::VulkanVideoConverter(VulkanDevice* device) : mDevice(nullptr)
{
    device->AddRef();
    mDevice = device;
}

void VulkanVideoConverter::InitResources(const ProtoVideoFrame& src, ProtoVideoFrame& dst)
{
    // Create source buffer and copy CPU memory into it
    const vk::DeviceSize srcBufferSize = src.stride * src.height;
    mSrcBuffer = mDevice->CreateBuffer(
        srcBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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
}

void VulkanVideoConverter::Cleanup()
{
    mDevice->DestroyCommand(mCommand);
    mDevice->DestroyComputePipeline(mPipelineResources);
    mDevice->DestroyBuffer(mSrcBuffer);
    mDevice->DestroyBuffer(mDstBuffer);
}

struct Timer {
    void Start() { beginTime = std::chrono::steady_clock::now(); }
    uint64_t ElapsedMillis()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beginTime).count();
    }
    std::chrono::steady_clock::time_point beginTime;
};

bool AreFramePropertiesEqual(const ProtoVideoFrame& frameA, const ProtoVideoFrame& frameB)
{
    return frameA.stride == frameB.stride && frameA.width == frameB.width && frameA.height == frameB.height &&
           frameA.pixelFormat == frameB.pixelFormat;
}

void VulkanVideoConverter::Convert(const ProtoVideoFrame& src, ProtoVideoFrame& dst)
{
    Timer timer;
    timer.Start();

    const bool wasInitialized = mPrevSourceFrame.has_value() && mPrevDstFrame.has_value();
    if (!wasInitialized || !AreFramePropertiesEqual(mPrevSourceFrame.value(), src) ||
        !AreFramePropertiesEqual(mPrevDstFrame.value(), dst)) {
        if (wasInitialized) {
            Cleanup();
        }
        InitResources(src, dst);
        mPrevSourceFrame = src;
        mPrevDstFrame = dst;
    }

    // Copy src buffer into GPU readable buffer
    Timer stageTimer;
    stageTimer.Start();
    const vk::DeviceSize srcBufferSize = src.stride * src.height;
    uint8_t* mappedSrcBuffer = mDevice->MapBuffer(mSrcBuffer);
    std::copy_n(src.buffer, srcBufferSize, mappedSrcBuffer);
    mDevice->UnmapBuffer(mSrcBuffer);
    PW_LOG("Copying src buffer took " << stageTimer.ElapsedMillis() << " ms");

    const vk::DeviceSize dstBufferSize = dst.stride * dst.height + dst.height * ((dst.width + 1) / 2) + dst.height * ((dst.width + 1) / 2);

    // Dispatch command in compute queue
    stageTimer.Start();
    vk::Fence computeFence = mDevice->CreateFence();
    mDevice->SubmitCommand(mCommand, computeFence);
    mDevice->WaitForFence(computeFence);
    mDevice->DestroyFence(computeFence);
    PW_LOG("Compute took " << stageTimer.ElapsedMillis() << " ms");

    // Copy contents into CPU buffer
    stageTimer.Start();
    uint8_t* mappedDstBuffer = mDevice->MapBuffer(mDstBuffer);
    std::copy_n(mappedDstBuffer, dstBufferSize, dst.buffer);
    mDevice->UnmapBuffer(mDstBuffer);
    PW_LOG("Copy back took " << stageTimer.ElapsedMillis() << " ms");

    PW_LOG("Total frame processing time: " << timer.ElapsedMillis() << " ms" << std::endl);
}

VulkanVideoConverter::~VulkanVideoConverter()
{
    Cleanup();
    mDevice->Release();
}

}  // namespace PixelWeave
