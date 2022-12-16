# TODOs

## Must
- Instead of adding control blocks to prevent writing out of bounds in `writeXXXSample`, add padding to frames so that their sizes are multiple of 16 in width and height.
- Benchmark `readXXXBilinear` and `readXXXSampleFromXXXBuffer`. Remove `readXXXSampleFromXXXBuffer` if no performance benefits are seen.
- Add `P216` reading. `I010`, `I210`, `I444` writing.
- Use the [Vulkan Memory Allocator](https://gpuopen.com/vulkan-memory-allocator/) instead of raw allocations.
- Add color space conversion.
- YUV BT. 709 and BT. 2020 matrices and inverses should be baked in the shader for performance.
- Similar to ^, YUV scale and offset should be baked in the shader. Currently computed at runtime.
- Move YUV matrix and video range conversion to write time instead of doing it in `readPixel`.

## Nice to have
- Use macros to template `420`, `422`, `444` functions and remove duplicate code.
- Split shader into several files and use the `#include` extension when compiling.
- Add RGB writing support.
