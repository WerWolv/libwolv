#include <wolv/io/handle.hpp>

#if defined(OS_WINDOWS)
    #include <windows.h>
#endif

namespace wolv::io {

    #if defined(OS_WINDOWS)
        NativeHandle::NativeHandle() : m_handle(INVALID_HANDLE_VALUE) {

        }

        NativeHandle::NativeHandle(void* handle) : m_handle(handle) {

        }

        NativeHandle::operator void*() const {
            return m_handle;
        }

        NativeHandle NativeHandle::operator=(void *handle) {
            m_handle = handle;
            return *this;
        }

    #else
        NativeHandle::NativeHandle() : m_handle(-1) { }

        NativeHandle::NativeHandle(int handle) : m_handle(handle) {

        }

        NativeHandle::operator int() const {
            return m_handle;
        }

        NativeHandle NativeHandle::operator=(int handle) {
            m_handle = handle;
            return *this;
        }
    #endif


}