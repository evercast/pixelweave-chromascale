#include "ColorSpaceUtils.h"

namespace Pixelweave
{

glm::mat3 GetLumaChromaMatrix(LumaChromaMatrix matrix)
{
    const auto nclMatrix = [](float kr, float kb) {
        float kg = 1.0f - kr - kb;
        return glm::mat3(
            kr,
            -0.5f * (kr / (1.0f - kb)),
            0.5f,
            kg,
            -0.5f * (kg / (1.0f - kb)),
            -0.5f * (kg / (1.0f - kr)),
            kb,
            0.5f,
            -0.5f * (kb / (1.0f - kr)));
    };

    switch (matrix) {
        case LumaChromaMatrix::BT709: {
            constexpr float KR = 0.2126f;
            constexpr float KB = 0.0722f;
            return nclMatrix(KR, KB);
        }
        case LumaChromaMatrix::BT2020NCL: {
            constexpr float KR = 0.2627f;
            constexpr float KB = 0.0593f;
            return nclMatrix(KR, KB);
        }
        default:
            return glm::mat3();
    }
}

glm::vec3 GetLumaChromaScale(bool fullRange, uint32_t bitDepth)
{
    return fullRange ? glm::vec3(1.0f)
                     : glm::vec3((235.0f - 16.0f) / 255.0f, (240.0f - 16.0f) / 255.0f, (240.0f - 16.0f) / 255.0f);
}

glm::vec3 GetLumaChromaOffset(bool fullRange, uint32_t bitDepth)
{
    return glm::vec3(
               fullRange ? 0.0f : static_cast<float>(1 << (bitDepth - 4)),
               static_cast<float>(1 << (bitDepth - 1)),
               static_cast<float>(1 << (bitDepth - 1))) /
           glm::vec3(static_cast<float>((1 << bitDepth) - 1));
}

}  // namespace Pixelweave
