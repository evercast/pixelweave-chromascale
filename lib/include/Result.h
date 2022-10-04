#pragma once

#include <tuple>

namespace PixelWeave
{

enum class Result { Success, Error };

template <typename T>
struct ResultValue {
    Result result;
    T value;

    operator std::tuple<Result&, T&>() { return std::tuple<Result&, T&>(result, value); }
};
}  // namespace PixelWeave