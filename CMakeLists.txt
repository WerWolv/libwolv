project(libwolv)
cmake_minimum_required(VERSION 3.20)

if (TARGET libwolv-types)
    return ()
endif ()

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(libs/types)
add_subdirectory(libs/utils)

add_subdirectory(libs/io)
add_subdirectory(libs/hash)
add_subdirectory(libs/net)
add_subdirectory(libs/containers)
add_subdirectory(libs/testing)

enable_testing()
add_subdirectory(tests)
