#pragma once

#include <memory>

#include "Result.h"

#ifdef PIXEL_WEAVE_DEVICE_IMPL
#define PIXEL_WEAVE_LIB_CLASS __declspec(dllexport)
#else
#define PIXEL_WEAVE_LIB_CLASS __declspec(dllimport)
#endif

namespace PixelWeave
{
class PIXEL_WEAVE_LIB_CLASS Device
{
public:
    static ResultValue<std::shared_ptr<Device>> Create();
};
}  // namespace PixelWeave