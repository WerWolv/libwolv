project(libwolv-template)

# Add library
add_library(${CMAKE_PROJECT_NAME} STATIC
        source/test.cpp
)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")


# Add example project
add_executable(${CMAKE_PROJECT_NAME}-test example/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${CMAKE_PROJECT_NAME}-test PRIVATE ${CMAKE_PROJECT_NAME})