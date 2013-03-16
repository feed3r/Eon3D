
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
link_directories(${eon3d_BINARY_DIR}/deps/cxkit)

target_link_libraries(example00_load eon3d)
target_link_libraries(example00_load eon3dx)
target_link_libraries(example00_load eon3dx_console)
target_link_libraries(example00_load eon3dx_reader)
target_link_libraries(example00_load eon3d_kit)
target_link_libraries(example00_load m)

target_link_libraries(example01_cube eon3d)
target_link_libraries(example01_cube eon3dx)
target_link_libraries(example01_cube eon3dx_console)
target_link_libraries(example01_cube eon3d_kit)
target_link_libraries(example01_cube m)

target_link_libraries(example02_cube_torus eon3d)
target_link_libraries(example02_cube_torus eon3dx)
target_link_libraries(example02_cube_torus eon3dx_console)
target_link_libraries(example02_cube_torus eon3d_kit)
target_link_libraries(example02_cube_torus m)

add_subdirectory(fly)

