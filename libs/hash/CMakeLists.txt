project(libwolv-hash)

# Add library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_link_libraries(${PROJECT_NAME} INTERFACE libwolv-types)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Add example project
add_executable(${PROJECT_NAME}-test example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})