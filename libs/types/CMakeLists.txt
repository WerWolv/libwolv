project(libwolv-types)

# Add library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (CMAKE_CXX_COMPILER MATCHES "\\/em\\+\\+(-[a-zA-Z0-9.])?$")
    target_compile_definitions(${PROJECT_NAME} INTERFACE OS_WEB)
elseif (WIN32)
    target_compile_definitions(${PROJECT_NAME} INTERFACE OS_WINDOWS)
elseif (UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} INTERFACE OS_LINUX)
elseif (APPLE)
    target_compile_definitions(${PROJECT_NAME} INTERFACE OS_MACOS)
endif ()

string(REPLACE "libwolv-" "" PROJECT_NAME_SPACE ${PROJECT_NAME})
add_library(wolv::${PROJECT_NAME_SPACE} ALIAS ${PROJECT_NAME})

target_link_libraries(libwolv INTERFACE ${PROJECT_NAME})

# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})