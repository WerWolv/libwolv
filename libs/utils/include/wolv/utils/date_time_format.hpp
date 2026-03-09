#pragma once
// date_time_format.hpp

#if defined(OS_WINDOWS)
# include <windows.h>
# include <wolv/types.hpp>
# include <optional>
# include <string>
#endif // #if defined(OS_WINDOWS)

#include <wolv/utils/date_time_format.hpp>

namespace wolv::util {

#if defined(OS_WINDOWS)
    std::optional<SYSTEMTIME> time_t_to_SYSTEMTIME(i64, bool bits64=true);
    std::optional<std::string> formatDateFromSYSTEMTIME(LPCSTR lc, const SYSTEMTIME* pss, bool bTime = true);

    std::optional<std::string> formatTT(const char *lang, i64, bool bits64=true);
    std::optional<std::string> formatTTPOSIX(const char *lang, i64, bool bits64=true);


#endif

} // namespace wolv::util
