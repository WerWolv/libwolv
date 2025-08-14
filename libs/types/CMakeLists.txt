project(libwolv-types)

# Add library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# Heuristic for detecting if we should use built-in uint128 type

include(CheckTypeSize)
check_type_size("__uint128_t" UINT128_T_SIZE)
if (HAVE_UINT128_T_SIZE)
    set(LIBWOLV_BUILTIN_UINT128_DEFAULT ON)
else()
    set(LIBWOLV_BUILTIN_UINT128_DEFAULT OFF)
endif()

option(LIBWOLV_BUILTIN_UINT128 "Use built-in uint128 type" ${LIBWOLV_BUILTIN_UINT128_DEFAULT})

if (LIBWOLV_BUILTIN_UINT128)
    target_compile_definitions(${PROJECT_NAME} INTERFACE LIBWOLV_BUILTIN_UINT128)
endif ()

if (EMSCRIPTEN OR WASI)
    target_compile_definitions(${PROJECT_NAME} INTERFACE OS_WEB)
elseif (WIN32)
    target_compile_definitions(${PROJECT_NAME} INTERFACE OS_WINDOWS)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
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