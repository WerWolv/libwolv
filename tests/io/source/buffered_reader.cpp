#include <wolv/test/tests.hpp>

#include <wolv/io/file.hpp>
#include <wolv/io/fs.hpp>
#include <wolv/io/buffered_reader.hpp>

#include <cstring>

void StringReader(std::string *userData, void *buffer, wolv::u64 address, size_t size) {
    memcpy(buffer, &(*userData)[address], size);
}


int test_iterator(){
    std::string testString = "Hello World";
    wolv::io::BufferedReader<std::string, StringReader> reader(&testString, testString.size());

    std::string outputString;
    for(char c : reader) {
        outputString += c;
    }

    TEST_ASSERT(outputString == testString);
    
    TEST_SUCCESS();
}

int test_read(){
    std::string testString = "Hello World";
    wolv::io::BufferedReader<std::string, StringReader> reader(&testString, testString.size());

    {
        // test to read everything
        auto vec = reader.read(0, testString.size());
        std::string outputString(vec.begin(), vec.end());
        TEST_ASSERT(outputString == testString);
    }

    {
        // test to read part of it
        auto vec = reader.read(1, testString.size()-2);
        std::string outputString(vec.begin(), vec.end());
        TEST_ASSERT(outputString == testString.substr(1, testString.size()-2));
    }

    {
        // test to read out of bounds bytes
        auto vec = reader.read(testString.size(), 2);
        TEST_ASSERT(vec.size()==2);
        TEST_ASSERT(vec.at(0)==0);
        TEST_ASSERT(vec.at(1)==0);
    }

    TEST_SUCCESS();
}

TEST_SEQUENCE("BufferedReader") {

    TEST_ASSERT(test_iterator()==EXIT_SUCCESS);
    TEST_ASSERT(test_read()==EXIT_SUCCESS);
    

    TEST_SUCCESS();
};
