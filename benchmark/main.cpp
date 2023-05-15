#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "Device.h"

using namespace PixelWeave;

PixelWeave::VideoFrameWrapper GetUYVYFrame(uint32_t width, uint32_t height)
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
    return VideoFrameWrapper{buffer, stride, 0, width, height, PixelWeave::PixelFormat::Interleaved8BitUYVY};
}

PixelWeave::VideoFrameWrapper GetPlanar420Frame(uint32_t width, uint32_t height)
{
    const uint32_t chromaWidth = (width + 1) / 2;
    const uint32_t stride = width + 8;
    const uint32_t chromaStride = chromaWidth + 8;
    const uint32_t chromaHeight = (height + 1) / 2;
    const uint32_t bufferSize = (height * stride) + chromaStride * chromaHeight * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
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
        PixelWeave::PixelFormat::Planar8Bit420,
        PixelWeave::Range::Limited};
}

PixelWeave::VideoFrameWrapper GetPlanar422Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{buffer, stride, (width + 1) / 2, width, height, PixelWeave::PixelFormat::Planar8Bit422};
}

PixelWeave::VideoFrameWrapper GetPlanar444Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 3;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{buffer, stride, stride, width, height, PixelWeave::PixelFormat::Planar8Bit444};
}

PixelWeave::VideoFrameWrapper GetPlanarYV12Frame(uint32_t width, uint32_t height)
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
    return VideoFrameWrapper{buffer, width, (width + 1) / 2, width, height, PixelWeave::PixelFormat::Planar8Bit420YV12};
}

PixelWeave::VideoFrameWrapper GetPlanarNV12Frame(uint32_t width, uint32_t height)
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
    return VideoFrameWrapper{buffer, width, (width + 1) / 2, width, height, PixelWeave::PixelFormat::Planar8Bit420NV12};
}

PixelWeave::VideoFrameWrapper GetRGBAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{buffer, width * 4, 0, width, height, PixelWeave::PixelFormat::Interleaved8BitRGBA, PixelWeave::Range::Full};
}

PixelWeave::VideoFrameWrapper GetBGRAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{buffer, width * 4, 0, width, height, PixelWeave::PixelFormat::Interleaved8BitBGRA, PixelWeave::Range::Full};
}

PixelWeave::VideoFrameWrapper GetPlanar42010BitFrame(uint32_t width, uint32_t height)
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
        PixelWeave::PixelFormat::Planar10Bit420,
        PixelWeave::Range::Full};
}

PixelWeave::VideoFrameWrapper GetPlanar42210BitFrame(uint32_t width, uint32_t height)
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
        PixelWeave::PixelFormat::Planar10Bit422};
}

PixelWeave::VideoFrameWrapper GetPlanar44410BitFrame(uint32_t width, uint32_t height)
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
        PixelWeave::PixelFormat::Planar10Bit444};
}

PixelWeave::VideoFrameWrapper Get10BitRGBBuffer(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = width * height;
    uint32_t* buffer = new uint32_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 4, 0, width, height, PixelWeave::PixelFormat::Interleaved10BitRGB};
}

PixelWeave::VideoFrameWrapper GetPlanarP126Frame(uint32_t width, uint32_t height)
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

    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), width * 2, 0, width, height, PixelWeave::PixelFormat::Planar16BitP216};
}

