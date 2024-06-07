#pragma once

#include <cstring>

#include <vector>

namespace wolv::io {

    template<typename T>
    using ReaderFunction = void(*)(T *userData, void *buffer, u64 address, size_t size);

    template<typename T, ReaderFunction<T> Reader>
    class BufferedReader {
    public:
        explicit BufferedReader(T *userData, size_t dataSize, size_t bufferSize = 0x100000)
                : m_userData(userData), m_bufferAddress(0x00), m_maxBufferSize(bufferSize),
                  m_startAddress(0x00), m_endAddress(std::max<size_t>(dataSize, 1) - 1LLU),
                  m_buffer(bufferSize) {

        }

        void seek(u64 address) {
            this->m_startAddress = address;
        }

        void setEndAddress(u64 address) {
            this->m_endAddress = address;
        }

        u64 getStartAddress() const {
            return this->m_startAddress;
        }

        u64 getEndAddress() const {
            return this->m_endAddress;
        }

        [[nodiscard]] std::vector<u8> read(u64 address, size_t size) {
            std::vector<u8> result;
            result.resize(size);
            this->read(address, result.data(), result.size());

            return result;
        }

        [[nodiscard]] std::vector<u8> readReverse(u64 address, size_t size) {
            std::vector<u8> result;
            result.resize(size);
            this->readReverse(address, result.data(), result.size());

            return result;
        }

        void read(u64 address, u8 *buffer, size_t size) {
            //Bypass m_buffer if necessary
            if (size > this->m_maxBufferSize) {
                Reader(this->m_userData, buffer, address, size);
                return;
            }

            if (this->updateBuffer(address, size, false) == 0)
                return;
            
            auto result = &this->m_buffer[address -  this->m_bufferAddress];

            std::memcpy(buffer, result, std::min<size_t>(size, this->m_buffer.size()));
        }

        void readReverse(u64 address, u8 *buffer, size_t size) {
            //Bypass m_buffer if necessary
            if (size > this->m_maxBufferSize) {
                Reader(this->m_userData, buffer, address, size);
                return;
            }

            if (this->updateBuffer(address, size, true) == 0)
                return;

            auto result = &this->m_buffer[address - this->m_bufferAddress];

            std::memcpy(buffer, result, std::min<size_t>(size, this->m_buffer.size() - (address - this->m_bufferAddress)));
        }

        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = u8;
            using pointer           = const value_type*;
            using reference         = const value_type&;

            Iterator(BufferedReader *reader, u64 address) : m_reader(reader), m_address(address) {}

            Iterator& operator++() {
                ++this->m_address;

                return *this;
            }

            Iterator operator++(int) {
                auto copy = *this;
                ++this->m_address;

                return copy;
            }

            Iterator& operator--() {
                --this->m_address;

                return *this;
            }

            Iterator operator--(int) {
                auto copy = *this;
                --this->m_address;

                return copy;
            }

            Iterator& operator+=(i64 offset) {
                this->m_address += offset;

                return *this;
            }

            Iterator& operator-=(i64 offset) {
                this->m_address -= offset;

                return *this;
            }

            value_type operator*() const {
                return (*this)[0];
            }

            [[nodiscard]] u64 getAddress() const {
                return this->m_address;
            }

            void setAddress(u64 address) {
                this->m_address = address;
            }

            difference_type operator-(const Iterator &other) const {
                return this->m_address - other.m_address;
            }

            Iterator operator-(i64 offset) const {
                return { this->m_reader, this->m_address - offset };
            }

            Iterator operator+(i64 offset) const {
                return { this->m_reader, this->m_address + offset };
            }

            value_type operator[](i64 offset) const {
                value_type value = 0x00;
                this->m_reader->read(this->m_address + offset, &value, sizeof(value));

                return value;
            }

            friend bool operator== (const Iterator& left, const Iterator& right) { return left.m_address == right.m_address; };
            friend bool operator!= (const Iterator& left, const Iterator& right) { return left.m_address != right.m_address; };
            friend bool operator>  (const Iterator& left, const Iterator& right) { return left.m_address >  right.m_address; };
            friend bool operator<  (const Iterator& left, const Iterator& right) { return left.m_address <  right.m_address; };
            friend bool operator>= (const Iterator& left, const Iterator& right) { return left.m_address >= right.m_address; };
            friend bool operator<= (const Iterator& left, const Iterator& right) { return left.m_address <= right.m_address; };

        private:
            BufferedReader *m_reader;
            u64 m_address;
        };

