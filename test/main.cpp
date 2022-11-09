#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#include "Device.h"

using namespace PixelWeave;

struct Timer {
    void Start() { beginTime = std::chrono::steady_clock::now(); }
    template <typename M>
    uint64_t Elapsed()
    {
        return std::chrono::duration_cast<M>(std::chrono::steady_clock::now() - beginTime).count();
    }
    uint64_t ElapsedMillis() { return Elapsed<std::chrono::milliseconds>(); }
    uint64_t ElapsedMicros() { return Elapsed<std::chrono::microseconds>(); }

    std::chrono::steady_clock::time_point beginTime;
};

PixelWeave::VideoFrameWrapper GetUYVYFrame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width * 2;
    const uint32_t bufferSize = height * stride;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t x = 0; x < ((width + 1) / 2); ++x) {
        for (uint32_t y = 0; y < height; ++y) {
            const uint32_t baseIndex = y * stride + x * 4;
            buffer[baseIndex] = 0xB0;      // U
            buffer[baseIndex + 1] = 0xFF;  // Y
            buffer[baseIndex + 2] = 0xC0;  // V
            buffer[baseIndex + 3] = 0xFF;  // Y
        }
    }
    return VideoFrameWrapper{buffer, stride, width, height, PixelWeave::PixelFormat::Interleaved8BitUYVY};
}

PixelWeave::VideoFrameWrapper GetPlanar420Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * width) + chromaWidth * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0xFF;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0xB0;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0xC0;
    }
    return VideoFrameWrapper{buffer, width, width, height, PixelWeave::PixelFormat::Planar8Bit420};
}

PixelWeave::VideoFrameWrapper GetPlanar422Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{buffer, stride, width, height, PixelWeave::PixelFormat::Planar8Bit422};
}

PixelWeave::VideoFrameWrapper GetPlanar444Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 3;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{buffer, stride, width, height, PixelWeave::PixelFormat::Planar8Bit444};
}

PixelWeave::VideoFrameWrapper GetPlanarYV12Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * width) + chromaWidth * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0xFF;
    }
    const uint32_t vSampleOffset = width * height;
    const uint32_t uSampleOffset = vSampleOffset + chromaWidth * chromaHeight;

    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0xC0;
    }
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0xB0;
    }
    return VideoFrameWrapper{buffer, width, width, height, PixelWeave::PixelFormat::Planar8Bit420YV12};
}

PixelWeave::VideoFrameWrapper GetPlanarNV12Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * width) + chromaWidth * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0xFF;
    }
    const uint32_t uvSampleOffset = width * height;
    for (uint32_t uvSampleIndex = 0; uvSampleIndex < 2 * chromaWidth * chromaHeight; uvSampleIndex += 2) {
        buffer[uvSampleOffset + uvSampleIndex] = 0xB0;
        buffer[uvSampleOffset + uvSampleIndex + 1] = 0xC0;
    }
    return VideoFrameWrapper{buffer, width, width, height, PixelWeave::PixelFormat::Planar8Bit420NV12};
}

PixelWeave::VideoFrameWrapper GetRGBAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0x10;
        buffer[sampleIndex * 4 + 1] = 0x10;
        buffer[sampleIndex * 4 + 2] = 0x10;
        buffer[sampleIndex * 4 + 3] = 0x00;
    }
    return VideoFrameWrapper{buffer, width * 4, width, height, PixelWeave::PixelFormat::Interleaved8BitRGBA};
}

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {
        constexpr uint32_t srcWidth = 32;
        constexpr uint32_t srcHeight = 32;
        VideoFrameWrapper srcFrame = GetRGBAFrame(srcWidth, srcHeight);
        constexpr uint32_t dstWidth = 32;
        constexpr uint32_t dstHeight = 32;
        VideoFrameWrapper dstFrame = GetPlanar444Frame(dstWidth, dstHeight);

        const auto videoConverter = device->CreateVideoConverter();
        for (int i = 0; i < 10; ++i) {
            Timer timer;
            timer.Start();
            videoConverter->Convert(srcFrame, dstFrame);
            std::cout << "Processing frame " << i << " took " << timer.ElapsedMillis() << "ms (" << timer.ElapsedMicros() << " us)"
                      << std::endl;
        }
        videoConverter->Release();
        device->Release();

        delete[] srcFrame.buffer;
        delete[] dstFrame.buffer;

        return 0;
    }
    return -1;
}