PixelWeave::VideoFrameWrapper Get10BitUYVYBuffer(uint32_t width, uint32_t height)
{
    const uint32_t stride = ((width + 47) / 48) * 128;
    const uint32_t bufferSize = stride * height;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{reinterpret_cast<uint8_t*>(buffer), stride, 0, width, height, PixelWeave::PixelFormat::Interleaved10BitUYVY};
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
    return PixelWeave::VideoFrameWrapper{};
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

int main()
{
    auto [result, device] = PixelWeave::Device::Create();
    if (result == PixelWeave::Result::Success) {
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

        const auto videoConverter = device->CreateVideoConverter();

        struct BenchmarkResult {
            PixelFormat inputFormat;
            uint32_t inputWidth;
            uint32_t inputHeight;
            PixelFormat outputFormat;
            uint32_t outputWidth;
            uint32_t outputHeight;
            uint64_t copyToDeviceVisibleTimeMicros = 0;
            uint64_t transferDeviceVisibleToDeviceLocalTimeMicros = 0;
            uint64_t computeConversionTimeMicros = 0;
            uint64_t copyDeviceVisibleToHostLocalTimeMicros = 0;
        };

        std::vector<BenchmarkResult> results;
        for (Resolution inputResolution : resolutions) {
            for (PixelFormat inputFormat : validInputFormats) {
                PixelWeave::VideoFrameWrapper inputFrame = CreateFrame(inputFormat, inputResolution.width, inputResolution.height);
                for (Resolution outputResolution : resolutions) {
                    for (PixelFormat outputFormat : validOutputFormats) {
                        PixelWeave::VideoFrameWrapper outputFrame =
                            CreateFrame(outputFormat, outputResolution.width, outputResolution.height);
                        {
                            BenchmarkResult benchmarkResult{
                                inputFormat,
                                inputResolution.width,
                                inputResolution.height,
                                outputFormat,
                                outputResolution.width,
                                outputResolution.height};
                            std::cout << "Benchmarking: Input(" << GetFormatName(inputFormat) << "-" << inputResolution.width << "x"
                                      << inputResolution.height << ") Output(" << GetFormatName(outputFormat) << "-"
                                      << outputResolution.width << "x" << outputResolution.height << ")" << std::endl;
                            constexpr uint32_t iterations = 10;
                            for (uint32_t i = 0; i < iterations + 1; ++i) {
                                auto resultAndBenchmark = videoConverter->ConvertWithBenchmark(inputFrame, outputFrame);

                                if (resultAndBenchmark.result != Result::Success) {
                                    std::cout << "Error converting frame" << std::endl;
                                    while (true) {
                                    };
                                }

                                if (i > 0) {
                                    benchmarkResult.copyToDeviceVisibleTimeMicros += resultAndBenchmark.value.copyToDeviceVisibleTimeMicros;
                                    benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros +=
                                        resultAndBenchmark.value.transferDeviceVisibleToDeviceLocalTimeMicros;
                                    benchmarkResult.computeConversionTimeMicros += resultAndBenchmark.value.computeConversionTimeMicros;
                                    benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros +=
                                        resultAndBenchmark.value.copyDeviceVisibleToHostLocalTimeMicros;
                                }
                            }
                            benchmarkResult.copyToDeviceVisibleTimeMicros /= iterations;
                            benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros /= iterations;
                            benchmarkResult.computeConversionTimeMicros /= iterations;
                            benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros /= iterations;
                            results.push_back(benchmarkResult);
                        }
                        delete[] outputFrame.buffer;
                    }
                }
                delete[] inputFrame.buffer;
            }
        }

        std::fstream benchmarkStream;
        const std::string separator = ",";
        benchmarkStream.open("benchmark.csv", std::ios::out);
        benchmarkStream << "InputFormat" << separator << "InputWidth" << separator << "InputHeight" << separator << "OutputFormat"
                        << separator << "OutputWidth" << separator << "OutputHeight" << separator << "CopyToDeviceVisibleTimeMicros"
                        << separator << "TransferDeviceVisibleToDeviceLocalTimeMicros" << separator << "ComputeConversionTimeMicros"
                        << separator << "CopyDeviceVisibleToHostLocalTimeMicros" << separator << "TotalTime" << std::endl;

        for (const BenchmarkResult& benchmarkResult : results) {
            benchmarkStream << GetFormatName(benchmarkResult.inputFormat) << separator << benchmarkResult.inputWidth << separator
                            << benchmarkResult.inputHeight << separator << GetFormatName(benchmarkResult.outputFormat) << separator
                            << benchmarkResult.outputWidth << separator << benchmarkResult.outputHeight << separator
                            << benchmarkResult.copyToDeviceVisibleTimeMicros << separator
                            << benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros << separator
                            << benchmarkResult.computeConversionTimeMicros << separator
                            << benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros << separator
                            << (benchmarkResult.copyToDeviceVisibleTimeMicros +
                                benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros + benchmarkResult.computeConversionTimeMicros +
                                benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros)
                            << std::endl;
        }
        benchmarkStream.close();

        videoConverter->Release();
        device->Release();
        return 0;
    }
    return -1;
}
