project(eon3d)
cmake_minimum_required(VERSION 2.8)

include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckFunctionExists)

include(FindSDL)

set(CMAKE_REQUIRED_INCLUDES 	stdlib.h)
set(CMAKE_REQUIRED_INCLUDES 	stdint.h)
set(CMAKE_REQUIRED_INCLUDES 	inttypes.h)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(core)
add_subdirectory(ext)
add_subdirectory(examples)

