#include <wolv/test/tests.hpp>

#include <wolv/hash/crc.hpp>

using namespace std::literals::string_literals;

TEST_SEQUENCE("CRC") {
    wolv::hash::Crc<32> crc32(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true);

    std::array<wolv::u8, 5> data = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    crc32.process(data);
    TEST_ASSERT(crc32.getResult() == 1191942644);

    TEST_SUCCESS();
};

TEST_SEQUENCE("CRC_Reflect") {
    wolv::hash::Crc<32> crc32(0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, false, false);

    std::array<wolv::u8, 5> data = { 0x01, 0x02, 0x03, 0x04, 0x05 };

    crc32.process(data);
    TEST_ASSERT(crc32.getResult() == 493925500);

    TEST_SUCCESS();
};
