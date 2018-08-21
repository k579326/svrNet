add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DHAVE_STDINT_H)
add_definitions(-D_REENTRANT)

include_directories(./)

# 判断操作系统
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(SYSTEM_WINDOWS ON)
    message("[libuv]SYSTEM_WINDOWS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message("[libuv]SYSTEM_LINUX")
    set(SYSTEM_LINUX ON)
endif()

if (SYSTEM_LINUX)
    include_directories(./unix-config)
elseif (SYSTEM_WINDOWS)
    include_directories(./win-config)
endif()

aux_source_directory(./ LIBXMLSRC)

add_library (libxml2 ${LIBXMLSRC})
set_target_properties(libxml2 PROPERTIES FOLDER "third")

if(SYSTEM_WINDOWS)
    target_link_libraries(libxml2 ws2_32)
endif()