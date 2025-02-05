project(libwolv-template)

# Add library
add_library(${PROJECT_NAME} STATIC
        source/test.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

string(REPLACE "libwolv-" "" PROJECT_NAME_SPACE ${PROJECT_NAME})
add_library(wolv::${PROJECT_NAME_SPACE} ALIAS ${PROJECT_NAME})

target_link_libraries(libwolv INTERFACE ${PROJECT_NAME})

# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})