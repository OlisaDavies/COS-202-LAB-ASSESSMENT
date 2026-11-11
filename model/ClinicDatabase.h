#pragma once

#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <vector>

#include "PatientRecord.h"
#include "Appointment.h"
#include "Appointable.h"
#include "AdminAction.h"

namespace scapms {
namespace model {

// The store of the clinic. It holds every patient record, every appointment,
// the waiting room and the administrative undo history.
//
// The class only stores and finds. The rules of the clinic - what age a
// patient may have, whether a booking is allowed - are not decided here but
// in the controller layer, so that the data and the rules stay apart.
//
// Ownership rules, so that the memory handling stays clear:
//   * the database owns the records in its patient list and deletes them,
//   * the undo stack owns one copy of the record for each patient action,
//   * the waiting room only borrows the pointers it is given.
class ClinicDatabase {
public:
    // Size of the fixed array that caches the recently accessed records.
    static constexpr int CACHE_SIZE = 10;

private:
    // The database owns every patient record. The unique pointers make sure
    // that a record is freed exactly once, even when the database is
    // cleared or an action is undone.
    std::vector<std::unique_ptr<PatientRecord>> patients;
    std::queue<PatientRecord*> waitingQueue;
    std::stack<AdminAction> undoStack;

    // Fixed size cache of the most recently accessed records.
    PatientRecord* recentPatients[CACHE_SIZE];
    int recentCount;

    // Adds one patient, with or without an entry on the undo stack.
    void addPatientInternal(std::unique_ptr<PatientRecord> patient, bool trackUndo);

    // Adds one appointment to the patient that owns it, with or without an
    // entry on the undo stack.
    void addAppointmentInternal(const Appointment& appointment, bool trackUndo);

    // Removes a record without touching the undo history. When backupOut is
    // given it receives a copy of the removed record.
    bool removePatientInternal(const std::string& id,
                               std::unique_ptr<PatientRecord>* backupOut);

    // The patient that holds the given appointment, or nullptr.
    PatientRecord* findOwnerOfAppointment(const std::string& appointmentID);

    // True when an appointment with that identifier is already booked.
    bool appointmentIDIsTaken(const std::string& appointmentID) const;

    void touchRecentPatient(PatientRecord* patient);
    void removeRecentPatient(PatientRecord* patient);
    void removeFromWaitingQueue(PatientRecord* patient);

public:
    ClinicDatabase();
    ~ClinicDatabase();

    // The database is the owner of its records, so copying it is not allowed.
    ClinicDatabase(const ClinicDatabase&) = delete;
    ClinicDatabase& operator=(const ClinicDatabase&) = delete;

    // Registers a patient and remembers the step, so that it can be undone.
    // The record is only stored when its identifier is still free.
    void addPatient(std::unique_ptr<PatientRecord> patient);

    // Registers a patient without remembering the step. This is used when a
    // whole file is read, because the load itself is one administrative step
    // and not one step per record.
    void addPatientWithoutUndo(std::unique_ptr<PatientRecord> patient);

    // Deletes a patient and keeps a copy on the undo stack.
    // Returns false when no patient has that ID.
    bool removePatient(const std::string& id);

    // Stores a copy of a record so that the change can be undone later.
    // The database takes over the snapshot, which must not be a live record.
    void pushPatientSnapshot(std::unique_ptr<PatientRecord> snapshot);

    // Looks a patient up by ID. The fixed size cache is checked first.
    PatientRecord* findPatientByID(const std::string& id);

    // Looks a patient up by ID without touching the cache. Used by the
    // reports, which work on a read only database.
    const PatientRecord* findPatientByID(const std::string& id) const;

    // The registered records. The pointers are owned by the database.
    std::vector<PatientRecord*> getAllPatients() const;
    std::size_t getPatientCount() const { return patients.size(); }

    // Every appointment of every patient, gathered into one list.
    std::vector<Appointment> getAllAppointments() const;

    // Waiting room.
    void enqueuePatient(PatientRecord* patient);
    PatientRecord* callNextPatient();
    std::size_t getQueueSize() const { return waitingQueue.size(); }
    std::vector<PatientRecord*> getWaitingQueue() const;

    // Appointments.
    void addAppointment(const Appointment& appointment);

    // Adds an appointment without remembering the step on the undo stack.
    void addAppointmentWithoutUndo(const Appointment& appointment);

    bool cancelAppointment(const std::string& appointmentID);

    // Reverses the last administrative operation.
    // Returns the description of the operation, or an empty string when
    // there is nothing to undo.
    std::string undoLastAction();

    // Removes every record, appointment and undo entry.
    void clearAll();

    // The recently accessed records, most recent last.
    std::vector<PatientRecord*> getRecentPatients() const;

    // Console output, used by the console demonstration.
    void displayAllPatients() const;
    void displayAllAppointments() const;
};

} // namespace model
} // namespace scapms
