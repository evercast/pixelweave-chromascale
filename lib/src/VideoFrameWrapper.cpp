#include "VideoFrameWrapper.h"

#include <cmath>

namespace PixelWeave
{

uint64_t VideoFrameWrapper::GetBufferSize() const
{
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY: {
            return static_cast<uint64_t>(stride) * height;
        }
        case PixelFormat::Interleaved8BitRGBA:
        case PixelFormat::Interleaved8BitBGRA: {
            return static_cast<uint64_t>(width) * height * 4;
        }
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444: {
            const uint64_t lumaSize = static_cast<uint64_t>(stride) * height;
            const uint64_t chromaSize = static_cast<uint64_t>(GetChromaWidth() * GetByteDepth()) * GetChromaHeight();
            return (lumaSize + chromaSize * 2);
        }
        case PixelFormat::Interleaved10BitUYVY: {
            return ((width + 47) / 48) * 128 * height;
        }
        case PixelFormat::Interleaved10BitRGB: {
            return (width * 32 / 8) * height;
        }
        case PixelFormat::Interleaved12BitRGB: {
            return ((width * 36) / 8) * height;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaWidth() const
{
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
            return (width + 1) / 2;
        case PixelFormat::Planar10Bit444:
        case PixelFormat::Planar8Bit444:
            return width;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetChromaHeight() const
{
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar10Bit422:
            return height;
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
            return (height + 1) / 2;
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar10Bit444:
            return height;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetChromaStride() const
{
    return GetChromaWidth() * GetByteDepth();
}

SubsampleType VideoFrameWrapper::GetSubsampleType() const
{
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitBGRA:
        case PixelFormat::Interleaved8BitRGBA:
            return SubsampleType::RGB;
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
            return SubsampleType::YUV420;
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar10Bit422:
            return SubsampleType::YUV422;
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar10Bit444:
            return SubsampleType::YUV444;
        case PixelFormat::Interleaved10BitUYVY:
            return SubsampleType::YUV422;
        case PixelFormat::Interleaved10BitRGB:
        case PixelFormat::Interleaved12BitRGB:
            return SubsampleType::RGB;
    }
    return SubsampleType::RGB;
}

uint32_t VideoFrameWrapper::GetUOffset() const
{
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444:
            return height * stride;
        case PixelFormat::Planar8Bit420YV12:
            return GetVOffset() + GetChromaHeight() * GetChromaStride();
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetVOffset() const
{
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444:
            return GetUOffset() + GetChromaHeight() * GetChromaStride();
        case PixelFormat::Planar8Bit420YV12:
            return height * stride;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetBitDepth() const {
    static_assert(AllPixelFormats.size() == 14);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Interleaved8BitBGRA:
        case PixelFormat::Interleaved8BitRGBA:
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
            return 8;
        case PixelFormat::Interleaved10BitUYVY:
        case PixelFormat::Interleaved10BitRGB:
            return 10;
        case PixelFormat::Interleaved12BitRGB:
            return 12;
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444:
            return 10;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetByteDepth() const
{
    const float bitDepth = static_cast<float>(GetBitDepth());
    return static_cast<uint32_t>(ceilf(bitDepth / 8.0f));
}

}  // namespace PixelWeave
