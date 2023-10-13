#include <wolv/types.hpp>
#include <wolv/literals.hpp>
#include <wolv/types/static_string.hpp>

int main() {

    static_assert(sizeof(wolv::u8)   == 1);
    static_assert(sizeof(wolv::u16)  == 2);
    static_assert(sizeof(wolv::u32)  == 4);
    static_assert(sizeof(wolv::u64)  == 8);
    static_assert(sizeof(wolv::u128) == 16);

    static_assert(sizeof(wolv::i8)   == 1);
    static_assert(sizeof(wolv::i16)  == 2);
    static_assert(sizeof(wolv::i32)  == 4);
    static_assert(sizeof(wolv::i64)  == 8);
    static_assert(sizeof(wolv::i128) == 16);

    static_assert(sizeof(wolv::f32)  == 4);
    static_assert(sizeof(wolv::f64)  == 8);

    using namespace wolv::literals;
    static_assert(5_bytes == 5ULL);
    static_assert(5_kiB   == 5ULL * 1024);
    static_assert(5_MiB   == 5ULL * 1024 * 1024);
    static_assert(5_GiB   == 5ULL * 1024 * 1024 * 1024);

}