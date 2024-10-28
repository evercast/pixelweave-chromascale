# Pixelweave Chromascale

Pixelweave is a multi-platform library that leverages GPU computing to perform video frame conversions. Pixelweave is based on the graphics library Vulkan, allowing for a single implementation of GPU kernels that is compatible with most modern graphics hardware. It supports multiple pixel formats such as RGBA and chroma-subsampled YUV, using up to 16 bits per channel. The implemented compute shaders use a novel generic approach that samples 2x2 pixel windows, making the rest of the code easily extensible and independent of the source and destination formats. Initial tests show that Pixelweave provides significant speedups when compared to the state-of-the-art CPU-based libraries, allowing real-time streaming applications to offload resource-demanding tasks from the CPU and therefore improve overall performance.

## How to cite

> Aguerre, J., Sena, B., & Stolarz, D. (2024). Using GPU-Accelerated Pixel Format Conversions for Efficient Real-Time Video Streaming. SMPTE Motion Imaging Journal, 133. https://doi.org/10.5594/JMI.2024/YPJG9446

## Overview

<p align="center">
    <img src="doc/Architecture.png" data-canonical-src="doc/Architecture.png" height="450" />
</p>

### Architecture (subject to change)

- `Device` will be responsible for handling all global resources (video memory, command pools, video device picking, command queue management, etc).

- `VideoConverter` will manage a single video conversion stream (for example, converting all frames coming from an NDI stream, file stream, etc.). Ideally isn’t shared, because it will cache some resources so it runs faster when used with the same parameters.

- `Task` a wrapper around internal sync primitives. Allows applications to wait for results and do things while it’s not done (e.g. process audio). **Currently, the converter works synchronously, which means the GPU processes a single frame at a time**

- `VideoFrameWrapper` is based on our WebRTC needs and is inspired by its internal structure.

### Frame lifecycle

<p align="center">
    <img src="doc/FrameLifeCycle.png" data-canonical-src="doc/Architecture.png" height="450" />
</p>

## Setup

1. Install:
    1. [CMake  3.19.2](https://cmake.org/download/) or later.
    2. [Vulkan SDK 1.3.224.1](https://vulkan.lunarg.com/sdk/home) or later.
        - On Windows, be sure to set up the `VULKAN_SDK` environment variable.
        - On macOS, run  `sudo ./install_vulkan.py` to set up your environment.
    3. `Visual Studio 2022` and `C++ tools` or `Xcode` depending on your platform.
2. Create a `build` folder and run:
    - Windows: `cmake -G "Visual Studio 17 2022" -S . -B build`
    - macOS: `cmake -G Xcode -S . -B build`
    - Linux: `cmake -G "Unix Makefiles" -S . -B build`
3. Open the project. You'll see two projects:
    - `Pixelweave`: The main library project. Compiling it generates the distributable `.dll` or `.framework`.
    - `Tests`: A set of helper functions that can aide in quick testing while developing (don't rely on them, though).
4. Run `Tests` to check your environment was correctly set up.

## Usage

After linking the binaries for your platform, use Pixelweave as shown below:

```cpp
auto [result, device] = Pixelweave::Device::Create();
if (result == Pixelweave::Result::Success) {
    // Generate wrapper around source frame buffer
    auto srcBuffer = Pixelweave::VideoFrameWrapper{
        srcBuffer,
        bytesPerLine,
        srcWidth,
        srcHeight,
        Pixelweave::PixelFormat::Interleaved8BitUYVY};

    // Generate wrapper around dst frame buffer (application controls allocation)
    auto dstBuffer = Pixelweave::VideoFrameWrapper{
        dstBuffer,
        dstStride,
        dstWidth,
        dstHeight,
        Pixelweave::PixelFormat::Planar8Bit444};

    // Create converter and call
    Pixelweave::VideoConverter* videoConverter = videoConversionDevice->CreateVideoConverter();
    videoConverter->Convert(srcBuffer, dstBuffer);

    // Release both converter and device
    videoConverter->Release();
    device->Release();
}
```
