set(SUBPROJECT_NAME "PixelWeave")

# Attach source files to build
set(HEADERS
    include/Device.h
    include/Result.h
    include/VideoConverter.h
    include/RefCountPtr.h
    include/Macros.h
    include/PixelFormat.h
    include/VideoFrameWrapper.h
)

# Private headers and source files
set(SOURCE
    src/VulkanDevice.h
    src/VulkanInstance.h
    src/DebugUtils.h
    src/VulkanVideoConverter.h
    src/ResourceLoader.h
    src/ResourceLoader.cpp
    src/VulkanDevice.cpp
    src/Device.cpp
    src/VulkanInstance.cpp
    src/VulkanVideoConverter.cpp
    src/RefCountPtr.cpp
    src/PixelFormat.cpp
    src/VulkanImage.h
    src/VulkanImage.cpp
    src/VulkanBuffer.h
    src/VulkanBuffer.cpp
    src/VulkanBase.h
)

if (WIN32)
    set(PLATFORM_SPECIFIC_SOURCES
        src/ShaderResources.h
        src/ShaderResources.rc
    )
endif()

add_library(${SUBPROJECT_NAME} SHARED
    ${HEADERS}
    ${SOURCE}
    ${PLATFORM_SPECIFIC_SOURCES}
)

include_directories(include)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CMakeCXXUtils.txt)
pw_configure_cxx(${SUBPROJECT_NAME})

# Setup DLL export macro
add_compile_definitions(PW_IS_LIB)

# Find Vulkan SDK
add_compile_definitions(VULKAN_HPP_NO_EXCEPTIONS)
find_package(Vulkan REQUIRED)
target_link_libraries(${SUBPROJECT_NAME} ${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})

if (APPLE)

set(RESOURCE_FILES src/shaders/convert.comp.spv)
message(STATUS ${RESOURCE_FILES})
set_target_properties(${SUBPROJECT_NAME} PROPERTIES
  FRAMEWORK TRUE
  FRAMEWORK_VERSION A
  MACOSX_BUNDLE TRUE
  MACOSX_FRAMEWORK_IDENTIFIER us.evercast.pixelweave
  VERSION 1.0.0
  SOVERSION 1.0.0
  PUBLIC_HEADER "${HEADERS}"
  RESOURCE "${RESOURCE_FILES}"
)
target_link_libraries(${SUBPROJECT_NAME} "-framework CoreFoundation")

endif()