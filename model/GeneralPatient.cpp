#include "GeneralPatient.h"

#include <iomanip>
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

std::string GeneralPatient::getPatientReport() const {
    std::ostringstream out;
    out << "-------------------------------------\n";
    out << "[Category: General Outpatient]\n";
    out << "Patient ID:       " << patientID << "\n";
    out << "Name:             " << name << "\n";
    out << "Age:              " << age << "\n";
    out << "Contact:          " << contactNumber << "\n";
    out << "Consultation Fee: " << formatFee(consultationFee) << "\n";
    out << "Appointment:      " << getAppointmentDetails() << "\n";
    out << "-------------------------------------";
    return out.str();
}

std::string GeneralPatient::getSpecialDetail() const {
    return "Fee: " + formatFee(consultationFee);
}

double GeneralPatient::getBillAmount() const {
    return consultationFee;
}

std::string GeneralPatient::describeBill() const {
    return "Consultation fee: " + formatFee(consultationFee);
}

void GeneralPatient::restoreFrom(const PatientRecord& previous) {
    ScheduledPatient::restoreFrom(previous);

    // The dynamic cast checks that the snapshot really is a general patient.
    // A static cast would be undefined behaviour when it is not.
    if (const GeneralPatient* snapshot = dynamic_cast<const GeneralPatient*>(&previous)) {
        consultationFee = snapshot->consultationFee;
    }
}

} // namespace model
} // namespace scapms
