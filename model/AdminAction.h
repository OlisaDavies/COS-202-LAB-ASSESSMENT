#pragma once

#include <memory>
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
//
// The class owns a record, so it follows the rule of five: it can be moved
// but not copied.
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
    std::string patientID;             // the record the action is about
    std::unique_ptr<PatientRecord> patientBackup;   // a copy of the record
    Appointment appointmentBackup;

    // The place the record held in the waiting queue when it was deleted,
    // counted from the front. A value of -1 means that the record was not
    // waiting, so undoing the delete must not put it back into the queue.
    int waitingQueuePosition;

    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID);
    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID,
                std::unique_ptr<PatientRecord> backup);
    AdminAction(Type actionType, const std::string& text, const std::string& affectedPatientID,
                const Appointment& appointment);

    // The entry owns a record, so copying it would copy the ownership as
    // well. It can be moved, which is what the stack does.
    AdminAction(const AdminAction&) = delete;
    AdminAction& operator=(const AdminAction&) = delete;
    AdminAction(AdminAction&&) = default;
    AdminAction& operator=(AdminAction&&) = default;
    ~AdminAction() = default;
};

} // namespace model
} // namespace scapms
