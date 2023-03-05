project(libwolv-io)

# Add library
add_library(libwolv-io STATIC
        source/io/file.cpp
        source/io/fs.cpp
)
target_include_directories(libwolv-io PUBLIC include)
target_link_libraries(libwolv-io PUBLIC libwolv-types libwolv-utils)


# Add example project
add_executable(libwolv-io-test example/main.cpp)
target_include_directories(libwolv-io-test PRIVATE include)
target_link_libraries(libwolv-io-test PRIVATE libwolv-io)