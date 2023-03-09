project(libwolv-template)

# Add library
add_library(libwolv-hash INTERFACE)
target_include_directories(libwolv-hash INTERFACE include)
target_link_libraries(libwolv-hash INTERFACE libwolv-types)


# Add example project
add_executable(libwolv-hash-test example/main.cpp)
target_include_directories(libwolv-hash-test PRIVATE example)
target_link_libraries(libwolv-hash-test PRIVATE libwolv-hash)