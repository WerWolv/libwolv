#pragma once

#include <functional>
#include <memory>
#include <mutex>

namespace wolv::container {

    template <typename T>
    class Lazy {
    public:
        // Constructor that accepts a function to initialize the value
        explicit Lazy(std::function<T()> initializer)
            : initializer_(std::move(initializer)) {}

        // Disable copying and assignment
        Lazy(const Lazy&) = delete;
        Lazy& operator=(const Lazy&) = delete;

        Lazy(Lazy &&other) noexcept
            : initializer_(std::move(other.initializer_)),
              value_(std::move(other.value_)),
              initFlag_() {
            other.value_ = nullptr; // Ensure the moved-from object has no value
        }

        Lazy& operator=(Lazy&& other) noexcept {
            if (this != &other) {
                initializer_ = std::move(other.initializer_);
                value_ = std::move(other.value_);
                // initFlag_ cannot be moved or reset, so create a new Lazy instead if needed
            }
            return *this;
        }

        // Access the lazily initialized value
        T& get() & {
            std::call_once(initFlag_, [this]() {
                value_ = std::make_unique<T>(initializer_());
            });
            return *value_;
        }

        T&& get() && {
            std::call_once(initFlag_, [this]() {
                value_ = std::make_unique<T>(initializer_());
            });

            return std::move(*value_);
        }

        // Check if the value has been initialized
        bool isInitialized() const {
            return value_ != nullptr;
        }

    private:
        std::function<T()> initializer_;
        std::unique_ptr<T> value_;
        mutable std::once_flag initFlag_;
    };

}