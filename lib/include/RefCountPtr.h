#pragma once

#include <atomic>
#include <cstdint>

#include "Macros.h"

namespace PixelWeave
{
class PIXEL_WEAVE_LIB_CLASS RefCountPtr
{
public:
    RefCountPtr();

    void AddRef();
    uint32_t Release();

protected:
    virtual ~RefCountPtr();

private:
    std::atomic<uint32_t>* refCount;
};
}  // namespace PixelWeave