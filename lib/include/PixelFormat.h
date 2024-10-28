#pragma once

#include <array>
#include <cstdint>

namespace Pixelweave
{

enum class PixelFormat : uint32_t {
    Interleaved8BitUYVY = 0,
    Interleaved8BitBGRA = 1,
    Interleaved8BitRGBA = 2,
    Planar8Bit420 = 3,
    Planar8Bit422 = 4,
    Planar8Bit444 = 5,
    Planar8Bit420YV12 = 6,     // YVU, 3 planes
    Planar8Bit420NV12 = 7,     // A Y plane followed by a UV plane
    Interleaved10BitUYVY = 8,  // UYVY in 32 bits (2 bits unused) - Blackmagic
    Interleaved10BitRGB = 9,   // RGB in 32 bits (2 bits unused) - Blackmagic
    Interleaved12BitRGB = 10,  // Blocks of 8 pixels (see bmdFormat12BitRGB)
    Planar10Bit420 = 11,
    Planar10Bit422 = 12,
    Planar10Bit444 = 13,
    Interleaved8BitARGB = 14,
    Interleaved12BitRGBLE = 15,
    Interleaved10BitRGBX = 16,
    Interleaved10BitRGBXLE = 17,
    Planar16BitP216 = 18
};

constexpr std::array<PixelFormat, 19> AllPixelFormats{
    PixelFormat::Interleaved8BitUYVY,
    PixelFormat::Interleaved8BitBGRA,
    PixelFormat::Interleaved8BitRGBA,
    PixelFormat::Planar8Bit420,
    PixelFormat::Planar8Bit422,
    PixelFormat::Planar8Bit444,
    PixelFormat::Planar8Bit420YV12,
    PixelFormat::Planar8Bit420NV12,
    PixelFormat::Interleaved10BitUYVY,
    PixelFormat::Interleaved10BitRGB,
    PixelFormat::Interleaved12BitRGB,
    PixelFormat::Planar10Bit420,
    PixelFormat::Planar10Bit422,
    PixelFormat::Planar10Bit444,
    PixelFormat::Interleaved8BitARGB,
    PixelFormat::Interleaved12BitRGBLE,
    PixelFormat::Interleaved10BitRGBX,
    PixelFormat::Interleaved10BitRGBXLE,
    PixelFormat::Planar16BitP216
};

enum class SubsampleType : uint32_t { RGB = 0, YUV420 = 1, YUV422 = 2, YUV444 = 3 };

}  // namespace Pixelweave
