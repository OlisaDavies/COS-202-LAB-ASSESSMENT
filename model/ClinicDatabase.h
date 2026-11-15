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

// The store of the clinic: the patient records, the waiting room and the
// undo history. It only stores and finds; the rules of the clinic live in
// the controller layer.
class ClinicDatabase {
public:
    static constexpr int CACHE_SIZE = 10;

private:
    // The database owns every patient record, so the unique pointers make
    // sure that a record is freed exactly once.
    std::vector<std::unique_ptr<PatientRecord>> patients;
    std::queue<PatientRecord*> waitingQueue;
    std::stack<AdminAction> undoStack;

    // Fixed size cache of the most recently accessed records. The array is
    // held from the oldest record at the front to the newest one at the
    // back, and only the first recentCount places of it are in use.
    PatientRecord* recentPatients[CACHE_SIZE];
    int recentCount;

    // Adds one patient, with or without an entry on the undo stack.
    void addPatientInternal(std::unique_ptr<PatientRecord> patient, bool trackUndo);

    // Adds one appointment to the patient that owns it, with or without an
    // entry on the undo stack.
    void addAppointmentInternal(const Appointment& appointment, bool trackUndo);

    // Removes a record without touching the undo history. The two output
    // parameters are optional: the first receives a copy of the record, the
    // second the place it held in the waiting queue.
    bool removePatientInternal(const std::string& id,
                               std::unique_ptr<PatientRecord>* backupOut,
                               int* waitingPositionOut);

    PatientRecord* findOwnerOfAppointment(const std::string& appointmentID);

    bool appointmentIDIsTaken(const std::string& appointmentID) const;

    void touchRecentPatient(PatientRecord* patient);
    void removeRecentPatient(PatientRecord* patient);
    void removeFromWaitingQueue(PatientRecord* patient);

    // The place the record holds in the waiting queue, or -1 when it is not
    // waiting.
    int waitingQueuePosition(const PatientRecord* patient) const;

    // Puts a record back into the waiting queue at the given place. A
    // std::queue cannot put an element in the middle, so it is rebuilt.
    void insertIntoWaitingQueue(PatientRecord* patient, int position);

public:
    ClinicDatabase();
    ~ClinicDatabase();

    // The database is the owner of its records, so copying it is not allowed.
    ClinicDatabase(const ClinicDatabase&) = delete;
    ClinicDatabase& operator=(const ClinicDatabase&) = delete;

    // Registers a patient and remembers the step, so that it can be undone.
    void addPatient(std::unique_ptr<PatientRecord> patient);

    // Registers a patient without remembering the step. Used when a whole
    // file is read, because the load is one step and not one per record.
    void addPatientWithoutUndo(std::unique_ptr<PatientRecord> patient);

    // Deletes a patient and keeps a copy on the undo stack.
    bool removePatient(const std::string& id);

    void pushPatientSnapshot(std::unique_ptr<PatientRecord> snapshot);

    // Looks a patient up by ID. The fixed size cache is checked first.
    PatientRecord* findPatientByID(const std::string& id);

    // Looks a patient up by ID without touching the cache. Used by the
    // reports, which work on a read only database.
    const PatientRecord* findPatientByID(const std::string& id) const;

    // The registered records. The pointers are owned by the database.
    std::vector<PatientRecord*> getAllPatients() const;
    std::size_t getPatientCount() const { return patients.size(); }

    // Puts the records in the given order. The pointers have to be the
    // records of this database. This is what the sort on the screen uses.
    void setPatientOrder(const std::vector<PatientRecord*>& order);

    std::vector<Appointment> getAllAppointments() const;

    void enqueuePatient(PatientRecord* patient);
    PatientRecord* callNextPatient();
    std::size_t getQueueSize() const { return waitingQueue.size(); }
    std::vector<PatientRecord*> getWaitingQueue() const;

    void addAppointment(const Appointment& appointment);

    void addAppointmentWithoutUndo(const Appointment& appointment);

    bool cancelAppointment(const std::string& appointmentID);

    // Marks a booking as completed, so that the visit counts towards the
    // attendance of the patient.
    bool completeAppointment(const std::string& appointmentID);

    // Reverses the last administrative operation. Returns the description of
    // the operation, or an empty string when there is nothing to undo.
    std::string undoLastAction();

    void clearAll();

    // The recently accessed records, most recently accessed first.
    std::vector<PatientRecord*> getRecentPatients() const;

    // Console output, used by the console demonstration.
    void displayAllPatients() const;
    void displayAllAppointments() const;
};

} // namespace model
} // namespace scapms
