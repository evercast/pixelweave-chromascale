#pragma once

#include "glm/glm.hpp"

#include "VideoFrameWrapper.h"

namespace Pixelweave
{

glm::mat3 GetMatrix(YUVMatrix matrix);

glm::vec3 GetYUVScale(Range range, [[maybe_unused]] uint32_t bitDepth);

glm::vec3 GetYUVOffset(Range range, uint32_t bitDepth);

}  // namespace Pixelweave
