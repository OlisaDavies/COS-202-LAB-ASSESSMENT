#pragma once

#include <vector>
#include <string>

#include "PatientRecord.h"

class SearchEngine {
public:
    static int linearSearchByID(const std::vector<PatientRecord*>& records, const std::string& targetID) {
        for (int i = 0; i < static_cast<int>(records.size()); ++i) {
            if (records[i]->getPatientID() == targetID) {
                return i;
            }
        }
        return -1;
    }

    static int binarySearchByID(const std::vector<PatientRecord*>& sortedRecords, const std::string& targetID) {
        int low = 0;
        int high = static_cast<int>(sortedRecords.size()) - 1;

        while (low <= high) {
            int mid = low + (high - low) / 2;

            std::string midID = sortedRecords[mid]->getPatientID();

            if (midID == targetID) {
                return mid;
            } else if (midID < targetID) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return -1;
    }

    static int recursiveSearchByName(const std::vector<PatientRecord*>& records,
                                     const std::string& targetName,
                                     int currentIndex = 0) {
        if (currentIndex >= static_cast<int>(records.size())) {
            return -1;
        }

        if (records[currentIndex]->getName() == targetName) {
            return currentIndex;
        }

        return recursiveSearchByName(records, targetName, currentIndex + 1);
    }
};
