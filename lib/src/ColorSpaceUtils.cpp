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
        default:
        case LumaChromaMatrix::Identity:
            return glm::mat3{1.0f};
        case LumaChromaMatrix::BT709:
            return nclMatrix(0.2126f, 0.0722f);
        case LumaChromaMatrix::BT2020NCL:
            return nclMatrix(0.2627f, 0.0593f);
    }
}

glm::vec3 GetLumaChromaScale(bool fullRange, uint32_t bitDepth)
{
    if (fullRange) {
        return glm::vec3(1.0f);
    }
    float scale = static_cast<float>(1 << (bitDepth - 8));
    float legalMin = 16.0f * scale;
    float lumaLegalMax = 235.0f * scale;
    float chromaLegalMax = 240.0f * scale;
    float maxValue = static_cast<float>((1 << bitDepth) - 1);
    return glm::vec3(lumaLegalMax - legalMin, chromaLegalMax - legalMin, chromaLegalMax - legalMin) /
           glm::vec3(maxValue);
}

glm::vec3 GetLumaChromaOffset(bool fullRange, uint32_t bitDepth)
{
    float blackLevel = !fullRange ? static_cast<float>(1 << (bitDepth - 4)) : 0.0f;
    float achromaticLevel = static_cast<float>(1 << (bitDepth - 1));
    float maxValue = static_cast<float>((1 << bitDepth) - 1);
    return glm::vec3(blackLevel, achromaticLevel, achromaticLevel) / glm::vec3(maxValue);
}

}  // namespace Pixelweave
