#include "VideoFrameWrapper.h"

namespace PixelWeave
{
uint64_t VideoFrameWrapper::GetBufferSize() const
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
            return stride * height;
        case PixelFormat::Planar8Bit422: {
            const uint64_t lumaSize = stride * height;
            const uint64_t chromaSize = ((width + 1) / 2) * height;
            return lumaSize + chromaSize * 2;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaWidth() const
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit422: {
            return (width + 1) / 2;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaHeight() const
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit422: {
            return height;
        }
    }
    return 0;
}

uint32_t VideoFrameWrapper::GetChromaStride() const
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
        case PixelFormat::Planar8Bit422: {
            return (width + 1) / 2;
        }
    }
    return 0;
}

SubsampleType VideoFrameWrapper::GetSubsampleType() const {
    static_assert(AllPixelFormats.size() == 8);
    switch (pixelFormat) {
        case PixelFormat::Interleved8BitBGRA:
        case PixelFormat::Interleaved8BitRGBA:
            return SubsampleType::RGB;
        case PixelFormat::Planar8Bit420:
        case PixelFormat::Planar8Bit420YV12:
        case PixelFormat::Planar8Bit422NV12:
            return SubsampleType::YUV420;
        case PixelFormat::Planar8Bit422:
        case PixelFormat::Interleaved8BitUYVY:
            return SubsampleType::YUV422;
        case PixelFormat::Planar8Bit444:
            return SubsampleType::YUV444;
    }
    return SubsampleType::RGB;
}

}  // namespace PixelWeave
