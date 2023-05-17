#pragma once

#include <wolv/types.hpp>
#include <wolv/io/fs.hpp>

#include <cstdio>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <sys/stat.h>

#if defined(OS_MACOS)

    #include <unistd.h>
    #include <sys/fcntl.h>

    #define off64_t     off_t
    #define fopen64     fopen
    #define fseeko64    fseek
    #define ftello64    ftell
    #define ftruncate64 ftruncate

#elif defined(OS_LINUX)

    #include <unistd.h>
    #include <fcntl.h>

#endif

namespace wolv::io {

    class File {
    public:
        enum class Mode {
            Read,
            Write,
            Create
        };

        explicit File(const std::fs::path &path, Mode mode) noexcept;
        File() noexcept = default;
        File(const File &) = delete;
        File(File &&other) noexcept;

        ~File();

        File &operator=(File &&other) noexcept;


        [[nodiscard]] bool isValid() const {
            return this->m_file != nullptr;
        }

        File clone();

        void seek(u64 offset);
        void close();

        void map();
        void unmap();
        [[nodiscard]] u8* getMapping() const { return this->m_map; }

        size_t readBuffer(u8 *buffer, size_t size);
        [[nodiscard]] std::vector<u8> readVector(size_t numBytes = 0);
        [[nodiscard]] std::string readString(size_t numBytes = 0);
        [[nodiscard]] std::u8string readU8String(size_t numBytes = 0);

        size_t writeBuffer(const u8 *buffer, size_t size);
        size_t writeVector(const std::vector<u8> &bytes);
        size_t writeString(const std::string &string);
        size_t writeU8String(const std::u8string &string);

        [[nodiscard]] size_t getSize() const;
        void setSize(u64 size);
        void updateSize();

        void flush();
        bool remove();

        [[nodiscard]] auto getHandle() const { return this->m_file; }
        [[nodiscard]] const std::fs::path &getPath() const { return this->m_path; }

        void disableBuffering();

        [[nodiscard]] std::optional<struct stat> getFileInfo();

    private:
        FILE *m_file = nullptr;
        std::fs::path m_path;
        Mode m_mode = Mode::Read;
        u8 *m_map = nullptr;

        size_t m_fileSize = 0;
    };

    class ChangeTracker {
    public:
        ChangeTracker() = default;
        explicit ChangeTracker(std::fs::path path) : m_path(std::move(path)) { }
        explicit ChangeTracker(const File &file) : m_path(file.getPath()) { }
        ~ChangeTracker() { this->stopTracking(); }

        ChangeTracker(const ChangeTracker &) = delete;
        ChangeTracker(ChangeTracker &&) = default;

        ChangeTracker& operator=(const ChangeTracker &) = delete;
        ChangeTracker& operator=(ChangeTracker &&) = default;

        [[nodiscard]] const std::fs::path& getPath() const { return this->m_path; }

        void startTracking(const std::function<void()> &callback);
        void stopTracking();

    private:
        std::fs::path m_path;
        std::jthread m_thread;
    };

}