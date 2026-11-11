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

inline bool equalsIgnoreCase(const std::string& left, const std::string& right) {
    return toLower(left) == toLower(right);
}

// Orders two strings ignoring letter case. The result is less than zero
// when the first one comes first in the alphabet, and greater than zero
// when the second one comes first.
//
// The sort of the clinic uses this, so that a name that was typed in small
// letters is not put at the end of the list. The search already ignores
// the letter case, and the two have to agree.
inline int compareIgnoreCase(const std::string& left, const std::string& right) {
    const std::string loweredLeft = toLower(left);
    const std::string loweredRight = toLower(right);

    if (loweredLeft < loweredRight) {
        return -1;
    }
    if (loweredRight < loweredLeft) {
        return 1;
    }
    return 0;
}

// True when the text holds the part, ignoring letter case. An empty part
// never matches, so that a search for nothing finds nothing.
inline bool containsIgnoreCase(const std::string& text, const std::string& part) {
    if (part.empty()) {
        return false;
    }
    return toLower(text).find(toLower(part)) != std::string::npos;
}

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
