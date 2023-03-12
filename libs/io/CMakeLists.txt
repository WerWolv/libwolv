project(libwolv-io)

# Add library
add_library(${PROJECT_NAME} STATIC
        source/io/file.cpp
        source/io/fs.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC libwolv-types libwolv-utils)


# Add example project
add_executable(${PROJECT_NAME}-test example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE include)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})