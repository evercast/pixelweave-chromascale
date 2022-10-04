#pragma once

#include <sstream>
#include <string>

#include <vulkan/vulkan.hpp>

#include <Windows.h>
#include <debugapi.h>

#define PW_LOG(message)                        \
    {                                          \
        std::wostringstream os_;               \
        os_ << message << std::endl;           \
        OutputDebugStringW(os_.str().c_str()); \
    }

#if _DEBUG

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
#define PW_ASSERT_MSG(condition, message) (condition)
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
