
set(EON3D_LIBS eon3d eon3dx eon3dx_console eon3dx_reader m gomp)

add_executable(example_fly example_fly.c)

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/modules)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

link_directories(${eon3d_BINARY_DIR}/core)

target_link_libraries(example_fly ${EON3D_LIBS})

