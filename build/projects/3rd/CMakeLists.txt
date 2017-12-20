j_project(3rd 3rd)

find_package(ZLIB)
add_library(zlib SHARED IMPORTED GLOBAL)
set_property(TARGET zlib PROPERTY IMPORTED_LOCATION ${ZLIB_LIBRARIES})  

find_package(Boost)
