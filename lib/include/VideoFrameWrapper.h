#pragma once

#include <cstdint>

#include "PixelFormat.h"

namespace PixelWeave
{

struct PIXEL_WEAVE_LIB_CLASS VideoFrameWrapper
{
    uint8_t* buffer;
    uint32_t stride;
    uint32_t width;
    uint32_t height;
    PixelFormat pixelFormat;

    uint64_t GetBufferSize() const;
    uint32_t GetChromaWidth() const;
    uint32_t GetChromaHeight() const;
    uint32_t GetChromaStride() const;
    SubsampleType GetSubsampleType() const;
    uint32_t GetUOffset() const;
    uint32_t GetVOffset() const;
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
