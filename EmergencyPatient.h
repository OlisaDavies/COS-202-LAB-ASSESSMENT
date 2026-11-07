#pragma once

#include "PatientRecord.h"

class EmergencyPatient : public PatientRecord {
private:
    int severityLevel;   // 1 = Critical, 2 = Urgent, 3 = Moderate
    std::string triageNote;

public:
    EmergencyPatient(std::string id, std::string patientName, int patientAge,
                     std::string contact, int severity, std::string note)
        : PatientRecord(id, patientName, patientAge, contact, "Emergency"),
          severityLevel(severity), triageNote(note) {}

    int getSeverityLevel() const { return severityLevel; }
    void setSeverityLevel(int level) { severityLevel = level; }

    std::string getTriageNote() const { return triageNote; }
    void setTriageNote(std::string note) { triageNote = note; }

    std::string getUrgencyDescription() const {
        switch (severityLevel) {
            case 1:  return "CRITICAL - Immediate Attention Required!";
            case 2:  return "URGENT - Attend within 15 minutes";
            default: return "MODERATE - Priority waiting";
        }
    }

    void displayPatient() const override {
        std::cout << "-------------------------------------" << std::endl;
        std::cout << "[CATEGORY: EMERGENCY - HIGH PRIORITY]" << std::endl;
        std::cout << "Patient ID:     " << patientID        << std::endl;
        std::cout << "Name:           " << name              << std::endl;
        std::cout << "Age:            " << age               << std::endl;
        std::cout << "Contact:        " << contactNumber     << std::endl;
        std::cout << "Severity (1-3): " << severityLevel     << " (" << getUrgencyDescription() << ")" << std::endl;
        std::cout << "Triage Note:    " << triageNote        << std::endl;
        std::cout << "-------------------------------------" << std::endl;
    }

    std::string getSpecialDetail() const override {
        return "Severity " + std::to_string(severityLevel) + " (" + triageNote + ")";
    }

    PatientRecord* clone() const override {
        return new EmergencyPatient(patientID, name, age, contactNumber, severityLevel, triageNote);
    }

    void restoreFrom(const PatientRecord& previous) override {
        const EmergencyPatient& p = static_cast<const EmergencyPatient&>(previous);
        name = p.name;
        age = p.age;
        contactNumber = p.contactNumber;
        severityLevel = p.severityLevel;
        triageNote = p.triageNote;
    }
};
