set(SUBPROJECT_NAME "Tests")

include_directories(
	../lib/include
)

# Attach source files to build
set(HEADERS)

set(SOURCES main.cpp)

add_executable(${SUBPROJECT_NAME} ${HEADERS} ${SOURCES})

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

add_dependencies(${SUBPROJECT_NAME} PixelWeave)
target_link_libraries(${SUBPROJECT_NAME} PixelWeave)