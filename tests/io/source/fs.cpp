#include <wolv/test/tests.hpp>
#include <wolv/types.hpp>
#include <wolv/io/file.hpp>

using namespace std::literals::string_literals;
using namespace wolv::unsigned_integers;

TEST_SEQUENCE("FsGetExecPath") {
    // check there is a path
    auto path = wolv::io::fs::getExecutablePath().value();
    TEST_ASSERT(!path.empty());
    TEST_SUCCESS();
};
TEST_SEQUENCE("FsToNormalizedPath") {
    // no idea if this is useful
    std::string path = wolv::io::fs::toNormalizedPathString("/my/path/../to/./file.txt");
    TEST_ASSERT(path == "/my/path/../to/./file.txt");
    
    TEST_SUCCESS();
};