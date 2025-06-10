#include "VideoFrameWrapper.h"

#include <cmath>

namespace Pixelweave
{

VideoFrameLayout VideoFrameWrapper::GetLayoutType() const
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        default:
        case PixelFormat::RGB8BitInterleavedRGBA:
        case PixelFormat::RGB8BitInterleavedBGRA:
        case PixelFormat::RGB8BitInterleavedARGB:
        case PixelFormat::RGB10BitInterleavedRGBXBE:
        case PixelFormat::RGB10BitInterleavedRGBXLE:
        case PixelFormat::RGB10BitInterleavedXRGBBE:
        case PixelFormat::RGB10BitInterleavedXRGBLE:
        case PixelFormat::RGB10BitInterleavedXBGRBE:
        case PixelFormat::RGB10BitInterleavedXBGRLE:
        case PixelFormat::RGB12BitInterleavedBGRBE:
        case PixelFormat::RGB12BitInterleavedBGRLE:
        case PixelFormat::YCC8Bit422InterleavedUYVY:
        case PixelFormat::YCC10Bit422InterleavedV210:
            return VideoFrameLayout::Interleaved;
        case PixelFormat::YCC8Bit420Planar:
        case PixelFormat::YCC8Bit420PlanarYV12:
        case PixelFormat::YCC8Bit422Planar:
        case PixelFormat::YCC8Bit444Planar:
        case PixelFormat::YCC10Bit420Planar:
        case PixelFormat::YCC10Bit422Planar:
        case PixelFormat::YCC10Bit444Planar:
            return VideoFrameLayout::Planar;
        case PixelFormat::YCC8Bit420BiplanarNV12:
        case PixelFormat::YCC10Bit420BiplanarP010:
        case PixelFormat::YCC10Bit422BiplanarP210:
        case PixelFormat::YCC10Bit444BiplanarP410:
        case PixelFormat::YCC16Bit422BiplanarP216:
            return VideoFrameLayout::Biplanar;
    }
}

uint64_t VideoFrameWrapper::GetBufferSize() const
{
    if (bufferSize != 0) {
        return bufferSize;
    }

    switch (GetLayoutType()) {
        case VideoFrameLayout::Interleaved:
            return static_cast<uint64_t>(stride) * height;
        case VideoFrameLayout::Planar:
        case VideoFrameLayout::Biplanar: {
            const uint64_t lumaSize = static_cast<uint64_t>(stride) * height;
            const uint64_t chromaSize = GetChromaStride() * GetChromaHeight();
            return lumaSize + chromaSize * 2;
        }
        default:
            return 0;
    }
}

size_t VideoFrameWrapper::GetPlaneOffset(size_t index) const
{
    if (planeOffsets.size() > index) {
        return planeOffsets[index];
    }

    switch (index) {
        case 0:
            return 0;
        case 1:
            return height * stride;
        case 2:
            return GetPlaneOffset(1) + GetChromaHeight() * GetChromaStride();
        default:
            return 0;
    }
}

ChromaSubsampling VideoFrameWrapper::GetChromaSubsampling() const
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        default:
        case PixelFormat::RGB8BitInterleavedBGRA:
        case PixelFormat::RGB8BitInterleavedRGBA:
        case PixelFormat::RGB8BitInterleavedARGB:
        case PixelFormat::RGB10BitInterleavedXRGBBE:
        case PixelFormat::RGB10BitInterleavedXRGBLE:
        case PixelFormat::RGB10BitInterleavedRGBXBE:
        case PixelFormat::RGB10BitInterleavedRGBXLE:
        case PixelFormat::RGB10BitInterleavedXBGRBE:
        case PixelFormat::RGB10BitInterleavedXBGRLE:
        case PixelFormat::RGB12BitInterleavedBGRBE:
        case PixelFormat::RGB12BitInterleavedBGRLE:
            return ChromaSubsampling::None;
        case PixelFormat::YCC8Bit420Planar:
        case PixelFormat::YCC8Bit420PlanarYV12:
        case PixelFormat::YCC8Bit420BiplanarNV12:
        case PixelFormat::YCC10Bit420Planar:
        case PixelFormat::YCC10Bit420BiplanarP010:
            return ChromaSubsampling::_420;
        case PixelFormat::YCC8Bit422Planar:
        case PixelFormat::YCC8Bit422InterleavedUYVY:
        case PixelFormat::YCC10Bit422Planar:
        case PixelFormat::YCC10Bit422InterleavedV210:
        case PixelFormat::YCC10Bit422BiplanarP210:
        case PixelFormat::YCC16Bit422BiplanarP216:
            return ChromaSubsampling::_422;
        case PixelFormat::YCC8Bit444Planar:
        case PixelFormat::YCC10Bit444Planar:
        case PixelFormat::YCC10Bit444BiplanarP410:
            return ChromaSubsampling::_444;
    }
}

