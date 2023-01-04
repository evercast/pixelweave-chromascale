#pragma once

#define PW_UNUSED(arg) (void)(arg)

// Library export attributes
#ifdef PW_PLATFORM_WINDOWS

// Define DLL export policy depeding on whether we're compiling the actual library or an external module
#ifdef PW_IS_LIB
#define PIXEL_WEAVE_LIB_CLASS __declspec(dllexport)
#else
#define PIXEL_WEAVE_LIB_CLASS __declspec(dllimport)
#endif

#else

// On Unix platforms, all functions in public headers are visible
#define PIXEL_WEAVE_LIB_CLASS

#endif

// Debug macro
#ifdef PW_PLATFORM_WINDOWS

#ifdef _DEBUG
#define PW_DEBUG
#endif

#else

#ifdef DEBUG
#define PW_DEBUG
#endif

#endif
