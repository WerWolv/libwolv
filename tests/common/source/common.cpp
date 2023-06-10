#include <wolv/test/tests.hpp>

using namespace std::literals::string_literals;

TEST_SEQUENCE("TestSucceeding") {
    TEST_SUCCESS();
};

TEST_SEQUENCE("TestFailing", FAILING) {
    TEST_FAIL();
};