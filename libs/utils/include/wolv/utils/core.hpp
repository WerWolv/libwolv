#pragma once

#include <wolv/types.hpp>

#include <array>
#include <cstring>

namespace wolv::util {

    void unused(auto && ... x) {
        ((void)x, ...);
    }


    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template<typename T>
    struct always_false : std::false_type { };


    template<typename T>
    std::array<u8, sizeof(T)> toBytes(const T &value) {
        std::array<u8, sizeof(T)> bytes;
        std::memcpy(bytes.data(), &value, sizeof(T));

        return bytes;
    }

    template<typename Out>
    Out toContainer(const auto &value) {
        return { value.begin(), value.end() };
    }
}