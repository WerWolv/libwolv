// date_time_format.cpp

#include <wolv/utils/date_time_format.hpp>

namespace wolv::util {

std::optional<SYSTEMTIME> time_t_to_SYSTEMTIME(std::int64_t t, bool bits64) {
    // *** The types ***
    //
    //  /----------------------------------------------------------------------------\
    //  | Type     | Bits | Signed | Resolution | Epoch      | Range                 |
    //  |----------+------+--------+------------+------------+-----------------------|
    //  | time_t   | 32   | Yes    | 1s         | 1970-01-01 | 1901 - 2038           |
    //  | time_t   | 64   | Yes    | 1s         | 1970-01-01 | ~+/-292 billion years |
    //  | FILETIME | 64   | No     | 100ns      | 1601-01-01 | 1601 - ~586,000 AD    |
    //  \----------------------------------------------------------------------------/
    //
    //  time_t_epoch - FILETIME_epoch (seconds):   11644473600
    //  In 100 nanoseconds:                        116444736000000000
    constexpr std::int64_t s_to_100ns = 10000000LL; // conversion factor from seconds to nanoseconds
    constexpr std::int64_t ediff_100ns = 116444736000000000LL; // epoch differene in 100 nanoseconds

    // *** We convert a time_t to a FILETIME like this ***
    //
    //  ft = (tt*s_to_100ns)+ediff_100ns;
    //
    // *** So what is the time_t range convertable to a FILETIME? ***
    //
    //  We're going to have to do some algebra.
    //
    //   The minimum time_t covertable to a FILETIME:
    //    (tt * s_to_100ns) + ediff_100ns >= 0
    //    tt * s_to_100ns >= -ediff_100ns
    //    tt >= -ediff_100ns / s_to_100ns
    //
    //   The maximum time_t covertable to a FILETIME:
    //    (tt * s_to_100ns) + ediff_100ns <= 2^64-1
    //    tt*s_to_100ns <= (2^64-1) - ediff_100ns
    //    tt <= ((2^64-1) - ediff_100ns) / s_to_100ns
    if (bits64) {
        constexpr std::int64_t first_conv = -ediff_100ns / s_to_100ns;
        constexpr std::int64_t last_conv = (static_cast<std::uint64_t>(-1) - ediff_100ns) / s_to_100ns;

        if (t<first_conv || t>last_conv) {
            return std::nullopt;
        }
    }
    else {
        constexpr std::int64_t first_conv = -ediff_100ns / s_to_100ns;
        constexpr std::int64_t last_conv = (static_cast<std::uint32_t>(-1) - ediff_100ns) / s_to_100ns;

         if (t<first_conv || t>last_conv) {
            return std::nullopt;
        }
    }

    std::uint64_t inFT = (t * s_to_100ns) + ediff_100ns;

    FILETIME ft;
    ft.dwLowDateTime = inFT & ((1ULL << 32) - 1);
    ft.dwHighDateTime = inFT >> 32;

    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&ft, &st)) {
        return std::nullopt;
    }

    return st;
}

} // namespace wolv::util
