#pragma once

#include "Macros.h"

#ifdef PIXELWEAVE_DEBUG
#define VULKAN_HPP_ASSERT(condition) PIXELWEAVE_UNUSED(condition)
#else
#define VULKAN_HPP_ASSERT(condition)
#endif

// Disabled due to a dependency on macOS 13.3
#define VULKAN_HPP_NO_TO_STRING

#include <vulkan/vulkan.hpp>

#pragma warning(push, 0)
#include "vk_mem_alloc.h"
#pragma warning(pop)
