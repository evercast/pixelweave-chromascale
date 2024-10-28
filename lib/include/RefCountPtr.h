#pragma once

#include <cstdint>

#include "Macros.h"

namespace Pixelweave
{
class PIXELWEAVE_LIB_CLASS RefCountPtr
{
public:
    RefCountPtr();

    void AddRef();
    uint32_t Release();

protected:
    virtual ~RefCountPtr() = default;

private:
    uint32_t refCount;
};
}  // namespace Pixelweave