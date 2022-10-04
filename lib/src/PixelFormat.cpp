#include "PixelFormat.h"

#include "DebugUtils.h"

namespace PixelWeave
{
uint32_t PixelFormatHelpers::BytesPerSample(const PixelFormat& pixelFormat)
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY:
            return 4;
        case PixelFormat::Planar8Bit422:
            return 4;
    }
    PW_ASSERT_MSG(false, "Unsupported pixel format");
    return 0;
}

}  // namespace PixelWeave
