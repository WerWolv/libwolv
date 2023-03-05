project(libwolv-utils)

# Add library
add_library(libwolv-utils STATIC
        source/utils/string.cpp
)
target_include_directories(libwolv-utils PUBLIC include)
target_link_libraries(libwolv-utils PUBLIC libwolv-types)

# Add example project
add_executable(libwolv-utils-test example/main.cpp)
target_include_directories(libwolv-utils-test PRIVATE example)
target_link_libraries(libwolv-utils-test PRIVATE libwolv-utils)