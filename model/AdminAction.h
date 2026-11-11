#pragma once

#include <string>

#include "PatientRecord.h"
#include "Appointment.h"

namespace scapms {
namespace model {

// One entry on the administrative undo stack.
//
// An entry remembers what was done, which record it was about and a copy of
// that record. The copy is a separate object, so the undo stack and the
// roster never share the same memory.
class AdminAction {
public:
    enum class Type {
        AddPatient,
        DeletePatient,
        UpdatePatient,
        AddAppointment,
        CancelAppointment
    };

    Type type;
    std::string description;
    std::string patientID;             // the record the action is about
    PatientRecord* patientBackup;      // a copy of the record
    Appointment appointmentBackup;

    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID,
                PatientRecord* backup = nullptr)
        : type(actionType), description(text), patientID(affectedPatientID),
          patientBackup(backup), appointmentBackup("", "", "", "", "") {}

    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID,
                const Appointment& appointment)
        : type(actionType), description(text), patientID(affectedPatientID),
          patientBackup(nullptr), appointmentBackup(appointment) {}
};

} // namespace model
} // namespace scapms
