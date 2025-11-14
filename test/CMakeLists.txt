set(TARGET_NAME "Tests")

include_directories(../lib/include)

set(HEADERS)
set(SOURCES main.cpp)

add_executable(${TARGET_NAME} ${HEADERS} ${SOURCES})

include(${CMAKE_CURRENT_SOURCE_DIR}/../CXXUtils.cmake)
pw_configure_target(${TARGET_NAME})

add_dependencies(${TARGET_NAME} Pixelweave)
target_link_libraries(${TARGET_NAME} Pixelweave)
