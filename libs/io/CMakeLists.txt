project(libwolv-io)

# Add library
set(SOURCE
        source/io/file.cpp
        $<$<PLATFORM_ID:Windows>:source/io/file_win.cpp>
        $<$<NOT:$<PLATFORM_ID:Windows>>:source/io/file_unix.cpp>
        source/io/fs.cpp
)

if (APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)

    set(OSX_11_0_SDK_PATH  /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.0.sdk)
    if (NOT CMAKE_OSX_SYSROOT)
        if (IS_DIRECTORY ${OSX_11_0_SDK_PATH})
            set(CMAKE_OSX_SYSROOT ${OSX_11_0_SDK_PATH})
        else ()
            message(WARNING "CMAKE_OSX_SYSROOT not set and macOS 10.9 SDK not found! Using default one.")
        endif ()
    endif ()

    set(SOURCE ${SOURCE} source/io/fs_macos.m)
endif ()

add_library(${PROJECT_NAME} STATIC ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC wolv::types wolv::utils)

if (APPLE)
    find_library(FOUNDATION NAMES Foundation)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${FOUNDATION})
elseif (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /EHsc)
endif()

string(REPLACE "libwolv-" "" PROJECT_NAME_SPACE ${PROJECT_NAME})
add_library(wolv::${PROJECT_NAME_SPACE} ALIAS ${PROJECT_NAME})

target_link_libraries(libwolv INTERFACE ${PROJECT_NAME})

# Add example project
add_executable(${PROJECT_NAME}-test EXCLUDE_FROM_ALL example/main.cpp)
target_include_directories(${PROJECT_NAME}-test PRIVATE include)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME})