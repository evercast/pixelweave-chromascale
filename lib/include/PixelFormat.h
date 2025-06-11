#pragma once

#include <array>
#include <cstdint>

namespace Pixelweave
{

enum class PixelFormat : uint32_t {
    RGB8BitInterleavedBGRA = 0,
    RGB8BitInterleavedRGBA,
    RGB8BitInterleavedARGB,
    YCC8Bit420Planar,      // Plane order YCbCr
    YCC8Bit420PlanarYV12,  // Plane order YCrCb
    YCC8Bit422Planar,
    YCC8Bit444Planar,
    YCC8Bit420BiplanarNV12,  // One Y plane, one interleaved CbCr plane
    YCC8Bit422InterleavedUYVY,
    RGB10BitInterleavedRGBXBE,
    RGB10BitInterleavedRGBXLE,
    RGB10BitInterleavedXRGBBE,
    RGB10BitInterleavedXRGBLE,
    RGB10BitInterleavedXBGRBE,
    RGB10BitInterleavedXBGRLE,
    YCC10Bit420Planar,
    YCC10Bit422Planar,
    YCC10Bit444Planar,
    YCC10Bit420BiplanarP010,  // One Y plane, one interleaved CbCr plane, 10 bpc in upper bits of 16-bit segments
    YCC10Bit422BiplanarP210,
    YCC10Bit444BiplanarP410,
    YCC10Bit422InterleavedV210,  // Six pixels in 16 bytes, upper two bits of each 32-bit block unused
    RGB12BitInterleavedBGRBE,    // Eight pixels in 36 bytes
    RGB12BitInterleavedBGRLE,
    YCC16Bit422BiplanarP216,
    ValueCount,  // Convenience value
};

constexpr size_t PixelFormatCount = static_cast<size_t>(PixelFormat::ValueCount);

constexpr std::array<PixelFormat, PixelFormatCount> AllPixelFormats{
    PixelFormat::RGB8BitInterleavedBGRA,
    PixelFormat::RGB8BitInterleavedRGBA,
    PixelFormat::RGB8BitInterleavedARGB,
    PixelFormat::YCC8Bit420Planar,
    PixelFormat::YCC8Bit420PlanarYV12,
    PixelFormat::YCC8Bit422Planar,
    PixelFormat::YCC8Bit444Planar,
    PixelFormat::YCC8Bit420BiplanarNV12,
    PixelFormat::YCC8Bit422InterleavedUYVY,
    PixelFormat::RGB10BitInterleavedRGBXBE,
    PixelFormat::RGB10BitInterleavedRGBXLE,
    PixelFormat::RGB10BitInterleavedXRGBBE,
    PixelFormat::RGB10BitInterleavedXRGBLE,
    PixelFormat::RGB10BitInterleavedXBGRBE,
    PixelFormat::RGB10BitInterleavedXBGRLE,
    PixelFormat::YCC10Bit420Planar,
    PixelFormat::YCC10Bit422Planar,
    PixelFormat::YCC10Bit444Planar,
    PixelFormat::YCC10Bit420BiplanarP010,
    PixelFormat::YCC10Bit422BiplanarP210,
    PixelFormat::YCC10Bit444BiplanarP410,
    PixelFormat::YCC10Bit422InterleavedV210,
    PixelFormat::RGB12BitInterleavedBGRBE,
    PixelFormat::RGB12BitInterleavedBGRLE,
    PixelFormat::YCC16Bit422BiplanarP216,
};

enum class ColorFormat : uint32_t {
    RGB = 0,
    YUV444,
    YUV422,
    YUV420,
};

}  // namespace Pixelweave
