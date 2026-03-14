#pragma once
// soo_buffer.hpp
/*
 * Small Object Optimization Buffer
 *
 *  A buffer for POD types that keeps small allocations on the stack and
 *  transparently moves to heap storage when the size exceeds a configurable
 *  limit. During reallocation, the contents of the previous buffer may
 *  optionally be copied to the new one.
 */

#include <cstddef>
#include <type_traits>
#include <cstdlib>
#include <initializer_list>
		
namespace {

    template <typename T>
    struct  NewAlloc {
        static T* alloc(T* old, size_t sz) {
            delete [] old;
            return new T[sz];
        }

        static void free(T* p) {
            delete[] p;
        }

        static void copy(T* desc, T* src, size_t sz) {}
    };

    template <typename T>
    struct RealloceAlloc {
        static T* alloc(T* old, size_t sz) {
            return static_cast<T*>(realloc(old, sz*sizeof(T)));
        }

        static void free(T* p) {
            ::free(p);
        }

        static void copy(T* desc, T* src, size_t sz) {
            memcpy(desc, src, sz*sizeof(T));
        }
    };

} // namespace

namespace wolv::util {

    // T          - Type
    // SZ         - Size in Ts of stack buffer
    // UseRealloc - If true copies the contents of the old buffer to the new on reallocation
    template <typename T, size_t SZ, bool UseRealloc=false>
    class SOOBuffer {
        static_assert(std::is_trivially_copyable_v<T>&& std::is_standard_layout_v<T>,
            "SOOBuffer only supports trivial, standard-layout types");

        using Alloc = std::conditional_t<UseRealloc, RealloceAlloc<T>, NewAlloc<T>>;

    public:
        using element_type = T;
        static const size_t small_size_ = SZ;

        SOOBuffer(size_t cap = 0) {
            m_size = SZ;
            grow(cap);
        }

        SOOBuffer(const SOOBuffer&) = delete;
        SOOBuffer& operator=(const SOOBuffer&) = delete;
        SOOBuffer(SOOBuffer&&) = delete;
        SOOBuffer& operator=(SOOBuffer&&) = delete;

        ~SOOBuffer() {
            if (!isSmall())
                Alloc::free(m_heap);
        }

        T* data() {
            return isSmall() ? m_small : m_heap;
        }

        const T* data() const {
            return isSmall() ? m_small : m_heap;
        }

        operator T* () {
            return data();
        }

        operator const T* () const {
            return data();
        }

        T& operator[](size_t i) {
            return data()[i];
        }

        const T& operator[](size_t i) const {
            return data()[i];
        }

        size_t small_size() const {
            return small_size_;
        }

        bool isSmall() const {
            return m_size <= SZ;
        }

        size_t size() const {
            return m_size;
        }

        void grow(size_t sz) {
            if (sz > m_size) { // we never get smaller
               growBuffer(sz);
            }
        }

        void grow(size_t sz, std::initializer_list<T**> ptrs) {
            if (sz <= m_size) { // we never get smaller
                return;
            }
            
            T *pOldBase = data();
            growBuffer(sz);
            T *pNewBase = data();

            for (T** p : ptrs) {
                *p = (*p - pOldBase) + pNewBase;
            }
        }

    private:
        void growBuffer(size_t sz) {
            if (isSmall()) {
                T *pHeap = Alloc::alloc(NULL, sz);
                Alloc::copy(pHeap, m_small, SZ);
                m_heap = pHeap;
            }
            else {
                m_heap = Alloc::alloc(m_heap, sz);
            }
            m_size = sz;
        }

        size_t m_size = 0;

        union {
            T* m_heap;
            T m_small[SZ];
        };
    };

} // namespace wolv::util
