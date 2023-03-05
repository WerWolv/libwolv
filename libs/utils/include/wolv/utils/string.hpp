#pragma once

#include <wolv/types.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace wolv::util {

    std::vector<std::string> splitString(const std::string &string, const std::string &delimiter);
    std::string combineStrings(const std::vector<std::string> &strings, const std::string &delimiter);
    std::string replaceStrings(std::string string, const std::string &search, const std::string &replace);

    template<typename T>
    concept Char8StringConvertable = requires(T t) { t.u8string(); };

    std::string toUTF8String(const Char8StringConvertable auto &value) {
        auto result = value.u8string();

        return { result.begin(), result.end() };
    }

    constexpr inline size_t strnlen(const char *s, size_t n) {
        size_t i = 0;

        while (i < n && s[i] != '\x00') i++;

        return i;
    }

    template<typename T = char>
    [[nodiscard]] std::basic_string<T> trim(std::basic_string<T> s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](u8 ch) {
            return !std::isspace(ch) && ch >= 0x20;
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](u8 ch) {
            return !std::isspace(ch) && ch >= 0x20;
        }).base(), s.end());

        return s;
    }

}