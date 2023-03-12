project(libwolv-utils)

# Add library
add_library(${PROJECT_NAME} STATIC
        source/utils/string.cpp
        source/utils/socket.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC libwolv-types)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})