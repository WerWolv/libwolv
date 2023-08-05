#include <wolv/test/tests.hpp>

#include <wolv/utils/thread_pool.hpp>

using namespace std::chrono_literals;

using namespace wolv::util;

TEST_SEQUENCE("ThreadPool") {

    std::condition_variable cv;
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);

    bool taskFinished = false;

    ThreadPool pool(1);
    pool.enqueue([&](const auto &shouldStop){
        std::lock_guard lock(mutex);

        taskFinished = true;
        cv.notify_all();
    });

    // if the task didn't complete in 5 seconds, make the test fail
    if (cv.wait_for(lock, 5s, [&taskFinished]{return taskFinished;})) {
        TEST_SUCCESS();
    } else {
        TEST_FAIL();
    }

};
