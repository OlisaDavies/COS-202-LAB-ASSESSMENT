#pragma once

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

// The clinic database. It holds every patient record, every appointment, the
// waiting room and the administrative undo history.
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
    std::vector<PatientRecord*> patients;
    std::vector<Appointment> appointments;
    std::queue<PatientRecord*> waitingQueue;
    std::stack<AdminAction> undoStack;

    // Fixed size cache of the most recently accessed records.
    PatientRecord* recentPatients[CACHE_SIZE];
    int recentCount;

    // Adds one patient, with or without an entry on the undo stack.
    void addPatientInternal(PatientRecord* patient, bool trackUndo);

    // Adds one appointment, with or without an entry on the undo stack.
    void addAppointmentInternal(const Appointment& appointment, bool trackUndo);

    // Removes a record without touching the undo history. When backupOut is
    // given it receives a copy of the removed record.
    bool removePatientInternal(const std::string& id, PatientRecord** backupOut);

    // Refreshes the booking summary of a patient from the appointment list.
    void syncPatientBooking(const std::string& patientID);

    // The earliest pending appointment of a patient, or nullptr.
    const Appointment* nextPendingAppointment(const std::string& patientID) const;

    void touchRecentPatient(PatientRecord* patient);
    void removeRecentPatient(PatientRecord* patient);
    void removeFromWaitingQueue(PatientRecord* patient);

public:
    ClinicDatabase();
    ~ClinicDatabase();

    // The database is the owner of its records, so copying it is not allowed.
    ClinicDatabase(const ClinicDatabase&) = delete;
    ClinicDatabase& operator=(const ClinicDatabase&) = delete;

    // Registers a new patient. The database takes over the pointer, also when
    // the record is rejected, so the caller never has to delete it.
    void addPatient(PatientRecord* patient);

    // Registers several patients at once, for example after a file was read.
    // The records are not put on the undo stack. Records that break a rule are
    // skipped and described in the warnings list.
    void addPatients(const std::vector<PatientRecord*>& newPatients,
                     std::vector<std::string>* warnings = nullptr);

    // Deletes a patient and keeps a copy on the undo stack.
    // Returns false when no patient has that ID.
    bool removePatient(const std::string& id);

    // Stores a copy of a record so that the change can be undone later.
    // The database takes over the snapshot, which must not be a live record.
    void pushPatientSnapshot(PatientRecord* snapshot);

    // Looks a patient up by ID. The fixed size cache is checked first.
    PatientRecord* findPatientByID(const std::string& id);

    // Looks a patient up by ID without touching the cache. Used by the
    // reports, which work on a read only database.
    PatientRecord* findPatientByID(const std::string& id) const;

    const std::vector<PatientRecord*>& getAllPatients() const { return patients; }
    std::size_t getPatientCount() const { return patients.size(); }

    // Every appointment that belongs to a patient.
    std::vector<Appointment> getAppointmentsFor(const std::string& patientID) const;

    // How many of the appointments of a patient are still pending.
    std::size_t getPendingCountFor(const std::string& patientID) const;

    // Waiting room.
    void enqueuePatient(PatientRecord* patient);
    PatientRecord* callNextPatient();
    std::size_t getQueueSize() const { return waitingQueue.size(); }
    std::vector<PatientRecord*> getWaitingQueue() const;

    // Appointments.
    void addAppointment(const Appointment& appointment);

    // Adds several appointments at once. The appointments are not put on the
    // undo stack.
    void addAppointments(const std::vector<Appointment>& newAppointments,
                         std::vector<std::string>* warnings = nullptr);

    bool cancelAppointment(const std::string& appointmentID);
    const std::vector<Appointment>& getAllAppointments() const { return appointments; }

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
