#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "Macros.h"
#include "VulkanBase.h"

#ifdef PW_PLATFORM_WINDOWS
#include <Windows.h>
#include <debugapi.h>
#endif

#ifdef PW_PLATFORM_WINDOWS

#define PW_LOG(message)                        \
    {                                          \
        std::wostringstream os_;               \
        os_ << message << std::endl;           \
        OutputDebugStringW(os_.str().c_str()); \
    }

#ifdef PW_DEBUG
#define PW_ASSERT_MSG(condition, message)                                                             \
    {                                                                                                 \
        if (!(condition)) {                                                                           \
            std::string fileInfo = message;                                                           \
            std::wstring wInfo = std::wstring(fileInfo.begin(), fileInfo.end());                      \
            PW_LOG(                                                                                   \
                "In file: " << __FILE__ << ", line: " << __LINE__ << " of function: " << __FUNCTION__ \
                            << "Condition failed : " << #condition << std::endl                       \
                            << message << std::endl);                                                 \
            __debugbreak();                                                                           \
        }                                                                                             \
    }
#else
#define PW_ASSERT_MSG(condition, message) PW_UNUSED(condition)
#endif

#endif

#if defined(PW_PLATFORM_MACOS) || defined(PW_PLATFORM_LINUX)

#define PW_LOG(message)              \
    {                                \
        std::ostringstream os_;      \
        os_ << message << std::endl; \
        std::cout << os_.str();      \
    }
#define PW_ASSERT_MSG(condition, message) PW_UNUSED(condition)

#endif

#define PW_ASSERT(condition) PW_ASSERT_MSG(condition, "")

template <typename ResultValue>
inline auto PW_ASSERT_VK(ResultValue resultValue) -> decltype(resultValue.value)
{
    PW_ASSERT(resultValue.result == vk::Result::eSuccess);
    return resultValue.value;
}

inline void PW_ASSERT_VK(vk::Result result)
{
    PW_ASSERT(result == vk::Result::eSuccess);
}
