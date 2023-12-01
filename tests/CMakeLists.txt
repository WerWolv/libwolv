project(libwolv-tests)

add_custom_target(libwolv-tests DEPENDS libwolv-io_tests libwolv-common_tests)

add_subdirectory(io)
add_subdirectory(hash)
add_subdirectory(utils)
add_subdirectory(common)
