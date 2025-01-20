cmake_minimum_required(VERSION 3.20)
project(libwolv)

if (TARGET wolv::types)
    return ()
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(libwolv INTERFACE)

add_subdirectory(libs/types EXCLUDE_FROM_ALL)
add_subdirectory(libs/utils EXCLUDE_FROM_ALL)

add_subdirectory(libs/io EXCLUDE_FROM_ALL)
add_subdirectory(libs/hash EXCLUDE_FROM_ALL)
add_subdirectory(libs/net EXCLUDE_FROM_ALL)
add_subdirectory(libs/containers EXCLUDE_FROM_ALL)
add_subdirectory(libs/math_eval EXCLUDE_FROM_ALL)
add_subdirectory(libs/testing EXCLUDE_FROM_ALL)

if (LIBWOLV_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif ()

