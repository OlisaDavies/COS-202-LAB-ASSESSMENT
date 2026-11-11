#include "FollowUpPatient.h"

#include <iostream>

namespace scapms {
namespace model {

void FollowUpPatient::displayPatient() const {
    std::cout << "-------------------------------------" << std::endl;
    std::cout << "[Category: Follow-Up Consultation]" << std::endl;
    std::cout << "Patient ID:       " << patientID << std::endl;
    std::cout << "Name:             " << name << std::endl;
    std::cout << "Age:              " << age << std::endl;
    std::cout << "Contact:          " << contactNumber << std::endl;
    std::cout << "Last Visit Date:  " << previousVisitDate << std::endl;
    std::cout << "Assigned Doctor:  " << assignedDoctor << std::endl;
    std::cout << "Appointment:      " << getAppointmentDetails() << std::endl;
    std::cout << "-------------------------------------" << std::endl;
}

std::string FollowUpPatient::getSpecialDetail() const {
    return "Follow-up with " + assignedDoctor + " (Last: " + previousVisitDate + ")";
}

void FollowUpPatient::restoreFrom(const PatientRecord& previous) {
    ScheduledPatient::restoreFrom(previous);

    const FollowUpPatient& snapshot = static_cast<const FollowUpPatient&>(previous);
    previousVisitDate = snapshot.previousVisitDate;
    assignedDoctor = snapshot.assignedDoctor;
}

} // namespace model
} // namespace scapms
