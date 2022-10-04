#pragma once

#include <cstdint>

#include "PixelFormat.h"

namespace PixelWeave
{
struct ProtoVideoFrame {
    uint8_t* buffer;
    uint64_t stride;
    uint64_t width;
    uint64_t height;
    PixelFormat pixelFormat;
};
}  // namespace PixelWeave
