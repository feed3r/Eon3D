
set(LIBEON3DX_CONSOLE_SOURCES eon3dx_console.c)
set(LIBEON3DX_READER_SOURCES eon3dx_reader.c)
set(SAVEPNG_SOURCES ${eon3d_SOURCE_DIR}/deps/SDL_SavePNG/savepng.c)
set(LIBRPLY_SOURCES ${eon3d_SOURCE_DIR}/deps/rply-1.1.2/rply.c)
set(LIBEON3D_KIT_SOURCES
    ${eon3d_SOURCE_DIR}/deps/cxkit/kits/stringkit.c)

add_definitions(-DHAVE_CONFIG_H) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/modules)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)
include_directories(${eon3d_SOURCE_DIR}/deps/rply-1.1.2)
include_directories(${eon3d_SOURCE_DIR}/deps/SDL_SavePNG)

add_library(eon3dx_console_s STATIC ${LIBEON3DX_CONSOLE_SOURCES}
                                    ${SAVEPNG_SOURCES}
				    ${LIBEON3D_KIT_SOURCES})
add_library(eon3dx_console SHARED ${LIBEON3DX_CONSOLE_SOURCES}
                                  ${SAVEPNG_SOURCES}
				  ${LIBEON3D_KIT_SOURCES})

add_library(eon3dx_reader_s STATIC ${LIBEON3DX_READER_SOURCES} ${LIBRPLY_SOURCES})
add_library(eon3dx_reader SHARED ${LIBEON3DX_READER_SOURCES} ${LIBRPLY_SOURCES})

find_package(SDL)
if(SDL_FOUND)
    include_directories(${SDL_INCLUDE_DIR} ${PNG_INCLUDE_DIR})
    target_link_libraries(eon3dx_console_s ${SDL_LIBRARY} ${PNG_LIBRARIES})
    target_link_libraries(eon3dx_console ${SDL_LIBRARY} ${PNG_LIBRARIES})
else(SDL_FOUND)
    message(WARNING, "libSDL not found (get it at http://www.libsdl.org)")
endif(SDL_FOUND)

target_link_libraries(eon3dx_console_s eon3d)
target_link_libraries(eon3dx_console eon3d)

target_link_libraries(eon3dx_reader_s eon3d)
target_link_libraries(eon3dx_reader eon3d)

