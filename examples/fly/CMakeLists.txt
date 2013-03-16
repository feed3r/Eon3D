
add_executable(example_fly example_fly.c)

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/core)
include_directories(${eon3d_SOURCE_DIR}/modules)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit)
include_directories(${eon3d_SOURCE_DIR}/deps/cxkit/kits)

link_directories(${eon3d_BINARY_DIR}/core)
link_directories(${eon3d_BINARY_DIR}/deps/cxkit)

target_link_libraries(example_fly eon3d)
target_link_libraries(example_fly eon3dx)
target_link_libraries(example_fly eon3dx_console)
target_link_libraries(example_fly eon3dx_reader)
target_link_libraries(example_fly eon3d_kit)
target_link_libraries(example_fly m)


