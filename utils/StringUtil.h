#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace scapms {
namespace utils {

// Lower cases an ASCII string. Used by the name search so that the clinic
// staff does not have to remember the exact capitalisation.
inline std::string toLower(const std::string& text) {
    std::string lowered = text;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lowered;
}

// Compares two strings ignoring letter case.
inline bool equalsIgnoreCase(const std::string& left, const std::string& right) {
    return toLower(left) == toLower(right);
}

// Removes leading and trailing spaces, tabs and carriage returns.
inline std::string trim(const std::string& text) {
    std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

} // namespace utils
} // namespace scapms
