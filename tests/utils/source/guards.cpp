#include <wolv/test/tests.hpp>

#include <wolv/utils/guards.hpp>

using namespace wolv::util;

TEST_SEQUENCE("Guard_ScopeGuard") {

    // ensure block is run at the end of the block and not before
    {
        int i = 0;
        SCOPE_GUARD {
            TEST_ASSERT(i == 1);
        };
        i = 1;
    }

    // ensure block is actually run
    {
        int i = 0;
        SCOPE_GUARD {
            i = 1;
        };
        TEST_ASSERT(i == 1);
    }

    // release the guard
    {
        auto guard = SCOPE_GUARD {
            TEST_FAIL();
        };
        guard.release();
    }

    TEST_SUCCESS();
};
