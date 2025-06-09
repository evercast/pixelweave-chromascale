#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "Device.h"

using namespace Pixelweave;

VideoFrameWrapper GetUYVYFrame(uint32_t width, uint32_t height)
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
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = stride,
        .chromaStride = 0,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC8Bit422InterleavedUYVY,
    };
}

VideoFrameWrapper GetPlanar420Frame(uint32_t width, uint32_t height)
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
        .buffer = buffer,
        .stride = stride,
        .chromaStride = chromaStride,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC8Bit420Planar,
        .range = VideoRange::Legal,
    };
}

VideoFrameWrapper GetPlanar422Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 2;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = stride,
        .chromaStride = (width + 1) / 2,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC8Bit422Planar,
    };
}

VideoFrameWrapper GetPlanar444Frame(uint32_t width, uint32_t height)
{
    const uint32_t stride = width;
    const uint32_t bufferSize = height * stride * 3;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0xFF;
    }
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = stride,
        .chromaStride = stride,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC8Bit444Planar,
    };
}

VideoFrameWrapper GetYV12Frame(uint32_t width, uint32_t height)
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
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = width,
        .chromaStride = (width + 1) / 2,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC8Bit420PlanarYV12,
    };
}

VideoFrameWrapper GetNV12Frame(uint32_t width, uint32_t height)
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
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = width,
        .chromaStride = (width + 1) / 2,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC8Bit420BiplanarNV12,
    };
}

VideoFrameWrapper GetRGBAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = width * 4,
        .chromaStride = 0,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::RGB8BitInterleavedRGBA,
        .range = VideoRange::Full,
    };
}

VideoFrameWrapper GetBGRAFrame(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = (height * width) * 4;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t sampleIndex = 0; sampleIndex < width * height; ++sampleIndex) {
        buffer[sampleIndex * 4] = 0xFF;
        buffer[sampleIndex * 4 + 1] = 0xFF;
        buffer[sampleIndex * 4 + 2] = 0xFF;
        buffer[sampleIndex * 4 + 3] = 0xFF;
    }
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = width * 4,
        .chromaStride = 0,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::RGB8BitInterleavedBGRA,
        .range = VideoRange::Full,
    };
}

VideoFrameWrapper GetPlanar42010BitFrame(uint32_t width, uint32_t height)
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
        .buffer = reinterpret_cast<uint8_t*>(buffer),
        .stride = width * 2,
        .chromaStride = ((width + 1) / 2) * 2,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC10Bit420Planar,
        .range = VideoRange::Full,
    };
}

VideoFrameWrapper GetPlanar42210BitFrame(uint32_t width, uint32_t height)
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
        .buffer = reinterpret_cast<uint8_t*>(buffer),
        .stride = width * 2,
        .chromaStride = ((width + 1) / 2) * 2,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC10Bit422Planar,
    };
}

VideoFrameWrapper GetPlanar44410BitFrame(uint32_t width, uint32_t height)
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
        .buffer = reinterpret_cast<uint8_t*>(buffer),
        .stride = width * 2,
        .chromaStride = width * 2,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC10Bit444Planar,
    };
}

VideoFrameWrapper Get10BitXRGBBEBuffer(uint32_t width, uint32_t height)
{
    const uint32_t bufferSize = width * height;
    uint32_t* buffer = new uint32_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{
        .buffer = reinterpret_cast<uint8_t*>(buffer),
        .stride = width * 4,
        .chromaStride = 0,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::RGB10BitInterleavedXRGBBE,
    };
}

VideoFrameWrapper GetP216Frame(uint32_t width, uint32_t height)
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

    return VideoFrameWrapper{
        .buffer = reinterpret_cast<uint8_t*>(buffer),
        .stride = width * 2,
        .chromaStride = 0,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC16Bit422BiplanarP216,
    };
}

VideoFrameWrapper GetV210Buffer(uint32_t width, uint32_t height)
{
    const uint32_t stride = ((width + 47) / 48) * 128;
    const uint32_t bufferSize = stride * height;
    uint8_t* buffer = new uint8_t[bufferSize];
    for (uint32_t bufferIndex = 0; bufferIndex < bufferSize; ++bufferIndex) {
        buffer[bufferIndex] = 0;
    }
    return VideoFrameWrapper{
        .buffer = buffer,
        .stride = stride,
        .chromaStride = 0,
        .width = width,
        .height = height,
        .pixelFormat = PixelFormat::YCC10Bit422InterleavedV210,
    };
}

