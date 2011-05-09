
set(LIBEON3D_SOURCES eon3d.c)

add_definitions(-DHAVE_CONFIG_H) # always
add_definitions(-DEON_PRIVATE=static) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

add_library(eon3d_s STATIC ${LIBEON3D_SOURCES})
add_library(eon3d SHARED ${LIBEON3D_SOURCES})

target_link_libraries(eon3d_s eon3d_kit_s m)
target_link_libraries(eon3d eon3d_kit m)

