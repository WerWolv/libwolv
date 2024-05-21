#include <wolv/test/tests.hpp>

#include <wolv/io/file.hpp>
#include <fmt/core.h>
#include <iostream>

using namespace std::literals::string_literals;

const auto FilePath    = std::fs::current_path() / "file.txt";
const auto FileContent = "Hello World";

TEST_SEQUENCE("BasicFileAccess") {
    // create and write to file
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.isValid());

        file.writeString(FileContent);
    }

    // read file
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.isValid());

        TEST_ASSERT(file.readString() == FileContent);
    }

    // remove file
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Write);
        TEST_ASSERT(file.isValid());


        file.remove();
        TEST_ASSERT(!file.isValid());
    }

    // try to read it again
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        if (file.isValid())
            TEST_FAIL();
    }

    TEST_SUCCESS();
};

void writeTestFile() {
    wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
    file.writeString(FileContent);
}

TEST_SEQUENCE("FileClone") {
    writeTestFile();

     // read file using readString methods
    wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
    TEST_ASSERT(file.isValid());
    TEST_ASSERT(file.readString() == FileContent);

    auto file2 = file.clone();
    TEST_ASSERT(file2.isValid());
    TEST_ASSERT(file2.readString() == FileContent);

    // check if old file is still valid
    file.seek(0);
    TEST_ASSERT(file.isValid());
    TEST_ASSERT(file.readString() == FileContent);

    TEST_SUCCESS();
};


TEST_SEQUENCE("FileReadString") {
    writeTestFile();

     // read file using readString methods
    wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
    TEST_ASSERT(file.isValid());

    TEST_ASSERT(file.readString() == FileContent);

    file.seek(0);
    auto u8str = file.readU8String();
    auto str = std::string(u8str.begin(), u8str.end());
    TEST_ASSERT(str == FileContent);

    // check readString operations again now that the file is at the end
    TEST_ASSERT(file.readString() == "");
    TEST_ASSERT(file.readU8String() == std::u8string());

    TEST_SUCCESS();
};
