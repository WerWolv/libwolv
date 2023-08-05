#include <stdexcept>

#include <wolv/test/tests.hpp>

#include <wolv/utils/string.hpp>

using namespace std::literals::string_literals;

using namespace wolv::util;

// test splitString()
TEST_SEQUENCE("String_Split") {

    // test with the space separator
    {
        std::vector<std::string> res = {"house", "window", "tree"};
        TEST_ASSERT(splitString("house window tree", " ") == res);
    }

    // test with another separator
    {
        std::vector<std::string> res = {"house", "window", "tree"};
        TEST_ASSERT(splitString("houseawindowatree", "a") == res);
    }

    // test with no separator
    {
        std::vector<std::string> res = {"housewindowtree"};
        TEST_ASSERT(splitString("housewindowtree", "") == res);
    }

    // test with no input string
    {
        std::vector<std::string> res = {""};
        TEST_ASSERT(splitString("", " ") == res);
    }

    TEST_SUCCESS();
};

// test combineStrings()
TEST_SEQUENCE("String_Combine") {

    // space delimiter
    {
        TEST_ASSERT(combineStrings({"house", "window", "tree"}, " ") == "house window tree");
    }

    // another delimiter
    {
        TEST_ASSERT(combineStrings({"house", "window", "tree"}, "b") == "housebwindowbtree");
    }

    // empty list
    {
        TEST_ASSERT(combineStrings({}, " ") == "");
    }

    // empty delimiter
    {
        TEST_ASSERT(combineStrings({"house", "window", "tree"}, "") == "housewindowtree");
    }

    // both empty list and empty delimiter
    {
        TEST_ASSERT(combineStrings({}, "") == "");
    }

    TEST_SUCCESS();
};

// test replaceStrings()
TEST_SEQUENCE("String_Replace") {
    // 0 occurence
    {
        TEST_ASSERT(replaceStrings("house tree mirror", "kitchen", "bedroom") == "house tree mirror");
    }

    // 1 occurence
    {
        TEST_ASSERT(replaceStrings("house tree mirror", "tree", "mirror") == "house mirror mirror");
    }

    // 2 occurences
    {
        TEST_ASSERT(replaceStrings("house window tree window mirror", "window", "glass") == "house glass tree glass mirror");
    }

    // test that it is recursive
    {
        TEST_ASSERT(replaceStrings("houhousese", "house", "") == "");
    }

    TEST_SUCCESS();
};

// test trim()
TEST_SEQUENCE("String_Trim") {
    // nothing to trim
    {
        TEST_ASSERT(trim(std::string("house")) == "house");
    }

    // trim multiple spaces before and after
    {
        TEST_ASSERT(trim(std::string("  house  ")) == "house");
    }

    TEST_SUCCESS();
};

char* to_char_pointer(std::initializer_list<char> list) {
    char* result = new char[list.size()];
    int index = 0;
    for(auto i = list.begin(); i != list.end(); i++){
        result[index++] = *i;
    }
    return result;
}

// test strnlen()
TEST_SEQUENCE("String_Strnlen") {
    // basic data to get length from
    {
        auto data = to_char_pointer({0x01, 0x7F, 0x7F, 0x00});
        TEST_ASSERT(strnlen(data, 255) == 3);
    }

    // Verify that we won't read more bytes than the specified maximum
    {
        char *data = to_char_pointer({0x01, 0x7F, 0x7F, 0x00});
        TEST_ASSERT(strnlen(data, 2) == 2);
    }

    // Make sure doesn't read past the null byte
    {
        char *data = to_char_pointer({0x7F, 0x7F, 0x00, 0x7F, 0x00});
        TEST_ASSERT(strnlen(data, 255) == 2);
    }

    // put the null byte first
    {
        char *data = to_char_pointer({0x00, 0x7F, 0x7F, 0x7F});
        TEST_ASSERT(strnlen(data, 255) == 0);
    }

    TEST_SUCCESS();
};
