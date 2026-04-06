#pragma once
// date_time_format.hpp

#include <optional>
#include <string>
#include <vector>
#include <type_traits>

#include <wolv/types.hpp>

#if defined(OS_WINDOWS)
# include <windows.h>
#else
# include <locale.h>
#endif // #if defined(OS_WINDOWS)

namespace wolv::util {

#if defined(OS_WINDOWS)

    class Locale {
    public:
        Locale() = default;
        explicit Locale(const char *str, bool longDate = false);
        explicit Locale(const std::string &str, bool longDate = false);
        Locale(const Locale &copyMe) = default;

        ~Locale() = default;

        Locale& operator=(const Locale &copyMe) = default;

        void set(const char *str, bool longDate = false);
        void set(const std::string &str, bool longDate = false);

        operator const char*() const {
            return m_locale.c_str();
        }

        bool longDate() const {
            return m_longDate;
        }

    private:
        std::string m_locale;
        bool m_longDate = false;
    };

#else

    class Locale {
    public:
        Locale();
        explicit Locale(const char *str, bool longDate = false);
        explicit Locale(const std::string &str, bool longDate = false);
        Locale(const Locale &copyMe);

        ~Locale();

        Locale& operator=(const Locale &copyMe);

        void set(const char *str, bool longDate = false);
        void set(const std::string &str, bool longDate = false);

        operator locale_t() const {
            return m_locale;
        }

        bool longDate() const {
            return false;
        }

    private:
        void setInvalid();
        void free();

        bool m_valid;
        locale_t m_locale;
    };

#endif

    class LocaleName {
    public:
        LocaleName(const std::string &lc);
        LocaleName(const LocaleName &) = default;
        LocaleName(LocaleName &&) = default;

        LocaleName& operator=(const LocaleName &) = default;
        LocaleName& operator=(LocaleName &&) = default;

        std::string displayName() const;
    
        std::string nativeName() const {
            return m_nativeName;
        }
    
        std::string englishName() const {
            return m_englishName;
        }

    private:
        std::string m_nativeName;
        std::string m_englishName;
    };

    enum class DTOpts {
        TT32        = 0b00000,  // 32-bits
        TT64        = 0b00001,  // 64-bits
        TTMask      = 0b00001,

        DandT       = 0b00110,  // date and time
        D           = 0b00100,  // date
        T           = 0b00010,  // time
        DTMask      = 0b00110,

        DefaultDate = 0b00000,  // Date length defined by locale
        ShortDate   = 0b01000,
        LongDate    = 0b10000,
        DateFmtMask = 0b11000
    };

    constexpr DTOpts operator|(DTOpts a, DTOpts b) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        return static_cast<DTOpts>(static_cast<T>(a) | static_cast<T>(b));
    }

    constexpr DTOpts operator&(DTOpts a, DTOpts b) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        return static_cast<DTOpts>(static_cast<T>(a) & static_cast<T>(b));
    }

    constexpr DTOpts operator~(DTOpts a) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        return static_cast<DTOpts>(~static_cast<T>(a));
    }

    constexpr DTOpts& operator&=(DTOpts& a, DTOpts b) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        a = static_cast<DTOpts>(static_cast<T>(a) & static_cast<T>(b));
        return a;
    }

    constexpr DTOpts& operator|=(DTOpts& a, DTOpts b) noexcept {
        using T = std::underlying_type_t<DTOpts>;
        a = static_cast<DTOpts>(static_cast<T>(a) | static_cast<T>(b));
        return a;
    }

#if defined(OS_WINDOWS)
    std::optional<SYSTEMTIME> timeTToSystemTime(i64 t, DTOpts sz = DTOpts::TT64);
    std::optional<std::string> formatSystemTime(LPCSTR lc, const SYSTEMTIME* pss, DTOpts opts = DTOpts::LongDate);
#endif

    std::optional<std::string> formatTT(const Locale &lc, wolv::i64 t, DTOpts opts = DTOpts::TT64|DTOpts::DandT|DTOpts::LongDate);
    std::vector<std::string> enumLocales();
    std::string localeName(const std::string &lc, bool english=true);

} // namespace wolv::util
