cmake_minimum_required(VERSION 3.20)
project(er-savepatcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message("-- Detected build type: ${CMAKE_BUILD_TYPE}")

# For my LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(er-savepatcher
    src/main.cpp
    src/savefile.cpp
)

target_compile_options(er-savepatcher PRIVATE
    -Wall
    -Wno-reorder
    -Wno-missing-braces
    -Wno-unused-variable
    -Wno-dangling-else
)

find_package(fmt REQUIRED)

target_link_libraries(er-savepatcher
    PRIVATE fmt::fmt
)

install(TARGETS er-savepatcher DESTINATION bin)
