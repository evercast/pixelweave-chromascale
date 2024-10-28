#pragma once

#define PIXELWEAVE_UNUSED(arg) (void)(arg)

// Library export attributes
#ifdef PIXELWEAVE_PLATFORM_WINDOWS

// Define DLL export policy depeding on whether we're compiling the actual library or an external module
#ifdef PIXELWEAVE_IS_LIB
#define PIXELWEAVE_LIB_CLASS __declspec(dllexport)
#else
#define PIXELWEAVE_LIB_CLASS __declspec(dllimport)
#endif

#else

// On Unix platforms, all functions in public headers are visible
#define PIXELWEAVE_LIB_CLASS

#endif

// Debug macro
#ifdef PIXELWEAVE_PLATFORM_WINDOWS

#ifdef _DEBUG
#define PIXELWEAVE_DEBUG
#endif

#else

#ifdef DEBUG
#define PIXELWEAVE_DEBUG
#endif

#endif
