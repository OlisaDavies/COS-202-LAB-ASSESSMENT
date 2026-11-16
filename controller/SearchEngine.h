#pragma once

#include <string>
#include <vector>

#include "../model/PatientRecord.h"

namespace scapms {
namespace controller {

// The searching routines of the clinic. Every one of them is written by hand,
// so the program never calls the standard library search functions.
//
// Linear search works on a list in any order, binary search needs a sorted
// list, and the recursive search walks the list step by step.
class SearchEngine {
public:
    // Scans the whole list and returns the position of the patient with that
    // ID, or -1.
    static int linearSearchByID(const std::vector<model::PatientRecord*>& records,
                                const std::string& targetID);

    // Scans the whole list and returns the position of every patient whose
    // identifier or whose name holds the given text.
    //
    // The text is matched without regard to letter case, and a part of the name
    // is enough. The staff expect that, because they type "chi" and mean
    // "Chioma".
    static std::vector<int> linearSearchByText(const std::vector<model::PatientRecord*>& records,
                                               const std::string& text);

    // Halves the search space on every step. The list must be sorted by ID.
    static int binarySearchByID(const std::vector<model::PatientRecord*>& sortedRecords,
                                const std::string& targetID);

    // Looks for patients by name. The search calls itself with the next position
    // until it runs off the end of the list.
    static std::vector<int> recursiveSearchByName(const std::vector<model::PatientRecord*>& records,
                                                  const std::string& targetName,
                                                  int currentIndex = 0);

    static std::vector<int> searchByCategory(const std::vector<model::PatientRecord*>& records,
                                             const std::string& category);
};

} // namespace controller
} // namespace scapms

