#include "RefCountPtr.h"

namespace PixelWeave
{
RefCountPtr::RefCountPtr() : refCount(1) {}

void RefCountPtr::AddRef()
{
    ++refCount;
}

uint32_t RefCountPtr::Release()
{
    refCount--;
    if (refCount == 0) {
        delete this;
    }
    return refCount;
}

}  // namespace PixelWeave
