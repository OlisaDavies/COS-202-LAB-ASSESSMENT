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

int SearchEngine::binarySearchByID(const std::vector<model::PatientRecord*>& sortedRecords,
                                   const std::string& targetID) {
    int low = 0;
    int high = static_cast<int>(sortedRecords.size()) - 1;

    while (low <= high) {
        // The middle of the range, written so that it cannot overflow.
        int middle = low + (high - low) / 2;

        const std::string middleID = sortedRecords[middle]->getPatientID();

        if (middleID == targetID) {
            return middle;
        } else if (middleID < targetID) {
            low = middle + 1;                   // look in the right half
        } else {
            high = middle - 1;                  // look in the left half
        }
    }
    return -1;
}

int SearchEngine::recursiveSearchByName(const std::vector<model::PatientRecord*>& records,
                                        const std::string& targetName,
                                        int currentIndex) {
    // Base case: the end of the list has been reached.
    if (currentIndex >= static_cast<int>(records.size())) {
        return -1;
    }

    if (records[currentIndex] != nullptr &&
        utils::equalsIgnoreCase(records[currentIndex]->getName(), targetName)) {
        return currentIndex;
    }

    // Recursive case: try the next position.
    return recursiveSearchByName(records, targetName, currentIndex + 1);
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
