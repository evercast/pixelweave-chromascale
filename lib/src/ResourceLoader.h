#pragma once

#include <cstdint>

namespace PixelWeave
{
enum class ResourceId { ComputeShader };

struct Resource {
    size_t size;
    uint8_t* buffer;
};

class ResourceLoader
{
public:
    static Resource Load(const ResourceId& resourceId);
};

}  // namespace PixelWeave