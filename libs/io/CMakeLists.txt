project(libwolv-io)

# Add library
set(SOURCE
        source/io/file.cpp
        source/io/fs.cpp
)

if (APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
    set(SOURCE ${SOURCE} source/io/fs_macos.m)
endif ()

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC libwolv-types libwolv-utils)


# Add example project
add_executable(${PROJECT_NAME}-test example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE include)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})