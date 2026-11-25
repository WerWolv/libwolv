#include <wolv/io/file.hpp>
#include <wolv/utils/guards.hpp>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#if defined(OS_MACOS) || defined(OS_FREEBSD)
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

        #if defined(OS_FREEBSD)
            m_map = static_cast<u8*>(mmap(nullptr, m_fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_handle, 0));
        #else
            m_map = static_cast<u8*>(mmap(nullptr, m_fileSize, m_mode == Mode::Read ? PROT_READ : PROT_READ | PROT_WRITE, MAP_SHARED, m_handle, 0));
        #endif
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

    File::Result File::readBuffer(u8 *buffer, size_t size) {
        if (!isValid())
            return -1;

        return read(m_handle, buffer, size);
    }

    File::Result File::readBufferAtomic(u64 address, u8 *buffer, size_t size) {
        if (!isValid())
            return -1;

        ssize_t acc = 0;
        while (acc < size) {
            const auto bytes = pread(m_handle, buffer, size - acc, address);
            if (bytes <= 0) {
                break;
            }

            acc += bytes;
            address += bytes;
            buffer += bytes;
        }
        return acc;
    }


    File::Result File::writeBuffer(const u8 *buffer, size_t size) {
        if (!isValid())
            return -1;

        m_sizeValid = false;
        return write(m_handle, buffer, size);
    }

    File::Result File::writeBufferAtomic(u64 address, const u8 *buffer, size_t size) {
        if (!isValid())
            return -1;

        m_sizeValid = false;
        ssize_t acc = 0;
        while (acc < size) {
            const auto bytes = pwrite(m_handle, buffer, size - acc, address);
            if (bytes <= 0) {
                break;
            }

            acc += bytes;
            address += bytes;
            buffer += bytes;
        }
        return acc;
    }

    bool File::setSize(u64 size) {
        if (!isValid())
            return false;

        return ftruncate(m_handle, size) == 0;
    }

    void File::updateSize() const {
        if (!isValid()) {
            m_fileSize = 0;
            return;
        }

        const auto currOffset = lseek(m_handle, 0, SEEK_CUR);
        auto result = lseek(m_handle, 0, SEEK_END);
        if (result < 0) {
            m_fileSize = 0;
            return;
        }

        m_fileSize = result;

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


    #if defined(OS_MACOS) || defined(OS_FREEBSD)
        void ChangeTracker::trackImpl(std::stop_token st, const std::fs::path &path, const std::function<void()> &callback) {
            const auto parentPath = path.has_parent_path() ? path.parent_path() : std::fs::path(".");
            int queue = kqueue();
            if (queue == -1)
                throw std::runtime_error("Failed to open kqueue");

            ON_SCOPE_EXIT { close(queue); };

            int fileDescriptor = ::open(parentPath.c_str(), O_RDONLY);
            if (fileDescriptor == -1)
                throw std::runtime_error("Failed to open file descriptor");

            ON_SCOPE_EXIT { close(fileDescriptor); };

            // Keep the directory watch for entry changes such as file replacement.
            struct kevent eventHandle = { };
            EV_SET(&eventHandle, fileDescriptor, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE | NOTE_DELETE | NOTE_RENAME, 0, nullptr);
            if (kevent(queue, &eventHandle, 1, nullptr, 0, nullptr) == -1)
                throw std::runtime_error("Failed to add event to kqueue");

            int watchedDescriptor = -1;
            ON_SCOPE_EXIT {
                if (watchedDescriptor != -1)
                    close(watchedDescriptor);
            };
            constexpr auto detachedNotes = NOTE_DELETE | NOTE_RENAME | NOTE_REVOKE;

            const timespec timeout = { 0, 0 };
            StoppableSleep sleeper(st);
            while (true) {
                // Hack: kqueue doesn't report writes to child files through the directory watch.
                // Watch the file directly, reopening it by path when it is replaced.
                if (watchedDescriptor == -1) {
                    watchedDescriptor = ::open(path.c_str(), O_RDONLY | O_NONBLOCK);
                    if (watchedDescriptor != -1) {
                        EV_SET(&eventHandle, watchedDescriptor, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE | NOTE_EXTEND | detachedNotes, 0, nullptr);
                        if (kevent(queue, &eventHandle, 1, nullptr, 0, nullptr) == -1)
                            throw std::runtime_error("Failed to add event to kqueue");
                    }
                }

                if (sleeper.sleep(1000))
                    break;

                struct kevent eventList[2] = {};
                int eventCount = kevent(queue, nullptr, 0, eventList, 2, &timeout);
                if (eventCount <= 0)
                    continue;

                for (int i = 0; i < eventCount; ++i) {
                    const auto &event = eventList[i];
                    if (watchedDescriptor != -1 && event.ident == static_cast<uintptr_t>(watchedDescriptor) && (event.fflags & detachedNotes)) {
                        // The descriptor follows the old file, not the original path.
                        close(watchedDescriptor);
                        watchedDescriptor = -1;
                    }
                }

                callback();
            }

        }
    #elif defined(OS_LINUX)
        void ChangeTracker::trackImpl(std::stop_token st, const std::fs::path &path, const std::function<void()> &callback) {
            const auto parentPath = path.has_parent_path() ? path.parent_path() : std::fs::path(".");
            int fileDescriptor = inotify_init();
            if (fileDescriptor == -1)
                throw std::runtime_error("Failed to open inotify");

            ON_SCOPE_EXIT { close(fileDescriptor); };

            int watchDescriptor = inotify_add_watch(fileDescriptor, parentPath.c_str(),
                IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
            if (watchDescriptor == -1)
                throw std::runtime_error("Failed to add watch");

            ON_SCOPE_EXIT { inotify_rm_watch(fileDescriptor, watchDescriptor); };

            std::array<char, 4096> buffer;
            pollfd pollDescriptor = { fileDescriptor, POLLIN, 0 };

            StoppableSleep sleeper(st);
            while (true) {
                if (sleeper.sleep(1000)) {
					break;
				}
    
                if (poll(&pollDescriptor, 1, 0) <= 0)
                    continue;

                ssize_t bytesRead = read(fileDescriptor, buffer.data(), buffer.size());
                if (bytesRead == -1)
                    continue;

                for (char *ptr = buffer.data(); ptr < buffer.data() + bytesRead;) {
                    auto *event = reinterpret_cast<inotify_event *>(ptr);
                    if (event->len > 0 && path.filename() == event->name) {
                        if (event->mask & (IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO))
                            callback();
                    }

                    ptr += sizeof(inotify_event) + event->len;
                }
            }

        }
    #else
        void ChangeTracker::trackImpl(std::stop_token st, const std::fs::path &, const std::function<void()> &) {}
    #endif

}
