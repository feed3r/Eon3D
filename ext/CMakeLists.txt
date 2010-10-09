
set(LIBEON3DX_SOURCES eon3dx.c)

add_definitions(-DHAVE_CONFIG_H) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/ext)

add_library(eon3dx_s STATIC ${LIBEON3DX_SOURCES})
add_library(eon3dx SHARED ${LIBEON3DX_SOURCES})


