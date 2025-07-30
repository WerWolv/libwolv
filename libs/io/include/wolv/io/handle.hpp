#pragma once

namespace wolv::io {

    class NativeHandle {
    public:
        NativeHandle();
        #if defined(OS_WINDOWS)
            NativeHandle(void *handle);
            operator void*() const;
            NativeHandle operator=(void *handle);
        #else
            NativeHandle(int handle);
            operator int() const;
            NativeHandle operator=(int handle);
        #endif

    private:
        #if defined(OS_WINDOWS)
            void *m_handle;
        #else
            int m_handle;
        #endif
    };

}