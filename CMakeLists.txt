project(libwolv)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(libs/types)
add_subdirectory(libs/utils)

add_subdirectory(libs/io)