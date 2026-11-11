#pragma once

#include <string>
#include <utility>

#include "ScheduledPatient.h"

namespace scapms {
namespace model {

// A standard outpatient visit. The patient pays a consultation fee and can be
// booked for a scheduled consultation.
class GeneralPatient : public ScheduledPatient {
private:
    double consultationFee;

public:
    GeneralPatient(std::string id, std::string patientName, int patientAge,
                   std::string contact, double fee)
        : ScheduledPatient(id, patientName, patientAge,
                           contact, PatientRecord::TYPE_GENERAL),
          consultationFee(fee) {}

    GeneralPatient(const GeneralPatient& other) = default;

    double getConsultationFee() const { return consultationFee; }
    void setConsultationFee(double fee) { consultationFee = fee; }

    std::string getTypeLabel() const override { return "General Outpatient"; }

    void displayPatient() const override;
    std::string getSpecialDetail() const override;

    PatientRecord* clone() const override { return new GeneralPatient(*this); }

    void restoreFrom(const PatientRecord& previous) override;
};

} // namespace model
} // namespace scapms
