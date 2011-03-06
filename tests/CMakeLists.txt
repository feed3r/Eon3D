
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

    add_executable(check_template check_template.c)
    target_link_libraries(check_template check)
    target_link_libraries(check_template eon3d)
#    target_link_libraries(check_template m)
    
    add_executable(check_rgb check_rgb.c)
    target_link_libraries(check_rgb check)
    target_link_libraries(check_rgb eon3d)
#    target_link_libraries(check_rgb m)
endif(ENABLE_TESTS)

