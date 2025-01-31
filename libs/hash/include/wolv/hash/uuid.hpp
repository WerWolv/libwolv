#pragma once

#include <wolv/types.hpp>

#include <random>
#include <string>

namespace wolv::hash {

    inline std::string generateUUID() {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());

        std::uniform_int_distribution<u16> distribution(0, 15);

        std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
        for (char &c : uuid) {
            if (c == 'x') {
                c = distribution(generator);
                c = c < 10 ? char('0' + c) : char('a' + c - 10);
            } else if (c == 'y') {
                c = distribution(generator);
                c = char(char(c & u8(0x03)) | char(0x08));
                c = c < 10 ? char('0' + c) : char('a' + c - 10);
            }
        }

        return uuid;
    }

}