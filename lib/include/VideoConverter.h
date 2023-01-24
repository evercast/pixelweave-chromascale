#pragma once

#include "Macros.h"
#include "RefCountPtr.h"
#include "VideoFrameWrapper.h"
#include "Result.h"

namespace PixelWeave
{
class PIXEL_WEAVE_LIB_CLASS VideoConverter : public RefCountPtr
{
public:
    VideoConverter() = default;

    virtual Result Convert(const VideoFrameWrapper& source, VideoFrameWrapper& dst) = 0;

    virtual ~VideoConverter() override = default;
};
}  // namespace PixelWeave