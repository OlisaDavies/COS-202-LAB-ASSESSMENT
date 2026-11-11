#include "ScheduledPatient.h"

namespace scapms {
namespace model {

bool ScheduledPatient::scheduleAppointment(const std::string& date, const std::string& time) {
    appointmentDate = date;
    appointmentTime = time;
    appointmentBooked = true;
    return true;
}

bool ScheduledPatient::cancelAppointment() {
    if (!appointmentBooked) {
        return false;                          // nothing to cancel
    }
    appointmentBooked = false;
    appointmentDate = "None";
    appointmentTime = "None";
    return true;
}

bool ScheduledPatient::hasScheduledAppointment() const {
    return appointmentBooked;
}

std::string ScheduledPatient::getAppointmentDetails() const {
    if (!appointmentBooked) {
        return "No active appointment";
    }
    return appointmentDate + " at " + appointmentTime;
}

void ScheduledPatient::restoreFrom(const PatientRecord& previous) {
    PatientRecord::restoreFrom(previous);

    const ScheduledPatient& snapshot = static_cast<const ScheduledPatient&>(previous);
    appointmentBooked = snapshot.appointmentBooked;
    appointmentDate = snapshot.appointmentDate;
    appointmentTime = snapshot.appointmentTime;
}

} // namespace model
} // namespace scapms