uint32_t VideoFrameWrapper::GetChromaWidth() const
{
    switch (GetChromaSubsampling()) {
        case ChromaSubsampling::_444:
            return width;
        case ChromaSubsampling::_422:
        case ChromaSubsampling::_420:
            return (width + 1) / 2;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetChromaHeight() const
{
    switch (GetChromaSubsampling()) {
        case ChromaSubsampling::_444:
        case ChromaSubsampling::_422:
            return height;
        case ChromaSubsampling::_420:
            return (height + 1) / 2;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetChromaStride() const
{
    return chromaStride;
}

uint32_t VideoFrameWrapper::GetChromaOffset() const
{
    switch (GetLayoutType()) {
        case VideoFrameLayout::Planar:
        case VideoFrameLayout::Biplanar:
            return static_cast<uint32_t>(GetPlaneOffset(1));
        default:
        case VideoFrameLayout::Interleaved:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetCbOffset() const
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        case PixelFormat::YCC8Bit420Planar:
        case PixelFormat::YCC8Bit422Planar:
        case PixelFormat::YCC8Bit444Planar:
        case PixelFormat::YCC10Bit420Planar:
        case PixelFormat::YCC10Bit422Planar:
        case PixelFormat::YCC10Bit444Planar:
        case PixelFormat::YCC8Bit420BiplanarNV12:
        case PixelFormat::YCC10Bit422BiplanarP210:
        case PixelFormat::YCC10Bit420BiplanarP010:
        case PixelFormat::YCC10Bit444BiplanarP410:
        case PixelFormat::YCC16Bit422BiplanarP216:
            return static_cast<uint32_t>(GetPlaneOffset(1));
        case PixelFormat::YCC8Bit420PlanarYV12:
            return static_cast<uint32_t>(GetPlaneOffset(2));
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetCrOffset() const
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        case PixelFormat::YCC8Bit420Planar:
        case PixelFormat::YCC8Bit422Planar:
        case PixelFormat::YCC8Bit444Planar:
        case PixelFormat::YCC10Bit420Planar:
        case PixelFormat::YCC10Bit422Planar:
        case PixelFormat::YCC10Bit444Planar:
            return static_cast<uint32_t>(GetPlaneOffset(2));
        case PixelFormat::YCC8Bit420PlanarYV12:
        case PixelFormat::YCC8Bit420BiplanarNV12:
        case PixelFormat::YCC10Bit420BiplanarP010:
        case PixelFormat::YCC10Bit422BiplanarP210:
        case PixelFormat::YCC10Bit444BiplanarP410:
        case PixelFormat::YCC16Bit422BiplanarP216:
            return static_cast<uint32_t>(GetPlaneOffset(1));
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetBitDepth() const
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        case PixelFormat::RGB8BitInterleavedBGRA:
        case PixelFormat::RGB8BitInterleavedRGBA:
        case PixelFormat::RGB8BitInterleavedARGB:
        case PixelFormat::YCC8Bit420Planar:
        case PixelFormat::YCC8Bit420PlanarYV12:
        case PixelFormat::YCC8Bit422Planar:
        case PixelFormat::YCC8Bit444Planar:
        case PixelFormat::YCC8Bit420BiplanarNV12:
        case PixelFormat::YCC8Bit422InterleavedUYVY:
            return 8;
        case PixelFormat::RGB10BitInterleavedXRGBBE:
        case PixelFormat::RGB10BitInterleavedXRGBLE:
        case PixelFormat::RGB10BitInterleavedRGBXBE:
        case PixelFormat::RGB10BitInterleavedRGBXLE:
        case PixelFormat::RGB10BitInterleavedXBGRBE:
        case PixelFormat::RGB10BitInterleavedXBGRLE:
        case PixelFormat::YCC10Bit420Planar:
        case PixelFormat::YCC10Bit422Planar:
        case PixelFormat::YCC10Bit444Planar:
        case PixelFormat::YCC10Bit420BiplanarP010:
        case PixelFormat::YCC10Bit422BiplanarP210:
        case PixelFormat::YCC10Bit444BiplanarP410:
        case PixelFormat::YCC10Bit422InterleavedV210:
            return 10;
        case PixelFormat::RGB12BitInterleavedBGRBE:
        case PixelFormat::RGB12BitInterleavedBGRLE:
            return 12;
        case PixelFormat::YCC16Bit422BiplanarP216:
            return 16;
        default:
            return 0;
    }
}

uint32_t VideoFrameWrapper::GetByteDepth() const
{
    const uint32_t bitDepth = GetBitDepth();
    const uint32_t fullBytes = bitDepth / 8;
    const uint32_t remainingBits = bitDepth % 8;
    return fullBytes + (remainingBits > 0 ? 1 : 0);
}

bool VideoFrameWrapper::AreFramePropertiesEqual(const VideoFrameWrapper& other) const
{
    return stride == other.stride && width == other.width && height == other.height &&
           pixelFormat == other.pixelFormat && range == other.range && lumaChromaMatrix == other.lumaChromaMatrix;
}

}  // namespace Pixelweave
