#pragma once

#include <memory>
#include <string>

#include "PatientRecord.h"
#include "Appointment.h"

namespace scapms {
namespace model {

// One entry on the administrative undo stack.
//
// An entry remembers what was done and holds a copy of the affected record.
// The copy is a separate object, so the undo stack and the roster never share
// the same memory.
class AdminAction {
public:
    enum class Type {
        AddPatient,
        DeletePatient,
        UpdatePatient,
        AddAppointment,
        CancelAppointment,
        CompleteAppointment
    };

    Type type;
    std::string description;
    std::string patientID;
    std::unique_ptr<PatientRecord> patientBackup;
    Appointment appointmentBackup;

    // The place the record held in the waiting queue when it was deleted, or
    // -1 when it was not waiting.
    int waitingQueuePosition;

    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID);
    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID,
                std::unique_ptr<PatientRecord> backup);
    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID,
                const Appointment& appointment);

    // The entry owns a record, so it can be moved but not copied.
    AdminAction(const AdminAction&) = delete;
    AdminAction& operator=(const AdminAction&) = delete;
    AdminAction(AdminAction&&) = default;
    AdminAction& operator=(AdminAction&&) = default;
    ~AdminAction() = default;
};

} // namespace model
} // namespace scapms

