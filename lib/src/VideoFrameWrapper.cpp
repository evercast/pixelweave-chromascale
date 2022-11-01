#include "VideoFrameWrapper.h"

namespace PixelWeave
{

uint64_t VideoFrameWrapper::GetBufferSize() const
{
    static_assert(AllPixelFormats.size() == 8);
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
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaWidth() const
{
    static_assert(AllPixelFormats.size() == 8);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12: {
            return (width + 1) / 2;
        }
        case PixelFormat::Planar8Bit444: {
            return width;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaHeight() const
{
    static_assert(AllPixelFormats.size() == 8);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit422: {
            return height;
        }
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12: {
            return (height + 1) / 2;
        }
        case PixelFormat::Planar8Bit444: {
            return height;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaStride() const
{
    return GetChromaWidth();
}

SubsampleType VideoFrameWrapper::GetSubsampleType() const
{
    static_assert(AllPixelFormats.size() == 8);
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitBGRA:
        case PixelFormat::Interleaved8BitRGBA:
            return SubsampleType::RGB;
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit420NV12:
            return SubsampleType::YUV420;
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Interleaved8BitUYVY:
            return SubsampleType::YUV422;
        case PixelFormat::Planar8Bit444:
            return SubsampleType::YUV444;
    }
    return SubsampleType::RGB;
}

uint32_t VideoFrameWrapper::GetUOffset() const
{
    static_assert(AllPixelFormats.size() == 8);
    switch (pixelFormat) {
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420NV12:
            return height * stride;
        case PixelFormat::Planar8Bit420YV12:
            return GetVOffset() + GetChromaHeight() * GetChromaStride();
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetVOffset() const
{
    static_assert(AllPixelFormats.size() == 8);
    switch (pixelFormat) {
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Planar8Bit444:
        case PixelFormat::Planar8Bit420NV12:
            return GetUOffset() + GetChromaHeight() * GetChromaStride();
        case PixelFormat::Planar8Bit420YV12:
            return height * stride;
    }
    return 0;
}

}  // namespace PixelWeave
