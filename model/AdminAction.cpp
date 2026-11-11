#include "AdminAction.h"

namespace scapms {
namespace model {

AdminAction::AdminAction(Type actionType, const std::string& text,
                         const std::string& affectedPatientID)
    : type(actionType),
      description(text),
      patientID(affectedPatientID),
      patientBackup(nullptr),
      appointmentBackup("", "", "", "", "") {}

AdminAction::AdminAction(Type actionType, const std::string& text,
                         const std::string& affectedPatientID,
                         std::unique_ptr<PatientRecord> backup)
    : type(actionType),
      description(text),
      patientID(affectedPatientID),
      patientBackup(std::move(backup)),
      appointmentBackup("", "", "", "", "") {}

AdminAction::AdminAction(Type actionType, const std::string& text,
                         const std::string& affectedPatientID,
                         const Appointment& appointment)
    : type(actionType),
      description(text),
      patientID(affectedPatientID),
      patientBackup(nullptr),
      appointmentBackup(appointment) {}

} // namespace model
} // namespace scapms
