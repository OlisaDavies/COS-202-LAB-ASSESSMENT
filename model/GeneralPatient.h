#pragma once

#include <cmath>
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
    // The consultation fee of the clinic. The rule lives with the data, so
    // that the screen, the controller and the file reader all use the same
    // numbers.
    static constexpr double MAX_CONSULTATION_FEE = 100000000.0;
    static constexpr double MIN_CONSULTATION_FEE = 0.0;

    GeneralPatient(std::string id, std::string patientName, int patientAge,
                   std::string contact, double fee)
        : ScheduledPatient(id, patientName, patientAge,
                           contact, PatientRecord::TYPE_GENERAL),
          consultationFee(fee) {}

    GeneralPatient(const GeneralPatient& other) = default;

    double getConsultationFee() const { return consultationFee; }
    void setConsultationFee(double fee) { consultationFee = fee; }

    // True when the fee is a real amount that the clinic can bill. A value
    // of "nan" or of infinity is not a real amount, even though every
    // comparison with it is false.
    static bool isValidConsultationFee(double fee) {
        return std::isfinite(fee) &&
               fee >= MIN_CONSULTATION_FEE &&
               fee <= MAX_CONSULTATION_FEE;
    }

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
