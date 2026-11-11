#pragma once

#include <string>

#include "Appointment.h"

namespace scapms {
namespace model {

// Interface for the patient types that can be booked for appointments. It
// only declares operations and holds no data, so any class that can be
// scheduled can implement it. Emergency patients deliberately do not, because
// a trauma case is never booked in advance.
class Appointable {
public:
    virtual ~Appointable() = default;

    // Books the given appointment for this patient. The patient keeps the
    // appointment in its own collection, so the booking and the patient are
    // always in step. Returns false when the booking is not allowed.
    virtual bool scheduleAppointment(const Appointment& appointment) = 0;

    // Cancels the booking that comes first. Returns false when the patient
    // holds no booking at all.
    virtual bool cancelAppointment() = 0;

    // Cancels the appointment with the given identifier. Returns false when
    // the patient does not hold that appointment.
    virtual bool cancelAppointment(const std::string& appointmentID) = 0;

    // True when the patient currently holds a booking that has not been
    // completed or cancelled yet.
    virtual bool hasScheduledAppointment() const = 0;

    // A short description of the booking that is due next.
    virtual std::string getAppointmentDetails() const = 0;
};

} // namespace model
} // namespace scapms
