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
}  // namespace PixelWeave
