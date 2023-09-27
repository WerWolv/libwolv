#include <wolv/io/file.hpp>

#include <wolv/utils/core.hpp>
#include <wolv/utils/string.hpp>
#include <wolv/utils/guards.hpp>

#include <unistd.h>

#if defined(OS_WEB)
    #include <sys/mman.h>
    #include <cstdio>

    #define fopen64 fopen
    #define fseeko64 fseek
    #define ftello64 ftell
    #define ftruncate64 ftruncate
#elif defined(OS_WINDOWS)
    #include <Windows.h>
    #include <share.h>
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
        #if defined(OS_WINDOWS)

            if (mode == File::Mode::Read)
                this->m_file = _wfsopen(path.c_str(), L"rb", _SH_DENYNO);
            else if (mode == File::Mode::Write)
                this->m_file = _wfsopen(path.c_str(), L"r+b", _SH_DENYNO);

            if (mode == File::Mode::Create || (mode == File::Mode::Write && this->m_file == nullptr))
                this->m_file = _wfsopen(path.c_str(), L"w+b", _SH_DENYNO);

        #else

            if (mode == File::Mode::Read)
                this->m_file = fopen(util::toUTF8String(path).c_str(), "rb");
            else if (mode == File::Mode::Write)
                this->m_file = fopen(util::toUTF8String(path).c_str(), "r+b");

            if (mode == File::Mode::Create || (mode == File::Mode::Write && this->m_file == nullptr))
                this->m_file = fopen(util::toUTF8String(path).c_str(), "w+b");

        #endif

        this->updateSize();
    }

    File::File(File &&other) noexcept {
        this->m_file = other.m_file;
        other.m_file = nullptr;

        this->m_path = std::move(other.m_path);
        this->m_mode = other.m_mode;
        this->m_fileSize = other.m_fileSize;
    }

    File::~File() {
        this->unmap();
        this->close();
    }

    File &File::operator=(File &&other) noexcept {
        this->m_file = other.m_file;
        other.m_file = nullptr;

        this->m_path = std::move(other.m_path);
        this->m_mode = other.m_mode;
        this->m_fileSize = other.m_fileSize;

        return *this;
    }


    File File::clone() {
        return File(this->m_path, this->m_mode);
    }

    void File::seek(u64 offset) {
        fseeko64(this->m_file, offset, SEEK_SET);
    }

    void File::close() {
        if (isValid()) {
            std::fclose(this->m_file);
            this->m_file = nullptr;
        }
    }


    void File::map() {
        if (!isValid()) return;

        #if defined(OS_WINDOWS)

            auto fileHandle = (HANDLE)_get_osfhandle(_fileno(this->m_file));
            auto fileMapping = CreateFileMapping(fileHandle, nullptr, SEC_RESERVE | (this->m_mode == Mode::Read ? PAGE_READONLY : PAGE_READWRITE), 0, 0, nullptr);

            if (fileMapping == nullptr)
                return;


            this->m_map = reinterpret_cast<u8*>(MapViewOfFile(fileMapping, this->m_mode == Mode::Read ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0));

            CloseHandle(fileMapping);

        #elif defined(OS_MACOS) || defined(OS_LINUX)

            auto fd = fileno(this->m_file);
            auto size = getSize();

            this->m_map = reinterpret_cast<u8*>(mmap(nullptr, size, this->m_mode == Mode::Read ? PROT_READ : PROT_WRITE, MAP_SHARED, fd, 0));

        #endif
    }

    void File::unmap() {
        if (this->m_map == nullptr) return;

        #if defined(OS_WINDOWS)

            UnmapViewOfFile(this->m_map);

        #elif defined(OS_MACOS) || defined(OS_LINUX)

            munmap(this->m_map, this->m_fileSize);

        #endif

        this->m_map = nullptr;
    }

    size_t File::readBuffer(u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        return fread(buffer, size, 1, this->m_file);
    }

    std::vector<u8> File::readVector(size_t numBytes) {
        if (!isValid()) return {};

        auto size = numBytes == 0 ? getSize() : numBytes;
        if (size == 0) return {};

        std::vector<u8> bytes(size);
        auto bytesRead = fread(bytes.data(), 1, bytes.size(), this->m_file);

        bytes.resize(bytesRead);

        return bytes;
    }

    std::string File::readString(size_t numBytes) {
        if (!isValid()) return {};

        if (getSize() == 0) return {};

        auto bytes = this->readVector(numBytes);

        if (bytes.empty())
            return "";

        auto cString = reinterpret_cast<const char *>(bytes.data());
        return { cString, util::strnlen(cString, bytes.size()) };
    }

    std::u8string File::readU8String(size_t numBytes) {
        if (!isValid()) return {};

        if (getSize() == 0) return {};

        auto bytes = this->readVector(numBytes);

        if (bytes.empty())
            return u8"";

        auto cString = reinterpret_cast<const char8_t *>(bytes.data());
        return { cString, util::strnlen(reinterpret_cast<const char*>(bytes.data()), bytes.size()) };
    }

    size_t File::writeBuffer(const u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        return std::fwrite(buffer, size, 1, this->m_file);
    }

    size_t File::writeVector(const std::vector<u8> &bytes) {
        if (!isValid()) return 0;

        return std::fwrite(bytes.data(), 1, bytes.size(), this->m_file);
    }

    size_t File::writeString(const std::string &string) {
        if (!isValid()) return 0;

        return std::fwrite(string.data(), string.size(), 1, this->m_file);
    }

    size_t File::writeU8String(const std::u8string &string) {
        if (!isValid()) return 0;

        return std::fwrite(string.data(), string.size(), 1, this->m_file);
    }

    size_t File::getSize() const {
        return this->m_fileSize;
    }

    void File::setSize(u64 size) {
        if (!isValid()) return;

        auto result = ftruncate64(fileno(this->m_file), size);
        util::unused(result);
        this->updateSize();
    }

    void File::updateSize() {
        if (!isValid()) {
            this->m_fileSize = 0;
            return;
        }

        auto startPos = ftello64(this->m_file);
        fseeko64(this->m_file, 0, SEEK_END);
        auto size = ftello64(this->m_file);
        fseeko64(this->m_file, startPos, SEEK_SET);

        if (this->m_map != nullptr && size != this->m_fileSize) {
            this->unmap();
            this->map();
        }

        if (size < 0) {
            this->m_fileSize = 0;
            return;
        }

        this->m_fileSize = size;
    }

    void File::flush() {
        std::fflush(this->m_file);
    }

    bool File::remove() {
        this->unmap();
        this->close();

        return fs::remove(this->m_path);
    }

    void File::disableBuffering() {
        if (!isValid()) return;

        std::setvbuf(this->m_file, nullptr, _IONBF, 0);
    }

    std::optional<struct stat> File::getFileInfo() {
        struct stat fileInfo = { };

        #if defined(OS_WINDOWS)

            if (wstat(this->m_path.c_str(), &fileInfo) != 0)
                return std::nullopt;

        #elif defined(OS_MACOS) || defined(OS_LINUX)

            if (stat(wolv::util::toUTF8String(this->m_path).c_str(), &fileInfo) != 0)
                return std::nullopt;

        #endif

        return fileInfo;
    }

