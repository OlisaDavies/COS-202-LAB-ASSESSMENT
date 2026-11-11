#pragma once

#include <string>
#include <utility>

#include "PatientRecord.h"
#include "Appointable.h"

namespace scapms {
namespace model {

// Intermediate class for the patient types that can be booked.
//
// It implements the Appointable interface once and keeps the current booking,
// so GeneralPatient and FollowUpPatient do not have to repeat the same code.
// The booking itself is a summary of the appointments held by the database;
// the database refreshes it whenever an appointment changes.
class ScheduledPatient : public PatientRecord, public Appointable {
protected:
    bool appointmentBooked;
    std::string appointmentDate;
    std::string appointmentTime;

public:
    ScheduledPatient(std::string id, std::string patientName, int patientAge,
                     std::string contact, std::string type)
        : PatientRecord(id, patientName, patientAge,
                        contact, type),
          appointmentBooked(false),
          appointmentDate("None"),
          appointmentTime("None") {}

    ScheduledPatient(const ScheduledPatient& other) = default;

    bool scheduleAppointment(const std::string& date, const std::string& time) override;
    bool cancelAppointment() override;
    bool hasScheduledAppointment() const override;
    std::string getAppointmentDetails() const override;

    void restoreFrom(const PatientRecord& previous) override;
};

} // namespace model
} // namespace scapms
