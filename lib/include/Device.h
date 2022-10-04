#pragma once

#include "Macros.h"
#include "RefCountPtr.h"
#include "Result.h"
#include "VideoConverter.h"

namespace PixelWeave
{
class PIXEL_WEAVE_LIB_CLASS Device : public RefCountPtr
{
public:
    static ResultValue<Device*> Create();

    virtual VideoConverter* CreateVideoConverter() = 0;

    virtual ~Device() = default;
};
}  // namespace PixelWeave