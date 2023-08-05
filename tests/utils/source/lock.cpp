#include <wolv/test/tests.hpp>

#include <wolv/utils/lock.hpp>

using namespace wolv::util;

TEST_SEQUENCE("Lock") {

    std::mutex lock;

    // lock one time
    if (TRY_LOCK(lock)) {

        // try to lock a second time
        if (TRY_LOCK(lock)) {
            TEST_FAIL();
        }
    } else {
        TEST_FAIL();
    }

    // verify we can lock again after going out of the condition scope
    if (TRY_LOCK(lock)) {
        
    } else {
        TEST_FAIL();
    }

    TEST_SUCCESS();
};
