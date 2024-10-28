#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

#include "Device.h"

using namespace Pixelweave;

struct Timer {
    void Start() { beginTime = std::chrono::steady_clock::now(); }
    template <typename M>
    uint64_t Elapsed()
    {
        return std::chrono::duration_cast<M>(std::chrono::steady_clock::now() - beginTime).count();
    }
    uint64_t ElapsedMillis() { return Elapsed<std::chrono::milliseconds>(); }
    uint64_t ElapsedMicros() { return Elapsed<std::chrono::microseconds>(); }

    std::chrono::steady_clock::time_point beginTime;
};

Pixelweave::VideoFrameWrapper GetUYVYFrame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width * 2;
    const uint32_t bufferSize = height * stride;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t x = 0; x < ((width + 1) / 2); ++x) {
        for (uint32_t y = 0; y < height; ++y) {
            const uint32_t baseIndex = y * stride + x * 4;
            buffer[baseIndex] = 0xB0;      // U
            buffer[baseIndex + 1] = 0xFF;  // Y
            buffer[baseIndex + 2] = 0xC0;  // V
            buffer[baseIndex + 3] = 0xFF;  // Y
        }
    }
    return VideoFrameWrapper{buffer, stride, 0, width, height, Pixelweave::PixelFormat::Interleaved8BitUYVY};
}

Pixelweave::VideoFrameWrapper GetPlanar420Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t stride = width + 8;
    const uint32_t chromaStride = chromaWidth + 8;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * stride) + chromaStride * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    std::memset(buffer, 0, bufferSize);
    for (uint32_t indexY = 0; indexY < height; ++indexY) {
        for (uint32_t indexX = 0; indexX < width; ++indexX) {
            uint32_t sampleIndex = indexY * stride + indexX;
            buffer[sampleIndex] = 0xFF;
        }
    }

    const uint32_t uSampleOffset = stride * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaStride * chromaHeight;
    for (uint32_t chromaIndexY = 0; chromaIndexY < chromaHeight; ++chromaIndexY) {
        for (uint32_t chromaIndexX = 0; chromaIndexX < chromaWidth; ++chromaIndexX) {
            uint32_t chromaSampleIndex = chromaIndexY * chromaStride + chromaIndexX;
            buffer[uSampleOffset + chromaSampleIndex] = chromaIndexY % 2 == 0 ? 0xAA : 0xBB;
            buffer[vSampleOffset + chromaSampleIndex] = chromaIndexY % 2 == 0 ? 0xCC : 0xDD;
        }
    }
    return VideoFrameWrapper{
        buffer,
        stride,
        chromaStride,
        width,
        height,
        Pixelweave::PixelFormat::Planar8Bit420,
        Pixelweave::Range::Limited};
}

Pixelweave::VideoFrameWrapper GetPlanar422Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{buffer, stride, (width + 1) / 2, width, height, Pixelweave::PixelFormat::Planar8Bit422};
}

Pixelweave::VideoFrameWrapper GetPlanar444Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 3;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{buffer, stride, stride, width, height, Pixelweave::PixelFormat::Planar8Bit444};
}

Pixelweave::VideoFrameWrapper GetPlanarYV12Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * width) + chromaWidth * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0xFF;
    }
    const uint32_t vSampleOffset = width * height;
    const uint32_t uSampleOffset = vSampleOffset + chromaWidth * chromaHeight;

    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0xC0;
    }
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0xB0;
    }
    return VideoFrameWrapper{buffer, width, (width + 1) / 2, width, height, Pixelweave::PixelFormat::Planar8Bit420YV12};
}

Pixelweave::VideoFrameWrapper GetPlanarNV12Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * width) + chromaWidth * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0xFF;
    }
    const uint32_t uvSampleOffset = width * height;
    for (uint32_t uvSampleIndex = 0; uvSampleIndex < 2 * chromaWidth * chromaHeight; uvSampleIndex += 2) {
        buffer[uvSampleOffset + uvSampleIndex] = 0xB0;
        buffer[uvSampleOffset + uvSampleIndex + 1] = 0xC0;
    }
    return VideoFrameWrapper{buffer, width, (width + 1) / 2, width, height, Pixelweave::PixelFormat::Planar8Bit420NV12};
}

Pixelweave::VideoFrameWrapper GetRGBAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{buffer, width * 4, 0, width, height, Pixelweave::PixelFormat::Interleaved8BitRGBA, Pixelweave::Range::Full};
}

Pixelweave::VideoFrameWrapper GetBGRAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{buffer, width * 4, 0, width, height, Pixelweave::PixelFormat::Interleaved8BitBGRA, Pixelweave::Range::Full};
}

