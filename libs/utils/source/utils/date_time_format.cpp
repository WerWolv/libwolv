// date_time_format.cpp

#include <wolv/utils/date_time_format.hpp>
#include <algorithm>

#if defined(OS_WINDOWS)
# include <limits>
# include <cassert>
# include <wolv/container/soo_buffer.hpp>
#else
# include <time.h>
#if defined(OS_MACOS)
# include <xlocale.h>
#endif
# include <locale.h>
#endif // #if defined(OS_WINDOWS)

namespace wolv::util {

#if defined(OS_WINDOWS)

    Locale::Locale(const char *str) {
        set(str);
    }

    Locale::Locale(const std::string &str) {
        set(str);
    }

    void Locale::set(const char *str) {
        m_locale = str;
    }

    void Locale::set(const std::string &str) {
        m_locale = str;
    }

#else

    Locale::Locale() {
        setInvalid();
        set(setlocale(LC_TIME_MASK, NULL));
    }

    Locale::Locale(const char *str) {
        setInvalid();
        set(str);
    }
    
    Locale::Locale(const std::string &str) {
        setInvalid();
        set(str);
    }

    Locale::Locale(const Locale &copyMe) {
        setInvalid();

        m_locale = duplocale(copyMe);
        if (!m_locale) {
            return;
        }
        m_valid = true;
    }

    Locale::~Locale() {
        free();
    }

    Locale& Locale::operator=(const Locale &copyMe) {
        free();
        if (copyMe.m_valid) {
            m_locale = duplocale(copyMe.m_locale);
            if (m_locale) {
                m_valid = true;
            }
        }

        return *this;
    }

    void Locale::set(const char *str) {
        free();
        m_locale = newlocale(LC_TIME_MASK, str, NULL);
        if (!m_locale) {
            m_locale = duplocale(LC_GLOBAL_LOCALE);
        }
        if (m_locale) {
            m_valid = true;
        }
    }

    void Locale::set(const std::string &str) {
        set(str.c_str());
    }

    void Locale::setInvalid() {
        m_valid = false;
        m_locale = 0;
    }

    void Locale::free() {
        if (m_valid) {
            freelocale(m_locale);
            setInvalid();
        }
    }

#endif

#if defined(OS_WINDOWS)

std::optional<SYSTEMTIME> timeTToSystemTime(i64 t, DTOpts sz) {
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
    constexpr wolv::i64 sTo100ns = 10000000LL; // conversion factor from seconds to nanoseconds
    constexpr wolv::i64 eDiffIn100ns = 116444736000000000LL; // epoch differene in 100 nanoseconds

    // *** We convert a time_t to a FILETIME like this ***
    //
    //  ft = (tt*sTo100ns)+eDiffIn100ns;
    //
    // *** So what is the time_t range convertable to a FILETIME? ***
    //
    //  We're going to have to do some algebra.
    //
    //   The minimum time_t covertable to a FILETIME:
    //    (tt * sTo100ns) + eDiffIn100ns >= 0
    //    tt * sTo100ns >= -eDiffIn100ns
    //    tt >= -eDiffIn100ns / sTo100ns
    //
    //   The maximum time_t covertable to a FILETIME:
    //    (tt * sTo100ns) + eDiffIn100ns <= 2^64-1
    //    tt*sTo100ns <= (2^64-1) - eDiffIn100ns
    //    tt <= ((2^64-1) - eDiffIn100ns) / sTo100ns
    if ((sz&DTOpts::TTMask) == DTOpts::TT64) {
        constexpr i64 firstConv = -eDiffIn100ns / sTo100ns;
        constexpr i64 lastConv = (static_cast<u64>(-1) - eDiffIn100ns) / sTo100ns;

        if (t<firstConv || t>lastConv) {
            return std::nullopt;
        }
    }
    else {
        assert(
            t >= std::numeric_limits<wolv::i32>::min() &&
            t <= std::numeric_limits<wolv::i32>::max()  );
    }

    u64 inFT = (t * sTo100ns) + eDiffIn100ns;

    FILETIME ft;
    ft.dwLowDateTime = inFT & ((1ULL << 32) - 1);
    ft.dwHighDateTime = inFT >> 32;

    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&ft, &st)) {
        return std::nullopt;
    }

    return st;
}

