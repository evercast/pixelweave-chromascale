#pragma once

#include <array>
#include <cstdint>

#include "Macros.h"

namespace PixelWeave
{

enum class PixelFormat : uint32_t {
    Interleaved8BitUYVY = 0,
    Interleved8BitBGRA = 1,
    Interleaved8BitRGBA = 2,
    Planar8Bit420 = 3,
    Planar8Bit422 = 4,
    Planar8Bit444 = 5,
    Planar8Bit420YV12 = 6,  // YVU, 3 planes
    Planar8Bit422NV12 = 7,  // A Y plane followed by a UV plane
};

constexpr std::array<PixelFormat, 8> AllPixelFormats{
    PixelFormat::Interleaved8BitUYVY,
    PixelFormat::Interleved8BitBGRA,
    PixelFormat::Interleaved8BitRGBA,
    PixelFormat::Planar8Bit420,
    PixelFormat::Planar8Bit422,
    PixelFormat::Planar8Bit444,
    PixelFormat::Planar8Bit420YV12,
    PixelFormat::Planar8Bit422NV12,
};

enum class SubsampleType : uint32_t { RGB = 0, YUV420 = 1, YUV422 = 2, YUV444 = 3 };

}  // namespace PixelWeave