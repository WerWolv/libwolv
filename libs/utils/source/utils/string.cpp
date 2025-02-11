#include <wolv/utils/string.hpp>

#include <locale>

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
        if (tabSize == 0 || string.empty() || string.find('\t') == std::string::npos)
            return string;

        auto stringVector = splitString(string, "\n", false);
        std::string result;
        for (auto &line : stringVector) {
            std::size_t pos = 0;
            while ((pos = line.find('\t', pos)) != std::string::npos) {
                auto spaces = tabSize - (pos % tabSize);
                line.replace(pos, 1, std::string(spaces, ' '));
                pos += spaces-1;
            }
            result += line + '\n';
        }
        result.pop_back();
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


    std::optional<std::string> utf16ToUtf8(const std::u16string& input) {
        std::string output;
        for (size_t i = 0; i < input.size(); ++i) {
            const char16_t ch = input[i];
            if (ch <= 0x7F) {
                output.push_back(static_cast<char>(ch));
            } else if (ch <= 0x7FF) {
                output.push_back(0xC0 | (ch >> 6));
                output.push_back(0x80 | (ch & 0x3F));
            } else if (ch >= 0xD800 && ch <= 0xDFFF) {
                // Surrogate pair
                if (i + 1 < input.size() && input[i + 1] >= 0xDC00 && input[i + 1] <= 0xDFFF) {
                    const char32_t fullChar = ((ch - 0xD800) << 10) + (input[i + 1] - 0xDC00) + 0x10000;
                    output.push_back(0xF0 | (fullChar >> 18));
                    output.push_back(0x80 | ((fullChar >> 12) & 0x3F));
                    output.push_back(0x80 | ((fullChar >> 6) & 0x3F));
                    output.push_back(0x80 | (fullChar & 0x3F));
                    ++i;
                } else {
                    return std::nullopt;
                }
            } else {
                output.push_back(0xE0 | (ch >> 12));
                output.push_back(0x80 | ((ch >> 6) & 0x3F));
                output.push_back(0x80 | (ch & 0x3F));
            }
        }
        return output;
    }

    std::optional<std::u16string> utf8ToUtf16(const std::string& input) {
        std::u16string output;
        size_t i = 0;
        while (i < input.size()) {
            char32_t codepoint = 0;
            unsigned char byte = input[i];
            if (byte <= 0x7F) {
                codepoint = byte;
            } else if ((byte & 0xE0) == 0xC0) {
                if (i + 1 >= input.size())
                    return std::nullopt;
                codepoint = ((byte & 0x1F) << 6) | (input[i + 1] & 0x3F);
                i += 1;
            } else if ((byte & 0xF0) == 0xE0) {
                if (i + 2 >= input.size())
                    return std::nullopt;
                codepoint = ((byte & 0x0F) << 12) | ((input[i + 1] & 0x3F) << 6) | (input[i + 2] & 0x3F);
                i += 2;
            } else if ((byte & 0xF8) == 0xF0) {
                if (i + 3 >= input.size())
                    return std::nullopt;
                codepoint = ((byte & 0x07) << 18) | ((input[i + 1] & 0x3F) << 12) | ((input[i + 2] & 0x3F) << 6) | (input[i + 3] & 0x3F);
                i += 3;
            } else {
                return std::nullopt;
            }

            if (codepoint > 0xFFFF) {
                output.push_back(0xD800 + ((codepoint - 0x10000) >> 10));
                output.push_back(0xDC00 + ((codepoint - 0x10000) & 0x3FF));
            } else {
                output.push_back(static_cast<char16_t>(codepoint));
            }
            i += 1;
        }
        return output;
    }

    std::optional<std::u32string> utf8ToUtf32(const std::string& input) {
        std::u32string output;
        size_t i = 0;
        while (i < input.size()) {
            char32_t codepoint = 0;
            unsigned char byte = input[i];
            if (byte <= 0x7F) {
                codepoint = byte;
            } else if ((byte & 0xE0) == 0xC0) {
                codepoint = ((byte & 0x1F) << 6) | (input[i + 1] & 0x3F);
                i += 1;
            } else if ((byte & 0xF0) == 0xE0) {
                codepoint = ((byte & 0x0F) << 12) | ((input[i + 1] & 0x3F) << 6) | (input[i + 2] & 0x3F);
                i += 2;
            } else if ((byte & 0xF8) == 0xF0) {
                codepoint = ((byte & 0x07) << 18) | ((input[i + 1] & 0x3F) << 12) | ((input[i + 2] & 0x3F) << 6) | (input[i + 3] & 0x3F);
                i += 3;
            } else {
                return std::nullopt;
            }
            output.push_back(codepoint);
            i += 1;
        }
        return output;
    }

    std::optional<std::string> utf32ToUtf8(const std::u32string& input) {
        std::string output;
        for (const char32_t ch : input) {
            if (ch <= 0x7F) {
                output.push_back(static_cast<char>(ch));
            } else if (ch <= 0x7FF) {
                output.push_back(0xC0 | (ch >> 6));
                output.push_back(0x80 | (ch & 0x3F));
            } else if (ch <= 0xFFFF) {
                output.push_back(0xE0 | (ch >> 12));
                output.push_back(0x80 | ((ch >> 6) & 0x3F));
                output.push_back(0x80 | (ch & 0x3F));
            } else if (ch <= 0x10FFFF) {
                output.push_back(0xF0 | (ch >> 18));
                output.push_back(0x80 | ((ch >> 12) & 0x3F));
                output.push_back(0x80 | ((ch >> 6) & 0x3F));
                output.push_back(0x80 | (ch & 0x3F));
            } else {
                return std::nullopt;
            }
        }
        return output;
    }

    std::optional<std::wstring> utf8ToWstring(const std::string& input) {
        if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
            const auto output = utf8ToUtf16(input);
            if (!output.has_value())
                 return std::nullopt;
            else
                return std::wstring(output->begin(), output->end());
        } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
            const auto output = utf8ToUtf32(input);
            if (!output.has_value())
                return std::nullopt;
            else
                return std::wstring(output->begin(), output->end());
        } else {
            static_assert("wchar_t is neither UTF-16 nor UTF-32!");
        }
    }

    std::optional<std::string> wstringToUtf8(const std::wstring& input) {
        if constexpr (sizeof(wchar_t) == sizeof(char16_t)) {
            const auto utf16 = std::u16string(input.begin(), input.end());
            return utf16ToUtf8(utf16);
        } else if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
            const auto utf32 = std::u32string(input.begin(), input.end());
            return utf32ToUtf8(utf32);
        } else {
            static_assert("wchar_t is neither UTF-16 nor UTF-32!");
        }
    }

}