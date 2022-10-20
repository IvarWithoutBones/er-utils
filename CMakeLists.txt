cmake_minimum_required(VERSION 3.17)
set(PROJECT "erutils")

project(${PROJECT} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For my LSP
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g3")
set(COMMON_COMPILE_OPTIONS
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

# Default to a release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message("-- Detected build type: ${CMAKE_BUILD_TYPE}")

# Enable link-time optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)

# Codegen for items
add_executable(codegen src/codegen/itemparser.cpp)
target_link_libraries(codegen PRIVATE fmt::fmt)
target_compile_options(codegen PRIVATE ${COMMON_COMPILE_OPTIONS})
add_custom_command(
    COMMENT "Generating generateditems.h"
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/codegen > ${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/generateditems.h
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/generateditems.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS codegen
)

# Main
add_executable(${PROJECT}
    src/main.cpp
    src/util.cpp
    src/savefile/savefile.cpp
    src/savefile/items.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/codegen/generateditems.h
)

if (VERSION)
    add_definitions(-DVERSION="${VERSION}")
endif()

target_link_libraries(${PROJECT}
    PRIVATE OpenSSL::Crypto
    PRIVATE fmt::fmt
)

target_compile_options(${PROJECT} PRIVATE ${COMMON_COMPILE_OPTIONS})
install(TARGETS ${PROJECT} DESTINATION bin)
