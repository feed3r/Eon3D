
set(LIBEON3D_SOURCES eon3d.c)
set(LIBEON3DX_SOURCES eon3dx.c)
set(LIBEON3D_KIT_SOURCES
    ${eon3d_SOURCE_DIR}/deps/cxkit/kits/memorykit.c
    ${eon3d_SOURCE_DIR}/deps/cxkit/kits/arraykit.c
    ${eon3d_SOURCE_DIR}/deps/cxkit/kits/logkit.c)

add_definitions(-DHAVE_CONFIG_H) # always
add_definitions(-DEON_PRIVATE=static) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

add_library(eon3d_s STATIC ${LIBEON3D_SOURCES} ${LIBEON3D_KIT_SOURCES})
add_library(eon3d SHARED ${LIBEON3D_SOURCES} ${LIBEON3D_KIT_SOURCES})

add_library(eon3dx_s STATIC ${LIBEON3DX_SOURCES})
add_library(eon3dx SHARED ${LIBEON3DX_SOURCES})

set(LIBEON3D_LIBS m)
if(ENABLE_OPENMP)
	set(LIBEON3D_LIBS ${LIBEON3D_LIBS} gomp)
endif(ENABLE_OPENMP)
target_link_libraries(eon3d_s ${LIBEON3D_LIBS})
target_link_libraries(eon3d ${LIBEON3D_LIBS})

target_link_libraries(eon3dx_s eon3d ${LIBEON3D_LIBS})
target_link_libraries(eon3dx eon3d ${LIBEON3D_LIBS})

