#pragma once

#include <cstdint>

#include "Macros.h"

namespace PixelWeave
{

enum class PixelFormat { Interleaved8BitUYVY, Planar8Bit422 };

class PIXEL_WEAVE_LIB_CLASS PixelFormatHelpers
{
public:
    static uint32_t BytesPerSample(const PixelFormat& pixelFormat);
};

}  // namespace PixelWeave