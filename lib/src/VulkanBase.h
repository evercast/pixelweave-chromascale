#pragma once

#include "Macros.h"

#if PW_DEBUG
#define VULKAN_HPP_ASSERT(condition) PW_UNUSED(condition)
#else
#define VULKAN_HPP_ASSERT(condition) 
#endif
#include <vulkan/vulkan.hpp>
