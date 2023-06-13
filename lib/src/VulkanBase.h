#pragma once

#include "Macros.h"

#ifdef PW_DEBUG
#define VULKAN_HPP_ASSERT(condition) PW_UNUSED(condition)
#else
#define VULKAN_HPP_ASSERT(condition)
#endif
#include <vulkan/vulkan.hpp>

#pragma warning(push, 0)
#include "vk_mem_alloc.h"
#pragma warning(pop)