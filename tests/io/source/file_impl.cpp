// We are still testing wolv::io::File, but in this file we target code coverage for file_unix.cpp and file_win.cpp in particular

#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>
#include <wolv/io/file.hpp>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;

const auto FilePath    = std::fs::current_path() / "file.txt";
const auto FileContent = "Hello World";

TEST_SEQUENCE("FileMove") {
    wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());

    wolv::io::File file2(std::move(file));
    TEST_ASSERT(file2.isValid());

    wolv::io::File file3;
    file3 = std::move(file2);
    TEST_ASSERT(file3.isValid());

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileHandle") {
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.isValid());
        file.writeString(FileContent);
    }

    wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
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
    wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
    TEST_ASSERT(file.isValid());
    file.writeString(FileContent);
    TEST_ASSERT(file.flush());

    auto infos = *file.getFileInfo();
    TEST_ASSERT(infos.st_size == 11);

    TEST_SUCCESS();
};
