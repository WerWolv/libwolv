#include <wolv/test/tests.hpp>

#include <wolv/io/file.hpp>

using namespace std::literals::string_literals;

TEST_SEQUENCE("FileAccess") {
    const auto FilePath    = std::fs::current_path() / "file.txt";
    const auto FileContent = "Hello World";

    std::fs::create_directories(FilePath.parent_path());

    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.isValid());

        file.writeString(FileContent);
    }

    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.isValid());

        TEST_ASSERT(file.readString() == FileContent);
    }

    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Write);
        TEST_ASSERT(file.isValid());


        file.remove();
        TEST_ASSERT(!file.isValid());
    }

    {
        wolv::io::File file(FilePath, wolv::io::File::Mode::Read);
        if (file.isValid())
            TEST_FAIL();
    }

    TEST_SUCCESS();
};
