#pragma once

#include <cstdint>

#include "PixelFormat.h"

namespace PixelWeave
{
struct ProtoVideoFrame {
    uint8_t* buffer;
    uint32_t stride;
    uint32_t width;
    uint32_t height;
    PixelFormat pixelFormat;

    uint64_t GetBufferSize() const
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

    uint32_t GetChromaWidth() const
    {
        switch (pixelFormat) {
            case PixelFormat::Interleaved8BitUYVY:
            case PixelFormat::Planar8Bit422: {
                return (width + 1) / 2;
            }
        }
        return 0;
    }

    uint32_t GetChromaHeight() const
    {
        switch (pixelFormat) {
            case PixelFormat::Interleaved8BitUYVY:
            case PixelFormat::Planar8Bit422: {
                return height;
            }
        }
        return 0;
    }

    uint32_t GetChromaStride() const
    {
        switch (pixelFormat) {
            case PixelFormat::Interleaved8BitUYVY:
            case PixelFormat::Planar8Bit422: {
                return (width + 1) / 2;
            }
        }
        return 0;
    }
};

struct PictureInfo {
    uint32_t width;
    uint32_t height;
    uint32_t stride;  // Luma stride in planar formats
    uint32_t chromaWidth;
    uint32_t chromaHeight;
    uint32_t chromaStride;
};

struct InOutPictureInfo {
    PictureInfo srcPicture;
    PictureInfo dstPicture;
};

}  // namespace PixelWeave
