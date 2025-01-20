#include <fcntl.h>
#include <wolv/io/file.hpp>

#include <wolv/utils/core.hpp>
#include <wolv/utils/guards.hpp>

#include <Windows.h>
#include <share.h>

// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/open-osfhandle
#ifdef _MSC_VER
#include <io.h>
#endif // _MSC_VER

namespace wolv::io {

    File::File(const std::fs::path &path, Mode mode) noexcept : m_path(path), m_mode(mode) {
        open();
    }

    File::File(File &&other) noexcept {
        m_handle = other.m_handle;
        other.m_handle = INVALID_HANDLE_VALUE;

        m_fileHandle = other.m_fileHandle;
        other.m_fileHandle = nullptr;

        m_path = std::move(other.m_path);
        m_mode = other.m_mode;
        m_fileSize = other.m_fileSize;
        m_sizeValid = other.m_sizeValid;
        m_openError = std::move(other.m_openError);
    }

    File::~File() {
        unmap();
        close();
    }

    File &File::operator=(File &&other) noexcept {
        m_handle = other.m_handle;
        other.m_handle = INVALID_HANDLE_VALUE;

        m_fileHandle = other.m_fileHandle;
        other.m_fileHandle = nullptr;

        m_path = std::move(other.m_path);
        m_mode = other.m_mode;
        m_fileSize = other.m_fileSize;
        m_sizeValid = other.m_sizeValid;
        m_openError = std::move(other.m_openError);

        return *this;
    }

    void File::seek(u64 offset) {
        ::SetFilePointerEx(m_handle, LARGE_INTEGER { .QuadPart = LONGLONG(offset) }, nullptr, FILE_BEGIN);
    }

    void File::open() {
        m_openError.reset();
        if (m_mode == File::Mode::Read)
            m_handle = ::CreateFileW(m_path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        else if (m_mode == File::Mode::Write)
            m_handle = ::CreateFileW(m_path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (m_mode == File::Mode::Create || (m_mode == File::Mode::Write && this->m_handle == INVALID_HANDLE_VALUE))
            m_handle = ::CreateFileW(m_path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (m_handle == INVALID_HANDLE_VALUE) {
            m_openError = ::GetLastError();
        }

        updateSize();
    }


    void File::close() {
        if (isValid()) {
            ::CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
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

        auto fileMapping = ::CreateFileMapping(m_handle, nullptr, SEC_RESERVE | (this->m_mode == Mode::Read ? PAGE_READONLY : PAGE_READWRITE), 0, 0, nullptr);
        if (fileMapping == nullptr) {
            m_openError = ::GetLastError();
            return false;
        }


        this->m_map = static_cast<u8*>(::MapViewOfFile(fileMapping, this->m_mode == Mode::Read ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0));
        if (this->m_map == nullptr) {
            m_openError = ::GetLastError();
            return false;
        }

        ::CloseHandle(fileMapping);

        return true;
    }

    void File::unmap() {
        if (this->m_map == nullptr) return;

        ::UnmapViewOfFile(this->m_map);

        this->m_map = nullptr;
    }

    size_t File::readBuffer(u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        DWORD bytesRead = 0;
        ::ReadFile(m_handle, buffer, size, &bytesRead, nullptr);

        return bytesRead;
    }

    size_t File::readBufferAtomic(u64 address, u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        OVERLAPPED overlapped = { };
        overlapped.Offset = static_cast<DWORD>(address);
        overlapped.OffsetHigh = static_cast<DWORD>(address >> 32);

        DWORD bytesRead = 0;
        if (::ReadFile(m_handle, buffer, size, &bytesRead, &overlapped)) {
            return bytesRead;
        } else {
            auto error = ::GetLastError();
            if (error == ERROR_IO_PENDING) {
                ::GetOverlappedResult(m_handle, &overlapped, &bytesRead, TRUE);
                return bytesRead;
            } else {
                return 0;
            }
        }
    }

    size_t File::writeBuffer(const u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        m_sizeValid = false;

        DWORD bytesWritten = 0;
        ::WriteFile(m_handle, buffer, size, &bytesWritten, nullptr);

        return bytesWritten;
    }

    size_t File::writeBufferAtomic(u64 address, const u8 *buffer, size_t size) {
        if (!isValid()) return 0;

        thread_local OVERLAPPED overlapped = { };
        overlapped.Offset = static_cast<DWORD>(address);
        overlapped.OffsetHigh = static_cast<DWORD>(address >> 32);

        m_sizeValid = false;

        DWORD bytesRead = 0;
        if (::WriteFile(m_handle, buffer, size, &bytesRead, &overlapped)) {
            return bytesRead;
        } else {
            if (::GetLastError() == ERROR_IO_PENDING) {
                ::GetOverlappedResult(m_handle, &overlapped, &bytesRead, TRUE);
                return bytesRead;
            } else {
                return 0;
            }
        }
    }

    void File::setSize(u64 size) {
        if (!isValid()) return;

        this->seek(size);
        ::SetEndOfFile(m_handle);
        this->updateSize();
    }

    void File::updateSize() const {
        if (!isValid()) {
            m_fileSize = 0;
            return;
        }

        DWORD highSize = 0;
        DWORD lowSize = ::GetFileSize(m_handle, &highSize);

        m_fileSize = (static_cast<u64>(highSize) << 32) | lowSize;
        m_sizeValid = true;
    }

    bool File::flush() {
        if (!isValid()) return false;

        ::FlushFileBuffers(m_handle);
        return true;
    }

    void File::disableBuffering() {

    }

    std::optional<struct stat> File::getFileInfo() {
        struct stat fileInfo = { };

#ifndef _USE_32BIT_TIME_T
        if (_wstat(this->m_path.c_str(), (struct _stat64i32*)&fileInfo) != 0)
            return std::nullopt;
#else
        if (_wstat(this->m_path.c_str(), (struct _stat32*)&fileInfo) != 0)
            return std::nullopt;
#endif // _USE_32BIT_TIME_T

        return fileInfo;
    }

    FILE* File::getHandle() const {
        if (m_fileHandle != nullptr)
            return m_fileHandle;

        auto fileDescriptor = _open_osfhandle(intptr_t(m_handle), m_mode == Mode::Read ? _O_RDONLY : _O_WRONLY);
        if (fileDescriptor == -1)
            return nullptr;

        m_fileHandle = _fdopen(fileDescriptor, m_mode == Mode::Read ? "rb" : "wb");

        return m_fileHandle;
    }

    bool File::isValid() const {
        return m_handle != INVALID_HANDLE_VALUE;
    }


    void ChangeTracker::trackImpl(const bool &stopped, const std::fs::path &path, const std::function<void()> &callback) {
        bool firstTime = true;
        WIN32_FILE_ATTRIBUTE_DATA previousAttributes = {};

        while (!stopped) {
            WIN32_FILE_ATTRIBUTE_DATA currentAttributes;
            if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &currentAttributes) == FALSE) {
                callback();
                break;
            }

            if (firstTime) {
                previousAttributes = currentAttributes;
                firstTime = false;
            }

            if (memcmp(&currentAttributes.ftLastWriteTime, &previousAttributes.ftLastWriteTime, sizeof(FILETIME)) != 0) {
                callback();

                previousAttributes = currentAttributes;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    }

}