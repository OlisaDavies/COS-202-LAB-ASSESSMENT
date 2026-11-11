#include "SortEngine.h"

#include "../utils/StringUtil.h"

namespace scapms {
namespace controller {

bool SortEngine::comparePatients(const model::PatientRecord* first,
                                 const model::PatientRecord* second,
                                 SortField field) {
    if (first == nullptr || second == nullptr) {
        return false;
    }

    switch (field) {
        case SortField::PatientID:
            // The letter case is ignored, so that a name that was typed in
            // small letters does not jump to the end of the list.
            return utils::compareIgnoreCase(first->getPatientID(),
                                            second->getPatientID()) < 0;
        case SortField::PatientName:
            return utils::compareIgnoreCase(first->getName(),
                                            second->getName()) < 0;
        case SortField::PatientAge:
            return first->getAge() < second->getAge();
        default:
            return utils::compareIgnoreCase(first->getPatientID(),
                                            second->getPatientID()) < 0;
    }
}

void SortEngine::selectionSort(std::vector<model::PatientRecord*>& records, SortField field) {
    const int count = static_cast<int>(records.size());

    for (int i = 0; i < count - 1; ++i) {
        int smallestIndex = i;

        // Find the smallest record in the part that is not sorted yet.
        for (int j = i + 1; j < count; ++j) {
            if (comparePatients(records[j], records[smallestIndex], field)) {
                smallestIndex = j;
            }
        }

        if (smallestIndex != i) {
            model::PatientRecord* temp = records[i];
            records[i] = records[smallestIndex];
            records[smallestIndex] = temp;
        }
    }
}

void SortEngine::mergeSort(std::vector<model::PatientRecord*>& records, SortField field) {
    if (records.size() <= 1) {
        return;
    }
    mergeSortRecursive(records, 0, static_cast<int>(records.size()) - 1, field);
}

void SortEngine::mergeSortRecursive(std::vector<model::PatientRecord*>& records,
                                    int left, int right, SortField field) {
    if (left >= right) {
        return;                                 // one record is already sorted
    }

    const int middle = left + (right - left) / 2;

    mergeSortRecursive(records, left, middle, field);
    mergeSortRecursive(records, middle + 1, right, field);

    mergeHalves(records, left, middle, right, field);
}

void SortEngine::mergeHalves(std::vector<model::PatientRecord*>& records,
                             int left, int middle, int right, SortField field) {
    const int leftCount = middle - left + 1;
    const int rightCount = right - middle;

    std::vector<model::PatientRecord*> leftHalf(leftCount);
    std::vector<model::PatientRecord*> rightHalf(rightCount);

    for (int i = 0; i < leftCount; ++i) {
        leftHalf[i] = records[left + i];
    }
    for (int j = 0; j < rightCount; ++j) {
        rightHalf[j] = records[middle + 1 + j];
    }

    int i = 0;
    int j = 0;
    int k = left;

    while (i < leftCount && j < rightCount) {
        // The right half is only taken when it is strictly smaller. When both
        // records have the same key the left one is taken first, which is what
        // makes merge sort stable.
        if (comparePatients(rightHalf[j], leftHalf[i], field)) {
            records[k++] = rightHalf[j++];
        } else {
            records[k++] = leftHalf[i++];
        }
    }

    while (i < leftCount) {
        records[k++] = leftHalf[i++];
    }
    while (j < rightCount) {
        records[k++] = rightHalf[j++];
    }
}

} // namespace controller
} // namespace scapms
