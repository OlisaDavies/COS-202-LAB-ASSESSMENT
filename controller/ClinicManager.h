#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../model/ClinicDatabase.h"
#include "../model/PatientRecord.h"
#include "../model/Appointment.h"
#include "AppointmentController.h"

namespace scapms {
namespace controller {

// The controller of the clinic. It is the only class that the graphical
// interface talks to.
//
// The program is built in three layers:
//
//   * the model layer holds the data and the rules that keep it consistent,
//   * the controller layer holds the rules of the clinic and coordinates the
//     work between the screen and the data,
//   * the GUI layer draws the screens and asks this class to do the work.
//
// The screens never touch the database directly, and the database never
// decides what is allowed. Everything the staff can do goes through this
// class, so the rules are written down in one place.
class ClinicManager {
public:
    ClinicManager();
    ~ClinicManager();

    // The manager owns its data, so it cannot be copied.
    ClinicManager(const ClinicManager&) = delete;
    ClinicManager& operator=(const ClinicManager&) = delete;

    // ---- patients ----

    // Registers a new patient. The record is checked against the rules of
    // the clinic first, so a record that breaks a rule is refused with an
    // exception and the caller is told why. The step is remembered, so it
    // can be undone.
    void registerPatient(std::unique_ptr<model::PatientRecord> patient);

    // Registers several patients at once, for example after a file was read.
    // The records that break a rule are skipped and described in the
    // warnings list. The load is one administrative step as a whole, so the
    // records are not remembered one by one.
    void registerPatients(std::vector<std::unique_ptr<model::PatientRecord>> patients,
                          std::vector<std::string>* warnings = nullptr);

    // Checks a record against the rules of the clinic without storing it.
    // The edit panel uses this, so that an edit cannot put an illegal
    // value into a record that was registered properly.
    void validatePatient(const model::PatientRecord& patient) const {
        checkPatientRules(patient);
    }

    // Deletes a patient. The step can be undone.
    bool deletePatient(const std::string& patientID);

    // Looks a patient up by its identifier.
    model::PatientRecord* findPatientByID(const std::string& patientID);
    const model::PatientRecord* findPatientByID(const std::string& patientID) const;

    // Every registered patient. The records belong to the manager.
    std::vector<model::PatientRecord*> getAllPatients() const;
    std::size_t getPatientCount() const;

    // Puts the records in the given order, so that the roster and the
    // search results show the same order.
    void setPatientOrder(const std::vector<model::PatientRecord*>& order);

    // The next free identifier for a category, for example "GEN-106".
    std::string suggestPatientId(const std::string& patientType) const;

    // Keeps a copy of a record so that the change can be undone later.
    void pushPatientSnapshot(std::unique_ptr<model::PatientRecord> snapshot);

    // ---- appointments ----

    // Books an appointment. The rules of the clinic are checked first.
    // A booking that comes from a file is not remembered on the undo stack.
    void scheduleAppointment(const model::Appointment& appointment, bool trackUndo = true);

    // Cancels an appointment. The step can be undone.
    bool cancelAppointment(const std::string& appointmentID);

    // Marks a visit as completed. The step can be undone.
    bool completeAppointment(const std::string& appointmentID);

    // Every appointment of the clinic.
    std::vector<model::Appointment> getAllAppointments() const;

    // ---- the waiting room ----

    void enqueuePatient(model::PatientRecord* patient);
    model::PatientRecord* callNextPatient();
    std::size_t getQueueSize() const;
    std::vector<model::PatientRecord*> getWaitingQueue() const;

    // ---- administration ----

    // Reverses the last administrative operation. The description of the
    // operation is returned, or an empty string when there is nothing to
    // undo.
    std::string undoLastAction();

    // Removes every record and every appointment.
    void clearAll();

    // The records that were used most recently, most recent last.
    std::vector<model::PatientRecord*> getRecentPatients() const;

    // Writes the patients and the appointments to the two data files.
    void saveToFiles(const std::string& patientFile, const std::string& appointmentFile);

    // Reads the patients and the appointments from the two data files.
    // The rows that cannot be read are described in the warnings list.
    void loadFromFiles(const std::string& patientFile, const std::string& appointmentFile,
                       std::vector<std::string>* warnings = nullptr);

    // ---- the parts of the manager ----

    // The controller of the appointments.
    AppointmentController& appointments() { return appointmentController; }
    const AppointmentController& appointments() const { return appointmentController; }

    // The data of the clinic, for the reports and for the console
    // demonstration.
    const model::ClinicDatabase& database() const { return clinicDatabase; }

private:
    // The rules that a patient record has to follow.
    void checkPatientRules(const model::PatientRecord& patient) const;

    model::ClinicDatabase clinicDatabase;
    AppointmentController appointmentController;
};

} // namespace controller
} // namespace scapms
