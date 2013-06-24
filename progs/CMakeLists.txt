
set(EON3D_LIBS eon3d eon3dx eon3dx_console eon3dx_reader)

add_executable(benchmark benchmark.c)
add_executable(eonview eonview.c)
add_executable(landgen landgen.c)

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/modules)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

link_directories(${eon3d_BINARY_DIR}/core)

target_link_libraries(benchmark ${EON3D_LIBS})
target_link_libraries(eonview ${EON3D_LIBS})
target_link_libraries(landgen ${EON3D_LIBS})

