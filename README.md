Pixelweave Chromascale
======================

Pixelweave is a multi-platform API and library that leverages GPU computing to perform video frame conversions. Pixelweave offers an implementation based on the Vulkan graphics API, allowing for a single implementation of GPU kernels that is compatible with most modern graphics hardware. It supports multiple pixel formats, such as RGBA and chroma-subsampled YUV, using up to 16 bits per channel. The implemented compute shaders use a novel generic approach that samples 2x2 pixel windows, making the rest of the code easily extensible and independent of the source and destination formats. Initial tests show that Pixelweave provides significant speedups when compared to state-of-the-art CPU-based libraries, allowing real-time streaming applications to offload resource-demanding tasks from the CPU and therefore improve overall performance.

## How to cite

> Aguerre, J., Sena, B., & Stolarz, D. (2024). Using GPU-Accelerated Pixel Format Conversions for Efficient Real-Time Video Streaming. SMPTE Motion Imaging Journal, 133. https://doi.org/10.5594/JMI.2024/YPJG9446

## Architecture

<p align="center">
    <img src="doc/Architecture.png" data-canonical-src="doc/Architecture.png" height="450" />
</p>

- `Device` is responsible for handling all global resources (video memory, command pools, video device picking, command queue management, etc.).

- `Task` is a wrapper around internal sync primitives. It allows applications to wait for results and do things while it’s not done (e.g. process audio). **Currently, the converter works synchronously, which means the GPU processes a single frame at a time.**

- `VideoConverter` manages a single video conversion stream (for example, converting all frames coming from an NDI stream, file stream, etc.). Ideally, it shouldn't be shared because it will cache some resources so it runs faster when used with the same parameters.

- `VideoFrameWrapper` wraps a single video frame in memory for conversion.

## Frame lifecycle

<p align="center">
    <img src="doc/FrameLifeCycle.png" data-canonical-src="doc/Architecture.png" height="450" />
</p>

## Dependencies

- [CMake 3.24](https://cmake.org/download/) or later. CMake 4 is not currently supported.
- A recent [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (tested with 1.4.328.1). Include the VMA and GLM headers. Be sure to set the `VULKAN_SDK` environment variable to point to the SDK path.
- Windows: Visual Studio 2022 and C++ tools
- macOS: Xcode
- Linux: GCC (Clang may also work) and Ninja or Make

## Building the project

Generate the project:

```sh
cmake -S . -B build -G "<PROJECT GENERATOR>"
```

Project generator:

- Windows: `Visual Studio 17 2022`
- macOS: `Xcode`
- Linux: `Ninja` or `Unix Makefiles`

It is possible to use vcpkg for dependencies. By default, the project will not use it. Set the CMake variable `PIXELWEAVE_USE_VCPKG` to `ON` to use it (`-D PIXELWEAVE_USE_VCPKG=ON`). Note that it is not possible to build a universal library on macOS when using vcpkg for dependencies.

The build architecture may be selected by setting the CMake variable `PIXELWEAVE_ARCH` to either `x86_64` or `arm64` (`-D PIXELWEAVE_ARCH=arm64`). On macOS, use `arm64;x86_64` to build a universal library (note that this is not possible when using vcpkg for dependencies).

Build the project:

```sh
cmake --build build --parallel
```

Install the library:

```sh
cmake --install build --prefix "<INSTALL PATH>"
```

The build and install steps can also be performed using an IDE (e.g., Visual Studio on Windows or Xcode on macOS).

There are two targets:

- `Pixelweave`: The main library target. Compiling it generates the distributable shared library.
- `Tests`: A set of helper functions that can aid in quick testing while developing (don't rely on them, though).

Run `Tests` to check that your environment is correctly set up.

## Usage

After linking the binaries for your platform, use Pixelweave as follows:

```cpp
auto [result, device] = Pixelweave::Device::Create();
if (result == Pixelweave::Result::Success) {
    // Generate wrapper around source frame buffer
    auto srcBuffer = Pixelweave::VideoFrameWrapper{
        .buffer = srcBuffer,
        .stride = bytesPerLine,
        .chromaStride = bytesPerLine / 2,
        .width = srcWidth,
        .height = srcHeight,
        .pixelFormat = Pixelweave::PixelFormat::YCC8Bit422InterleavedUYVY,
    };

    // Generate wrapper around dst frame buffer (application controls allocation)
    auto dstBuffer = Pixelweave::VideoFrameWrapper{
        .buffer = dstBuffer,
        .stride = dstStride,
        .chromaStride = dstStride,
        .width = dstWidth,
        .height = dstHeight,
        .pixelFormat = Pixelweave::PixelFormat::YCC8Bit444Planar,
    };

    // Create converter and call
    Pixelweave::VideoConverter* videoConverter = videoConversionDevice->CreateVideoConverter();
    videoConverter->Convert(srcBuffer, dstBuffer);

    // Release both converter and device
    videoConverter->Release();
    device->Release();
}
```
