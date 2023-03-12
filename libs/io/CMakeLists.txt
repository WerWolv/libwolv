project(libwolv-io)

# Add library
add_library(${CMAKE_PROJECT_NAME} STATIC
        source/io/file.cpp
        source/io/fs.cpp
)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC libwolv-types libwolv-utils)


# Add example project
add_executable(${CMAKE_PROJECT_NAME}-test example/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME}-test PRIVATE include)
target_link_libraries(${CMAKE_PROJECT_NAME}-test PRIVATE ${CMAKE_PROJECT_NAME})