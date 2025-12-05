#pragma once

#include <charconv>

namespace wolv::util {

    template<std::integral T>
    [[nodiscard]] std::optional<T> from_chars(std::string_view string, int base = 0) {
        T value;
        auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), value, base);
        if (ec != std::errc())
            return std::nullopt;
        return value;
    }

    template<std::floating_point T>
    [[nodiscard]] std::optional<T> from_chars(std::string_view string, std::chars_format format = std::chars_format::general) {
        T value;
        if constexpr (requires { std::from_chars(string.data(), string.data() + string.size(), value, format); }) {
            auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), value, format);
            if (ec != std::errc())
                return std::nullopt;
            return value;
        } else {
            // TODO: This sucks but std::from_chars for floating point types is not supported by some clang versions yet
            // These functions do not handle errors properly and also are locale dependent.
            // Once std::from_chars for floating point types is supported, recompiling will
            // automatically switch to the from_chars implementation above.
            if constexpr (std::same_as<T, float>)
                return std::strtof(string.data(), nullptr);
            else if constexpr (std::same_as<T, double>)
                return std::strtod(string.data(), nullptr);
            else if constexpr (std::same_as<T, long double>)
                return std::strtold(string.data(), nullptr);
            else {
                static_assert("T must be a floating point type");
                return std::nullopt;
            }
        }
    }

}