project(eon3d)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake/")

include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckFunctionExists)

include(FindSDL)
include(FindPNG)

set(CMAKE_REQUIRED_INCLUDES 	stdlib.h)
set(CMAKE_REQUIRED_INCLUDES 	stdint.h)
set(CMAKE_REQUIRED_INCLUDES 	inttypes.h)

option(ENABLE_PROFILE "enable the gprof instrumentation" OFF)

if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-g -O2 -Wall -fopenmp)
    if(ENABLE_PROFILE)
         message(STATUS "Enabling the profiling instrumentation")
         add_definitions(-pg)
         set(CMAKE_EXE_LINKER_FLAGS -pg)
    endif(ENABLE_PROFILE)
endif(CMAKE_COMPILER_IS_GNUCC)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

add_subdirectory(core)
add_subdirectory(modules)
add_subdirectory(examples)
add_subdirectory(progs)
#add_subdirectory(tests)

