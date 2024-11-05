#include <stdexcept>

#include <wolv/utils/string.hpp>

namespace wolv::util {

    std::vector<std::string> splitString(const std::string &string, const std::string &delimiter, bool removeEmpty) {
        if (delimiter.empty()) {
            return { string };
        }

        std::vector<std::string> result;

        size_t start = 0, end = 0;
        while ((end = string.find(delimiter, start)) != std::string::npos) {
            size_t size = end - start;
            if (start + size > string.length())
                break;

            auto token = string.substr(start, end - start);
            start = end + delimiter.length();
            result.emplace_back(std::move(token));
        }

        result.emplace_back(string.substr(start));

        if (removeEmpty)
            std::erase_if(result, [](const auto &string) { return string.empty(); });

        return result;
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

        std::size_t pos = 0;
        while ((pos = string.find(search, pos)) != std::string::npos) {
            string.replace(pos, search.size(), replace);
            pos += replace.size();
        }

        return string;
    }

    std::string replaceTabsWithSpaces(const std::string& string, u32 tabSize) {
        if (tabSize == 0)
            return string;

        auto stringVector = splitString(string, "\n", false);
        std::string result;
        for (auto &line : stringVector) {
            std::size_t pos = 0;
            while ((pos = line.find('\t', pos)) != std::string::npos) {
                auto spaces = tabSize - (pos % tabSize);
                line.replace(pos, 1, std::string(spaces, ' '));
                pos += tabSize;
            }
            result += line + "\n";
        }
        return result;
    }

    std::string wrapMonospacedString(const std::string &string, const f32 charWidth, const f32 maxWidth) {
        // If the arguments don't make sense, just immediately return the incoming string.
        if (string.empty() || charWidth < 0 || maxWidth < 0) {
            return string;
        }

        std::string result;

        u32 startOfLineIndex = 0;
        u32 candidateLineBreakIndex = 0;
        f32 currentWidth = 0;

        for (u32 i = 0; i < string.size(); i++) {
            if (currentWidth + charWidth <= maxWidth) {
                //Character fits, so increase width.
                currentWidth += charWidth;
            } else if (candidateLineBreakIndex > 0) {
                //Character doesn't fit, and there is a candidate for a line-break.
                result.append(string.substr(startOfLineIndex, (candidateLineBreakIndex + 1) - startOfLineIndex));
                result.append("\n");

                //Start a new line
                startOfLineIndex = candidateLineBreakIndex + 1;
                candidateLineBreakIndex = 0;
                currentWidth = (i - startOfLineIndex + 1) * charWidth; // NOLINT(*-narrowing-conversions)
            } else {
                //Character doesn't fit, and there is no candidate for a line-break, so force a word-break instead.
                result.append(string.substr(startOfLineIndex, i - startOfLineIndex));
                result.append("\n");

                //Start a new line
                startOfLineIndex = i;
                candidateLineBreakIndex = 0;
                currentWidth = charWidth;
            }

            const auto c = string[i];

            if (std::ispunct(c) != 0 || c == ' ') {
                //Character is a candidate for a line-break;
                candidateLineBreakIndex = i;
            }
        }

        //Add the remainder of the string, if any.
        result.append(string.substr(startOfLineIndex, string.size() - startOfLineIndex));

        return result;
    }

    std::string capitalizeString(std::string string) {
        for (const auto delimiter : { "_", "-", " " }) {
            auto parts = splitString(string, delimiter);

            for (auto &part : parts) {
                if (!part.empty())
                    part[0] = char(std::toupper(part[0]));
            }

            string = combineStrings(parts, delimiter);
        }

        return string;
    }

}