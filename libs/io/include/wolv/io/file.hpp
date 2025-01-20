#pragma once

#include <wolv/types.hpp>
#include <wolv/io/fs.hpp>

#include <atomic>
#include <cstdio>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <functional>

#include <sys/types.h>
#include <sys/stat.h>

#if defined(OS_WINDOWS)
    using HANDLE = void*;
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

        [[nodiscard]] bool isValid() const;
        [[nodiscard]] const std::optional<i64>& getOpenError() const {
            return m_openError;
        }

        File clone();

        void seek(u64 offset);
        void open();
        void close();

        bool map();
        void unmap();
        [[nodiscard]] u8* getMapping() const { return this->m_map; }

        size_t readBuffer(u8 *buffer, size_t size);
        [[nodiscard]] std::vector<u8> readVector(size_t numBytes = 0);
        [[nodiscard]] std::string readString(size_t numBytes = 0);
        [[nodiscard]] std::u8string readU8String(size_t numBytes = 0);

        size_t readBufferAtomic(u64 address, u8 *buffer, size_t size);
        [[nodiscard]] std::vector<u8> readVectorAtomic(u64 address, size_t numBytes);
        [[nodiscard]] std::string readStringAtomic(u64 address, size_t numBytes);
        [[nodiscard]] std::u8string readU8StringAtomic(u64 address, size_t numBytes);

        size_t writeBuffer(const u8 *buffer, size_t size);
        size_t writeVector(const std::vector<u8> &bytes);
        size_t writeString(const std::string &string);
        size_t writeU8String(const std::u8string &string);

        size_t writeBufferAtomic(u64 address, const u8 *buffer, size_t size);
        size_t writeVectorAtomic(u64 address, const std::vector<u8> &bytes);
        size_t writeStringAtomic(u64 address, const std::string &string);
        size_t writeU8StringAtomic(u64 address, const std::u8string &string);

        [[nodiscard]] size_t getSize() const;
        void setSize(u64 size);

        bool flush();
        bool remove();

        [[nodiscard]] FILE* getHandle() const;
        [[nodiscard]] const std::fs::path &getPath() const { return this->m_path; }

        void disableBuffering();

        [[nodiscard]] std::optional<struct stat> getFileInfo();

    private:
        void updateSize() const;

    private:
        mutable FILE *m_fileHandle = nullptr;
    #if defined (OS_WINDOWS)
        HANDLE m_handle = reinterpret_cast<void*>(-1);
    #else
        int m_handle = -1;
    #endif

        std::fs::path m_path;
        Mode m_mode = Mode::Read;
        u8 *m_map = nullptr;
        std::optional<i64> m_openError;

        mutable bool m_sizeValid = false;
        mutable size_t m_fileSize = 0;
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
        static void trackImpl(const bool &stopped, const std::fs::path &path, const std::function<void()> &callback);

    private:
        bool m_stopped = false;
        std::fs::path m_path;
        std::thread m_thread;
    };

}