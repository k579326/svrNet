
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(xtest test.c)

target_link_libraries(xtest SvrNet)