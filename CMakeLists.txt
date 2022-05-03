cmake_minimum_required(VERSION 3.20)
project(er-saveinstaller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("-- Detected build type: ${CMAKE_BUILD_TYPE}")

#if(CMAKE_BUILD_TYPE STREQUAL Debug)
#    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
#    add_definitions(DEBUG)
#endif()

# For my LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(er-saveinstaller
    src/main.cpp
)

target_compile_options(er-saveinstaller PRIVATE
    -Wall
    -Wno-reorder
    -Wno-missing-braces
    -Wno-unused-variable
    -Wno-dangling-else
)

find_package(fmt REQUIRED)

target_link_libraries(er-saveinstaller
    PRIVATE fmt::fmt
)

install(TARGETS er-saveinstaller DESTINATION bin)
