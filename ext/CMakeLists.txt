
set(LIBEON3DX_SOURCES eon3dx.c)

add_definitions(-DHAVE_CONFIG_H) # always

add_library(eon3dx_s STATIC ${LIBEON3DX_SOURCES})
add_library(eon3dx SHARED ${LIBEON3DX_SOURCES})

find_package(SDL)
if(SDL_FOUND)
    include_directories(${SDL_INCLUDE_DIR})
    target_link_libraries(eon3dx_s ${SDL_LIBRARY})
    target_link_libraries(eon3dx ${SDL_LIBRARY})
else(SDL_FOUND)
    message(WARNING, "libSDL not found (get it at http://www.libsdl.org)")
endif(SDL_FOUND)


include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/ext)



