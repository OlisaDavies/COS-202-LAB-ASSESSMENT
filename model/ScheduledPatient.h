#pragma once

#include <string>

#include "PatientRecord.h"
#include "Appointable.h"
#include "Billable.h"

namespace scapms {
namespace model {

// Intermediate abstract class for the patient types that can be booked. It
// inherits from the base class and from two interfaces at once, so a class
// may implement more than one contract, and it writes the booking code once
// for GeneralPatient and FollowUpPatient.
class ScheduledPatient : public PatientRecord, public Appointable, public Billable {
public:
    ScheduledPatient(std::string id, std::string patientName, int patientAge,
                     std::string contact, std::string type)
        : PatientRecord(id, patientName, patientAge, contact, type) {}

    ScheduledPatient(const ScheduledPatient& other) = default;

    // Books the appointment in this patient's own collection. The booking is
    // refused when it has no date or no time.
    bool scheduleAppointment(const Appointment& appointment) override;

    bool cancelAppointment() override;

    // Marks the appointment with that identifier as cancelled. The booking
    // stays in the collection, because the clinic keeps a record of it.
    bool cancelAppointment(const std::string& appointmentID) override;

    bool hasScheduledAppointment() const override;

    std::string getAppointmentDetails() const override;

    // The billing contract is answered here with the default of a category
    // that does not charge for its visits. The categories that do charge
    // override the two methods.
    double getBillAmount() const override;
    std::string describeBill() const override;
};

} // namespace model
} // namespace scapms
