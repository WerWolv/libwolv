// We are still testing wolv::io::File, but in this file we target code coverage for file_unix.cpp and file_win.cpp in particular

#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>
#include <wolv/io/file.hpp>
#include <helper.hpp>

#include <iostream>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;


TEST_SEQUENCE("EmptyFileTracker") {
    wolv::io::File file;
    auto changeTracker = wolv::io::ChangeTracker(file);
    changeTracker.startTracking([]{});
    TEST_SUCCESS();
};

TEST_SEQUENCE("FileTracker") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    wolv::io::File file(filePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());

    auto changeTracker = wolv::io::ChangeTracker(file);
    
    bool hasChanged = false;
    changeTracker.startTracking([&hasChanged]{
        std::cerr << "File has changed" << std::endl;
        hasChanged = true;
    });

    // sleep for a bit to let the change tracker start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    file.writeString("hello");
    file.close();

    // sleep 1s because tracker on Windows checks every second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    TEST_ASSERT(hasChanged);
    TEST_SUCCESS();
};

TEST_SEQUENCE("CloneFileTracker") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    wolv::io::File file(filePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());

    auto changeTracker = wolv::io::ChangeTracker(file);
    auto changeTracker2 = std::move(changeTracker);
    wolv::io::ChangeTracker changeTracker3;
    changeTracker3 = std::move(changeTracker2);
    
    bool hasChanged = false;
    changeTracker3.startTracking([&hasChanged]{
        std::cerr << "File has changed" << std::endl;
        hasChanged = true;
    });

    // see test above
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    file.writeString("hello");
    file.flush();
    file.close();

    // see test above
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    TEST_ASSERT(hasChanged);
    TEST_SUCCESS();
};
