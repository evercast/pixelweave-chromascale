set(SUBPROJECT_NAME "PixelWeave")

# Public headers
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
    src/Timer.h
    src/ColorSpaceUtils.h
    src/ColorSpaceUtils.cpp
)

if(WIN32)
    set(PLATFORM_SPECIFIC_SOURCES
        src/ShaderResources.h
        src/ShaderResources.rc
    )
elseif(LINUX)
    set(PLATFORM_SPECIFIC_SOURCES
        ../thirdparty/incbin/incbin.h
        ../thirdparty/incbin/incbin.c
    )
endif()

set(SHADERS convert.comp)

set(ALL_SHADERS ${SHADERS})
foreach(SHADER ${ALL_SHADERS})
    set(RELATIVE_SHADER_PATH "shaders/${SHADER}")
    list(APPEND RELATIVE_SHADER_PATHS ${RELATIVE_SHADER_PATH})
endforeach()
source_group("Shaders" FILES ${RELATIVE_SHADER_PATHS})

if(APPLE)
    set(RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${RELATIVE_SHADER_PATH}")
elseif(LINUX)
    set(RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${RELATIVE_SHADER_PATH}")
    add_compile_options("-Wa,-I${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")
endif()

add_library(${SUBPROJECT_NAME} SHARED
    ${HEADERS}
    ${SOURCE}
    ${PLATFORM_SPECIFIC_SOURCES}
    ${RESOURCE_FILES}
    ${RELATIVE_SHADER_PATHS}
)
set_target_properties(${SUBPROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

include_directories(include)

include(${CMAKE_CURRENT_SOURCE_DIR}/../CXXUtils.cmake)
pw_configure_cxx(${SUBPROJECT_NAME})

# Set up DLL export macro
add_compile_definitions(PW_IS_LIB)

# Find Vulkan SDK
add_compile_definitions(VULKAN_HPP_NO_EXCEPTIONS)
add_compile_definitions(VMA_VULKAN_VERSION=1002000)
add_compile_definitions(VMA_STATIC_VULKAN_FUNCTIONS=0)
add_compile_definitions(VMA_DYNAMIC_VULKAN_FUNCTIONS=1)
find_package(Vulkan REQUIRED)
target_link_libraries(${SUBPROJECT_NAME} PRIVATE Vulkan::Vulkan)
include_directories(${Vulkan_INCLUDE_DIRS})

# Find and link VMA
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::vulkan-memory-allocator::vulkan-memory-allocator)

# Add libshaderc dependency
find_package(unofficial-shaderc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::shaderc::shaderc)

# Find and link GLM
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# Set up macOS framework
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
    target_link_libraries(${SUBPROJECT_NAME} PRIVATE "-framework CoreFoundation")
    add_compile_definitions(PW_BUNDLE_ID="${PW_BUNDLE_ID}")
elseif(LINUX)
    install(TARGETS ${SUBPROJECT_NAME} LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/PixelWeave)
endif()
