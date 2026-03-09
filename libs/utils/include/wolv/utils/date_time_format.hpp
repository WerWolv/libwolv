#pragma once
// date_time_format.hpp

#include <windows.h>
#include <optional>
#include <cstdint>
#include <string>

#include <wolv/utils/date_time_format.hpp>

namespace wolv::util {

    std::optional<SYSTEMTIME> time_t_to_SYSTEMTIME(std::int64_t t, bool bits64=true);
    std::optional<std::string> formatDateFromSYSTEMTIME(LPCSTR lc, const SYSTEMTIME* pss, bool bTime = true);

} // namespace wolv::util
