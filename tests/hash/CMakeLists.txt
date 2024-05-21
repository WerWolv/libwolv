cmake_minimum_required(VERSION 3.16)

project(libwolv-hash_tests)
set(TEST_CATEGORY HASH)

# Add new tests here #
set(AVAILABLE_TESTS
    UUID
    CRC
    CRC_Reflect
)

add_executable(${PROJECT_NAME}
        source/uuid.cpp
        source/crc.cpp
)

# ---- No need to change anything from here downwards unless you know what you're doing ---- #

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE wolv::hash wolv::testing ${FMT_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

foreach (test IN LISTS AVAILABLE_TESTS)
    add_test(NAME "${TEST_CATEGORY}/${test}" COMMAND ${PROJECT_NAME} "${test}" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach ()
add_dependencies(libwolv-tests ${PROJECT_NAME})
