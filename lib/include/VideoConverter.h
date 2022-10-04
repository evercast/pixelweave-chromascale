#pragma once

#include <cstdint>

#include "Macros.h"
#include "RefCountPtr.h"

namespace PixelWeave
{

enum class PixelFormat { Interleaved8BitUYVY, Planar8Bit422 };

struct ProtoVideoFrame {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
    PixelFormat pixelFormat;
};

class PIXEL_WEAVE_LIB_CLASS VideoConverter : public RefCountPtr
{
public:
    VideoConverter() = default;
    virtual void Convert(const ProtoVideoFrame& source, ProtoVideoFrame& dst) = 0;
    virtual ~VideoConverter() override = default;
};
}  // namespace PixelWeave