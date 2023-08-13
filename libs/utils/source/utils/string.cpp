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

    std::string wrapMonospacedString(const std::string &string, const f32 charWidth, const f32 maxWidth) {
        //If the arguments don't make sense, just immediately return the incoming string.
        if(string.empty() || charWidth < 0 || maxWidth < 0) {
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
}