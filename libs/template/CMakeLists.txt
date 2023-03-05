project(libwolv-template)

# Add library
add_library(libwolv-template STATIC
        source/test.cpp
)
target_include_directories(libwolv-template PUBLIC include)


# Add example project
add_executable(libwolv-template-test example/main.cpp)
target_include_directories(libwolv-template-test PRIVATE example)
target_link_libraries(libwolv-template-test PRIVATE libwolv-template)