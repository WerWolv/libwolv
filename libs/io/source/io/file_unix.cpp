#include <wolv/io/file.hpp>
#include <wolv/utils/guards.hpp>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#if defined(OS_MACOS)
    #include <sys/types.h>
    #include <sys/event.h>
#elif defined(OS_LINUX)
    #include <sys/types.h>
    #include <sys/inotify.h>
    #include <poll.h>
#endif

namespace wolv::io {

    File::File(const std::fs::path &path, Mode mode) noexcept : m_path(path), m_mode(mode) {
        this->open();
    }


    File::File(File &&other) noexcept {
        m_handle = other.m_handle;
        other.m_handle = -1;

        m_fileHandle = other.m_fileHandle;
        other.m_fileHandle = nullptr;

        m_path = std::move(other.m_path);
        m_mode = other.m_mode;
        m_fileSize = other.m_fileSize;
        m_openError = std::move(other.m_openError);
    }

    File::~File() {
        this->unmap();
        this->close();
    }

    File &File::operator=(File &&other) noexcept {
        m_handle = other.m_handle;
        other.m_handle = -1;

        m_fileHandle = other.m_fileHandle;
        other.m_fileHandle = nullptr;

        m_path = std::move(other.m_path);
        m_mode = other.m_mode;
        m_fileSize = other.m_fileSize;
        m_openError = std::move(other.m_openError);

        return *this;
    }

    void File::seek(u64 offset) {
        lseek(m_handle, offset, SEEK_SET);
    }

    void File::open() {
        m_openError.reset();

        if (m_mode == Mode::Read)
            m_handle = ::open(m_path.c_str(), O_RDONLY);
        else if (m_mode == Mode::Write || m_mode == Mode::Create)
            m_handle = ::open(m_path.c_str(), O_RDWR);

        if (m_mode == Mode::Create || (m_mode == Mode::Write && m_handle == -1))
            m_handle = ::open(m_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);

        if (m_handle < 0) {
            m_openError = errno;
        }

        this->updateSize();
    }

    void File::close() {
        if (isValid()) {
            ::close(m_handle);
            m_handle = -1;
        }

        if (m_fileHandle != nullptr) {
            fclose(m_fileHandle);
            m_fileHandle = nullptr;
        }
    }


    bool File::map() {
        m_openError.reset();

        if (!isValid())
            return false;

        m_map = static_cast<u8*>(mmap(nullptr, m_fileSize, m_mode == Mode::Read ? PROT_READ : PROT_READ | PROT_WRITE, MAP_SHARED, m_handle, 0));
        if (m_map == reinterpret_cast<void*>(-1)) {
            m_openError = errno;
        }
        return true;
    }

    void File::unmap() {
        if (m_map == nullptr)
            return;

        munmap(m_map, m_fileSize);

        m_map = nullptr;
    }

    size_t File::readBuffer(u8 *buffer, size_t size) {
        if (!isValid())
            return 0;

        return read(m_handle, buffer, size);
    }

    size_t File::readBufferAtomic(u64 address, u8 *buffer, size_t size) {
        if (!isValid())
            return 0;

        return pread(m_handle, buffer, size, address);
    }


    size_t File::writeBuffer(const u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        m_sizeValid = false;
        return write(m_handle, buffer, size);
    }

    size_t File::writeBufferAtomic(u64 address, const u8 *buffer, size_t size) {
        if (!isValid())
            return 0;

        m_sizeValid = false;
        return pwrite(m_handle, buffer, size, address);
    }

    void File::setSize(u64 size) {
        if (!isValid())
            return;

        if (ftruncate(m_handle, size) < 0) {
            // Handle error, although there's really nothing to handle.
        }
    }

    void File::updateSize() const {
        if (!isValid()) {
            m_fileSize = 0;
            return;
        }

        const auto currOffset = lseek(m_handle, 0, SEEK_CUR);
        m_fileSize = lseek(m_handle, 0, SEEK_END);
        lseek(m_handle, currOffset, SEEK_SET);
        m_sizeValid = true;
    }

    bool File::flush() {
        return fsync(m_handle) == 0;
        // TODO handle error message
    }

    void File::disableBuffering() {

    }

    std::optional<struct stat> File::getFileInfo() {
        struct stat fileInfo = { };

        if (stat(this->m_path.c_str(), &fileInfo) != 0)
            return std::nullopt;

        return fileInfo;
    }

    FILE* File::getHandle() const {
        if (m_fileHandle != nullptr)
            return m_fileHandle;

        m_fileHandle = fdopen(m_handle, m_mode == Mode::Read ? "rb" : "wb");

        return m_fileHandle;
    }

    bool File::isValid() const {
        return m_handle != -1;
    }


    #if defined(OS_MACOS)
        void ChangeTracker::trackImpl(const bool &stopped, const std::fs::path &path, const std::function<void()> &callback) {
            int queue = kqueue();
            if (queue == -1)
                throw std::runtime_error("Failed to open kqueue");

            ON_SCOPE_EXIT { close(queue); };

            int fileDescriptor = ::open(path.c_str(), O_RDONLY);
            if (fileDescriptor == -1)
                throw std::runtime_error("Failed to open file descriptor");

            ON_SCOPE_EXIT { close(fileDescriptor); };

            struct kevent eventHandle = { };
            EV_SET(&eventHandle, fileDescriptor, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE, 0, nullptr);
            if (kevent(queue, &eventHandle, 1, nullptr, 0, nullptr) == -1)
                throw std::runtime_error("Failed to add event to kqueue");

            const timespec timeout = { 1, 0 };
            while (!stopped) {
                struct kevent eventList[1] = {};
                int eventCount = kevent(queue, nullptr, 0, eventList, 1, &timeout);
                if (eventCount <= 0)
                    continue;

                if (eventList[0].fflags & NOTE_WRITE) {
                    callback();
                }
            }

        }
    #elif defined(OS_LINUX)
        void ChangeTracker::trackImpl(const bool &stopped, const std::fs::path &path, const std::function<void()> &callback) {
            int fileDescriptor = inotify_init();
            if (fileDescriptor == -1)
                throw std::runtime_error("Failed to open inotify");

            ON_SCOPE_EXIT { close(fileDescriptor); };

            int watchDescriptor = inotify_add_watch(fileDescriptor, path.c_str(), IN_MODIFY);
            if (watchDescriptor == -1)
                throw std::runtime_error("Failed to add watch");

            ON_SCOPE_EXIT { inotify_rm_watch(fileDescriptor, watchDescriptor); };

            std::array<char, 4096> buffer;
            pollfd pollDescriptor = { fileDescriptor, POLLIN, 0 };

            while (!stopped) {
                if (poll(&pollDescriptor, 1, 1000) <= 0)
                    continue;

                ssize_t bytesRead = read(fileDescriptor, buffer.data(), buffer.size());
                if (bytesRead == -1)
                    continue;

                for (char *ptr = buffer.data(); ptr < buffer.data() + bytesRead;) {
                    auto *event = reinterpret_cast<inotify_event *>(ptr);
                    if (event->mask & IN_MODIFY) {
                        callback();
                    }

                    ptr += sizeof(inotify_event) + event->len;
                }
            }

        }
    #else
        void ChangeTracker::trackImpl(const bool &, const std::fs::path &, const std::function<void()> &) {}
    #endif

}