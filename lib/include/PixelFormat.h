#pragma once

#include <cstdint>

#include "Macros.h"

namespace PixelWeave
{

enum class PixelFormat {
    Interleaved8BitUYVY = 0,
    Interleved8BitBGRA = 1,
    Interleaved8BitRGBA = 2,
    Planar8Bit420 = 3,
    Planar8Bit422 = 4,
    Planar8Bit444 = 5,
    Planar8Bit420YV12 = 6,  // YVU, 3 planes
    Planar8Bit422NV12 = 7,  // A Y plane followed by a UV plane
};

}  // namespace PixelWeave