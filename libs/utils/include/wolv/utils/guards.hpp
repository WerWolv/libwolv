#pragma once

#include <utility>

#include <wolv/utils/preproc.hpp>

namespace wolv::util {

    namespace scope_guard {

        #define SCOPE_GUARD   ::wolv::util::scope_guard::ScopeGuardOnExit() + [&]() -> void
        #define ON_SCOPE_EXIT [[maybe_unused]] auto WOLV_ANONYMOUS_VARIABLE(SCOPE_EXIT_) = SCOPE_GUARD

        template<class F>
        class ScopeGuard {
        private:
            F m_func;
            bool m_active;

        public:
            explicit constexpr ScopeGuard(F func) : m_func(std::move(func)), m_active(true) { }
            ~ScopeGuard() noexcept(false) {
                if (this->m_active) { this->m_func(); }
            }

            void release() { this->m_active = false; }

            ScopeGuard(ScopeGuard &&other) noexcept : m_func(std::move(other.m_func)), m_active(other.m_active) {
                other.release();
            }

            ScopeGuard &operator=(ScopeGuard &&) = delete;
        };

        enum class ScopeGuardOnExit { };

        template<typename F>
        constexpr ScopeGuard<F> operator+(ScopeGuardOnExit, F &&f) {
            return ScopeGuard<F>(std::forward<F>(f));
        }

    }

    namespace first_time_exec {

        #define AT_FIRST_TIME [[maybe_unused]] static auto WOLV_ANONYMOUS_VARIABLE(FIRST_TIME_) = ::wolv::util::first_time_exec::FirstTimeExecutor() + [&]()

        template<class F>
        class FirstTimeExecute {
        public:
            explicit constexpr FirstTimeExecute(F func) { func(); }

            FirstTimeExecute &operator=(FirstTimeExecute &&) = delete;
        };

        enum class FirstTimeExecutor { };

        template<typename F>
        constexpr FirstTimeExecute<F> operator+(FirstTimeExecutor, F &&f) {
            return FirstTimeExecute<F>(std::forward<F>(f));
        }

    }

    namespace final_cleanup {

        #define AT_FINAL_CLEANUP [[maybe_unused]] static auto WOLV_ANONYMOUS_VARIABLE(ON_EXIT_) = ::wolv::util::final_cleanup::FinalCleanupExecutor() + [&]()

        template<class F>
        class FinalCleanupExecute {
            F m_func;

        public:
            explicit constexpr FinalCleanupExecute(F func) : m_func(func) { }
            constexpr ~FinalCleanupExecute() { this->m_func(); }

            FinalCleanupExecute &operator=(FinalCleanupExecute &&) = delete;
        };

        enum class FinalCleanupExecutor { };

        template<typename F>
        constexpr FinalCleanupExecute<F> operator+(FinalCleanupExecutor, F &&f) {
            return FinalCleanupExecute<F>(std::forward<F>(f));
        }

    }

}