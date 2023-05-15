#include "VulkanVideoConverter.h"

#include <algorithm>

#include "DebugUtils.h"
#include "Timer.h"

namespace PixelWeave
{
VulkanVideoConverter::VulkanVideoConverter(VulkanDevice* device) : mDevice(nullptr), mEnableBenchmark(false)
{
    device->AddRef();
    mDevice = device;
}

Result VulkanVideoConverter::ValidateInput(const VideoFrameWrapper& src, const VideoFrameWrapper& dst)
{
    // Validate resolution
    if (src.width == 0 || src.height == 0) {
        return Result::InvalidInputResolutionError;
    }

    if (dst.width == 0 || dst.height == 0) {
        return Result::InvalidInputResolutionError;
    }

    // Validate input format
    static_assert(AllPixelFormats.size() == 19);
    std::vector<PixelFormat> validInputFormats{
        PixelFormat::Interleaved8BitUYVY,    PixelFormat::Interleaved8BitBGRA,   PixelFormat::Interleaved8BitRGBA,
        PixelFormat::Planar8Bit420,          PixelFormat::Planar8Bit420YV12,     PixelFormat::Planar8Bit420NV12,
        PixelFormat::Interleaved10BitUYVY,   PixelFormat::Interleaved10BitRGB,   PixelFormat::Interleaved12BitRGB,
        PixelFormat::Interleaved8BitARGB,    PixelFormat::Interleaved12BitRGBLE, PixelFormat::Interleaved10BitRGBX,
        PixelFormat::Interleaved10BitRGBXLE, PixelFormat::Planar16BitP216,       PixelFormat::Planar8Bit422,
        PixelFormat::Planar8Bit444,          PixelFormat::Planar10Bit420,        PixelFormat::Planar10Bit422,
        PixelFormat::Planar10Bit444,
    };
    const bool isInputFormatSupported = std::any_of(validInputFormats.begin(), validInputFormats.end(), [&src](const PixelFormat& format) {
        return src.pixelFormat == format;
    });
    if (!isInputFormatSupported) {
        return Result::InvalidInputFormatError;
    }

    // Validate output format
    std::vector<PixelFormat> validOutputFormats{
        PixelFormat::Planar8Bit420,
        PixelFormat::Planar8Bit422,
        PixelFormat::Planar8Bit444,
        PixelFormat::Planar10Bit420,
        PixelFormat::Planar10Bit422,
        PixelFormat::Planar10Bit444,
        PixelFormat::Interleaved8BitUYVY,
        PixelFormat::Interleaved8BitBGRA,
        PixelFormat::Interleaved10BitRGB,
    };
    const bool isOutputFormatSupported =
        std::any_of(validOutputFormats.begin(), validOutputFormats.end(), [&dst](const PixelFormat& format) {
            return dst.pixelFormat == format;
        });
    if (!isOutputFormatSupported) {
        return Result::InvalidOutputFormatError;
    }
    return Result::Success;
}

void VulkanVideoConverter::InitResources(const VideoFrameWrapper& src, VideoFrameWrapper& dst)
{
    // Create source buffer and copy CPU memory into it
    const vk::DeviceSize srcBufferSize = src.GetBufferSize();
    mSrcDeviceBuffer = mDevice->CreateBuffer(
        srcBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_MAPPED_BIT);

    // Create CPU readable dest buffer to do conversions in
    const vk::DeviceSize dstBufferSize = dst.GetBufferSize();
    mDstDeviceBuffer = mDevice->CreateBuffer(
        dstBufferSize,
        vk::BufferUsageFlagBits::eStorageBuffer,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

    // Create compute pipeline and bindings
    mPipelineResources = mDevice->CreateVideoConversionPipeline(src, mSrcDeviceBuffer, dst, mDstDeviceBuffer);
    mCommand = mDevice->CreateCommandBuffer();

    // Record command buffer
    {
        const vk::CommandBufferBeginInfo commandBeginInfo = vk::CommandBufferBeginInfo();
        PW_ASSERT_VK(mCommand.begin(commandBeginInfo));

        if (mEnableBenchmark) {
            mTimestampQueryPool = mDevice->CreateTimestampQueryPool(sTimemestampQueryCount);
        }

        // Copy local memory into VRAM and add barrier for next stage
        /* {
            if (mEnableBenchmark) {
                mCommand.writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe, mTimestampQueryPool, sTimestampStartIndex);
            }
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
            if (mEnableBenchmark) {
                mCommand.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, mTimestampQueryPool, sTimestampSrcTransferDoneIndex);
            }
        }*/

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

        if (mEnableBenchmark) {
            mCommand.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, mTimestampQueryPool, sTimestampConvertIndex);
        }

        // Wait for compute stage and copy results back to local memory
        /* {
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
            if (mEnableBenchmark) {
                mCommand.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, mTimestampQueryPool, sTimestampDstTransferDoneIndex);
            }
        }*/

        PW_ASSERT_VK(mCommand.end());
    }
}

void VulkanVideoConverter::CleanUp()
{
    const bool wasInitialized = mPrevSourceFrame.has_value() && mPrevDstFrame.has_value();
    if (wasInitialized) {
        mDevice->DestroyCommand(mCommand);
        if (mEnableBenchmark) {
            mDevice->DestroyQueryPool(mTimestampQueryPool);
        }
        mDevice->DestroyVideoConversionPipeline(mPipelineResources);
        // mSrcLocalBuffer->Release();
        mSrcDeviceBuffer->Release();
        mDstDeviceBuffer->Release();
        // mDstLocalBuffer->Release();
        mPrevSourceFrame = std::optional<VideoFrameWrapper>();
        mPrevDstFrame = std::optional<VideoFrameWrapper>();
    }
}

Result VulkanVideoConverter::Convert(const VideoFrameWrapper& src, VideoFrameWrapper& dst)
{
    ResultValue<BenchmarkResult> withBenchmarkResult = ConvertInternal(src, dst, false);
    return withBenchmarkResult.result;
}

ResultValue<BenchmarkResult> VulkanVideoConverter::ConvertWithBenchmark(const VideoFrameWrapper& src, VideoFrameWrapper& dst)
{
    return ConvertInternal(src, dst, true);
}

ResultValue<BenchmarkResult> VulkanVideoConverter::ConvertInternal(
    const VideoFrameWrapper& src,
    VideoFrameWrapper& dst,
    const bool enableBenchmark)
{
    // Enable benchmark if GPU timestamps are supported
    mEnableBenchmark = enableBenchmark && mDevice->SupportsTimestamps();

    // Validate input, return nothing on failure
    const Result validationResult = ValidateInput(src, dst);
    if (validationResult != Result::Success) {
        return ResultValue<BenchmarkResult>{validationResult, {}};
    }

    // Initialize resources and cache shaders, buffers, etc
    const bool wasInitialized = mPrevSourceFrame.has_value() && mPrevDstFrame.has_value();
    if (!wasInitialized || !src.AreFramePropertiesEqual(mPrevSourceFrame.value()) || !dst.AreFramePropertiesEqual(mPrevDstFrame.value())) {
        if (wasInitialized) {
            CleanUp();
        }
        InitResources(src, dst);
        mPrevSourceFrame = src;
        mPrevDstFrame = dst;
    }

    // Copy src buffer into GPU readable buffer
    BenchmarkResult benchmarkResult;
    PixelWeave::Timer cpuTimer;
    cpuTimer.Start();
    const vk::DeviceSize srcBufferSize = src.GetBufferSize();
    uint8_t* mappedSrcBuffer = mSrcDeviceBuffer->MapBuffer();
    std::copy_n(src.buffer, srcBufferSize, mappedSrcBuffer);
    mSrcDeviceBuffer->UnmapBuffer();
    benchmarkResult.copyToDeviceVisibleTimeMicros = cpuTimer.ElapsedMicros();

    // Dispatch command in compute queue
    cpuTimer.Start();
    vk::Fence computeFence = mDevice->CreateFence();
    mDevice->SubmitCommand(mCommand, computeFence);
    mDevice->WaitForFence(computeFence);
    mDevice->DestroyFence(computeFence);
    if (mEnableBenchmark) {
        // std::vector<uint64_t> queryResult = mDevice->GetTimestampQueryResults(mTimestampQueryPool, sTimemestampQueryCount);
        // mDevice->ResetQueryPool(mTimestampQueryPool, sTimemestampQueryCount);
        // benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros = queryResult[1] - queryResult[0];
        // benchmarkResult.computeConversionTimeMicros = queryResult[2] - queryResult[1];
        // benchmarkResult.transferDeviceLocalToHostVisibleTimeMicros = queryResult[3] - queryResult[2];
    }

    benchmarkResult.gpuConversionTimeMicros = cpuTimer.ElapsedMicros();

    // Copy contents into CPU buffer
    cpuTimer.Start();
    const vk::DeviceSize dstBufferSize = dst.GetBufferSize();
    uint8_t* mappedDstBuffer = mDstDeviceBuffer->MapBuffer();
    std::copy_n(mappedDstBuffer, dstBufferSize, dst.buffer);
    mDstDeviceBuffer->UnmapBuffer();
    benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros = cpuTimer.ElapsedMicros();

    return ResultValue<BenchmarkResult>{Result::Success, benchmarkResult};
}

VulkanVideoConverter::~VulkanVideoConverter()
{
    CleanUp();
    mDevice->Release();
}

}  // namespace PixelWeave
