cmake_minimum_required(VERSION 3.17)
project(erutils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For my LSP
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g3")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message("-- Detected build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Enable link-time optimizations
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(erutils
    src/main.cpp
    src/savefile.cpp
    src/util.cpp
    src/items.cpp
)

target_compile_options(erutils PRIVATE
    -Wall
    -Weverything
    -Wno-weak-vtables
    -Wno-padded
    -Wno-shadow-field
    -Wno-shadow-field-in-constructor
    -Wno-sign-conversion
    -Wno-c++98-compat
    -Wno-c++98-compat-extra-semi
)

if (VERSION)
    add_definitions(-DVERSION="${VERSION}")
endif()

find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)
message("-- Found fmt: ${fmt_DIR}")

target_link_libraries(erutils PRIVATE
    OpenSSL::Crypto
    fmt::fmt
)

install(TARGETS erutils DESTINATION bin)
