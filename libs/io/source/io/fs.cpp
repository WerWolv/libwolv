#include <wolv/io/fs.hpp>

#include <wolv/utils/string.hpp>

#include <optional>

#if defined(OS_WINDOWS)

    #include <windows.h>

#elif defined(OS_MACOS)

    #include <wolv/io/fs_macos.hpp>

#elif defined(OS_LINUX)

    #include <unistd.h>
    #include <limits.h>

#endif

#include <filesystem>

namespace wolv::io::fs {

    std::optional<std::fs::path> getExecutablePath() {
        #if defined(OS_WINDOWS)

            std::wstring exePath(MAX_PATH, '\0');
            if (GetModuleFileNameW(nullptr, exePath.data(), exePath.length()) == 0)
                return std::nullopt;

            return util::trim(exePath);

        #elif defined(OS_LINUX)

            std::string exePath(PATH_MAX, '\0');
            if (readlink("/proc/self/exe", exePath.data(), PATH_MAX) < 0)
                return std::nullopt;

            return util::trim(exePath);

        #elif defined(OS_MACOS)
            std::string exePath;

            {
                auto string = getMacExecutablePath();
                exePath = string;
                macFree(string);
            }

            return util::trim(exePath);

        #else

            return std::nullopt;

        #endif
    }

    std::fs::path toShortPath(const std::fs::path &path) {
        #if defined(OS_WINDOWS)

            size_t size = GetShortPathNameW(path.c_str(), nullptr, 0);
            if (size == 0)
                return path;

            std::wstring newPath(size, 0x00);
            GetShortPathNameW(path.c_str(), newPath.data(), newPath.size());
            newPath.pop_back();

            return newPath;

        #else

            return path;

        #endif
    }

    std::string toNormalizedPathString(const std::fs::path &path) {
        auto fixedPath = util::toUTF8String(path);
        #if defined(OS_WINDOWS)
            std::replace(fixedPath.begin(), fixedPath.end(), '\\', '/');
        #endif

        return fixedPath;
    }

    #if defined(OS_MACOS)

        std::fs::path getApplicationSupportDirectoryPath() {
            std::string exePath;

            {
                auto string = getMacApplicationSupportDirectoryPath();
                exePath = string;
                macFree(string);
            }

            return util::trim(exePath);
        }

    #endif


}
