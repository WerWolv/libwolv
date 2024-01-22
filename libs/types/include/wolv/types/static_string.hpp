#pragma once

#include <array>
#include <algorithm>

namespace wolv::type {

    template<size_t N>
    struct StaticString {
        constexpr StaticString(const char (&str)[N]) {
            std::copy_n(str, N, this->value.begin());
        }

        std::array<char, N> value = {};
    };

}