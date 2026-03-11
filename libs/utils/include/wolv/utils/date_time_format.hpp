#pragma once
// date_time_format.hpp

#include <optional>
#include <string>
#include <type_traits>

#include <wolv/types.hpp>

#if defined(OS_WINDOWS)
#include <windows.h>
#endif // #if defined(OS_WINDOWS)

namespace wolv::util {

    enum class DTOpts {
        TT32        = 0b0000,  // 32-bits
        TT64        = 0b0001,  // 64-bits
        TTMask      = 0b0001,

        DandT       = 0b0110,  // date and time
        D           = 0b0100,  // date
        T           = 0b0010,  // time
        DTMask      = 0b0110,

        ShortDate   = 0b0000,
        LongDate    = 0b1000,
        DateFmtMask = 0b1000
    };

    constexpr DTOpts operator|(DTOpts a, DTOpts b) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        return static_cast<DTOpts>(static_cast<T>(a) | static_cast<T>(b));
    }

    constexpr DTOpts operator&(DTOpts a, DTOpts b) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        return static_cast<DTOpts>(static_cast<T>(a) & static_cast<T>(b));
    }

#if defined(OS_WINDOWS)
    std::optional<SYSTEMTIME> time_t_to_SYSTEMTIME(i64 t, DTOpts sz = DTOpts::TT64);
    std::optional<std::string> formatDateFromSYSTEMTIME(LPCSTR lc, const SYSTEMTIME* pss, DTOpts opts = DTOpts::LongDate);

    std::optional<std::string> formatTT(const char *lang, i64 t, DTOpts opts = DTOpts::TT64|DTOpts::DandT|DTOpts::LongDate);
#else
    std::optional<std::string> formatTT(const char *lang, i64 t, DTOpts opts = DTOpts::TT64|DTOpts::DandT|DTOpts::LongDate);
#endif

} // namespace wolv::util
