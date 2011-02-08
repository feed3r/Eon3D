
set(LIBEON3DX_CONSOLE_SOURCES eon3dx_console.c)

add_definitions(-DHAVE_CONFIG_H) # always

add_library(eon3dx_console_s STATIC ${LIBEON3DX_CONSOLE_SOURCES})
add_library(eon3dx_console SHARED ${LIBEON3DX_CONSOLE_SOURCES})

find_package(SDL)
if(SDL_FOUND)
    include_directories(${SDL_INCLUDE_DIR})
    target_link_libraries(eon3dx_console_s ${SDL_LIBRARY})
    target_link_libraries(eon3dx_console ${SDL_LIBRARY})
else(SDL_FOUND)
    message(WARNING, "libSDL not found (get it at http://www.libsdl.org)")
endif(SDL_FOUND)


include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/modules)
include_directories(${eon3d_SOURCE_DIR}/deps/rply-1.01)


