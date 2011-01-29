
set(LIBRPLY_SOURCES rply.c)

add_definitions(-DHAVE_CONFIG_H) # always

include_directories(${eon3d_BINARY_DIR})
include_directories(${eon3d_SOURCE_DIR})
include_directories(${eon3d_SOURCE_DIR}/rply)

add_library(rply_s STATIC ${LIBRPLY_SOURCES})
add_library(rply SHARED ${LIBRPLY_SOURCES})

