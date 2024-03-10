#include <wolv/io/file.hpp>
#include <wolv/utils/string.hpp>

namespace wolv::io {

    File File::clone() {
        return File(m_path, m_mode);
    }

    bool File::remove() {
        this->unmap();
        this->close();

        return fs::remove(this->m_path);
    }

    size_t File::getSize() const {
        if (!m_sizeValid) {
            updateSize();
        }
        return m_fileSize;
    }

    std::vector<u8> File::readVector(size_t numBytes) {
        if (!isValid()) return {};

        auto size = numBytes == 0 ? getSize() : numBytes;
        if (size == 0) return {};

        std::vector<u8> bytes(size);
        auto bytesRead = readBuffer(bytes.data(), bytes.size());

        bytes.resize(bytesRead);

        return bytes;
    }

    std::string File::readString(size_t numBytes) {
        if (!isValid()) return {};

        auto bytes = this->readVector(numBytes);

        if (bytes.empty())
            return "";

        auto cString = reinterpret_cast<const char *>(bytes.data());
        return { cString, util::strnlen(cString, bytes.size()) };
    }

    std::u8string File::readU8String(size_t numBytes) {
        if (!isValid()) return {};

        auto bytes = this->readVector(numBytes);

        if (bytes.empty())
            return u8"";

        auto cString = reinterpret_cast<const char8_t *>(bytes.data());
        return { cString, util::strnlen(reinterpret_cast<const char*>(bytes.data()), bytes.size()) };
    }

    std::vector<u8> File::readVectorAtomic(u64 address, size_t numBytes) {
        if (!isValid()) return {};

        auto size = numBytes == 0 ? getSize() : numBytes;
        if (size == 0) return {};

        std::vector<u8> bytes(size);
        auto bytesRead = readBufferAtomic(address, bytes.data(), bytes.size());

        bytes.resize(bytesRead);

        return bytes;
    }

    std::string File::readStringAtomic(u64 address, size_t numBytes) {
        if (!isValid()) return {};

        auto bytes = this->readVectorAtomic(address, numBytes);

        if (bytes.empty())
            return "";

        auto cString = reinterpret_cast<const char *>(bytes.data());
        return { cString, util::strnlen(cString, bytes.size()) };
    }

    std::u8string File::readU8StringAtomic(u64 address, size_t numBytes) {
        if (!isValid()) return {};

        auto bytes = this->readVectorAtomic(address, numBytes);

        if (bytes.empty())
            return u8"";

        auto cString = reinterpret_cast<const char8_t *>(bytes.data());
        return { cString, util::strnlen(reinterpret_cast<const char*>(bytes.data()), bytes.size()) };
    }

    size_t File::writeVector(const std::vector<u8> &bytes) {
        return writeBuffer(bytes.data(), bytes.size());
    }

    size_t File::writeString(const std::string &string) {
        return writeBuffer(reinterpret_cast<const u8*>(string.data()), string.size());
    }

    size_t File::writeU8String(const std::u8string &string) {
        return writeBuffer(reinterpret_cast<const u8*>(string.data()), string.size());
    }

    size_t File::writeVectorAtomic(u64 address, const std::vector<u8> &bytes) {
        return writeBufferAtomic(address, bytes.data(), bytes.size());
    }

    size_t File::writeStringAtomic(u64 address, const std::string &string) {
        return writeBufferAtomic(address, reinterpret_cast<const u8*>(string.data()), string.size());
    }

    size_t File::writeU8StringAtomic(u64 address, const std::u8string &string) {
        return writeBufferAtomic(address, reinterpret_cast<const u8*>(string.data()), string.size());
    }

    void ChangeTracker::startTracking(const std::function<void()> &callback) {
        if (this->m_path.empty())
            return;

        this->m_thread = std::thread([this, callback]() {
            trackImpl(this->m_stopped, this->m_path, callback);
        });
    }

    void ChangeTracker::stopTracking() {
        this->m_stopped = true;

        if (this->m_thread.joinable())
            this->m_thread.join();
    }

}