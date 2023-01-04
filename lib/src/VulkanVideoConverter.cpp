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

void VulkanVideoConverter::InitResources(const VideoFrameWrapper& src, VideoFrameWrapper& dst)
{
    // Create source buffer and copy CPU memory into it
    const vk::DeviceSize srcBufferSize = src.GetBufferSize();
    mSrcLocalBuffer = mDevice->CreateBuffer(
        srcBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    mSrcDeviceBuffer = mDevice->CreateBuffer(
        srcBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // Create CPU readable dest buffer to do conversions in
    const vk::DeviceSize dstBufferSize = dst.GetBufferSize();
    mDstLocalBuffer = mDevice->CreateBuffer(
        dstBufferSize,
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    mDstDeviceBuffer = mDevice->CreateBuffer(
        dstBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    // Create compute pipeline and bindings
    mPipelineResources = mDevice->CreateVideoConversionPipeline(src, mSrcDeviceBuffer, dst, mDstDeviceBuffer);
    mCommand = mDevice->CreateCommandBuffer();

    // Record command buffer
    {
        const vk::CommandBufferBeginInfo commandBeginInfo = vk::CommandBufferBeginInfo();
        PW_ASSERT_VK(mCommand.begin(commandBeginInfo));

        // Copy local memory into VRAM and add barrier for next stage
        {
            mCommand.copyBuffer(
                mSrcLocalBuffer->GetBufferHandle(),
                mSrcDeviceBuffer->GetBufferHandle(),
                vk::BufferCopy().setSize(mSrcLocalBuffer->GetBufferSize()).setDstOffset(0).setSrcOffset(0));
            const vk::BufferMemoryBarrier bufferBarrier = vk::BufferMemoryBarrier()
                                                              .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                                                              .setBuffer(mSrcDeviceBuffer->GetBufferHandle())
                                                              .setOffset(0)
                                                              .setSize(mSrcDeviceBuffer->GetBufferSize());
            mCommand.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlags{},
                {},
                bufferBarrier,
                {});
        }

        // Bind compute shader resources
        mCommand.bindPipeline(vk::PipelineBindPoint::eCompute, mPipelineResources.pipeline);
        mCommand.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            mPipelineResources.pipelineLayout,
            0,
            mPipelineResources.descriptorSet,
            {});

        constexpr uint32_t blockSizeX = 2;
        constexpr uint32_t blockSizeY = 2;

        constexpr uint32_t dispatchSizeX = 16;
        constexpr uint32_t dispatchSizeY = 16;

        const uint32_t blockCountX = ((dst.width + (blockSizeX - 1)) / blockSizeX);
        const uint32_t blockCountY = ((dst.height + (blockSizeY - 1)) / blockSizeY);

        // Add additional execution blocks if dimensions aren't divisible by dispatchSize. The shader will handle graceful reading/writing
        // for now.
        const uint32_t groupCountX = (blockCountX / dispatchSizeX) + (dispatchSizeX - (blockCountX % dispatchSizeX));
        const uint32_t groupCountY = (blockCountY / dispatchSizeY) + (dispatchSizeY - (blockCountY % dispatchSizeY));
        mCommand.dispatch(groupCountX, groupCountY, 1);

        // Wait for compute stage and copy results back to local memory
        {
            const vk::BufferMemoryBarrier bufferBarrier = vk::BufferMemoryBarrier()
                                                              .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                                                              .setBuffer(mDstDeviceBuffer->GetBufferHandle())
                                                              .setOffset(0)
                                                              .setSize(mDstDeviceBuffer->GetBufferSize());
            mCommand.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags{},
                {},
                bufferBarrier,
                {});

            mCommand.copyBuffer(
                mDstDeviceBuffer->GetBufferHandle(),
                mDstLocalBuffer->GetBufferHandle(),
                vk::BufferCopy().setSize(mDstDeviceBuffer->GetBufferSize()).setDstOffset(0).setSrcOffset(0));
        }

        PW_ASSERT_VK(mCommand.end());
    }
}

void VulkanVideoConverter::CleanUp()
{
    const bool wasInitialized = mPrevSourceFrame.has_value() && mPrevDstFrame.has_value();
    if (wasInitialized) {
        mDevice->DestroyCommand(mCommand);
        mDevice->DestroyVideoConversionPipeline(mPipelineResources);
        mSrcLocalBuffer->Release();
        mSrcDeviceBuffer->Release();
        mDstDeviceBuffer->Release();
        mDstLocalBuffer->Release();
        mPrevSourceFrame = std::optional<VideoFrameWrapper>();
        mPrevDstFrame = std::optional<VideoFrameWrapper>();
    }
}

bool AreFramePropertiesEqual(const VideoFrameWrapper& frameA, const VideoFrameWrapper& frameB)
{
    return frameA.stride == frameB.stride && frameA.width == frameB.width && frameA.height == frameB.height &&
           frameA.pixelFormat == frameB.pixelFormat;
}

void VulkanVideoConverter::Convert(const VideoFrameWrapper& src, VideoFrameWrapper& dst)
{
    const bool wasInitialized = mPrevSourceFrame.has_value() && mPrevDstFrame.has_value();
    if (!wasInitialized || !AreFramePropertiesEqual(mPrevSourceFrame.value(), src) ||
        !AreFramePropertiesEqual(mPrevDstFrame.value(), dst)) {
        if (wasInitialized) {
            CleanUp();
        }
        InitResources(src, dst);
        mPrevSourceFrame = src;
        mPrevDstFrame = dst;
    }

    // Copy src buffer into GPU readable buffer
    const vk::DeviceSize srcBufferSize = src.GetBufferSize();
    uint8_t* mappedSrcBuffer = mSrcLocalBuffer->MapBuffer();
    std::copy_n(src.buffer, srcBufferSize, mappedSrcBuffer);
    mSrcLocalBuffer->UnmapBuffer();

    // Dispatch command in compute queue
    vk::Fence computeFence = mDevice->CreateFence();
    mDevice->SubmitCommand(mCommand, computeFence);
    mDevice->WaitForFence(computeFence);
    mDevice->DestroyFence(computeFence);

    // Copy contents into CPU buffer
    const vk::DeviceSize dstBufferSize = dst.GetBufferSize();
    uint8_t* mappedDstBuffer = mDstLocalBuffer->MapBuffer();
    std::copy_n(mappedDstBuffer, dstBufferSize, dst.buffer);
    mDstLocalBuffer->UnmapBuffer();
}

VulkanVideoConverter::~VulkanVideoConverter()
{
    CleanUp();
    mDevice->Release();
}

}  // namespace PixelWeave