Pixelweave::VideoFrameWrapper GetPlanar42010BitFrame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = 2 * ((height * width) + chromaWidth * chromaHeight * 2);
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0x3FF;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0x3FF;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0x3FF;
    }
    return VideoFrameWrapper{
        reinterpret_cast<uint8_t*>(buffer),
        width * 2,
        ((width + 1) / 2) * 2,
        width,
        height,
        Pixelweave::PixelFormat::Planar10Bit420,
        Pixelweave::Range::Full};
}

Pixelweave::VideoFrameWrapper GetPlanar42210BitFrame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = height;
    const uint32_t bufferSize = 2 * ((height * width) + chromaWidth * chromaHeight * 2);
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0x03FF;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0x03FF;
    }
    return VideoFrameWrapper{
        reinterpret_cast<uint8_t*>(buffer),
        width * 2,
        ((width + 1) / 2) * 2,
        width,
        height,
        Pixelweave::PixelFormat::Planar10Bit422};
}

Pixelweave::VideoFrameWrapper GetPlanar44410BitFrame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = width;
    const uint32_t chromaHeight = height;
    const uint32_t bufferSize = 2 * ((height * width) + chromaWidth * chromaHeight * 2);
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0;
    }
    const uint32_t uSampleOffset = width * height;
    const uint32_t vSampleOffset = uSampleOffset + chromaWidth * chromaHeight;
    for (uint32_t uSampleIndex = 0; uSampleIndex < chromaWidth * chromaHeight; ++uSampleIndex) {
        buffer[uSampleOffset + uSampleIndex] = 0x3FF;
    }
    for (uint32_t vSampleIndex = 0; vSampleIndex < chromaWidth * chromaHeight; ++vSampleIndex) {
        buffer[vSampleOffset + vSampleIndex] = 0x3FF;
    }
    return VideoFrameWrapper{
        reinterpret_cast<uint8_t*>(buffer),
        width * 2,
        width * 2,
        width,
        height,
        Pixelweave::PixelFormat::Planar10Bit444};
}

Pixelweave::VideoFrameWrapper Get10BitRGBBuffer(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = width * height;
    uint32_t* buffer = new uint32_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 4, 0, width, height, Pixelweave::PixelFormat::Interleaved10BitRGB};
}

Pixelweave::VideoFrameWrapper GetPlanarP126Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t chromaHeight = height;
    const uint32_t bufferSize = height * width + chromaWidth * chromaHeight * 2;
    uint16_t* buffer = new uint16_t[bufferSize];
    for (uint32_t ySampleIndex = 0; ySampleIndex < width * height; ++ySampleIndex) {
        buffer[ySampleIndex] = 0x00FF;
    }
    const uint32_t uvSampleOffset = width * height;

    for (uint32_t lineIndex = 0; lineIndex < chromaHeight; ++lineIndex) {
        const uint16_t sample = lineIndex % 2 == 1 ? 0 : 0xFFFF;
        const uint32_t lineOffset = chromaWidth * 2 * lineIndex;
        for (uint32_t uvSampleIndex = 0; uvSampleIndex < chromaWidth * 2; uvSampleIndex += 2) {
            buffer[uvSampleOffset + lineOffset + uvSampleIndex] = sample;
            buffer[uvSampleOffset + lineOffset + uvSampleIndex + 1] = sample;
        }
    }

    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 2, 0, width, height, Pixelweave::PixelFormat::Planar16BitP216};
}

Pixelweave::VideoFrameWrapper Get10BitUYVYBuffer(uint32_t width, uint32_t height)
{
    const uint32_t stride = ((width + 47) / 48) * 128;
    const uint32_t bufferSize = stride * height;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), stride, 0, width, height, Pixelweave::PixelFormat::Interleaved10BitUYVY};
}

