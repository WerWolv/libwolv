#pragma once

#include <wolv/types.hpp>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace wolv::util {

    [[nodiscard]] std::vector<std::string> splitString(const std::string &string, const std::string &delimiter, bool removeEmpty = false);
    [[nodiscard]] std::string combineStrings(const std::vector<std::string> &strings, const std::string &delimiter);
    [[nodiscard]] std::string replaceStrings(std::string string, const std::string &search, const std::string &replace);
    [[nodiscard]] std::string replaceTabsWithSpaces(const std::string& string, u32 tabSize = 4);
    [[nodiscard]] std::string wrapMonospacedString(const std::string& string, f32 charWidth, f32 maxWidth);
    [[nodiscard]] std::string capitalizeString(std::string string);

    [[nodiscard]] std::optional<std::string> utf16ToUtf8(const std::u16string &string);
    [[nodiscard]] std::optional<std::u16string> utf8ToUtf16(const std::string &string);
    [[nodiscard]] std::optional<std::string> utf32ToUtf8(const std::u32string &string);
    [[nodiscard]] std::optional<std::u32string> utf8ToUtf32(const std::string &string);
    [[nodiscard]] std::optional<std::string> wstringToUtf8(const std::wstring &string);
    [[nodiscard]] std::optional<std::wstring> utf8ToWstring(const std::string &string);

    template<typename T>
    concept Char8StringConvertable = requires(T t) { t.u8string(); };

    std::string toUTF8String(const Char8StringConvertable auto &value) {
        auto result = value.u8string();

        return { result.begin(), result.end() };
    }

    constexpr size_t strnlen(const char *s, size_t n) {
        size_t i = 0;

        while (i < n && s[i] != '\x00') i++;

        return i;
    }

    template<typename T = char>
    [[nodiscard]] std::basic_string<T> trim(std::basic_string<T> s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](T ch) {
            return !std::isspace(ch) && ch >= 0x20;
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](T ch) {
            return !std::isspace(ch) && ch >= 0x20;
        }).base(), s.end());

        return s;
    }
    
    template<typename T = char>
    [[nodiscard]] std::basic_string_view<T> trim(std::basic_string_view<T> s) {
        while (!s.empty() && std::isspace(s.front())) s.remove_prefix(1);
        while (!s.empty() && std::isspace(s.back())) s.remove_suffix(1);

        return s;
    }

}
