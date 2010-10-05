
add_executable(ex01_cube ex01_cube.c)

include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_BINARY_DIR}/core)

link_directories(${eon3d_BINARY_DIR}/core)
target_link_libraries(ex01_cube eon3d)
target_link_libraries(ex01_cube eon3dx)



