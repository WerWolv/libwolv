#pragma once

namespace wolv::literals {

    /* Byte literals */

    constexpr static inline unsigned long long operator""_bytes(unsigned long long bytes) noexcept {
        return bytes;
    }

    constexpr static inline unsigned long long operator""_kiB(unsigned long long kiB) noexcept {
        return operator""_bytes(kiB * 1024);
    }

    constexpr static inline unsigned long long operator""_MiB(unsigned long long MiB) noexcept {
        return operator""_kiB(MiB * 1024);
    }

    constexpr static inline unsigned long long operator""_GiB(unsigned long long GiB) noexcept {
        return operator""_MiB(GiB * 1024);
    }

}