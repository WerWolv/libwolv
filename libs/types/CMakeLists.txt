project(libwolv-types)

# Add library
add_library(${CMAKE_PROJECT_NAME} INTERFACE)
target_include_directories(${CMAKE_PROJECT_NAME} INTERFACE include)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")

if (WIN32)
    target_compile_definitions(${CMAKE_PROJECT_NAME} INTERFACE OS_WINDOWS)
elseif (UNIX AND NOT APPLE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} INTERFACE OS_LINUX)
elseif (APPLE)
    target_compile_definitions(${CMAKE_PROJECT_NAME} INTERFACE OS_MACOS)
endif ()

# Add example project
add_executable(${CMAKE_PROJECT_NAME}-test example/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${CMAKE_PROJECT_NAME}-test PRIVATE ${CMAKE_PROJECT_NAME})