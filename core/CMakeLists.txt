
set(LIBEON3D_SOURCES eon3d.c)

add_definitions(-DHAVE_CONFIG_H) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)

add_library(eon3d_s STATIC ${LIBEON3D_SOURCES})
add_library(eon3d SHARED ${LIBEON3D_SOURCES})

target_link_libraries(eon3d_s m)
target_link_libraries(eon3d m)

