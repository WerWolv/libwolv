project(libwolv-net)

# Add library
add_library(${PROJECT_NAME} STATIC
        source/net/socket_client.cpp
        source/net/socket_server.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC libwolv-types libwolv-utils)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32.lib)
endif ()

# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE example)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})