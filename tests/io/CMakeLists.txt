cmake_minimum_required(VERSION 3.16)

project(libwolv-io_tests)
set(TEST_CATEGORY IO)

# Add new tests here #
set(AVAILABLE_TESTS
    BasicFileAccess
    FileVectorOps
    FileStringOps
    FileClone
    FileAtomicStringOps
    FileAtomicVectorOps
    FileSize

    FileMove
    FileHandle
    FileInfo

    EmptyFileTracker
    FileTracker
    CloneFileTracker

    FsGetExecPath
    FsToNormalizedPath

    BufferedReader
)

add_executable(${PROJECT_NAME}
        source/file.cpp
        source/file_impl.cpp
        source/file_tracker.cpp
        source/fs.cpp
        source/helper.cpp
        source/buffered_reader.cpp
)

# ---- No need to change anything from here downwards unless you know what you're doing ---- #

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE wolv::io wolv::testing ${FMT_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

foreach (test IN LISTS AVAILABLE_TESTS)
    add_test(NAME "${TEST_CATEGORY}/${test}" COMMAND ${PROJECT_NAME} "${test}" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endforeach ()
add_dependencies(libwolv-tests ${PROJECT_NAME})
