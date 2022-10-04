#define PIXEL_WEAVE_REF_COUNT_PTR_IMPL

#include "RefCountPtr.h"

namespace PixelWeave
{
RefCountPtr::RefCountPtr()
{
    refCount = new std::atomic<uint32_t>(1);
}

void RefCountPtr::AddRef()
{
    ++(*refCount);
}

uint32_t RefCountPtr::Release()
{
    const uint32_t currentCount = (*refCount)--;
    if (currentCount == 0) {
        delete this;
    }
    return currentCount;
}

RefCountPtr::~RefCountPtr()
{
    delete refCount;
}

}  // namespace PixelWeave
