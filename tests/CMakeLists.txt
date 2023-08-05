project(libwolv-tests)

add_custom_target(libwolv-tests DEPENDS libwolv-io-tests libwolv-common-tests)

add_subdirectory(io)
add_subdirectory(hash)
add_subdirectory(utils)
add_subdirectory(common)
