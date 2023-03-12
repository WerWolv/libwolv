project(libwolv-utils)

# Add library
add_library(${CMAKE_PROJECT_NAME} STATIC
        source/utils/string.cpp
        source/utils/socket.cpp
)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC libwolv-types)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")

# Add example project
add_executable(${CMAKE_PROJECT_NAME}-test example/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${CMAKE_PROJECT_NAME}-test PRIVATE ${CMAKE_PROJECT_NAME})