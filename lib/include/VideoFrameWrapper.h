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
