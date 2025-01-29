#pragma once

#include <cstdint>

// Disable builtin uint128 support when building with the MSVC standard library
#if defined(LIBWOLV_BUILTIN_UINT128) && defined(_MSC_VER)
#undef LIBWOLV_BUILTIN_UINT128
#endif // LIBWOLV_BUILTIN_UINT128

#if !defined(LIBWOLV_BUILTIN_UINT128)
#include "types/uintwide_t.h"
#endif // LIBWOLV_BUILTIN_UINT128

namespace wolv {

    namespace unsigned_integers {

        using u8  = std::uint8_t;
        using u16 = std::uint16_t;
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;

#if defined(LIBWOLV_BUILTIN_UINT128)
        using u128 = __uint128_t;
#else
        using u128 = ::math::wide_integer::uint128_t;
#endif // LIBWOLV_BUILTIN_UINT128

    }

    namespace signed_integers {

        using i8  = std::int8_t;
        using i16 = std::int16_t;
        using i32 = std::int32_t;
        using i64 = std::int64_t;

#if defined(LIBWOLV_BUILTIN_UINT128)
        using i128 = __int128_t;
#else
        using i128 = ::math::wide_integer::int128_t;
#endif // LIBWOLV_BUILTIN_UINT128

    }

    namespace floating_point_numbers {

        using f32 = float;
        using f64 = double;

    }

    using namespace unsigned_integers;
    using namespace signed_integers;
    using namespace floating_point_numbers;

}

#if !defined(LIBWOLV_BUILTIN_UINT128)

inline std::strong_ordering operator<=>(wolv::unsigned_integers::u128 const &lhs, wolv::unsigned_integers::u128 const &rhs) {
    if (lhs == rhs)
        return std::strong_ordering::equal;
    if (lhs < rhs)
        return std::strong_ordering::less;

    return std::strong_ordering::greater;
}

inline std::strong_ordering operator<=>(wolv::signed_integers::i128 const &lhs, wolv::signed_integers::i128 const &rhs) {
    if (lhs == rhs)
        return std::strong_ordering::equal;
    if (lhs < rhs)
        return std::strong_ordering::less;

    return std::strong_ordering::greater;
}

#endif