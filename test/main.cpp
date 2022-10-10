#include <vector>
#include <cassert>

#include "Device.h"

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {
        auto videoConverter = device->CreateVideoConverter();

        std::vector<uint8_t> fakeSrcBuffer(16 * 16 * 4, 0x0F);
        PixelWeave::ProtoVideoFrame srcFrame{fakeSrcBuffer.data(), 16 * 4, 16, 16, PixelWeave::PixelFormat::Interleaved8BitUYVY};

        std::vector<uint8_t> fakeDstBuffer(16 * 16 * 4, 0xFF);
        PixelWeave::ProtoVideoFrame dstFrame{fakeSrcBuffer.data(), 16 * 4, 16, 16, PixelWeave::PixelFormat::Interleaved8BitUYVY};

        videoConverter->Convert(srcFrame, dstFrame);

        videoConverter->Release();
        device->Release();
        return 0;
    }
    return -1;
}