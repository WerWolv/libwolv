#include <stdexcept>

#include <wolv/utils/string.hpp>

namespace wolv::util {

    std::vector<std::string> splitString(const std::string &string, const std::string &delimiter) {
        if (delimiter.empty()) {
            return { string };
        } 
        size_t start = 0, end = 0;
        std::string token;
        std::vector<std::string> res;

        while ((end = string.find(delimiter, start)) != std::string::npos) {
            size_t size = end - start;
            if (start + size > string.length())
                break;

            token = string.substr(start, end - start);
            start = end + delimiter.length();
            res.push_back(token);
        }

        res.emplace_back(string.substr(start));
        return res;
    }

    std::string combineStrings(const std::vector<std::string> &strings, const std::string &delimiter) {
        std::string result;
        for (const auto &string : strings) {
            result += string;
            result += delimiter;
        }

        return result.substr(0, result.length() - delimiter.length());
    }

    std::string replaceStrings(std::string string, const std::string &search, const std::string &replace) {
        if (search.empty())
            return string;

        std::size_t pos;
        while ((pos = string.find(search)) != std::string::npos)
            string.replace(pos, search.size(), replace);

        return string;
    }

}