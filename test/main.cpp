#include <memory>
#include <vector>

#include "Device.h"

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {
        const uint32_t srcWidth = 1920;
        const uint32_t srcHeight = 1080;
        const uint64_t srcStride = srcWidth * 2;
        const uint64_t srcBufferSize = srcHeight * srcStride;
        uint8_t* srcBuffer = new uint8_t[srcBufferSize];
        for (uint32_t x = 0; x < ((srcWidth + 1) / 2); ++x) {
            for (uint32_t y = 0; y < srcHeight; ++y) {
                const uint32_t baseIndex = y * srcStride + x * 4;
                srcBuffer[baseIndex] = 0xC0;      // U
                srcBuffer[baseIndex + 1] = 0xFF;  // Y
                srcBuffer[baseIndex + 2] = 0x0C;  // V
                srcBuffer[baseIndex + 3] = 0xFF;  // Y
            }
        }
        PixelWeave::ProtoVideoFrame srcFrame{srcBuffer, srcStride, srcWidth, srcHeight, PixelWeave::PixelFormat::Interleaved8BitUYVY};

        const uint32_t dstWidth = 1920;
        const uint32_t dstHeight = 1080;
        const uint64_t dstStride = dstWidth;
        const uint64_t dstBufferSize = dstHeight * dstStride * 2;
        uint8_t* dstBuffer = new uint8_t[dstBufferSize];
        for (uint32_t bufferIndex = 0; bufferIndex < dstBufferSize; ++bufferIndex) {
            dstBuffer[bufferIndex] = 0;
        }
        PixelWeave::ProtoVideoFrame dstFrame{dstBuffer, dstStride, dstWidth, dstHeight, PixelWeave::PixelFormat::Planar8Bit422};

        const auto videoConverter = device->CreateVideoConverter();
        videoConverter->Convert(srcFrame, dstFrame);
        videoConverter->Release();
        device->Release();

        delete[] srcBuffer;
        delete[] dstBuffer;

        return 0;
    }
    return -1;
}