VideoFrameWrapper CreateFrame(PixelFormat pixelFormat, uint32_t width, uint32_t height)
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        case PixelFormat::RGB8BitInterleavedBGRA:
            return GetBGRAFrame(width, height);
        case PixelFormat::RGB8BitInterleavedRGBA:
            return GetRGBAFrame(width, height);
        case PixelFormat::RGB8BitInterleavedARGB:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::YCC8Bit420Planar:
            return GetPlanar420Frame(width, height);
        case PixelFormat::YCC8Bit420PlanarYV12:
            return GetYV12Frame(width, height);
        case PixelFormat::YCC8Bit422Planar:
            return GetPlanar422Frame(width, height);
        case PixelFormat::YCC8Bit444Planar:
            return GetPlanar444Frame(width, height);
        case PixelFormat::YCC8Bit420BiplanarNV12:
            return GetNV12Frame(width, height);
        case PixelFormat::YCC8Bit422InterleavedUYVY:
            return GetUYVYFrame(width, height);
        case PixelFormat::RGB10BitInterleavedRGBXBE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::RGB10BitInterleavedRGBXLE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::RGB10BitInterleavedXRGBBE:
            return Get10BitXRGBBEBuffer(width, height);
        case PixelFormat::RGB10BitInterleavedXRGBLE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::RGB10BitInterleavedXBGRBE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::RGB10BitInterleavedXBGRLE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::YCC10Bit420Planar:
            return GetPlanar42010BitFrame(width, height);
        case PixelFormat::YCC10Bit422Planar:
            return GetPlanar42210BitFrame(width, height);
        case PixelFormat::YCC10Bit444Planar:
            return GetPlanar44410BitFrame(width, height);
        case PixelFormat::YCC10Bit420BiplanarP010:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::YCC10Bit422BiplanarP210:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::YCC10Bit444BiplanarP410:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::YCC10Bit422InterleavedV210:
            return GetV210Buffer(width, height);
        case PixelFormat::RGB12BitInterleavedBGRBE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::RGB12BitInterleavedBGRLE:
            // TODO
            return VideoFrameWrapper{};
        case PixelFormat::YCC16Bit422BiplanarP216:
            return GetP216Frame(width, height);
        default:
            return VideoFrameWrapper{};
    }
}

std::string GetFormatName(PixelFormat pixelFormat)
{
    static_assert(AllPixelFormats.size() == 25);
    switch (pixelFormat) {
        case PixelFormat::RGB8BitInterleavedBGRA:
            return "RGB8BitInterleavedBGRA";
        case PixelFormat::RGB8BitInterleavedRGBA:
            return "RGB8BitInterleavedRGBA";
        case PixelFormat::RGB8BitInterleavedARGB:
            return "RGB8BitInterleavedARGB";
        case PixelFormat::YCC8Bit420Planar:
            return "YCC8Bit420Planar";
        case PixelFormat::YCC8Bit420PlanarYV12:
            return "YCC8Bit420PlanarYV12";
        case PixelFormat::YCC8Bit422Planar:
            return "YCC8Bit422Planar";
        case PixelFormat::YCC8Bit444Planar:
            return "YCC8Bit444Planar";
        case PixelFormat::YCC8Bit420BiplanarNV12:
            return "YCC8Bit420BiplanarNV12";
        case PixelFormat::YCC8Bit422InterleavedUYVY:
            return "YCC8Bit422InterleavedUYVY";
        case PixelFormat::RGB10BitInterleavedRGBXBE:
            return "RGB10BitInterleavedRGBXBE";
        case PixelFormat::RGB10BitInterleavedRGBXLE:
            return "RGB10BitInterleavedRGBXLE";
        case PixelFormat::RGB10BitInterleavedXRGBBE:
            return "RGB10BitInterleavedXRGBBE";
        case PixelFormat::RGB10BitInterleavedXRGBLE:
            return "RGB10BitInterleavedXRGBLE";
        case PixelFormat::RGB10BitInterleavedXBGRBE:
            return "RGB10BitInterleavedXBGRBE";
        case PixelFormat::RGB10BitInterleavedXBGRLE:
            return "RGB10BitInterleavedXBGRLE";
        case PixelFormat::YCC10Bit420Planar:
            return "YCC10Bit420Planar";
        case PixelFormat::YCC10Bit422Planar:
            return "YCC10Bit422Planar";
        case PixelFormat::YCC10Bit444Planar:
            return "YCC10Bit444Planar";
        case PixelFormat::YCC10Bit420BiplanarP010:
            return "YCC10Bit420BiplanarP010";
        case PixelFormat::YCC10Bit422BiplanarP210:
            return "YCC10Bit422BiplanarP210";
        case PixelFormat::YCC10Bit444BiplanarP410:
            return "YCC10Bit444BiplanarP410";
        case PixelFormat::YCC10Bit422InterleavedV210:
            return "YCC10Bit422InterleavedV210";
        case PixelFormat::RGB12BitInterleavedBGRBE:
            return "RGB12BitInterleavedBGRBE";
        case PixelFormat::RGB12BitInterleavedBGRLE:
            return "RGB12BitInterleavedBGRLE";
        case PixelFormat::YCC16Bit422BiplanarP216:
            return "YCC16Bit422BiplanarP216";
        default:
            return "";
    }
}

