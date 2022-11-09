cmake_minimum_required(VERSION 3.19.2 FATAL_ERROR)

function(pw_configure_cxx SUBPROJECT_NAME)

    # Configure C++ compiler
    set_property(TARGET ${SUBPROJECT_NAME} PROPERTY CXX_STANDARD 17)
    set_property(TARGET ${SUBPROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

    # Make compiler extra pedantic for now
    if(MSVC)
        target_compile_options(${SUBPROJECT_NAME} PRIVATE /W4 /WX)
    else()
        # Disable Werror for now, since I didn't find a way having the compiler ignore warnings in vulkan headers
        #target_compile_options(${SUBPROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    # Disable exceptions
    if(MSVC)
        string(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    else()
        string(REGEX REPLACE "-fexceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    endif()

    if(MSVC)
        add_compile_definitions(PW_PLATFORM_WINDOWS)
    else()
        add_compile_definitions(PW_PLATFORM_MACOS)
    endif()

endfunction()
