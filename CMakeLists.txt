cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

cmake_policy(SET CMP0126 NEW)

set(VERSION 0.0.1)
set(PROJECT_NAME PixelWeave)

if(UNIX AND NOT APPLE)
    set(LINUX YES)
endif()

# Must be set before invoking project()
set(CMAKE_OSX_ARCHITECTURES x86_64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

# Add library subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Add test subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

# Configure 'Tests' as startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Tests)
