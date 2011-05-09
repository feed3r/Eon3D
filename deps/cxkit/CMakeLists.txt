
set(LIBEON3D_KIT_SOURCES kits/memorykit.c kits/arraykit.c)

add_definitions(-DHAVE_CONFIG_H) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

add_library(eon3d_kit_s STATIC ${LIBEON3D_KIT_SOURCES})
add_library(eon3d_kit SHARED ${LIBEON3D_KIT_SOURCES})

