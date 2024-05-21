#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>

#include <wolv/io/file.hpp>
#include <fmt/core.h>
#include <iostream>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;

const auto FilePath    = std::fs::current_path() / "file.txt";
const auto FileContent = "Hello World";

TEST_SEQUENCE("BasicFileAccess") {
    // ensure file doesn't already exist
    if (std::fs::exists(FilePath))
        std::fs::remove(FilePath);

    // create and write to file
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.isValid());

        file.writeString(FileContent);
    }

    // ensure file was created
    if (!std::fs::exists(FilePath)) 
        throw std::runtime_error("File doesn't exist");

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

    // ensure file was deleted
    if (std::fs::exists(FilePath)) 
        throw std::runtime_error("File wasn't deleted");

    // try to read it again
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        if (file.isValid())
            TEST_FAIL();
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileVectorOps") {
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        file.writeVector({ 'a', 'b', 'c' });
    }

    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.readVector() == std::vector<u8>({ 'a', 'b', 'c' }));
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileStringOps") {
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        file.writeString("Hello");
        file.writeU8String(u8" world");
    }

     // read file using readString methods
    wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
    TEST_ASSERT(file.isValid());

    TEST_ASSERT(file.readString() == "Hello world");

    file.seek(0);
    TEST_ASSERT(file.readU8String() == u8"Hello world");

    // check readString operations again now that the file is at the end
    TEST_ASSERT(file.readString() == "");
    TEST_ASSERT(file.readU8String() == std::u8string());

    TEST_SUCCESS();
};

// helper method to create a file.
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

TEST_SEQUENCE("FileAtomicStringOps") {
    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        file.writeStringAtomic(0, "Hello");
        file.writeU8StringAtomic(5, u8" World");
    }

    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.readStringAtomic(0, 5) == "Hello");
        TEST_ASSERT(file.readStringAtomic(5, 6) == " World");
    }

    TEST_SUCCESS();
};