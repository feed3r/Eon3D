
set(LIBEON3D_DBG_SOURCES ${eon3d_SOURCE_DIR}/core/eon3d.c)

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

    link_directories(${eon3d_BINARY_DIR}/core)
    link_directories(${eon3d_BINARY_DIR}/tests)

    # we need a custom debug build

    add_definitions(-DHAVE_CONFIG_H) # always
    add_definitions(-DEON_PRIVATE=extern) # always

    add_library(eon3d_dbg STATIC ${LIBEON3D_DBG_SOURCES})
    target_link_libraries(eon3d_dbg m)

    add_executable(check_template check_template.c)
    target_link_libraries(check_template check)
    target_link_libraries(check_template eon3d_dbg)
    
    add_executable(check_rgb check_rgb.c)
    target_link_libraries(check_rgb check)
    target_link_libraries(check_rgb eon3d_dbg)
endif(ENABLE_TESTS)

