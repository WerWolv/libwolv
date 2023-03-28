#include <wolv/io/file.hpp>

#include <wolv/utils/core.hpp>
#include <wolv/utils/string.hpp>

#include <unistd.h>

namespace wolv::io {

    File::File(const std::fs::path &path, Mode mode) noexcept : m_path(path), m_mode(mode) {
        #if defined(OS_WINDOWS)

            if (mode == File::Mode::Read)
                this->m_file = _wfopen(path.c_str(), L"rb");
            else if (mode == File::Mode::Write)
                this->m_file = _wfopen(path.c_str(), L"r+b");

            if (mode == File::Mode::Create || (mode == File::Mode::Write && this->m_file == nullptr))
                this->m_file = _wfopen(path.c_str(), L"w+b");

        #else

            if (mode == File::Mode::Read)
                this->m_file = fopen64(util::toUTF8String(path).c_str(), "rb");
            else if (mode == File::Mode::Write)
                this->m_file = fopen64(util::toUTF8String(path).c_str(), "r+b");

            if (mode == File::Mode::Create || (mode == File::Mode::Write && this->m_file == nullptr))
                this->m_file = fopen64(util::toUTF8String(path).c_str(), "w+b");

        #endif
    }

    File::File() noexcept {
        this->m_file = nullptr;
    }

    File::File(File &&other) noexcept {
        this->m_file = other.m_file;
        other.m_file = nullptr;

        this->m_path = std::move(other.m_path);
        this->m_mode = other.m_mode;
    }

    File::~File() {
        this->close();
    }

    File &File::operator=(File &&other) noexcept {
        this->m_file = other.m_file;
        other.m_file = nullptr;

        this->m_path = std::move(other.m_path);
        this->m_mode = other.m_mode;

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
        if (!isValid()) return 0;

        auto startPos = ftello64(this->m_file);
        fseeko64(this->m_file, 0, SEEK_END);
        auto size = ftello64(this->m_file);
        fseeko64(this->m_file, startPos, SEEK_SET);

        if (size < 0)
            return 0;

        return size;
    }

    void File::setSize(u64 size) {
        if (!isValid()) return;

        auto result = ftruncate64(fileno(this->m_file), size);
        util::unused(result);
    }

    void File::flush() {
        std::fflush(this->m_file);
    }

    bool File::remove() {
        this->close();
        return std::remove(util::toUTF8String(this->m_path).c_str()) == 0;
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

        #else

            if (stat(wolv::util::toUTF8String(this->m_path).c_str(), &fileInfo) != 0)
                return std::nullopt;

        #endif

        return fileInfo;
    }

}