std::optional<std::string> formatSystemTime(LPCSTR lc, const SYSTEMTIME* pss, DTOpts opts) {
    // We try to minimize heap allocations by preferring stack-based buffers.
    // If the data exceeds the stack buffer size, we fall back to the heap.
    // Functions like GetDateFormatEx and WideCharToMultiByte are somewhat
    // awkward: they can either calculate the required buffer size or write
    // into a supplied buffer, but not both at the same time. Our approach is
    // to start with a stack buffer and, if it proves too small, query the API
    // for the required size and retry using a suitably sized buffer.
    // These constants are guesses at reasonably sized stack-based buffers.
    constexpr size_t dateBufLen = 48;
    constexpr size_t timeBufLen = 24;
    constexpr WCHAR dtSep[] = L" ";
    constexpr size_t dtSepStrLen = sizeof(dtSep)/sizeof(dtSep[0])-1;
    constexpr size_t dtStrLen = dateBufLen + dtSepStrLen + timeBufLen;

    WCHAR wideLocale[LOCALE_NAME_MAX_LENGTH];
    for (size_t i=0; i<LOCALE_NAME_MAX_LENGTH; ++i) {
        wideLocale[i] = lc[i];
        if (!lc[i]) {
            break;
        }
    }
    wideLocale[LOCALE_NAME_MAX_LENGTH-1] = 0;

    SOOBuffer<WCHAR, dtStrLen+1, true> date;
    LPWSTR pCursor = date; 

    DWORD dateFlags = ((opts & DTOpts::DateFmtMask) == DTOpts::LongDate) ? DATE_LONGDATE : 0;

    int gdfLen = 0;
    if ((opts & DTOpts::D) == DTOpts::D) {
        gdfLen = GetDateFormatEx(wideLocale, dateFlags, pss, NULL, pCursor, static_cast<int>(date.size()), NULL);
        if (gdfLen == 0) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                // Our stack buffer was too small. Measure, alloc and convert.
                gdfLen = GetDateFormatEx(wideLocale, dateFlags, pss, NULL, NULL, 0, NULL);
                if (gdfLen == 0) {
                    return std::nullopt;
                }
                date.grow(gdfLen-1 + ((opts & DTOpts::T)==DTOpts::T ? dtSepStrLen+timeBufLen : 0)+1, {&pCursor});
                gdfLen = GetDateFormatEx(wideLocale, dateFlags, pss, NULL, pCursor, static_cast<int>(date.size()), NULL);
                if (gdfLen == 0) {
                    return std::nullopt;
                }
            }
            else {
                return std::nullopt;
            }
        }
        pCursor += gdfLen-1;
    }

    if ((opts & DTOpts::T) == DTOpts::T) {
        if ((opts & DTOpts::D) == DTOpts::D) {
            date.grow(gdfLen-1 + dtSepStrLen + timeBufLen + 1, {&pCursor});
            memcpy(pCursor, dtSep, sizeof(dtSep));
            pCursor += dtSepStrLen;
        }
    
        size_t time_sz = date.size()-(pCursor-date);

        int gtfLen = GetTimeFormatEx(wideLocale, 0, pss, NULL, pCursor, static_cast<int>(time_sz));
        if (gtfLen == 0) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                // Our stack buffer was too small. Measure, alloc and convert.
                gtfLen = GetTimeFormatEx(wideLocale, 0, pss, NULL, NULL, 0);
                if (gtfLen == 0) {
                    return std::nullopt;
                }

                date.grow(gdfLen-1 + dtSepStrLen + gtfLen-1 + 1, {&pCursor});

                time_sz = date.size() - (pCursor - date);

                gtfLen = GetTimeFormatEx(
                    wideLocale, 0, pss, NULL, pCursor, static_cast<int>(gtfLen-1 + 1));
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
    out.resize(dtStrLen);

    int res = WideCharToMultiByte(CP_UTF8, 0, date, -1, &out[0], dtStrLen, NULL, NULL);
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
    out.resize(res-1);

    return out;
}

