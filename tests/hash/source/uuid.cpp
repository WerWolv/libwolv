
#include <regex>

#include <wolv/test/tests.hpp>

#include <wolv/hash/uuid.hpp>

using namespace std::literals::string_literals;

TEST_SEQUENCE("UUID") {

    // verify returned UUID has the right format
    auto uuid = wolv::hash::generateUUID();
    std::regex regex("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");
    if (!std::regex_search(uuid, regex)) {
        TEST_FAIL();
    }

    TEST_SUCCESS();
};
