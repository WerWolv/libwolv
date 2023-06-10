#include <wolv/test/tests.hpp>

#include <cstdlib>

int test(int argc, char **argv) {
    // Check if a test to run has been provided
    if (argc != 2) {
        std::printf("Invalid number of arguments specified! %d\n", argc);
        return EXIT_FAILURE;
    }

    // Check if that test exists
    std::string testName = argv[1];
    if (!wolv::test::Tests::get().contains(testName)) {
        std::printf("No test with name %s found!\n", testName.c_str());
        return EXIT_FAILURE;
    }

    auto test = wolv::test::Tests::get()[testName];

    auto result = test.function();

    if (test.shouldFail) {
        switch (result) {
            case EXIT_SUCCESS:
                return EXIT_FAILURE;
            case EXIT_FAILURE:
                return EXIT_SUCCESS;
            default:
                return result;
        }
    } else {
        return result;
    }
}

int main(int argc, char **argv) {
    int result = test(argc, argv);

    if (result == EXIT_SUCCESS) {
        std::printf("Success!\n");
    } else {
        std::printf("Failed!\n");
    }
    return result;
}