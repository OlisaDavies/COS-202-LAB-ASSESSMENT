#pragma once

#include <string>
#include <utility>

#include "ScheduledPatient.h"

namespace scapms {
namespace model {

// A returning patient that is seen again for the same complaint. The record
// keeps the previous visit and the doctor who is looking after the case.
class FollowUpPatient : public ScheduledPatient {
private:
    std::string previousVisitDate;
    std::string assignedDoctor;

public:
    FollowUpPatient(std::string id, std::string patientName, int patientAge,
                    std::string contact, std::string prevDate, std::string doctor)
        : ScheduledPatient(id, patientName, patientAge,
                           contact, PatientRecord::TYPE_FOLLOWUP),
          previousVisitDate(prevDate),
          assignedDoctor(doctor) {}

    FollowUpPatient(const FollowUpPatient& other) = default;

    const std::string& getPreviousVisitDate() const { return previousVisitDate; }
    void setPreviousVisitDate(std::string date) { previousVisitDate = date; }

    const std::string& getAssignedDoctor() const { return assignedDoctor; }
    void setAssignedDoctor(std::string doctor) { assignedDoctor = doctor; }

    std::string getTypeLabel() const override { return "Follow-Up Patient"; }

    void displayPatient() const override;
    std::string getSpecialDetail() const override;

    PatientRecord* clone() const override { return new FollowUpPatient(*this); }

    void restoreFrom(const PatientRecord& previous) override;
};

} // namespace model
} // namespace scapms
