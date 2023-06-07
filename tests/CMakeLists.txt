project(libwolv-tests)

add_custom_target(libwolv-tests DEPENDS libwolv-io-tests)

add_subdirectory(io)
