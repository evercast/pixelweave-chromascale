cmake_minimum_required(VERSION 3.19.2 FATAL_ERROR)

set(VERSION 0.0.1)
set(PROJECT_NAME PixelWeave)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

set(CMAKE_OSX_ARCHITECTURES "x86_64")

# Add library subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

# Add test subproject
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

# Configure 'Tests' as startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 'Tests')
