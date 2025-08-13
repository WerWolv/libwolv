#pragma once

namespace wolv::container {

#include <vector>
#include <stdexcept>
#include <cstddef>

    template<typename T>
    class RingBuffer {
    public:
        explicit RingBuffer(std::size_t capacity)
            : m_capacity(capacity),
              m_buffer(capacity)
        {
            if (capacity == 0) {
                throw std::invalid_argument("Capacity must be greater than zero");
            }
        }

        bool push(const T& item) {
            if (full()) {
                m_tail = (m_tail + 1) % m_capacity; // overwrite oldest
            }
            m_buffer[m_head] = item;
            m_head = (m_head + 1) % m_capacity;
            if (m_size < m_capacity) ++m_size;
            return true;
        }

        bool push(T&& item) {
            if (full()) {
                m_tail = (m_tail + 1) % m_capacity;
            }
            m_buffer[m_head] = std::move(item);
            m_head = (m_head + 1) % m_capacity;
            if (m_size < m_capacity) ++m_size;
            return true;
        }

        bool pop(T& out) {
            if (empty()) return false;
            out = std::move(m_buffer[m_tail]);
            m_tail = (m_tail + 1) % m_capacity;
            --m_size;
            return true;
        }

        bool empty() const noexcept {
            return m_size == 0;
        }

        bool full() const noexcept {
            return m_size == m_capacity;
        }

        std::size_t size() const noexcept {
            return m_size;
        }

        std::size_t capacity() const noexcept {
            return m_capacity;
        }

        void clear() noexcept {
            m_head = m_tail = m_size = 0;
        }

    T& front() {
        if (empty()) throw std::out_of_range("RingBuffer is empty");
        return m_buffer[m_tail];
    }

    const T& front() const {
        if (empty()) throw std::out_of_range("RingBuffer is empty");
        return m_buffer[m_tail];
    }

    T& back() {
        if (empty()) throw std::out_of_range("RingBuffer is empty");
        return m_buffer[(m_head + m_capacity - 1) % m_capacity];
    }

    const T& back() const {
        if (empty()) throw std::out_of_range("RingBuffer is empty");
        return m_buffer[(m_head + m_capacity - 1) % m_capacity];
    }

    // ---- Iterator support ----
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        iterator(RingBuffer* buf, std::size_t pos, std::size_t count)
            : m_buf(buf), m_pos(pos), m_remaining(count) {}

        reference operator*() { return m_buf->m_buffer[m_pos]; }
        pointer operator->() { return &m_buf->m_buffer[m_pos]; }

        iterator& operator++() {
            m_pos = (m_pos + 1) % m_buf->m_capacity;
            --m_remaining;
            return *this;
        }

        bool operator==(const iterator& other) const {
            return m_remaining == other.m_remaining;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        RingBuffer* m_buf;
        std::size_t m_pos;
        std::size_t m_remaining;
    };

    iterator begin() { return iterator(this, m_tail, m_size); }
    iterator end() { return iterator(this, 0, 0); }

    // ---- Const iterator ----
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        const_iterator(const RingBuffer* buf, std::size_t pos, std::size_t count)
            : m_buf(buf), m_pos(pos), m_remaining(count) {}

        reference operator*() const { return m_buf->m_buffer[m_pos]; }
        pointer operator->() const { return &m_buf->m_buffer[m_pos]; }

        const_iterator& operator++() {
            m_pos = (m_pos + 1) % m_buf->m_capacity;
            --m_remaining;
            return *this;
        }

        bool operator==(const const_iterator& other) const {
            return m_remaining == other.m_remaining;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        const RingBuffer* m_buf;
        std::size_t m_pos;
        std::size_t m_remaining;
    };

    const_iterator begin() const { return const_iterator(this, m_tail, m_size); }
    const_iterator end() const { return const_iterator(this, 0, 0); }

    private:
        std::size_t m_capacity;
        std::vector<T> m_buffer;
        std::size_t m_head = 0;
        std::size_t m_tail = 0;
        std::size_t m_size = 0;
    };


}