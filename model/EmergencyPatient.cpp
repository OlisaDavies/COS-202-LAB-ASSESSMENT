#include "EmergencyPatient.h"

#include <sstream>

namespace scapms {
namespace model {

std::string EmergencyPatient::getUrgencyDescription() const {
    switch (severityLevel) {
        case 1:  return "CRITICAL - Immediate Attention Required!";
        case 2:  return "URGENT - Attend within 15 minutes";
        default: return "MODERATE - Priority waiting";
    }
}

std::string EmergencyPatient::getPatientReport() const {
    std::ostringstream out;
    out << "-------------------------------------\n";
    out << "[CATEGORY: EMERGENCY - HIGH PRIORITY]\n";
    out << "Patient ID:     " << patientID << "\n";
    out << "Name:           " << name << "\n";
    out << "Age:            " << age << "\n";
    out << "Contact:        " << contactNumber << "\n";
    out << "Severity (1-3): " << severityLevel
        << " (" << getUrgencyDescription() << ")\n";
    out << "Triage Note:    " << triageNote << "\n";
    out << "-------------------------------------";
    return out.str();
}

std::string EmergencyPatient::getSpecialDetail() const {
    return "Severity " + std::to_string(severityLevel) + " (" + triageNote + ")";
}

void EmergencyPatient::restoreFrom(const PatientRecord& previous) {
    PatientRecord::restoreFrom(previous);

    // The dynamic cast checks that the snapshot really is an emergency
    // patient. A static cast would be undefined behaviour when it is not.
    if (const EmergencyPatient* snapshot = dynamic_cast<const EmergencyPatient*>(&previous)) {
        severityLevel = snapshot->severityLevel;
        triageNote = snapshot->triageNote;
    }
}

} // namespace model
} // namespace scapms
