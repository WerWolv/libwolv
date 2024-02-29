#pragma once

#include <variant>
#include <string>
#include <concepts>

namespace wolv::util {

    template<typename E>
    class Unexpected {
    public:
        Unexpected(const Unexpected &) = default;
        Unexpected(Unexpected &&) noexcept = default;

        Unexpected(std::convertible_to<E> auto &&e) noexcept : m_value(std::move(e)) { }

    private:
        E m_value;

        template<typename, typename>
        friend class Expected;
    };

    template<typename E>
    Unexpected(E) -> Unexpected<E>;

    template<typename T, typename E>
    class Expected {
    public:
        using value_type = T;
        using error_type = E;
        using unexpected_type = wolv::util::Unexpected<E>;

        template<typename U>
        using rebind = Expected<U, error_type>;
    public:
        constexpr Expected() = default;

        constexpr Expected(const Expected &) = default;
        constexpr Expected(Expected &&) noexcept = default;

        constexpr Expected(const T &t)  : m_value(t) { }
        constexpr Expected(T &&t) noexcept : m_value(std::move(t)) { }

        constexpr Expected(const Unexpected<E> &e) : m_value(e.m_value) { }
        constexpr Expected(Unexpected<E> &&e) noexcept : m_value(std::move(e.m_value)) { }

        [[nodiscard]] constexpr bool has_value() const {
            return this->m_value.index() == 0;
        }

        [[nodiscard]] constexpr const T* operator->() const noexcept {
            return &std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr T* operator->() noexcept {
            return &std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr const T& operator*() const & noexcept {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr T& operator*() & noexcept {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr const T&& operator*() const && noexcept {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr T&& operator*() && noexcept {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr T& value() & {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr const T& value() const & {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr T& value() && {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr const T&& value() const && {
            return std::get<0>(this->m_value);
        }

        [[nodiscard]] constexpr E& error() & {
            return std::get<1>(this->m_value);
        }

        [[nodiscard]] constexpr const E& error() const & {
            return std::get<1>(this->m_value);
        }

        [[nodiscard]] constexpr E& error() && {
            return std::get<1>(this->m_value);
        }

        [[nodiscard]] constexpr const E&& error() const && {
            return std::get<1>(this->m_value);
        }

        [[nodiscard]] constexpr operator bool() const {
            return this->has_value();
        }

        [[nodiscard]] constexpr T value_or(auto &&defaultValue) const & {
            if (this->has_value())
                return this->value();
            else
                return defaultValue;
        }

        [[nodiscard]] T value_or(auto &&defaultValue) && {
            if (this->has_value())
                return this->value();
            else
                return defaultValue;
        }

        [[nodiscard]] constexpr bool operator==(const Expected &other) const {
            return this->m_value == other.m_value;
        }

        [[nodiscard]] constexpr bool operator==(const T &other) const {
            return this->value() == other;
        }

        [[nodiscard]] constexpr bool operator==(const Unexpected<E> &other) const {
            return this->error() == other.m_value;
        }

        [[nodiscard]] constexpr bool operator==(const E &other) const {
            return this->error() == other;
        }

        constexpr Expected& operator=(const Expected &) = default;
        constexpr Expected& operator=(Expected &&) noexcept = default;

        constexpr Expected& operator=(const T &t) {
            this->m_value = t;
            return *this;
        }

        constexpr Expected& operator=(T &&t) noexcept {
            this->m_value = std::move(t);
            return *this;
        }

        template<typename U>
        constexpr Expected& operator=(const Unexpected<U> &e) {
            this->m_value = e.m_value;
            return *this;
        }

        template<typename U>
        constexpr Expected& operator=(Unexpected<U> &&e) noexcept {
            this->m_value = std::move(e.m_value);
            return *this;
        }

    private:
        std::variant<T, E> m_value;
    };

}