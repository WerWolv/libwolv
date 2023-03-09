#pragma once

#include <wolv/types.hpp>

#include <array>
#include <bit>
#include <span>

namespace wolv::hash {

    template<size_t NumBits> requires (std::has_single_bit(NumBits))
    class Crc {
    public:
        constexpr Crc(u64 polynomial, u64 init, u64 xorOut, bool reflectInput, bool reflectOutput)
                : m_value(0x00), m_init(init & ((0b10ULL << (NumBits - 1)) - 1)), m_xorOut(xorOut & ((0b10ULL << (NumBits - 1)) - 1)),
                  m_reflectInput(reflectInput), m_reflectOutput(reflectOutput),
                  m_table([polynomial]() {
                      auto reflectedPoly = reflect(polynomial & ((0b10ULL << (NumBits - 1)) - 1), NumBits);
                      std::array<u64, 256> table = { 0 };

                      for (u32 i = 0; i < 256; i++) {

                          u64 c = i;
                          for (size_t j = 0; j < 8; j++) {
                              if (c & 0b01)
                                  c = reflectedPoly ^ (c >> 1);
                              else
                                  c >>= 1;
                          }

                          table[i] = c;
                      }

                      return table;
                  }())
        {
            this->reset();
        }

        constexpr void reset() {
            this->m_value = reflect(this->m_init, NumBits);
        }

        constexpr void process(std::span<const u8> bytes) {
            for (u8 byte : bytes) {
                if (!this->m_reflectInput)
                    byte = reflect(byte);

                this->m_value = this->m_table[(this->m_value ^ byte) & 0xFFULL] ^ (this->m_value >> 8);
            }
        }

        constexpr void process(auto begin, auto end) {
            this->process({ begin, end });
        }

        [[nodiscard]]
        constexpr u64 getResult() const {
            if (this->m_reflectOutput)
                return this->m_value ^ this->m_xorOut;
            else
                return reflect(this->m_value, NumBits) ^ this->m_xorOut;
        }

    private:
        template<typename T>
        constexpr static T reflect(T in, size_t bits) {
            T out = { };
            for (size_t i = 0; i < bits; i++) {
                out <<= 1;

                if (in & 0b0000'0001)
                    out |= 1;

                in >>= 1;
            }

            return out;
        }

        template<typename T>
        constexpr static T reflect(T in) {
            if constexpr (sizeof(T) == 1) {
                T out = in;

                out = ((out & 0xF0) >> 4) | ((out & 0x0F) << 4);
                out = ((out & 0xCC) >> 2) | ((out & 0x33) << 2);
                out = ((out & 0xAA) >> 1) | ((out & 0x55) << 1);

                return out;
            } else {
                return reflect(in, sizeof(T) * 8);
            }
        }

    private:
        u64 m_value;

        u64 m_init;
        u64 m_xorOut;
        bool m_reflectInput;
        bool m_reflectOutput;

        std::array<u64, 256> m_table;
    };

}