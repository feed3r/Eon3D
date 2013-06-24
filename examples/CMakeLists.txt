
set(EON3D_LIBS eon3d eon3dx eon3dx_console)

add_executable(example00_load example00_load.c)
add_executable(example01_cube example01_cube.c)
add_executable(example02_cube_torus example02_cube_torus.c)

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/modules)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

link_directories(${eon3d_BINARY_DIR}/core)

target_link_libraries(example00_load ${EON3D_LIBS} eon3dx_reader)
target_link_libraries(example01_cube ${EON3D_LIBS})
target_link_libraries(example02_cube_torus ${EON3D_LIBS})

add_subdirectory(fly)

