
set(LIBEON3D_SOURCES eon3d.c)
set(LIBEON3DX_SOURCES eon3dx.c)

add_definitions(-DHAVE_CONFIG_H) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)

add_library(eon3d STATIC ${LIBEON3D_SOURCES})

add_library(eon3dx STATIC ${LIBEON3DX_SOURCES})


