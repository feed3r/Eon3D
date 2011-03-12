
set(LIBEON3D_DBG_SOURCES ${eon3d_SOURCE_DIR}/core/eon3d.c)
set(BUILD_HEADER ${eon3d_SOURCE_DIR}/tests/build_eon3d_private_h.py)

option(ENABLE_TESTS "compile testsuite")

if(ENABLE_TESTS)
    find_package(LIBCHECK)
    if(LIBCHECK_NOTFOUND)
        message(FATAL_ERROR, "libcheck not found (get it at http://check.sf.net) - can't compile testsuite")
    endif(LIBCHECK_NOTFOUND)

    include_directories(${LIBCHECK_INCLUDE_DIR})

    include_directories(${eon3d_BINARY_DIR})
    include_directories(${eon3d_SOURCE_DIR})
    include_directories(${eon3d_SOURCE_DIR}/core)
    # for the custom header
    include_directories(${eon3d_BINARY_DIR}/tests)

    link_directories(${eon3d_BINARY_DIR}/core)
    # for the custom library
    link_directories(${eon3d_BINARY_DIR}/tests)

    # we need a custom debug build

    add_definitions(-DHAVE_CONFIG_H) # always
    add_definitions(-DEON_PRIVATE=extern) # always
    add_definitions(-DEON_DEBUG=1)

    add_library(eon3d_dbg STATIC ${LIBEON3D_DBG_SOURCES})
    target_link_libraries(eon3d_dbg m)

    # and we need a custom private header
    add_custom_command(TARGET eon3d_dbg
                       PRE_BUILD
                       COMMAND ${BUILD_HEADER} eon3d_private.h ${LIBEON3D_DBG_SOURCES}
                       WORKING_DIRECTORY ${eon3d_BINARY_DIR}/tests)

    add_executable(check_template check_template.c)
    target_link_libraries(check_template check)
    target_link_libraries(check_template eon3d_dbg)
    
    add_executable(check_rgb check_rgb.c)
    target_link_libraries(check_rgb check)
    target_link_libraries(check_rgb eon3d_dbg)

    add_executable(check_array check_array.c)
    target_link_libraries(check_array check)
    target_link_libraries(check_array eon3d_dbg)
endif(ENABLE_TESTS)

