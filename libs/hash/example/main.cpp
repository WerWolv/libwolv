#include <wolv/hash/crc.hpp>
#include <wolv/hash/uuid.hpp>

#include <cstdio>

int main() {
    wolv::hash::Crc<32> crc32(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true);

    std::array<wolv::u8, 5> data = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    crc32.process(data);
    printf("CRC32: %08llX\n", crc32.getResult());

    crc32.reset();
    crc32.process(data.begin(), data.end());
    printf("CRC32: %08llX\n", crc32.getResult());

    auto uuid = wolv::hash::generateUUID();
    printf("UUID: %s\n", uuid.c_str());

}