        class ReverseIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = u8;
            using pointer           = const value_type*;
            using reference         = const value_type&;

            ReverseIterator(BufferedReader *reader, u64 address) : m_reader(reader), m_address(address) {}

            ReverseIterator& operator++() {
                --this->m_address;

                return *this;
            }

            ReverseIterator operator++(int) {
                auto copy = *this;
                --this->m_address;

                return copy;
            }

            ReverseIterator& operator--() {
                ++this->m_address;

                return *this;
            }

            ReverseIterator operator--(int) {
                auto copy = *this;
                ++this->m_address;

                return copy;
            }

            ReverseIterator& operator+=(i64 offset) {
                this->m_address -= offset;

                return *this;
            }

            ReverseIterator& operator-=(i64 offset) {
                this->m_address += offset;

                return *this;
            }

            value_type operator*() const {
                return (*this)[0];
            }

            [[nodiscard]] u64 getAddress() const {
                return this->m_address;
            }

            void setAddress(u64 address) {
                this->m_address = address;
            }

            difference_type operator-(const ReverseIterator &other) const {
                return other.m_address - this->m_address;
            }

            ReverseIterator operator-(i64 offset) const {
                return { this->m_reader, this->m_address + offset };
            }

            ReverseIterator operator+(i64 offset) const {
                return { this->m_reader, this->m_address - offset };
            }

            value_type operator[](i64 offset) const {
                auto result = this->m_reader->readReverse(this->m_address - offset, 1);
                if (result.empty())
                    return 0x00;

                return result[0];
            }

            friend bool operator== (const ReverseIterator& left, const ReverseIterator& right) { return left.m_address == right.m_address; }
            friend bool operator!= (const ReverseIterator& left, const ReverseIterator& right) { return left.m_address != right.m_address; }
            friend bool operator>  (const ReverseIterator& left, const ReverseIterator& right) { return left.m_address >  right.m_address; }
            friend bool operator<  (const ReverseIterator& left, const ReverseIterator& right) { return left.m_address <  right.m_address; }
            friend bool operator>= (const ReverseIterator& left, const ReverseIterator& right) { return left.m_address >= right.m_address; }
            friend bool operator<= (const ReverseIterator& left, const ReverseIterator& right) { return left.m_address <= right.m_address; }

        private:
            BufferedReader *m_reader;
            u64 m_address = 0x00;
        };

        Iterator begin() {
            return { this, this->m_startAddress };
        }

        Iterator end() {
            return { this, this->m_endAddress + 1 };
        }

        ReverseIterator rbegin() {
            return { this, this->m_endAddress };
        }

        ReverseIterator rend() {
            return { this, this->m_startAddress - 1 };
        }

    private:
        size_t updateBuffer(u64 address, size_t size, bool reverse) {
            if (!this->m_bufferValid || address < this->m_bufferAddress || address + size > (this->m_bufferAddress + this->m_buffer.size())) {
                u64 addressStart, addressEndPlus1;
                if (reverse) {
                    addressEndPlus1 = address + size;
                    if (addressEndPlus1 > this->m_endAddress + 1U)
                        addressEndPlus1 = this->m_endAddress + 1U;
                    addressStart = addressEndPlus1 - this->m_maxBufferSize;
                    if (addressEndPlus1 - this->m_startAddress < this->m_maxBufferSize)
                        addressStart = this->m_startAddress;
                } else {
                    addressEndPlus1 = address + this->m_maxBufferSize;
                    if (addressEndPlus1 > this->m_endAddress + 1U)
                        addressEndPlus1 = this->m_endAddress + 1U;
                    addressStart = address;
                    if (addressStart < this->m_startAddress)
                        addressStart = this->m_startAddress;
                }

                if (addressStart <= address && address < addressEndPlus1) {
                    const auto remainingBytes = addressEndPlus1 - addressStart;
                    this->m_buffer.resize(remainingBytes);

                    Reader(this->m_userData, this->m_buffer.data(), addressStart, remainingBytes);
                    this->m_bufferAddress = addressStart;
                    this->m_bufferValid = true;
                    return remainingBytes;
                }

                //Nothing can be read
                return 0;
            }

            return size;
        }

    private:
        T *m_userData;

        u64 m_bufferAddress = 0x00;
        size_t m_maxBufferSize;
        bool m_bufferValid = false;
        u64 m_startAddress = 0x00, m_endAddress;
        std::vector<u8> m_buffer;
    };

}
