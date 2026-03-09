// date_time_format.cpp

#include <limits>
#include <cassert>

#include <wolv/utils/date_time_format.hpp>
#include <wolv/utils/soo_buffer.hpp>

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
        assert(
            t >= std::numeric_limits<std::int32_t>::min() &&
            t <= std::numeric_limits<std::int32_t>::max()  );
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

std::optional<std::string> formatDateFromSYSTEMTIME(LPCWSTR lc, const SYSTEMTIME* pss, bool bTime) {
    // We try to minimize heap allocations by preferring stack-based buffers.
    // If the data exceeds the stack buffer size, we fall back to the heap.
    // Functions like GetDateFormatEx and WideCharToMultiByte are somewhat
    // awkward: they can either calculate the required buffer size or write
    // into a supplied buffer, but not both at the same time. Our approach is
    // to start with a stack buffer and, if it proves too small, query the API
    // for the required size and retry using a suitably sized buffer.
    // These constants are guesses at reasonably sized stack-based buffers.
    constexpr size_t datebuflen = 48;
    constexpr size_t timebuflen = 24;
    constexpr WCHAR dtsep[] = L" ";
    constexpr size_t dtsep_strlen = sizeof(dtsep)/sizeof(dtsep[0])-1;
    constexpr size_t dt_strlen = datebuflen + dtsep_strlen + timebuflen;

    wolv::util::SOOBuffer<WCHAR, datebuflen + dtsep_strlen + timebuflen + 1, true> date;

    int gdfLen = GetDateFormatEx(
        lc, DATE_LONGDATE, pss, NULL, date, static_cast<int>(date.size()), NULL);
    if (gdfLen == 0) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            // Our stack buffer was too small. Measure, alloc and convert.
            gdfLen = GetDateFormatEx(
                lc, DATE_LONGDATE, pss, NULL, NULL, 0, NULL);
            if (gdfLen == 0) {
                return std::nullopt;
            }
            date.grow(gdfLen-1 + (bTime ? dtsep_strlen+timebuflen : 0) + 1);
            gdfLen = GetDateFormatEx(
                lc, DATE_LONGDATE, pss, NULL, date, static_cast<int>(date.size()), NULL);
            if (gdfLen == 0) {
                return std::nullopt;
            }
        }
        else {
            return std::nullopt;
        }
    }

    if (bTime) {
        date.grow(gdfLen-1 + dtsep_strlen + timebuflen + 1);
        memcpy(date+gdfLen-1, dtsep, sizeof(dtsep));

        WCHAR* pTime = date + gdfLen-1 + dtsep_strlen;
        size_t time_sz = date.size()-(pTime-date);

        int gtfLen = GetTimeFormatEx(lc, 0, pss, NULL, pTime, static_cast<int>(time_sz));
        if (gtfLen == 0) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                // Our stack buffer was too small. Measure, alloc and convert.
                gtfLen = GetTimeFormatEx(lc, 0, pss, NULL, NULL, 0);
                if (gtfLen == 0) {
                    return std::nullopt;
                }

                date.grow(gdfLen-1 + dtsep_strlen + gtfLen-1 + 1);

                pTime = date + gdfLen - 1 + dtsep_strlen;
                time_sz = date.size() - (pTime - date);

                gtfLen = GetTimeFormatEx(
                    lc, 0, pss, NULL, pTime, static_cast<int>(gtfLen-1 + 1));
                if (gtfLen == 0) {
                    return std::nullopt;
                }
            }
            else {
                return std::nullopt;
            }
        }
    }

    std::string out;
    out.resize(dt_strlen);

    int res = WideCharToMultiByte(CP_UTF8, 0, date, -1, &out[0], dt_strlen, NULL, NULL);
    if (res == 0) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            // Our buffer was too small. Measure, alloc and convert.
            res = WideCharToMultiByte(CP_UTF8, 0, date, -1, NULL, 0, NULL, NULL);
            if (res == 0) {
                return std::nullopt;
            }

            out.resize(res);

            res = WideCharToMultiByte(CP_UTF8, 0, date, -1, &out[0], res, NULL, NULL);
            if (res == 0) {
                return std::nullopt;
            }
        }
        else {
            return std::nullopt;
        }
    }

    return out;
}

} // namespace wolv::util
