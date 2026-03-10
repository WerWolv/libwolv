#pragma once
// date_time_format.hpp

#if defined(OS_WINDOWS)
# include <windows.h>
# include <wolv/types.hpp>
# include <optional>
# include <string>
# include <type_traits>
#endif // #if defined(OS_WINDOWS)

#include <wolv/utils/date_time_format.hpp>

namespace wolv::util {

    enum class DTOpts {
        TT32 =       0b000,  // 32-bits
        TT64 =       0b001,  // 64-bits
        TTMask =  0b001,

        DandT =     0b110,  // date and time
        D =         0b100,  // date
        T =         0b010,  // time
        DTMask =    0b110
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
    std::optional<std::string> formatDateFromSYSTEMTIME(LPCSTR lc, const SYSTEMTIME* pss, bool bTime = true);

    std::optional<std::string> formatTT(const char *lang, i64 t, DTOpts opts = DTOpts::TT64|DTOpts::DandT);
    std::optional<std::string> formatTTPOSIX(const char *lang, i64 t, DTOpts opts = DTOpts::TT64|DTOpts::DandT);
#endif

} // namespace wolv::util
