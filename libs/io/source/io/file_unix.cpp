#if defined(OS_WEB)
    #include <sys/mman.h>
    #include <cstdio>

    #define fopen64 fopen
    #define ftruncate64 ftruncate
#elif defined(OS_MACOS)
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/event.h>
    #include <sys/mman.h>
    #include <fcntl.h>
#elif defined(OS_LINUX)
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/inotify.h>
    #include <sys/mman.h>
    #include <poll.h>
#endif

namespace wolv::io {

    File::File(const std::fs::path &path, Mode mode) noexcept : m_path(path), m_mode(mode) {
        if (mode == File::Mode::Read)
            this->m_file = fopen(util::toUTF8String(path).c_str(), "rb");
        else if (mode == File::Mode::Write)
            this->m_file = fopen(util::toUTF8String(path).c_str(), "r+b");

        if (mode == File::Mode::Create || (mode == File::Mode::Write && this->m_file == nullptr))
            this->m_file = fopen(util::toUTF8String(path).c_str(), "w+b");

        this->updateSize();
    }

}