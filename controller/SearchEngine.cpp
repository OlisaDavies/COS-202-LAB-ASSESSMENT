#include "SearchEngine.h"

#include "../utils/StringUtil.h"

namespace scapms {
namespace controller {

int SearchEngine::linearSearchByID(const std::vector<model::PatientRecord*>& records,
                                   const std::string& targetID) {
    for (int i = 0; i < static_cast<int>(records.size()); ++i) {
        if (records[i] != nullptr && records[i]->getPatientID() == targetID) {
            return i;
        }
    }
    return -1;
}

std::vector<int> SearchEngine::linearSearchByText(
    const std::vector<model::PatientRecord*>& records, const std::string& text) {
    std::vector<int> matches;

    if (text.empty()) {
        return matches;                          // a search for nothing finds nothing
    }

    for (int i = 0; i < static_cast<int>(records.size()); ++i) {
        if (records[i] == nullptr) {
            continue;
        }
        // The identifier and the name are both looked at, because the
        // staff may type either one.
        if (utils::containsIgnoreCase(records[i]->getPatientID(), text) ||
            utils::containsIgnoreCase(records[i]->getName(), text)) {
            matches.push_back(i);
        }
    }
    return matches;
}

int SearchEngine::binarySearchByID(const std::vector<model::PatientRecord*>& sortedRecords,
                                   const std::string& targetID) {
    int low = 0;
    int high = static_cast<int>(sortedRecords.size()) - 1;

    while (low <= high) {
        // The middle of the range, written so that it cannot overflow.
        int middle = low + (high - low) / 2;

        const std::string& middleID = sortedRecords[middle]->getPatientID();

        // The comparison is the same one the sort uses, ignoring the letter
        // case. If the two were different, the list would not really be
        // sorted in the way the binary search expects.
        const int order = utils::compareIgnoreCase(middleID, targetID);

        if (order == 0) {
            return middle;
        } else if (order < 0) {
            low = middle + 1;                   // look in the right half
        } else {
            high = middle - 1;                  // look in the left half
        }
    }
    return -1;
}

std::vector<int> SearchEngine::recursiveSearchByName(
    const std::vector<model::PatientRecord*>& records, const std::string& targetName,
    int currentIndex) {
    std::vector<int> matches;

    // Base case: the end of the list has been reached.
    if (currentIndex >= static_cast<int>(records.size())) {
        return matches;
    }

    if (records[currentIndex] != nullptr &&
        utils::containsIgnoreCase(records[currentIndex]->getName(), targetName)) {
        matches.push_back(currentIndex);
    }

    // Recursive case: try the next position.
    std::vector<int> rest = recursiveSearchByName(records, targetName, currentIndex + 1);
    matches.insert(matches.end(), rest.begin(), rest.end());

    return matches;
}

std::vector<int> SearchEngine::searchByCategory(const std::vector<model::PatientRecord*>& records,
                                                const std::string& category) {
    std::vector<int> matches;

    for (int i = 0; i < static_cast<int>(records.size()); ++i) {
        if (records[i] != nullptr &&
            utils::equalsIgnoreCase(records[i]->getPatientType(), category)) {
            matches.push_back(i);
        }
    }
    return matches;
}

} // namespace controller
} // namespace scapms
