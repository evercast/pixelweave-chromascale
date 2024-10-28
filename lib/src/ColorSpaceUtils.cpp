#include "ColorSpaceUtils.h"

namespace Pixelweave
{
glm::mat3 GetMatrix(YUVMatrix matrix)
{
    constexpr float KR709 = 0.2126f;
    constexpr float KB709 = 0.0722f;

    constexpr float KR2020 = 0.2627f;
    constexpr float KB2020 = 0.0593f;
    const auto getYUVMatrix = [](float kr, float kb) -> glm::mat3 {
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
        case YUVMatrix::BT709:
            return getYUVMatrix(KR709, KB709);
        case YUVMatrix::BT2020:
            return getYUVMatrix(KR2020, KB2020);
    }
    return glm::mat3();
}

glm::vec3 GetYUVScale(Range range, [[maybe_unused]] uint32_t bitDepth)
{
    return range == Range::Limited ? glm::vec3((235.0f - 16.0f) / 255.0f, (240.0f - 16.0f) / 255.0f, (240.0f - 16.0f) / 255.0f)
                                   : glm::vec3(1.0f);
}

glm::vec3 GetYUVOffset(Range range, uint32_t bitDepth)
{
    return glm::vec3(
               (range == Range::Limited) ? static_cast<float>(1 << (bitDepth - 4)) : 0.0f,
               static_cast<float>(1 << (bitDepth - 1)),
               static_cast<float>(1 << (bitDepth - 1))) /
           glm::vec3(static_cast<float>((1 << bitDepth) - 1));
}
}  // namespace Pixelweave