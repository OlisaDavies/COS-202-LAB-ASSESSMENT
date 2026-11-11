#pragma once

#include <string>
#include <vector>

#include "../model/PatientRecord.h"

namespace scapms {
namespace controller {

// The field the records are ordered by.
enum class SortField {
    PatientID,
    PatientName,
    PatientAge
};

// The sorting routines of the clinic. Both are written by hand.
//
// Selection sort is the simple quadratic routine, merge sort is the divide
// and conquer routine that always needs O(n log n) comparisons.
class SortEngine {
public:
    static bool comparePatients(const model::PatientRecord* first,
                                const model::PatientRecord* second,
                                SortField field);

    // Orders the records with selection sort. It sorts in place and performs
    // at most n - 1 swaps, which suits small lists.
    static void selectionSort(std::vector<model::PatientRecord*>& records, SortField field);

    // Orders the records with merge sort. It is stable, so records with the
    // same key keep the order they had before.
    static void mergeSort(std::vector<model::PatientRecord*>& records, SortField field);

private:
    // Splits the range in two and sorts both halves.
    static void mergeSortRecursive(std::vector<model::PatientRecord*>& records,
                                   int left, int right, SortField field);

    // Joins two sorted halves back into one sorted range.
    static void mergeHalves(std::vector<model::PatientRecord*>& records,
                            int left, int middle, int right, SortField field);
};

} // namespace controller
} // namespace scapms
