cmake_minimum_required(VERSION 3.16)

project(libwolv-testing)

add_library(libwolv-testing STATIC
        source/main.cpp
)
target_include_directories(libwolv-testing PUBLIC include)
target_link_libraries(libwolv-testing PUBLIC libwolv-utils ${FMT_LIBRARIES})
