#pragma once

#ifdef PW_IS_LIB
#define PIXEL_WEAVE_LIB_CLASS __declspec(dllexport)
#else
#define PIXEL_WEAVE_LIB_CLASS __declspec(dllimport)
#endif