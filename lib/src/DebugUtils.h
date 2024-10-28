#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "Macros.h"
#include "VulkanBase.h"

#ifdef PIXELWEAVE_PLATFORM_WINDOWS
#include <Windows.h>
#include <debugapi.h>
#endif

#ifdef PIXELWEAVE_PLATFORM_WINDOWS

#define PIXELWEAVE_LOG(message)                        \
    {                                          \
        std::wostringstream os_;               \
        os_ << message << std::endl;           \
        OutputDebugStringW(os_.str().c_str()); \
    }

#ifdef PIXELWEAVE_DEBUG
#define PIXELWEAVE_ASSERT_MSG(condition, message)                                                             \
    {                                                                                                 \
        if (!(condition)) {                                                                           \
            std::string fileInfo = message;                                                           \
            std::wstring wInfo = std::wstring(fileInfo.begin(), fileInfo.end());                      \
            PIXELWEAVE_LOG(                                                                                   \
                "In file: " << __FILE__ << ", line: " << __LINE__ << " of function: " << __FUNCTION__ \
                            << "Condition failed : " << #condition << std::endl                       \
                            << message << std::endl);                                                 \
            __debugbreak();                                                                           \
        }                                                                                             \
    }
#else
#define PIXELWEAVE_ASSERT_MSG(condition, message) PIXELWEAVE_UNUSED(condition)
#endif

#endif

#if defined(PIXELWEAVE_PLATFORM_MACOS) || defined(PIXELWEAVE_PLATFORM_LINUX)

#define PIXELWEAVE_LOG(message)              \
    {                                \
        std::ostringstream os_;      \
        os_ << message << std::endl; \
        std::cout << os_.str();      \
    }
#define PIXELWEAVE_ASSERT_MSG(condition, message) PIXELWEAVE_UNUSED(condition)

#endif

#define PIXELWEAVE_ASSERT(condition) PIXELWEAVE_ASSERT_MSG(condition, "")

template <typename ResultValue>
inline auto PIXELWEAVE_ASSERT_VK(ResultValue resultValue) -> decltype(resultValue.value)
{
    PIXELWEAVE_ASSERT(resultValue.result == vk::Result::eSuccess);
    return resultValue.value;
}

inline void PIXELWEAVE_ASSERT_VK(vk::Result result)
{
    PIXELWEAVE_ASSERT(result == vk::Result::eSuccess);
}

inline void PIXELWEAVE_ASSERT_VK(VkResult result)
{
    PIXELWEAVE_ASSERT(result == VK_SUCCESS);
}