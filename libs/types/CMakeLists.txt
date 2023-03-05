project(libwolv-types)

# Add library
add_library(libwolv-types INTERFACE)
target_include_directories(libwolv-types INTERFACE include)

if (WIN32)
    target_compile_definitions(libwolv-types INTERFACE OS_WINDOWS)
elseif (UNIX AND NOT APPLE)
    target_compile_definitions(libwolv-types INTERFACE OS_LINUX)
elseif (APPLE)
    target_compile_definitions(libwolv-types INTERFACE OS_MACOS)
endif ()

# Add example project
add_executable(libwolv-template-test example/main.cpp)
target_include_directories(libwolv-template-test PRIVATE example)
target_link_libraries(libwolv-template-test PRIVATE libwolv-types)