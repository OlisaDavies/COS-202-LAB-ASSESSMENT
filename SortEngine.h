#pragma once

#include <vector>
#include <string>
#include <utility>

#include "PatientRecord.h"

enum SortField {
    SORT_BY_ID,
    SORT_BY_NAME,
    SORT_BY_AGE
};

class SortEngine {
public:
    static bool comparePatients(const PatientRecord* a, const PatientRecord* b, SortField field) {
        if (!a || !b) return false;

        switch (field) {
            case SORT_BY_ID:
                return a->getPatientID() < b->getPatientID();
            case SORT_BY_NAME:
                return a->getName() < b->getName();
            case SORT_BY_AGE:
                return a->getAge() < b->getAge();
            default:
                return a->getPatientID() < b->getPatientID();
        }
    }

    static void selectionSort(std::vector<PatientRecord*>& records, SortField field) {
        int n = static_cast<int>(records.size());

        for (int i = 0; i < n - 1; ++i) {
            int targetIdx = i;

            for (int j = i + 1; j < n; ++j) {
                if (comparePatients(records[j], records[targetIdx], field)) {
                    targetIdx = j;
                }
            }

            if (targetIdx != i) {
                PatientRecord* temp = records[i];
                records[i] = records[targetIdx];
                records[targetIdx] = temp;
            }
        }
    }

    static void mergeSort(std::vector<PatientRecord*>& records, SortField field) {
        if (records.size() <= 1) return;
        mergeSortRecursive(records, 0, static_cast<int>(records.size()) - 1, field);
    }

private:
    static void mergeSortRecursive(std::vector<PatientRecord*>& arr, int left, int right, SortField field) {
        if (left >= right) return;

        int mid = left + (right - left) / 2;

        mergeSortRecursive(arr, left, mid, field);
        mergeSortRecursive(arr, mid + 1, right, field);

        mergeHalves(arr, left, mid, right, field);
    }

    static void mergeHalves(std::vector<PatientRecord*>& arr, int left, int mid, int right, SortField field) {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        std::vector<PatientRecord*> L(n1);
        std::vector<PatientRecord*> R(n2);

        for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
        for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

        int i = 0;
        int j = 0;
        int k = left;

        while (i < n1 && j < n2) {
            if (comparePatients(L[i], R[j], field) || L[i] == R[j]) {
                arr[k++] = L[i++];
            } else {
                arr[k++] = R[j++];
            }
        }

        while (i < n1) {
            arr[k++] = L[i++];
        }

        while (j < n2) {
            arr[k++] = R[j++];
        }
    }
};
