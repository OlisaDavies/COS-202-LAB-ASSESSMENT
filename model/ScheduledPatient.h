#pragma once

#include <string>

#include "PatientRecord.h"
#include "Appointable.h"
#include "Billable.h"

namespace scapms {
namespace model {

// Intermediate abstract class for the patient types that can be booked.
//
// It inherits from the abstract base class and from two pure abstract
// interfaces at the same time, which shows that a class may implement more
// than one interface. It writes the booking code once, so GeneralPatient and
// FollowUpPatient do not have to repeat it.
//
// The booking itself lives in the appointment collection of the base class,
// so there is only ever one copy of the truth.
class ScheduledPatient : public PatientRecord, public Appointable, public Billable {
public:
    ScheduledPatient(std::string id, std::string patientName, int patientAge,
                     std::string contact, std::string type)
        : PatientRecord(id, patientName, patientAge, contact, type) {}

    ScheduledPatient(const ScheduledPatient& other) = default;

    // ---- Appointable ----

    // Books the appointment in this patient's own collection. The booking is
    // refused when it has no date or no time.
    bool scheduleAppointment(const Appointment& appointment) override;

    // Cancels the booking that comes first.
    bool cancelAppointment() override;

    // Marks the appointment with that identifier as cancelled. The booking
    // stays in the collection, because the clinic keeps a record of it.
    bool cancelAppointment(const std::string& appointmentID) override;

    // True when the patient holds a booking that has not been dealt with yet.
    bool hasScheduledAppointment() const override;

    // A short description of the booking that comes first.
    std::string getAppointmentDetails() const override;

    // ---- Billable ----
    // The billing contract is answered here with the default of a category
    // that does not charge for its visits. The categories that do charge
    // override the two methods.
    double getBillAmount() const override;
    std::string describeBill() const override;
};

} // namespace model
} // namespace scapms
