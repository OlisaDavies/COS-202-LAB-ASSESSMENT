#include "EmergencyPatient.h"

#include <iostream>

namespace scapms {
namespace model {

std::string EmergencyPatient::getUrgencyDescription() const {
    switch (severityLevel) {
        case 1:  return "CRITICAL - Immediate Attention Required!";
        case 2:  return "URGENT - Attend within 15 minutes";
        default: return "MODERATE - Priority waiting";
    }
}

void EmergencyPatient::displayPatient() const {
    std::cout << "-------------------------------------" << std::endl;
    std::cout << "[CATEGORY: EMERGENCY - HIGH PRIORITY]" << std::endl;
    std::cout << "Patient ID:     " << patientID << std::endl;
    std::cout << "Name:           " << name << std::endl;
    std::cout << "Age:            " << age << std::endl;
    std::cout << "Contact:        " << contactNumber << std::endl;
    std::cout << "Severity (1-3): " << severityLevel << " (" << getUrgencyDescription() << ")" << std::endl;
    std::cout << "Triage Note:    " << triageNote << std::endl;
    std::cout << "-------------------------------------" << std::endl;
}

std::string EmergencyPatient::getSpecialDetail() const {
    return "Severity " + std::to_string(severityLevel) + " (" + triageNote + ")";
}

void EmergencyPatient::restoreFrom(const PatientRecord& previous) {
    PatientRecord::restoreFrom(previous);

    const EmergencyPatient& snapshot = static_cast<const EmergencyPatient&>(previous);
    severityLevel = snapshot.severityLevel;
    triageNote = snapshot.triageNote;
}

} // namespace model
} // namespace scapms
