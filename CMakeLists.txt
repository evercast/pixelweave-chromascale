cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

cmake_policy(SET CMP0126 NEW)

set(PROJECT_NAME PixelWeave)
set(VERSION 0.1.0)

if(UNIX AND NOT APPLE)
    set(LINUX ON)
endif()

set(PIXELWEAVE_ARCH x86_64 CACHE STRING "PixelWeave build architecture (may be `arm64` or `x86_64`).")

# Set up vcpkg
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg-triplets")
set(VCPKG_FEATURE_FLAGS manifests)
if(APPLE)
    # TODO: Figure out how to support fat binaries with vcpkg (or drop vcpkg)
    if(PIXELWEAVE_ARCH STREQUAL x86_64)
        set(VCPKG_HOST_TRIPLET x64-osx-evercast)
        set(VCPKG_TARGET_TRIPLET x64-osx-evercast)
        set(CMAKE_OSX_ARCHITECTURES x86_64)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
    else()
        set(VCPKG_HOST_TRIPLET arm64-osx-evercast)
        set(VCPKG_TARGET_TRIPLET arm64-osx-evercast)
        set(CMAKE_OSX_ARCHITECTURES arm64)
        set(CMAKE_OSX_DEPLOYMENT_TARGET 11)
    endif()
else()
    if(PIXELWEAVE_ARCH STREQUAL arm64)
        set(VCPKG_TARGET_ARCHITECTURE arm64)
    else()
        set(VCPKG_TARGET_ARCHITECTURE x64)
    endif()
endif()
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Build files in the top directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

# Add library subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Add test subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

# Add benchmark subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/benchmark)

# Configure 'Tests' as startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Tests)
