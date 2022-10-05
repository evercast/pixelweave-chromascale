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
    src/ShaderResources.h
    src/ShaderResources.rc
    src/ResourceLoader.h
    src/ResourceLoader.cpp

    src/VulkanDevice.cpp
    src/Device.cpp
    src/VulkanInstance.cpp
    src/VulkanVideoConverter.cpp
    src/RefCountPtr.cpp
    src/PixelFormat.cpp
)

add_library(${SUBPROJECT_NAME} SHARED
    ${HEADERS}
    ${SOURCE}
)

include_directories(include)

# Configure C++ compiler
set_property(TARGET ${SUBPROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${SUBPROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Make compiler extra pedantic for now
if(MSVC)
    target_compile_options(${SUBPROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${SUBPROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Disable exceptions
string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

# Setup DLL export macro
add_compile_definitions(PW_IS_LIB)

# Find Vulkan SDK
add_compile_definitions(VULKAN_HPP_NO_EXCEPTIONS)
find_package(Vulkan REQUIRED)
target_link_libraries(${SUBPROJECT_NAME} ${Vulkan_LIBRARIES})
include_directories(${Vulkan_INCLUDE_DIRS})