#include "GeneralPatient.h"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace scapms {
namespace model {

namespace {

// The clinic quotes its fees in naira.
std::string formatFee(double fee) {
    std::ostringstream out;
    out << "N" << std::fixed << std::setprecision(2) << fee;
    return out.str();
}

} // namespace

void GeneralPatient::displayPatient() const {
    std::cout << "-------------------------------------" << std::endl;
    std::cout << "[Category: General Outpatient]" << std::endl;
    std::cout << "Patient ID:       " << patientID << std::endl;
    std::cout << "Name:             " << name << std::endl;
    std::cout << "Age:              " << age << std::endl;
    std::cout << "Contact:          " << contactNumber << std::endl;
    std::cout << "Consultation Fee: " << formatFee(consultationFee) << std::endl;
    std::cout << "Appointment:      " << getAppointmentDetails() << std::endl;
    std::cout << "-------------------------------------" << std::endl;
}

std::string GeneralPatient::getSpecialDetail() const {
    return "Fee: " + formatFee(consultationFee);
}

void GeneralPatient::restoreFrom(const PatientRecord& previous) {
    ScheduledPatient::restoreFrom(previous);

    const GeneralPatient& snapshot = static_cast<const GeneralPatient&>(previous);
    consultationFee = snapshot.consultationFee;
}

} // namespace model
} // namespace scapms
