#pragma once

#include <string>

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

    // The full record of the patient as text.
    std::string getPatientReport() const override;

    std::string getSpecialDetail() const override;

    // A general outpatient visit is billed for the consultation fee.
    double getBillAmount() const override;
    std::string describeBill() const override;

    // The covariant return type hands back a GeneralPatient, so the caller
    // does not have to cast the copy back down.
    GeneralPatient* clone() const override { return new GeneralPatient(*this); }

    void restoreFrom(const PatientRecord& previous) override;
};

} // namespace model
} // namespace scapms
