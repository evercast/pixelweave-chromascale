set(SUBPROJECT_NAME "Tests")

include_directories(
    ../lib/include
)

# Attach source files to build
set(HEADERS)

set(SOURCES main.cpp)

add_executable(${SUBPROJECT_NAME} ${HEADERS} ${SOURCES})

include(${CMAKE_CURRENT_SOURCE_DIR}/../CXXUtils.cmake)
pw_configure_cxx(${SUBPROJECT_NAME})

add_dependencies(${SUBPROJECT_NAME} PixelWeave)
target_link_libraries(${SUBPROJECT_NAME} PixelWeave)
