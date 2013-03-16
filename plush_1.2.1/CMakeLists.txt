
option(ENABLE_PLUSH "compile the plush library" OFF)

if(ENABLE_PLUSH)
    set(LIBPLUSH_SOURCES 
	    cam.c clip.c light.c make.c mat.c math.c obj.c
    	pf_ptex.c pf_solid.c pf_tex.c pf_trans.c plush.c read_3ds.c
	    read_cob.c read_jaw.c read_pcx.c render.c spline.c text.c)
    add_definitions(-DHAVE_CONFIG_H) # always

    include_directories(${eon3d_BINARY_DIR})
    include_directories(${eon3d_SOURCE_DIR})
    include_directories(${eon3d_SOURCE_DIR}/render)

    add_library(plush_s STATIC ${LIBPLUSH_SOURCES})
    add_library(plush SHARED ${LIBPLUSH_SOURCES})

    target_link_libraries(plush_s m)
    target_link_libraries(plush m)
endif(ENABLE_PLUSH)

