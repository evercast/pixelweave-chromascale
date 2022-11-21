#include "VideoFrameWrapper.h"

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
        case PixelFormat::Planar8Bit420NV12: {
            const uint64_t lumaSize = static_cast<uint64_t>(stride) * height;
            const uint64_t chromaSize = static_cast<uint64_t>(GetChromaWidth()) * GetChromaHeight();
            return lumaSize + chromaSize * 2;
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
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444: {
            const uint64_t lumaSize = static_cast<uint64_t>(stride) * height * 2;
            const uint64_t chromaSize = static_cast<uint64_t>(GetChromaWidth()) * GetChromaHeight() * 2;
            return lumaSize + chromaSize * 2;
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
    return GetChromaWidth();
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

}  // namespace PixelWeave
