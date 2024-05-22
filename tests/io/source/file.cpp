#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>
#include <wolv/io/file.hpp>

#include <helper.hpp>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;

TEST_SEQUENCE("BasicFileAccess") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    // ensure file doesn't already exist
    if (std::fs::exists(filePath))
        std::fs::remove(filePath);

    // create and write to file
    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.isValid());

        file.writeString(FileContent);
    }

    // ensure file was created
    if (!std::fs::exists(filePath)) 
        throw std::runtime_error("File doesn't exist");

    // read file
    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.isValid());

        TEST_ASSERT(file.readString() == FileContent);
    }

    // remove file
    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Write);
        TEST_ASSERT(file.isValid());


        file.remove();
        TEST_ASSERT(!file.isValid());
    }

    // ensure file was deleted
    if (std::fs::exists(filePath)) 
        throw std::runtime_error("File wasn't deleted");

    // try to read it again
    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Read);
        if (file.isValid())
            TEST_FAIL();
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileVectorOps") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        file.writeVector({ 'a', 'b', 'c' });
    }

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.readVector() == std::vector<u8>({ 'a', 'b', 'c' }));
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileStringOps") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        file.writeString("Hello");
        file.writeU8String(u8" world");
    }

     // read file using readString methods
    wolv::io::File file(filePath, wolv::io::File::Mode::Read);
    TEST_ASSERT(file.isValid());

    TEST_ASSERT(file.readString() == "Hello world");

    file.seek(0);
    TEST_ASSERT(file.readU8String() == u8"Hello world");

    // check readString operations again now that the file is at the end
    TEST_ASSERT(file.readString() == "");
    TEST_ASSERT(file.readU8String() == std::u8string());

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileClone") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };
    
    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        file.writeString(FileContent);
    }

     // read file using readString methods
    wolv::io::File file(filePath, wolv::io::File::Mode::Read);
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
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        file.writeStringAtomic(0, "Hello");
        file.writeU8StringAtomic(5, u8" World");
    }

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.readStringAtomic(0, 5) == "Hello");
        TEST_ASSERT(file.readU8StringAtomic(5, 6) == u8" World");

        // check invalid addresses
        TEST_ASSERT(file.readStringAtomic(100, 105) == "");
        TEST_ASSERT(file.readU8StringAtomic(100, 105) == u8"");
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileAtomicVectorOps") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        file.writeVectorAtomic(0, {'a', 'b', 'c'});
    }

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.readVectorAtomic(0, 3) == std::vector<u8>({ 'a', 'b', 'c' }));
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("FileSize") {
    auto filePath = std::fs::current_path() / randomFilename(); \
    ON_SCOPE_EXIT { std::fs::remove(filePath); };

    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Create);
        TEST_ASSERT(file.getSize() == 0);
        file.writeVector({'a', 'b', 'c'});
        file.flush();
        TEST_ASSERT(file.getSize() == 3);
    }
    
    {
        wolv::io::File file(filePath, wolv::io::File::Mode::Read);
        TEST_ASSERT(file.getSize() == 3);
    }
    TEST_SUCCESS();
};