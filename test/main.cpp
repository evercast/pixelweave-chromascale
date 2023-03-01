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
    return VideoFrameWrapper{buffer, stride, 0, width, height, PixelWeave::PixelFormat::Interleaved8BitUYVY};
}

PixelWeave::VideoFrameWrapper GetPlanar420Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t stride = width + 8;
    const uint32_t chromaStride = chromaWidth + 8;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * stride) + chromaStride * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t indexY = 0; indexY < height; ++indexY) {
        for (uint32_t indexX = 0; indexX < width; ++indexX) {
            uint32_t sampleIndex = indexY * stride + indexX;
            buffer[sampleIndex] = 0xFF;
        }
    }
    
    const uint32_t uSampleOffset = stride * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaStride * chromaHeight;
    for (uint32_t chromaIndexY = 0; chromaIndexY < chromaHeight; ++chromaIndexY) {
        for (uint32_t chromaIndexX = 0; chromaIndexX < chromaWidth; ++chromaIndexX) {
            uint32_t chromaSampleIndex = chromaIndexY * chromaStride + chromaIndexX;
            buffer[uSampleOffset + chromaSampleIndex] = chromaIndexY % 2 == 0 ? 0xAA : 0xBB;
            buffer[vSampleOffset + chromaSampleIndex] = chromaIndexY % 2 == 0 ? 0xCC : 0xDD;
        }
    }
    return VideoFrameWrapper{buffer, stride, chromaStride, width, height, PixelWeave::PixelFormat::Planar8Bit420, PixelWeave::Range::Limited};
}

PixelWeave::VideoFrameWrapper GetPlanar422Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{buffer, stride, (width + 1) / 2, width, height, PixelWeave::PixelFormat::Planar8Bit422};
}

PixelWeave::VideoFrameWrapper GetPlanar444Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 3;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{buffer, stride, stride, width, height, PixelWeave::PixelFormat::Planar8Bit444};
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
    return VideoFrameWrapper{buffer, width, (width + 1) / 2, width, height, PixelWeave::PixelFormat::Planar8Bit420YV12};
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
    return VideoFrameWrapper{buffer, width, (width + 1) / 2, width, height, PixelWeave::PixelFormat::Planar8Bit420NV12};
}

PixelWeave::VideoFrameWrapper GetRGBAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{buffer, width * 4, 0, width, height, PixelWeave::PixelFormat::Interleaved8BitRGBA, PixelWeave::Range::Full};
}

PixelWeave::VideoFrameWrapper GetBGRAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{buffer, width * 4, 0, width, height, PixelWeave::PixelFormat::Interleaved8BitBGRA, PixelWeave::Range::Full};
}

PixelWeave::VideoFrameWrapper GetPlanar42010BitFrame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = 2 * ((height * width) + chromaWidth * chromaHeight * 2);
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0x3FF;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0x3FF;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0x3FF;
    }
    return VideoFrameWrapper{
        reinterpret_cast<uint8_t*>(buffer),
        width * 2,
        ((width + 1) / 2) * 2,
        width,
        height,
        PixelWeave::PixelFormat::Planar10Bit420,
        PixelWeave::Range::Full};
}

PixelWeave::VideoFrameWrapper GetPlanar42210BitFrame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = height;
    const uint32_t bufferSize = 2 * ((height * width) + chromaWidth * chromaHeight * 2);
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0x03FF;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0x03FF;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 2, ((width + 1) / 2) * 2,  width, height, PixelWeave::PixelFormat::Planar10Bit422};
}

PixelWeave::VideoFrameWrapper GetPlanar44410BitFrame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = width;
    const uint32_t chromaHeight = height;
    const uint32_t bufferSize = 2 * ((height * width) + chromaWidth * chromaHeight * 2);
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0x3FF;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0x3FF;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 2, width * 2, width, height, PixelWeave::PixelFormat::Planar10Bit444};
}

PixelWeave::VideoFrameWrapper Get10BitRGBBuffer(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = width * height;
    uint32_t* buffer = new uint32_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 4, 0, width, height, PixelWeave::PixelFormat::Interleaved10BitRGB};
}

PixelWeave::VideoFrameWrapper GetPlanarP126Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = height;
    const uint32_t bufferSize = height * width + chromaWidth * chromaHeight * 2;
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0x00FF;
    }
    const uint32_t uvSampleOffset = width * height;
    
    for (uint32_t lineIndex = 0; lineIndex < chromaHeight; ++lineIndex) {
        const uint16_t sample = lineIndex % 2 == 1 ? 0 : 0xFFFF;
        const uint32_t lineOffset = chromaWidth * 2 * lineIndex;
        for (uint32_t uvSampleIndex = 0; uvSampleIndex < chromaWidth * 2; uvSampleIndex += 2) {
            buffer[uvSampleOffset + lineOffset + uvSampleIndex] = sample;
            buffer[uvSampleOffset + lineOffset + uvSampleIndex + 1] = sample;
        }
    }

    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 2, 0, width, height, PixelWeave::PixelFormat::Planar16BitP216};
}

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {
        constexpr uint32_t srcWidth = 240;
        constexpr uint32_t srcHeight = 135;
        VideoFrameWrapper srcFrame = GetPlanar420Frame(srcWidth, srcHeight);
        constexpr uint32_t dstWidth = 240;
        constexpr uint32_t dstHeight = 135;
        VideoFrameWrapper dstFrame = Get10BitRGBBuffer(dstWidth, dstHeight);

        const auto videoConverter = device->CreateVideoConverter();
        uint64_t totalTime = 0;
        const int totalFrames = 100;
        for (int i = 0; i < totalFrames; ++i) {
            Timer timer;
            timer.Start();
            if (videoConverter->Convert(srcFrame, dstFrame) != PixelWeave::Result::Success) {
                std::cout << "Conversion failed" << std::endl;
            }
            totalTime += timer.ElapsedMicros();
            std::cout << "Processing frame " << i << " took " << timer.ElapsedMillis() << "ms (" << timer.ElapsedMicros() << " us)"
                      << std::endl;
        }
        std::cout << "Average time: " << static_cast<double>(totalTime) / (1000.0 * static_cast<double>(totalFrames)) << " ms" << std::endl;

        videoConverter->Release();
        device->Release();

        delete[] srcFrame.buffer;
        delete[] dstFrame.buffer;

        return 0;
    }
    return -1;
}
