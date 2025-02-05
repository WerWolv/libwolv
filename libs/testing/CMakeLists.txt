cmake_minimum_required(VERSION 3.16)

project(libwolv-testing)

add_library(${PROJECT_NAME} STATIC
        source/main.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC wolv::utils ${FMT_LIBRARIES})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

string(REPLACE "libwolv-" "" PROJECT_NAME_SPACE ${PROJECT_NAME})
add_library(wolv::${PROJECT_NAME_SPACE} ALIAS ${PROJECT_NAME})