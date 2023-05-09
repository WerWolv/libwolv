project(libwolv-containers)

# Add library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} INTERFACE libwolv-types)


# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})