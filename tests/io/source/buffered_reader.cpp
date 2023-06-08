#include <wolv/test/tests.hpp>

#include <wolv/io/file.hpp>
#include <wolv/io/fs.hpp>
#include <wolv/io/buffered_reader.hpp>

#include <cstring>

void StringReader(std::string *userData, void *buffer, wolv::u64 address, size_t size) {
    memcpy(buffer, &(*userData)[address], size);
}

TEST_SEQUENCE("BufferedReader") {
    std::string testString = "Hello World";

    wolv::io::BufferedReader<std::string, StringReader> reader(&testString, testString.size());

    std::string outputString;
    for(char c : reader) {
        outputString += c;
    }

    TEST_ASSERT(outputString == testString);

    TEST_SUCCESS();
};
