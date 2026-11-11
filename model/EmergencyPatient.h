#pragma once

#include <string>
#include <utility>

#include "PatientRecord.h"

namespace scapms {
namespace model {

// An urgent case that is treated as soon as it arrives.
//
// It deliberately does not implement Appointable: a trauma case is never
// booked for a future slot, it goes straight into the waiting room.
class EmergencyPatient : public PatientRecord {
private:
    int severityLevel;      // 1 = Critical, 2 = Urgent, 3 = Moderate
    std::string triageNote;

public:
    EmergencyPatient(std::string id, std::string patientName, int patientAge,
                     std::string contact, int severity, std::string note)
        : PatientRecord(id, patientName, patientAge,
                        contact, PatientRecord::TYPE_EMERGENCY),
          severityLevel(severity),
          triageNote(note) {}

    EmergencyPatient(const EmergencyPatient& other) = default;

    int getSeverityLevel() const { return severityLevel; }
    void setSeverityLevel(int level) { severityLevel = level; }

    const std::string& getTriageNote() const { return triageNote; }
    void setTriageNote(std::string note) { triageNote = note; }

    // Reads the severity level as a short piece of text.
    std::string getUrgencyDescription() const;

    std::string getTypeLabel() const override { return "Emergency Patient"; }

    void displayPatient() const override;
    std::string getSpecialDetail() const override;

    PatientRecord* clone() const override { return new EmergencyPatient(*this); }

    void restoreFrom(const PatientRecord& previous) override;
};

} // namespace model
} // namespace scapms
