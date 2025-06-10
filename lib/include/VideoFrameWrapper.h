#pragma once

#include <cstdint>
#include <vector>

#include "Macros.h"
#include "PixelFormat.h"

namespace Pixelweave
{

// Values match Rec. ITU-T H.273, Coding-Independent Code Points for Video Signal Type Identification
enum class LumaChromaMatrix {
    BT709 = 1,
    BT2020NCL = 9,
};

// Video range is actually a simple flag (see Rec. H.273), but represented as an enum here
enum class VideoRange {
    Legal = 0,
    Full,
};

enum class VideoFrameLayout {
    Planar,
    Biplanar,
    Interleaved,
};

struct PIXELWEAVE_LIB_CLASS VideoFrameWrapper {
    uint8_t* buffer = nullptr;
    size_t bufferSize = 0;             // Buffer size in bytes: set to zero to calculate based on stride and height
#pragma warning(suppress : 4251)       // MSVC: suppress spurious MSVC warning caused by exporting `std::vector`
    std::vector<size_t> planeOffsets;  // Plane offsets in bytes: leave empty to calculate based on stride and height
    uint32_t stride = 0;               // Luma/interleaved stride in bytes
    uint32_t chromaStride = 0;         // Chroma stride in bytes
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat pixelFormat = PixelFormat::RGB8BitInterleavedRGBA;
    VideoRange range = VideoRange::Legal;
    LumaChromaMatrix lumaChromaMatrix = LumaChromaMatrix::BT709;

    VideoFrameLayout GetLayoutType() const;
    uint64_t GetBufferSize() const;
    size_t GetPlaneOffset(size_t index) const;

    uint32_t GetBitDepth() const;
    uint32_t GetByteDepth() const;

    uint32_t GetChromaOffset() const;
    uint32_t GetCbOffset() const;
    uint32_t GetCrOffset() const;
    uint32_t GetChromaStride() const;
    ChromaSubsampling GetChromaSubsampling() const;
    uint32_t GetChromaWidth() const;
    uint32_t GetChromaHeight() const;

    bool AreFramePropertiesEqual(const VideoFrameWrapper& other) const;
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

}  // namespace Pixelweave
