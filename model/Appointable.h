#pragma once

#include <string>

namespace scapms {
namespace model {

// Interface for the patient types that can be booked for an appointment.
//
// It only declares operations and holds no data, so any class that can be
// scheduled can implement it. Emergency patients deliberately do not, because
// a trauma case is never booked in advance.
class Appointable {
public:
    virtual ~Appointable() = default;

    // Stores the given booking as the current appointment.
    virtual bool scheduleAppointment(const std::string& date, const std::string& time) = 0;

    // Removes the current booking.
    virtual bool cancelAppointment() = 0;

    // True when the patient currently holds a booking.
    virtual bool hasScheduledAppointment() const = 0;

    // A short description of the current booking.
    virtual std::string getAppointmentDetails() const = 0;
};

} // namespace model
} // namespace scapms
