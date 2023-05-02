#pragma once

#include <string_view>

namespace wolv::type {

    template<typename T>
    constexpr std::string_view getTypeName() {
        #if defined(__clang__)
            constexpr auto Prefix = std::string_view("[T = ");
            constexpr auto Suffix = "]";
            constexpr auto Function = std::string_view(__PRETTY_FUNCTION__);
        #elif defined(__GNUC__)
            constexpr auto Prefix = std::string_view("with T = ");
            constexpr auto Suffix = "; ";
            constexpr auto Function = std::string_view(__PRETTY_FUNCTION__);
        #elif defined(_MSC_VER)
            constexpr auto Prefix = std::string_view("wolv::type::getTypeName<");
            constexpr auto Suffix = ">(void)";
            constexpr auto Function = std::string_view(__FUNCSIG__);
        #else
            #error Unsupported compiler
        #endif

        const auto start = Function.find(Prefix) + Prefix.size();
        const auto end = Function.find(Suffix);
        const auto size = end - start;

        return Function.substr(start, size);
    }

}