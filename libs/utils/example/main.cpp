#include <wolv/utils/core.hpp>
#include <wolv/utils/guards.hpp>
#include <wolv/utils/preproc.hpp>
#include <wolv/utils/string.hpp>

int main() {
    puts("Start");

    {
        puts("Start Scope");

        ON_SCOPE_EXIT {
            puts("Exiting scope");
        };

        AT_FIRST_TIME {
            puts("First time");
        };

        AT_FINAL_CLEANUP {
            puts("Final cleanup");
        };

        puts("End Scope");
    }

    puts("End");

    auto bytes = wolv::util::toBytes(wolv::u32(0xAABBCCDD));
    for (auto b : bytes)
        printf("%02X ", b);
    puts("");
}