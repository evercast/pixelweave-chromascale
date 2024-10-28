#pragma once

#include <cstddef>
#include <cstdint>

#include "Macros.h"

namespace Pixelweave
{

struct Resource {
    const uint8_t* buffer;
    size_t size;

    enum class Id { ComputeShader };
};

class ResourceLoader
{
public:
    static Resource Load(const Resource::Id& resourceId);
    static void CleanUp(Resource& resource);
};

}  // namespace Pixelweave
