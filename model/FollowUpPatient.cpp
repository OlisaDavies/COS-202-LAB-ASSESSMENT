#include "FollowUpPatient.h"

#include <sstream>

namespace scapms {
namespace model {

std::string FollowUpPatient::getPatientReport() const {
    std::ostringstream out;
    out << "-------------------------------------\n";
    out << "[Category: Follow-Up Consultation]\n";
    out << "Patient ID:       " << patientID << "\n";
    out << "Name:             " << name << "\n";
    out << "Age:              " << age << "\n";
    out << "Contact:          " << contactNumber << "\n";
    out << "Last Visit Date:  " << previousVisitDate << "\n";
    out << "Assigned Doctor:  " << assignedDoctor << "\n";
    out << "Appointment:      " << getAppointmentDetails() << "\n";
    out << "-------------------------------------";
    return out.str();
}

std::string FollowUpPatient::getSpecialDetail() const {
    return "Follow-up with " + assignedDoctor + " (Last: " + previousVisitDate + ")";
}

void FollowUpPatient::restoreFrom(const PatientRecord& previous) {
    ScheduledPatient::restoreFrom(previous);

    // The dynamic cast checks that the snapshot really is a follow-up
    // patient. A static cast would be undefined behaviour when it is not.
    if (const FollowUpPatient* snapshot = dynamic_cast<const FollowUpPatient*>(&previous)) {
        previousVisitDate = snapshot->previousVisitDate;
        assignedDoctor = snapshot->assignedDoctor;
    }
}

} // namespace model
} // namespace scapms
