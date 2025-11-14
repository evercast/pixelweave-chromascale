#pragma once

#include <glm/glm.hpp>

#include "VideoFrameWrapper.h"

namespace Pixelweave
{

glm::mat3 GetLumaChromaMatrix(LumaChromaMatrix matrix);
glm::vec3 GetLumaChromaScale(bool fullRange, uint32_t bitDepth);
glm::vec3 GetLumaChromaOffset(bool fullRange, uint32_t bitDepth);

}  // namespace Pixelweave