VideoFrameWrapper CreateFrame(PixelFormat pixelFormat, uint32_t width, uint32_t height)
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY: {
            return GetUYVYFrame(width, height);
        } break;
        case PixelFormat::Interleaved8BitBGRA: {
            return GetBGRAFrame(width, height);
        } break;
        case PixelFormat::Interleaved8BitRGBA: {
            return GetRGBAFrame(width, height);
        } break;
        case PixelFormat::Planar8Bit420: {
            return GetPlanar420Frame(width, height);
        } break;
        case PixelFormat::Planar8Bit422: {
            return GetPlanar422Frame(width, height);
        } break;
        case PixelFormat::Planar8Bit444: {
            return GetPlanar444Frame(width, height);
        } break;
        case PixelFormat::Planar8Bit420YV12: {
            return GetPlanarYV12Frame(width, height);
        } break;
        case PixelFormat::Planar8Bit420NV12: {
            return GetPlanarNV12Frame(width, height);
        } break;
        case PixelFormat::Interleaved10BitUYVY: {
            return Get10BitUYVYBuffer(width, height);
        } break;
        case PixelFormat::Interleaved10BitRGB: {
            return Get10BitRGBBuffer(width, height);
        } break;
        case PixelFormat::Interleaved12BitRGB: {
            //
        } break;
        case PixelFormat::Planar10Bit420: {
            return GetPlanar42010BitFrame(width, height);
        } break;
        case PixelFormat::Planar10Bit422: {
            return GetPlanar42210BitFrame(width, height);
        } break;
        case PixelFormat::Planar10Bit444: {
            return GetPlanar44410BitFrame(width, height);
        } break;
        case PixelFormat::Interleaved8BitARGB: {
            //
        } break;
        case PixelFormat::Interleaved12BitRGBLE: {
            //
        } break;
        case PixelFormat::Interleaved10BitRGBX: {
            //
        } break;
        case PixelFormat::Interleaved10BitRGBXLE: {
            //
        } break;
        case PixelFormat::Planar16BitP216: {
            return GetPlanarP126Frame(width, height);
        } break;
    }
    return Pixelweave::VideoFrameWrapper{};
}

std::string GetFormatName(PixelFormat pixelFormat)
{
    switch (pixelFormat) {
        case PixelFormat::Interleaved8BitUYVY: {
            return "Interleaved8BitUYVY";
        } break;
        case PixelFormat::Interleaved8BitBGRA: {
            return "Interleaved8BitBGRA";
        } break;
        case PixelFormat::Interleaved8BitRGBA: {
            return "Interleaved8BitRGBA";
        } break;
        case PixelFormat::Planar8Bit420: {
            return "Planar8Bit420";
        } break;
        case PixelFormat::Planar8Bit422: {
            return "Planar8Bit422";
        } break;
        case PixelFormat::Planar8Bit444: {
            return "Planar8Bit444";
        } break;
        case PixelFormat::Planar8Bit420YV12: {
            return "Planar8Bit420YV12";
        } break;
        case PixelFormat::Planar8Bit420NV12: {
            return "Planar8Bit420NV12";
        } break;
        case PixelFormat::Interleaved10BitUYVY: {
            return "Interleaved10BitUYVY";
        } break;
        case PixelFormat::Interleaved10BitRGB: {
            return "Interleaved10BitRGB";
        } break;
        case PixelFormat::Interleaved12BitRGB: {
            return "Interleaved12BitRGB";
        } break;
        case PixelFormat::Planar10Bit420: {
            return "Planar10Bit420";
        } break;
        case PixelFormat::Planar10Bit422: {
            return "Planar10Bit422";
        } break;
        case PixelFormat::Planar10Bit444: {
            return "Planar10Bit444";
        } break;
        case PixelFormat::Interleaved8BitARGB: {
            return "Interleaved8BitARGB";
        } break;
        case PixelFormat::Interleaved12BitRGBLE: {
            return "Interleaved12BitRGBLE";
        } break;
        case PixelFormat::Interleaved10BitRGBX: {
            return "Interleaved10BitRGBX";
        } break;
        case PixelFormat::Interleaved10BitRGBXLE: {
            return "Interleaved10BitRGBXLE";
        } break;
        case PixelFormat::Planar16BitP216: {
            return "Planar16BitP216";
        } break;
    }
    return "";
}

std::string GetRangeName(Range range)
{
    switch (range) {
        case Range::Limited:
            return "Limited";
        case Range::Full:
            return "Full";
        default:
            return "";
    }
}

std::string GetYUVMatrixName(YUVMatrix matrix)
{
    switch (matrix) {
        case YUVMatrix::BT709:
            return "BT709";
        case YUVMatrix::BT2020:
            return "BT2020";
        default:
            return "";
    }
}

