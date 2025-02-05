project(libwolv-utils)

# Add library
add_library(${PROJECT_NAME} STATIC
        source/utils/string.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC wolv::types)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

string(REPLACE "libwolv-" "" PROJECT_NAME_SPACE ${PROJECT_NAME})
add_library(wolv::${PROJECT_NAME_SPACE} ALIAS ${PROJECT_NAME})

target_link_libraries(libwolv INTERFACE ${PROJECT_NAME})

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
endif()

# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})