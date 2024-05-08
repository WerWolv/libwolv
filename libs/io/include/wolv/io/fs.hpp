#pragma once

#include <optional>
#include <string>
#include <filesystem>

namespace std::fs {
    using namespace std::filesystem;
}

namespace wolv::io::fs {

    [[maybe_unused]]
    inline bool exists(const std::fs::path &path) {
        std::error_code error;
        return std::filesystem::exists(path, error) && !error;
    }

    [[maybe_unused]]
    inline bool createDirectories(const std::fs::path &path) {
        std::error_code error;
        return std::filesystem::create_directories(path, error) && !error;
    }

    [[maybe_unused]]
    inline bool isRegularFile(const std::fs::path &path) {
        std::error_code error;
        return std::filesystem::is_regular_file(path, error) && !error;
    }

    [[maybe_unused]]
    inline bool copyFile(const std::fs::path &from, const std::fs::path &to, std::fs::copy_options = std::fs::copy_options::none) {
        std::error_code error;
        return std::filesystem::copy_file(from, to, error) && !error;
    }

    [[maybe_unused]]
    inline bool isDirectory(const std::fs::path &path) {
        std::error_code error;
        return std::filesystem::is_directory(path, error) && !error;
    }

    [[maybe_unused]]
    inline bool remove(const std::fs::path &path) {
        std::error_code error;
        return std::filesystem::remove(path, error) && !error;
    }

    [[maybe_unused]]
    inline bool removeAll(const std::fs::path &path) {
        std::error_code error;
        return std::filesystem::remove_all(path, error) && !error;
    }

    [[maybe_unused]]
    inline uintmax_t getFileSize(const std::fs::path &path) {
        std::error_code error;
        auto size = std::filesystem::file_size(path, error);

        if (error) return 0;
        else return size;
    }

    inline bool isSubPath(const std::fs::path& base, const std::fs::path& destination) {
        const auto relative = std::fs::relative(destination, base).u8string();

        return relative.size() == 1 || (relative[0] != '.' && relative[1] != '.');
    }

    std::fs::path toShortPath(const std::fs::path &path);

    std::optional<std::fs::path> getExecutablePath();

    std::string toNormalizedPathString(const std::fs::path &path);

    #if defined(OS_MACOS)

    std::fs::path getApplicationSupportDirectoryPath();

    #endif

}