std::optional<std::string> formatTT(const Locale &lc, wolv::i64 t, DTOpts opts) {
    auto st = timeTToSystemTime(t, opts);
    if (!st) {
        return std::nullopt;
    }

    auto dt = formatSystemTime(lc, &st.value(), opts);
    if (!dt) {
        return std::nullopt;
    }

    return dt.value();
}

namespace {

BOOL CALLBACK LocaleEnumprocex(LPWSTR name, DWORD flags, LPARAM ud) {
    auto* locales = reinterpret_cast<std::vector<std::string>*>(ud);

    if (name[0]==0) {
        return TRUE;
    }

    char u8LocaleName[LOCALE_NAME_MAX_LENGTH];
    int res = WideCharToMultiByte(CP_UTF8, 0, name, -1, u8LocaleName, sizeof(u8LocaleName), NULL, NULL);
    if (res==0) {
        return TRUE;
    }

    locales->emplace_back(u8LocaleName);

    return TRUE;
}

}

std::vector<std::string> enumLocales() {
    std::vector<std::string> locales;

    EnumSystemLocalesEx(
        LocaleEnumprocex,
        LOCALE_WINDOWS,
        reinterpret_cast<LPARAM>(&locales),
        NULL);

    std::sort(locales.begin(), locales.end());

    return locales;
}

std::string localeName(const std::string &lc, bool english) {
    WCHAR wideLocale[LOCALE_NAME_MAX_LENGTH];
    {
        size_t i=0;
        for (; i<lc.size() && i<LOCALE_NAME_MAX_LENGTH-1; ++i) {
            wideLocale[i] = lc[i];
        }
        wideLocale[i] = 0;
    }

    LCTYPE tp = english ? LOCALE_SENGLISHDISPLAYNAME : LOCALE_SNATIVEDISPLAYNAME;

    int len = GetLocaleInfoEx(wideLocale, tp, 0, 0);
    SOOBuffer<WCHAR, 64> wideBuffer(len);
    len = GetLocaleInfoEx(wideLocale, tp, wideBuffer, wideBuffer.size());

    len = WideCharToMultiByte(CP_UTF8, 0, wideBuffer, -1, 0, 0, NULL, NULL);
    std::string name;
    name.resize(len);
    len = WideCharToMultiByte(CP_UTF8, 0, wideBuffer, -1, &name[0], len, NULL, NULL);
    name.resize(len-1);

    return name;
}

#else

std::optional<std::string> formatTT(const Locale &lc, wolv::i64 t, DTOpts opts) {
    constexpr size_t szMin = 64;
    constexpr size_t szMax = 1024;

    const char *datetime_fs = "%c";
    const char *date_fs = "%x";
    const char *time_fs = "%X";

    const char *fs = datetime_fs;
    switch (opts & DTOpts::DTMask) {
    case DTOpts::DandT:
        fs = datetime_fs;
        break;

    case DTOpts::D:
        fs = date_fs;
        break;

    case DTOpts::T:
        fs = time_fs;
        break;
    }

    struct tm tm;
    time_t tt = static_cast<time_t>(t);
    gmtime_r(&tt, &tm);

    std::string str;
    for (size_t bsz=szMin; bsz<=szMax; bsz*=2) {
        str.resize(bsz);
        size_t sz = strftime_l(&str[0], bsz, fs, &tm, lc);
        if (sz != 0) {
            str.resize(sz);
            return str;
        }
    }

    return std::nullopt;
}

#endif // #if defined(OS_WINDOWS)

} // namespace wolv::util
