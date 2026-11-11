#pragma once

#include <string>
#include <vector>

#include "../model/ClinicDatabase.h"
#include "../model/Appointment.h"

namespace scapms {
namespace controller {

// The rules that govern an appointment.
//
// The controller checks the rules of the clinic and then hands the booking to
// the database, which only stores it. That keeps the two apart: the rules are
// here, the data is in the model layer.
class AppointmentController {
public:
    explicit AppointmentController(model::ClinicDatabase& database);

    // Books an appointment for a patient.
    //
    // The date and the time have to be written in the format the clinic
    // uses, the patient has to be registered, and the category of the patient
    // has to be one that can be booked in advance. Every rule that is broken
    // is reported with an InvalidAppointmentException.
    void scheduleAppointment(const model::Appointment& appointment);

    // Cancels the appointment with the given identifier. Returns false when
    // no such appointment is booked.
    bool cancelAppointment(const std::string& appointmentID);

    // The next free appointment identifier, for example "APT-1004".
    std::string suggestAppointmentId() const;

private:
    // The rules that a booking has to follow.
    void checkBookingRules(const model::Appointment& appointment) const;

    // True when the patient is registered and can be booked in advance.
    bool patientCanBeBooked(const model::Appointment& appointment) const;

    model::ClinicDatabase& database;
};

} // namespace controller
} // namespace scapms
