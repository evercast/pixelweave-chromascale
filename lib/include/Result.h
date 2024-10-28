#pragma once

#include <tuple>

namespace Pixelweave
{

enum class Result {
    Success,
    InvalidInputFormatError,
    InvalidOutputFormatError,
    InvalidInputResolutionError,
    InvalidOutputResolutionError,
    DriverNotFoundError,
    InvalidDeviceError,
    NoSuitableDeviceError,
    AllocationFailed,
    ShaderCompilationFailed,
    UnknownError
};

template <typename T>
struct ResultValue {
    Result result;
    T value;

    operator std::tuple<Result&, T&>() { return std::tuple<Result&, T&>(result, value); }
};
}  // namespace Pixelweave