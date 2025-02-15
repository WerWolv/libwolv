#pragma once

namespace wolv {

    template<typename T>
    struct always_false : std::false_type { };

    template<typename T>
    constexpr static auto always_false_v = always_false<T>::value;

}