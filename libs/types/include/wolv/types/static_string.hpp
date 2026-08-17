#pragma once

#include <array>
#include <algorithm>

namespace wolv::type {

    template<size_t N>
    struct StaticString {
        constexpr StaticString(const char (&str)[N]) {
            std::copy_n(str, N, this->value.begin());
        }

        constexpr std::string_view get() const { return { value.data(), value.size() - 1 }; }

        std::array<char, N> value = {};
    };

}