// We are still testing wolv::io::File, but in this file we target code coverage for file_unix.cpp and file_win.cpp in particular

#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>
#include <wolv/io/file.hpp>

#include <helper.hpp>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;

TEST_SEQUENCE("FileMove") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    wolv::io::File file(filePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());

    wolv::io::File file2(std::move(file));
    TEST_ASSERT(file2.isValid());

    wolv::io::File file3;
    file3 = std::move(file2);
    TEST_ASSERT(file3.isValid());

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileHandle") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.isValid());
        file.writeString(FileContent);
    }

    wolv::io::File file(filePath, wolv::io::File::Mode::Read);
    auto f = file.getHandle();

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), f) != nullptr) {
        std::string str(buffer);
        TEST_ASSERT(str == FileContent);
    } else {
        perror("Error reading file");
        TEST_FAIL();
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileInfo") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    wolv::io::File file(filePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());
    file.writeString(FileContent);
    TEST_ASSERT(file.flush());

    auto infos = *file.getFileInfo();
    TEST_ASSERT(infos.st_size == 11);

    TEST_SUCCESS();
};
