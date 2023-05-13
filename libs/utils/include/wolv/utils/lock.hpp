#pragma once

#include <wolv/types.hpp>

#include <mutex>

#include <wolv/utils/preproc.hpp>

#define TRY_LOCK(mutex) [[maybe_unused]] auto WOLV_ANONYMOUS_VARIABLE(TRY_LOCK_) = ::wolv::util::ScopedTryLock(mutex)

namespace wolv::util {

    class ScopedTryLock {
    public:
        explicit ScopedTryLock(std::mutex &mutex) : m_mutex(&mutex) {
            this->m_locked = this->m_mutex->try_lock();
        }

        ScopedTryLock(const ScopedTryLock &) = delete;
        ScopedTryLock &operator=(const ScopedTryLock &) = delete;

        ScopedTryLock(ScopedTryLock &&other) noexcept : m_mutex(other.m_mutex) {
            other.m_mutex = nullptr;
            this->m_locked = other.m_locked;
        }

        ~ScopedTryLock() {
            if (this->m_locked && this->m_mutex != nullptr) {
                this->m_mutex->unlock();
            }
        }

        operator bool() const {
            return this->m_locked && this->m_mutex != nullptr;
        }
    private:
        std::mutex *m_mutex;
        bool m_locked;
    };

}