#if __cpp_lib_jthread >= 201911L

    #if defined(OS_WINDOWS)
        static void trackWindows(const std::stop_token &stopToken, const std::fs::path &path, const std::function<void()> &callback) {

            WIN32_FILE_ATTRIBUTE_DATA previousAttributes = {};

            while (!stopToken.stop_requested()) {
                WIN32_FILE_ATTRIBUTE_DATA currentAttributes;
                if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &currentAttributes) == FALSE) {
                    callback();
                    break;
                }

                if (memcmp(&currentAttributes, &previousAttributes, sizeof(WIN32_FILE_ATTRIBUTE_DATA)) != 0) {
                    callback();

                    previousAttributes = currentAttributes;
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

        }
    #endif

    #if defined(OS_MACOS)
        static void trackMacOS(const std::stop_token &stopToken, const std::fs::path &path, const std::function<void()> &callback) {
            int queue = kqueue();
            if (queue == -1)
                throw std::runtime_error("Failed to open kqueue");

            ON_SCOPE_EXIT { close(queue); };

            int fileDescriptor = open(path.c_str(), O_RDONLY);
            if (fileDescriptor == -1)
                throw std::runtime_error("Failed to open file descriptor");

            ON_SCOPE_EXIT { close(fileDescriptor); };

            struct kevent eventHandle = { };
            EV_SET(&eventHandle, fileDescriptor, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_WRITE, 0, nullptr);
            if (kevent(queue, &eventHandle, 1, nullptr, 0, nullptr) == -1)
                throw std::runtime_error("Failed to add event to kqueue");

            const timespec timeout = { 1, 0 };
            while (!stopToken.stop_requested()) {
                struct kevent eventList[1];
                int eventCount = kevent(queue, nullptr, 0, eventList, 1, &timeout);
                if (eventCount == -1)
                    continue;

                if (eventList[0].fflags & NOTE_WRITE) {
                    callback();
                }
            }

        }
    #endif

    #if defined(OS_LINUX)
        static void trackLinux(const std::stop_token &stopToken, const std::fs::path &path, const std::function<void()> &callback) {
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

            while (!stopToken.stop_requested()) {
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
    #endif

    void ChangeTracker::startTracking(const std::function<void()> &callback) {
        if (this->m_path.empty())
            return;

        this->m_thread = std::jthread([this, callback](const std::stop_token &stopToken) {
            #if defined(OS_WINDOWS)
                trackWindows(stopToken, this->m_path, callback);
            #elif defined(OS_MACOS)
                trackMacOS(stopToken, this->m_path, callback);
            #elif defined(OS_LINUX)
                trackLinux(stopToken, this->m_path, callback);
            #endif
        });
    }

    void ChangeTracker::stopTracking() {
        this->m_thread.request_stop();
        this->m_thread.join();
    }

#endif

}