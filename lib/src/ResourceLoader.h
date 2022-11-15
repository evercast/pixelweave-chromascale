#pragma once

#include <cstddef>
#include <cstdint>

#include "Macros.h"

namespace PixelWeave
{

struct Resource {
    size_t size;
    uint8_t* buffer;

    enum class Id { ComputeShader };
};

class ResourceLoader
{
public:
    static Resource Load(const Resource::Id& resourceId);
    static void Cleanup(Resource& resource);
};

}  // namespace PixelWeave
