#include "VideoFrameWrapper.h"

#include <cmath>

namespace PixelWeave
{

uint64_t VideoFrameWrapper::GetBufferSize() const
{
    static_assert(AllPixelFormats.size() == 19);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY: {
            return static_cast<uint64_t>(stride) * height;
        }
        case PixelFormat::Interleaved8BitRGBA:
        case PixelFormat::Interleaved8BitBGRA:
        case PixelFormat::Interleaved8BitARGB: {
            if (stride > 0) {
                return static_cast<uint64_t>(stride) * height;
            } else {
                return static_cast<uint64_t>(width) * height * 4;
            }
        }
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444:
        case PixelFormat::Planar16BitP216: {
            const uint64_t lumaSize = static_cast<uint64_t>(stride) * height;
            const uint64_t chromaSize = GetChromaStride() * GetChromaHeight();
            return (lumaSize + chromaSize * 2);
        }
        case PixelFormat::Interleaved10BitUYVY:
        case PixelFormat::Interleaved10BitRGB:
        case PixelFormat::Interleaved12BitRGB:
        case PixelFormat::Interleaved12BitRGBLE:
        case PixelFormat::Interleaved10BitRGBX:
        case PixelFormat::Interleaved10BitRGBXLE: {
            return static_cast<uint64_t>(stride) * height;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaWidth() const
{
    static_assert(AllPixelFormats.size() == 19);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar16BitP216:
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
    static_assert(AllPixelFormats.size() == 19);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar16BitP216:
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
    return chromaStride;
}

SubsampleType VideoFrameWrapper::GetSubsampleType() const
{
    static_assert(AllPixelFormats.size() == 19);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitBGRA:
        case PixelFormat::Interleaved8BitRGBA:
        case PixelFormat::Interleaved8BitARGB:
            return SubsampleType::RGB;
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
            return SubsampleType::YUV420;
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar16BitP216:
            return SubsampleType::YUV422;
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar10Bit444:
            return SubsampleType::YUV444;
        case PixelFormat::Interleaved10BitUYVY:
            return SubsampleType::YUV422;
        case PixelFormat::Interleaved10BitRGB:
        case PixelFormat::Interleaved12BitRGB:
        case PixelFormat::Interleaved12BitRGBLE:
        case PixelFormat::Interleaved10BitRGBX:
        case PixelFormat::Interleaved10BitRGBXLE:
            return SubsampleType::RGB;
    }
    return SubsampleType::RGB;
}

uint32_t VideoFrameWrapper::GetUOffset() const
{
    static_assert(AllPixelFormats.size() == 19);
    switch (pixelFormat) {
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420NV12:
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444:
        case PixelFormat::Planar16BitP216:
            return height * stride;
        case PixelFormat::Planar8Bit420YV12:
            return GetVOffset() + GetChromaHeight() * GetChromaStride();
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetVOffset() const
{
    static_assert(AllPixelFormats.size() == 19);
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

uint32_t VideoFrameWrapper::GetBitDepth() const
{
    static_assert(AllPixelFormats.size() == 19);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Interleaved8BitBGRA:
        case PixelFormat::Interleaved8BitRGBA:
        case PixelFormat::Interleaved8BitARGB:
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
        case PixelFormat::Interleaved12BitRGBLE:
            return 12;
        case PixelFormat::Planar10Bit420:
        case PixelFormat::Planar10Bit422:
        case PixelFormat::Planar10Bit444:
            return 10;
        case PixelFormat::Interleaved10BitRGBX:
        case PixelFormat::Interleaved10BitRGBXLE:
            return 10;
        case PixelFormat::Planar16BitP216:
            return 16;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetByteDepth() const
{
    const uint32_t bitDepth = GetBitDepth();
    const uint32_t fullBytes = bitDepth / 8;
    const uint32_t remainingBits = bitDepth % 8;
    return fullBytes + (remainingBits > 0 ? 1 : 0);
}

bool VideoFrameWrapper::AreFramePropertiesEqual(const VideoFrameWrapper& other) const
{
    return stride == other.stride && width == other.width && height == other.height && pixelFormat == other.pixelFormat &&
           range == other.range && yuvMatrix == other.yuvMatrix;
}

}  // namespace PixelWeave
