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
    src/VideoFrameWrapper.cpp
    src/VulkanBuffer.h
    src/VulkanBuffer.cpp
    src/VulkanBase.h
)

if(WIN32)
    set(PLATFORM_SPECIFIC_SOURCES
        src/ShaderResources.h
        src/ShaderResources.rc
    )
endif()

set(SHADER ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/convert.comp)

if(APPLE)
    set(RESOURCE_FILES "${SHADER}.spv")
endif()

add_library(${SUBPROJECT_NAME} SHARED
    ${HEADERS}
    ${SOURCE}
    ${PLATFORM_SPECIFIC_SOURCES}
    ${RESOURCE_FILES}
)

include_directories(include)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CXXUtils.cmake)
pw_configure_cxx(${SUBPROJECT_NAME})

# Setup DLL export macro
add_compile_definitions(PW_IS_LIB)

# Find Vulkan SDK
add_compile_definitions(VULKAN_HPP_NO_EXCEPTIONS)
find_package(Vulkan REQUIRED)
target_link_libraries(${SUBPROJECT_NAME} ${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})

# Add shader compilation build step
if (WIN32)
set(SHADER_COMPILER_PATH "$ENV{VULKAN_SDK}/Bin/glslc.exe")
elseif(APPLE)
endif()

set(COMPILED_SHADER "${SHADER}.spv")
add_custom_command(
    OUTPUT ${COMPILED_SHADER}
    COMMAND ${SHADER_COMPILER_PATH} ${SHADER} -o ${COMPILED_SHADER}
    DEPENDS ${SHADER}
)
set(SHADER_DEPENDENCIES "${SUBPROJECT_NAME}Shaders")
add_custom_target(${SHADER_DEPENDENCIES} DEPENDS ${COMPILED_SHADER})
add_dependencies(${SUBPROJECT_NAME} ${SHADER_DEPENDENCIES})

# Setup Mac framework
if(APPLE)
    set(PW_BUNDLE_ID us.evercast.pixelweave)
    set_target_properties(${SUBPROJECT_NAME} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_BUNDLE TRUE
        MACOSX_FRAMEWORK_IDENTIFIER ${PW_BUNDLE_ID}
        VERSION 1.0.0
        SOVERSION 1.0.0
        PUBLIC_HEADER "${HEADERS}"
        RESOURCE "${RESOURCE_FILES}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${PW_BUNDLE_ID}
        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        XCODE_ATTRIBUTE_SKIP_INSTALL "YES"
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "$(inherited)"
        XCODE_ATTRIBUTE_OTHER_LDFLAGS "$(inherited)"
        XCODE_ATTRIBUTE_OTHER_SWIFT_FLAGS "$(inherited)"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK "YES"
        XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
        XCODE_ATTRIBUTE_CLANG_WARN_SUSPICIOUS_IMPLICIT_CONVERSION "YES"
        XCODE_ATTRIBUTE_CLANG_WARN_IMPLICIT_RETAIN_SELF "YES"
        XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE "YES"
        XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION "YES"
        XCODE_ATTRIBUTE_ENABLE_TESTABILITY "YES"
    )
    target_link_libraries(${SUBPROJECT_NAME} "-framework CoreFoundation")
    add_compile_definitions(PW_BUNDLE_ID="${PW_BUNDLE_ID}")
endif()
