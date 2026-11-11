#include "IDGenerator.h"

#include <cstdlib>

#include "../model/PatientRecord.h"

namespace scapms {
namespace utils {

const char* IDGenerator::patientPrefix(const std::string& patientType) {
    if (patientType == model::PatientRecord::TYPE_EMERGENCY) {
        return "EMG-";
    }
    if (patientType == model::PatientRecord::TYPE_FOLLOWUP) {
        return "FOL-";
    }
    return "GEN-";
}

int IDGenerator::highestUsedNumber(const std::string& prefix,
                                   const std::vector<std::string>& takenIds) {
    int highest = 0;

    for (const std::string& id : takenIds) {
        if (id.size() <= prefix.size() || id.compare(0, prefix.size(), prefix) != 0) {
            continue;                            // different prefix
        }

        std::string digits = id.substr(prefix.size());
        if (digits.find_first_not_of("0123456789") != std::string::npos) {
            continue;                            // not a plain number
        }

        int number = std::atoi(digits.c_str());
        if (number > highest) {
            highest = number;
        }
    }
    return highest;
}

std::string IDGenerator::nextAppointmentId(const std::vector<std::string>& takenIds) {
    const std::string prefix = "APT-";
    int next = highestUsedNumber(prefix, takenIds) + 1;

    // Keep looking in the unlikely case that the number is already taken by
    // an identifier that does not follow the numeric pattern.
    while (true) {
        std::string candidate = prefix + std::to_string(next);
        bool taken = false;
        for (const std::string& id : takenIds) {
            if (id == candidate) {
                taken = true;
                break;
            }
        }
        if (!taken) {
            return candidate;
        }
        ++next;
    }
}

std::string IDGenerator::nextPatientId(const std::string& prefix,
                                       const std::vector<std::string>& takenIds) {
    int next = highestUsedNumber(prefix, takenIds) + 1;

    while (true) {
        std::string candidate = prefix + std::to_string(next);
        bool taken = false;
        for (const std::string& id : takenIds) {
            if (id == candidate) {
                taken = true;
                break;
            }
        }
        if (!taken) {
            return candidate;
        }
        ++next;
    }
}

} // namespace utils
} // namespace scapms
