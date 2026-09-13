#pragma once

#include <utility>

#include <wolv/utils/preproc.hpp>

#include <string>
#include <map>
#include <functional>
#include <cstdlib>

#define TEST_SEQUENCE(...) static auto WOLV_ANONYMOUS_VARIABLE(TEST_SEQUENCE) = ::wolv::test::TestSequenceExecutor(__VA_ARGS__) + []() -> int
#define TEST_FAIL()        return EXIT_FAILURE
#define TEST_SUCCESS()     return EXIT_SUCCESS
#define FAILING            true
#define TEST_ASSERT(x)                                                 \
    do {                                                               \
        auto ret = (x);                                                \
        if (!ret) {                                                    \
            std::printf("Test assert '%s' failed at %s:%i\n",          \
                #x, __FILE__, __LINE__);                               \
            return EXIT_FAILURE;                                       \
        }                                                              \
    } while (0)

namespace wolv::test {

    struct Test {
        std::function<int()> function;
        bool shouldFail;
    };

    class Tests {
    public:
        static auto &get() noexcept {
            // s_tests is function local static to avoid initialization order fiasco
            static std::map<std::string, Test> s_tests;
            return s_tests;
        }

        static auto addTest(const std::string &name, const std::function<int()> &func, bool shouldFail) noexcept {
            get().insert({
                name, {func, shouldFail}
            });

            return 0;
        }
    };

    template<class F>
    class TestSequence {
    public:
        TestSequence(const std::string &name, F func, bool shouldFail) noexcept {
            Tests::addTest(name, func, shouldFail);
        }

        TestSequence &operator=(TestSequence &&) = delete;
    };

    struct TestSequenceExecutor {
        explicit TestSequenceExecutor(std::string name, bool shouldFail = false) noexcept : m_name(std::move(name)), m_shouldFail(shouldFail) {
        }

        [[nodiscard]] const auto &getName() const noexcept {
            return this->m_name;
        }

        [[nodiscard]] bool shouldFail() const noexcept {
            return this->m_shouldFail;
        }

    private:
        std::string m_name;
        bool m_shouldFail;
    };


    template<typename F>
    TestSequence<F> operator+(const TestSequenceExecutor &executor, F &&f) noexcept {
        return TestSequence<F>(executor.getName(), std::forward<F>(f), executor.shouldFail());
    }

}
