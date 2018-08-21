
add_library(netcache STATIC cache.cpp cache.h)

target_link_libraries(netcache pthread)