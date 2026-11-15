#pragma once

#include <string>

#include "Appointment.h"

namespace scapms {
namespace model {

// Interface for the patient types that can be booked for appointments. It only
// declares operations and holds no data. Emergency patients deliberately do
// not implement it, because a trauma case is never booked in advance.
class Appointable {
public:
    virtual ~Appointable() = default;

    // Books the given appointment for this patient. The patient keeps the
    // appointment in its own collection, so the booking and the patient stay
    // in step.
    virtual bool scheduleAppointment(const Appointment& appointment) = 0;

    // Cancels the booking that comes first. Returns false when the patient
    // holds no booking.
    virtual bool cancelAppointment() = 0;

    // Returns false when the patient does not hold that appointment.
    virtual bool cancelAppointment(const std::string& appointmentID) = 0;

    // True when the patient holds a booking that is still open.
    virtual bool hasScheduledAppointment() const = 0;

    // A short description of the booking that is due next.
    virtual std::string getAppointmentDetails() const = 0;
};

} // namespace model
} // namespace scapms
