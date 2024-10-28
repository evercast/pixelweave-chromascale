#include "RefCountPtr.h"

namespace Pixelweave
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

}  // namespace Pixelweave