int main()
{
    auto [result, device] = Device::Create();
    if (result != Result::Success) {
        return -1;
    }

    std::vector<PixelFormat> validInputFormats{
        PixelFormat::RGB8BitInterleavedBGRA,
        PixelFormat::RGB8BitInterleavedRGBA,
        PixelFormat::YCC8Bit420Planar,
        PixelFormat::YCC8Bit420PlanarYV12,
        PixelFormat::YCC8Bit422Planar,
        PixelFormat::YCC8Bit444Planar,
        PixelFormat::YCC8Bit420BiplanarNV12,
        PixelFormat::YCC8Bit422InterleavedUYVY,
        PixelFormat::RGB10BitInterleavedXRGBBE,
        PixelFormat::YCC10Bit420Planar,
        PixelFormat::YCC10Bit422Planar,
        PixelFormat::YCC10Bit444Planar,
        PixelFormat::YCC16Bit422BiplanarP216,
    };

    std::vector<PixelFormat> validOutputFormats{
        PixelFormat::RGB8BitInterleavedBGRA,
        PixelFormat::YCC8Bit420Planar,
        PixelFormat::YCC8Bit422Planar,
        PixelFormat::YCC8Bit444Planar,
        PixelFormat::YCC8Bit422InterleavedUYVY,
        PixelFormat::RGB10BitInterleavedXRGBBE,
        PixelFormat::YCC10Bit420Planar,
        PixelFormat::YCC10Bit422Planar,
        PixelFormat::YCC10Bit444Planar,
    };

    struct Resolution {
        uint32_t width, height;
    };
    std::vector<Resolution> resolutions{
        Resolution{3840, 2160},
        Resolution{2560, 1440},
        Resolution{1920, 1080},
        Resolution{1280, 720}};

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
            VideoFrameWrapper inputFrame = CreateFrame(inputFormat, inputResolution.width, inputResolution.height);
            for (Resolution outputResolution : resolutions) {
                for (PixelFormat outputFormat : validOutputFormats) {
                    VideoFrameWrapper outputFrame =
                        CreateFrame(outputFormat, outputResolution.width, outputResolution.height);
                    {
                        BenchmarkResult benchmarkResult{
                            inputFormat,
                            inputResolution.width,
                            inputResolution.height,
                            outputFormat,
                            outputResolution.width,
                            outputResolution.height};
                        std::cout << "Benchmarking: Input(" << GetFormatName(inputFormat) << "-"
                                  << inputResolution.width << "x" << inputResolution.height << ") Output("
                                  << GetFormatName(outputFormat) << "-" << outputResolution.width << "x"
                                  << outputResolution.height << ")" << std::endl;
                        constexpr uint32_t iterations = 10;
                        for (uint32_t i = 0; i < iterations + 1; ++i) {
                            auto resultAndBenchmark = videoConverter->ConvertWithBenchmark(inputFrame, outputFrame);
                            if (resultAndBenchmark.result != Result::Success) {
                                std::cout << "Error converting frame" << std::endl;
                                while (true)
                                    ;
                            }

                            if (i > 0) {
                                benchmarkResult.copyToDeviceVisibleTimeMicros +=
                                    resultAndBenchmark.value.copyToDeviceVisibleTimeMicros;
                                benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros +=
                                    resultAndBenchmark.value.transferDeviceVisibleToDeviceLocalTimeMicros;
                                benchmarkResult.computeConversionTimeMicros +=
                                    resultAndBenchmark.value.computeConversionTimeMicros;
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
    benchmarkStream << "InputFormat" << separator << "InputWidth" << separator << "InputHeight" << separator
                    << "OutputFormat" << separator << "OutputWidth" << separator << "OutputHeight" << separator
                    << "CopyToDeviceVisibleTimeMicros" << separator << "TransferDeviceVisibleToDeviceLocalTimeMicros"
                    << separator << "ComputeConversionTimeMicros" << separator
                    << "CopyDeviceVisibleToHostLocalTimeMicros" << separator << "TotalTime" << std::endl;

    for (const BenchmarkResult& benchmarkResult : results) {
        benchmarkStream << GetFormatName(benchmarkResult.inputFormat) << separator << benchmarkResult.inputWidth
                        << separator << benchmarkResult.inputHeight << separator
                        << GetFormatName(benchmarkResult.outputFormat) << separator << benchmarkResult.outputWidth
                        << separator << benchmarkResult.outputHeight << separator
                        << benchmarkResult.copyToDeviceVisibleTimeMicros << separator
                        << benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros << separator
                        << benchmarkResult.computeConversionTimeMicros << separator
                        << benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros << separator
                        << (benchmarkResult.copyToDeviceVisibleTimeMicros +
                            benchmarkResult.transferDeviceVisibleToDeviceLocalTimeMicros +
                            benchmarkResult.computeConversionTimeMicros +
                            benchmarkResult.copyDeviceVisibleToHostLocalTimeMicros)
                        << std::endl;
    }
    benchmarkStream.close();

    videoConverter->Release();
    device->Release();
    return 0;
}
