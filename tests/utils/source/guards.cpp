#include <wolv/test/tests.hpp>

#include <wolv/utils/guards.hpp>

using namespace wolv::util;

TEST_SEQUENCE("Guard_ScopeGuard") {

    // ensure block is run at the end of the block and not before
    int i = 0;
    {
        ON_SCOPE_EXIT {
            i = 2;
        };
        i = 1;
    }
    TEST_ASSERT(i == 2);

    // ensure block is actually run
    {
        ON_SCOPE_EXIT {
            i = 3;
        };
    }
    TEST_ASSERT(i == 3);

    // release the guard
    bool guardRan = false;
    {
        auto guard = SCOPE_GUARD {
            guardRan = true;
        };
        guard.release();
    }
    TEST_ASSERT(!guardRan);

    TEST_SUCCESS();
};
