cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

cmake_policy(SET CMP0126 NEW)

set(PROJECT_NAME Pixelweave)
set(VERSION 0.1)

if(APPLE)
    set(DEFAULT_PIXELWEAVE_ARCH "arm64;x86_64")
else()
    set(DEFAULT_PIXELWEAVE_ARCH x86_64)
endif()
set(PIXELWEAVE_ARCH ${DEFAULT_PIXELWEAVE_ARCH} CACHE STRING
    "Pixelweave build architecture (may be `arm64` or `x86_64`, or a combination such as `arm64;x86_64` on macOS).")
message("-- Pixelweave build architecture is ${PIXELWEAVE_ARCH}")

option(PIXELWEAVE_USE_VCPKG "Use vcpkg for Pixelweave dependencies." OFF)
message("-- Use of vcpkg for dependencies is ${PIXELWEAVE_USE_VCPKG}")

if(PIXELWEAVE_USE_VCPKG)
    # Set up vcpkg
    set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg-triplets")
    set(VCPKG_FEATURE_FLAGS manifests)
    if(WIN32)
        if(PIXELWEAVE_ARCH STREQUAL arm64)
            set(VCPKG_HOST_TRIPLET arm64-windows)
            set(VCPKG_TARGET_TRIPLET arm64-windows)
        else()
            set(VCPKG_HOST_TRIPLET x64-windows)
            set(VCPKG_TARGET_TRIPLET x64-windows)
        endif()
    elseif(APPLE)
        if(PIXELWEAVE_ARCH STREQUAL x86_64)
            set(VCPKG_HOST_TRIPLET x64-osx)
            set(VCPKG_TARGET_TRIPLET x64-osx)
        else()
            set(VCPKG_HOST_TRIPLET arm64-osx)
            set(VCPKG_TARGET_TRIPLET arm64-osx)
        endif()
    elseif(LINUX)
        if(PIXELWEAVE_ARCH STREQUAL arm64)
            set(VCPKG_HOST_TRIPLET arm64-linux)
            set(VCPKG_TARGET_TRIPLET arm64-linux)
        else()
            set(VCPKG_HOST_TRIPLET x64-linux)
            set(VCPKG_TARGET_TRIPLET x64-linux)
        endif()
    endif()
    set(VCPKG_CRT_LINKAGE static)
    set(VCPKG_LIBRARY_LINKAGE static)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

# Build files in the top directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Must be set before invoking `project()`
set(CMAKE_OSX_ARCHITECTURES ${PIXELWEAVE_ARCH})
set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

# Add library subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Add test subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

# Add benchmark subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/benchmark)

# Configure 'Tests' as Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Tests)
