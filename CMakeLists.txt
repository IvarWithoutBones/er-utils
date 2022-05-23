cmake_minimum_required(VERSION 3.17)
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

find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
message("-- Found fmt: ${fmt_DIR}")

target_link_libraries(er-savepatcher PRIVATE
    OpenSSL::Crypto
    fmt::fmt
)

install(TARGETS er-savepatcher DESTINATION bin)
