#include "ScheduledPatient.h"

namespace scapms {
namespace model {

bool ScheduledPatient::scheduleAppointment(const Appointment& appointment) {
    if (appointment.getDate().empty() || appointment.getTime().empty()) {
        return false;                          // a booking needs a date and a time
    }
    return addAppointment(appointment);
}

bool ScheduledPatient::cancelAppointment() {
    const Appointment* next = getNextPendingAppointment();
    if (next == nullptr) {
        return false;                          // there is no booking to cancel
    }
    return cancelAppointment(next->getAppointmentID());
}

bool ScheduledPatient::cancelAppointment(const std::string& appointmentID) {
    Appointment* target = findAppointmentForUpdate(appointmentID);
    if (target == nullptr) {
        return false;                          // the patient does not hold that booking
    }
    if (target->getStatus() != Appointment::STATUS_PENDING) {
        return false;                          // it was cancelled or completed already
    }
    target->setStatus(Appointment::STATUS_CANCELLED);
    return true;
}

bool ScheduledPatient::hasScheduledAppointment() const {
    return getPendingAppointmentCount() > 0;
}

std::string ScheduledPatient::getAppointmentDetails() const {
    const Appointment* next = getNextPendingAppointment();
    if (next == nullptr) {
        return "No active appointment";
    }
    return next->getDate() + " at " + next->getTime();
}

double ScheduledPatient::getBillAmount() const {
    return 0.0;                                // this category does not charge
}

std::string ScheduledPatient::describeBill() const {
    return "No charge is recorded for this category.";
}

} // namespace model
} // namespace scapms
