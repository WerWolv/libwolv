#pragma once

#include <charconv>

namespace wolv::util {

    template<std::integral T>
    [[nodiscard]] std::optional<T> from_chars(std::string_view string, int base = 0) {
        T value;
        auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), value, base);
        if (ec != std::errc() || ptr != string.data() + string.size())
            return std::nullopt;
        return value;
    }

    template<std::floating_point T>
    [[nodiscard]] std::optional<T> from_chars(std::string_view string, std::chars_format format = std::chars_format::general) {
        T value;
        auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), value, format);
        if (ec != std::errc() || ptr != string.data() + string.size())
            return std::nullopt;
        return value;
    }
    
}