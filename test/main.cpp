#include <vector>

#include "Device.h"

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {
        auto videoConverter = device->CreateVideoConverter();

        std::vector<uint8_t> fakeSrcBuffer(1280 * 720 * 4, 0);
        PixelWeave::ProtoVideoFrame srcFrame{fakeSrcBuffer.data(), 1280 * 4, 1280, 720, PixelWeave::PixelFormat::Interleaved8BitUYVY};

        std::vector<uint8_t> fakeDstBuffer(1280 * 720 * 4, 0);
        PixelWeave::ProtoVideoFrame dstFrame{fakeSrcBuffer.data(), 1280 * 4, 1280, 720, PixelWeave::PixelFormat::Planar8Bit422};

        videoConverter->Convert(srcFrame, dstFrame);

        videoConverter->Release();
        device->Release();
        return 0;
    }
    return -1;
}