int main()
{
    auto [result, device] = Pixelweave::Device::Create();
    if (result == Pixelweave::Result::Success) {
        std::vector<PixelFormat> validInputFormats{
            PixelFormat::Interleaved8BitUYVY,
            PixelFormat::Interleaved8BitBGRA,
            PixelFormat::Interleaved8BitRGBA,
            PixelFormat::Planar8Bit420,
            PixelFormat::Planar8Bit420YV12,
            PixelFormat::Planar8Bit420NV12,
            PixelFormat::Interleaved10BitRGB,
            PixelFormat::Planar16BitP216,
            PixelFormat::Planar8Bit422,
            PixelFormat::Planar8Bit444,
            PixelFormat::Planar10Bit420,
            PixelFormat::Planar10Bit422,
            PixelFormat::Planar10Bit444,
        };

        std::vector<PixelFormat> validOutputFormats{
            PixelFormat::Planar8Bit420,
            PixelFormat::Planar8Bit422,
            PixelFormat::Planar8Bit444,
            PixelFormat::Planar10Bit420,
            PixelFormat::Planar10Bit422,
            PixelFormat::Planar10Bit444,
            PixelFormat::Interleaved8BitUYVY,
            PixelFormat::Interleaved8BitBGRA,
            PixelFormat::Interleaved10BitRGB,
        };

        struct Resolution {
            uint32_t width, height;
        };
        std::vector<Resolution> resolutions{Resolution{3840, 2160}, Resolution{2560, 1440}, Resolution{1920, 1080}, Resolution{1280, 720}};

        std::vector<Pixelweave::Range> ranges{Pixelweave::Range::Limited, Pixelweave::Range::Full};

        std::vector<Pixelweave::YUVMatrix> matrices{Pixelweave::YUVMatrix::BT709, Pixelweave::YUVMatrix::BT2020};

        const auto videoConverter = device->CreateVideoConverter();

        // Iterates over all possible types of conversions
        for (Resolution inputResolution : resolutions) {
            for (PixelFormat inputFormat : validInputFormats) {
                for (Range inputRange : ranges) {
                    for (YUVMatrix inputMatrix : matrices) {
                        Pixelweave::VideoFrameWrapper inputFrame = CreateFrame(inputFormat, inputResolution.width, inputResolution.height);
                        inputFrame.range = inputRange;
                        inputFrame.yuvMatrix = inputMatrix;
                        for (Resolution outputResolution : resolutions) {
                            for (PixelFormat outputFormat : validOutputFormats) {
                                for (Range outputRange : ranges) {
                                    for (YUVMatrix outputMatrix : matrices) {
                                        Pixelweave::VideoFrameWrapper outputFrame =
                                            CreateFrame(outputFormat, outputResolution.width, outputResolution.height);
                                        outputFrame.range = outputRange;
                                        outputFrame.yuvMatrix = outputMatrix;

                                        std::cout << "Testing:" << std::endl
                                                  << "\t Input: " << inputFrame.width << "x" << inputFrame.height << "("
                                                  << GetFormatName(inputFrame.pixelFormat) << "," << GetRangeName(inputFrame.range) << ","
                                                  << GetYUVMatrixName(inputFrame.yuvMatrix) << ")" << std::endl
                                                  << "\t Output: " << outputFrame.width << "x" << outputFrame.height << "("
                                                  << GetFormatName(outputFrame.pixelFormat) << "," << GetRangeName(outputFrame.range) << ","
                                                  << GetYUVMatrixName(outputFrame.yuvMatrix) << ")" << std::endl;

                                        if (videoConverter->Convert(inputFrame, outputFrame) != Pixelweave::Result::Success) {
                                            std::cout << "Error converting" << std::endl;
                                            return -1;
                                        }
                                        delete[] outputFrame.buffer;
                                    }
                                }
                            }
                        }
                        delete[] inputFrame.buffer;
                    }
                }
            }
        }

        // Sanity check, compare 'copy'
        std::vector<PixelFormat> validInputOutputFormats;
        {
            std::set<PixelFormat> inputFormatSet(validInputFormats.begin(), validInputFormats.end());
            std::set<PixelFormat> outputFormatSet(validOutputFormats.begin(), validOutputFormats.end());
            std::set_intersection(
                inputFormatSet.begin(),
                inputFormatSet.end(),
                outputFormatSet.begin(),
                outputFormatSet.end(),
                std::back_inserter(validInputOutputFormats));
        }
        resolutions = std::vector<Resolution>{Resolution{64, 64}};

        for (Resolution inputResolution : resolutions) {
            for (PixelFormat format : validInputOutputFormats) {
                Pixelweave::VideoFrameWrapper inputFrame = CreateFrame(format, inputResolution.width, inputResolution.height);
                Pixelweave::VideoFrameWrapper outputFrame = CreateFrame(format, inputResolution.width, inputResolution.height);
                std::cout << "Testing comparison:" << std::endl
                          << "\t Input: " << inputFrame.width << "x" << inputFrame.height << "(" << GetFormatName(inputFrame.pixelFormat)
                          << "," << GetRangeName(inputFrame.range) << "," << GetYUVMatrixName(inputFrame.yuvMatrix) << ")" << std::endl;
                if (videoConverter->Convert(inputFrame, outputFrame) != Pixelweave::Result::Success) {
                    std::cout << "Error converting" << std::endl;
                    return -1;
                }
                if (memcmp(inputFrame.buffer, outputFrame.buffer, inputFrame.GetBufferSize()) != 0) {
                    std::cout << "Frames aren't equal" << std::endl;
                    return -1;
                }
                delete[] outputFrame.buffer;
                delete[] inputFrame.buffer;
            }
        }

        videoConverter->Release();
        device->Release();
        return 0;
    }
    return -1;
}
