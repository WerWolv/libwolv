// We are still testing wolv::io::File, but in this file we target code coverage for file_unix.cpp and file_win.cpp in particular

#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>
#include <wolv/io/file.hpp>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;

const auto FilePath    = std::fs::current_path() / "file.txt";
const auto FileContent = "Hello World";

TEST_SEQUENCE("EmptyFileTracker") {
    wolv::io::File file;
    auto changeTracker = wolv::io::ChangeTracker(file);
    changeTracker.startTracking([]{});
    TEST_SUCCESS();
};

TEST_SEQUENCE("FileTracker") {
    wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());

    auto changeTracker = wolv::io::ChangeTracker(file);
    
    bool hasChanged = false;
    changeTracker.startTracking([&hasChanged]{
        hasChanged = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    file.writeString("hello");
    file.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    TEST_ASSERT(hasChanged);
    TEST_SUCCESS();
};

TEST_SEQUENCE("CloneFileTracker") {
    wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());

    auto changeTracker = wolv::io::ChangeTracker(file);
    auto changeTracker2 = std::move(changeTracker);
    wolv::io::ChangeTracker changeTracker3;
    changeTracker3 = std::move(changeTracker2);
    
    bool hasChanged = false;
    changeTracker3.startTracking([&hasChanged]{
        hasChanged = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    file.writeString("hello");
    file.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    TEST_ASSERT(hasChanged);
    TEST_SUCCESS();
};
