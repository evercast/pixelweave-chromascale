# TODOs

## Must
- Instead of adding control blocks to prevent writing out of bounds in `writeXXXSample`, add padding to frames so that their sizes are multiple of 16 in width and height.
- Benchmark `readXXXBilinear` and `readXXXSampleFromXXXBuffer`. Remove `readXXXSampleFromXXXBuffer` if no performance benefits are seen.
- Add `P216` reading. `I010`, `I210`, `I444` writing.
- Add range conversion (limited, full).
- Add yuv matrix conversion (`bt709`, `bt2020`).
- Add color space conversion.

## Nice to have
- Use macros to template `420`, `422`, `444` functions and remove duplicate code.
- Split shader into several files and use the `#include` extension when compiling.
- Add RGB writing support.
