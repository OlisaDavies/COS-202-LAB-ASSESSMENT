#include "ClinicDatabase.h"

#include <algorithm>
#include <iostream>

#include "../utils/ClinicExceptions.h"

namespace scapms {
namespace model {

ClinicDatabase::ClinicDatabase() : recentCount(0) {
    for (int i = 0; i < CACHE_SIZE; ++i) {
        recentPatients[i] = nullptr;
    }
}

ClinicDatabase::~ClinicDatabase() {
    clearAll();
}

void ClinicDatabase::addPatient(std::unique_ptr<PatientRecord> patient) {
    addPatientInternal(std::move(patient), true);
}

void ClinicDatabase::addPatientWithoutUndo(std::unique_ptr<PatientRecord> patient) {
    addPatientInternal(std::move(patient), false);
}

void ClinicDatabase::addPatientInternal(std::unique_ptr<PatientRecord> patient, bool trackUndo) {
    if (patient == nullptr) {
        throw utils::InvalidPatientDataException("Cannot register a null patient record.");
    }

    // The rule that a patient identifier is used only once belongs to the
    // data itself, so it is kept here. The other rules of the clinic are
    // checked by the controller before it calls this method.
    if (findPatientByID(patient->getPatientID()) != nullptr) {
        throw utils::DuplicateIDException(patient->getPatientID());
    }

    const std::string patientID = patient->getPatientID();
    patients.push_back(std::move(patient));
    touchRecentPatient(patients.back().get());

    if (trackUndo) {
        // Undoing this removes the record again, so no copy is needed here.
        undoStack.push(AdminAction(AdminAction::Type::AddPatient,
                                   "Add Patient " + patientID,
                                   patientID));
    }
}

bool ClinicDatabase::removePatientInternal(const std::string& id,
                                           std::unique_ptr<PatientRecord>* backupOut) {
    for (auto it = patients.begin(); it != patients.end(); ++it) {
        if ((*it)->getPatientID() != id) {
            continue;
        }

        PatientRecord* target = it->get();

        if (backupOut != nullptr) {
            // The undo stack owns this copy.
            backupOut->reset(target->clone());
        }

        removeRecentPatient(target);
        removeFromWaitingQueue(target);

        // The unique pointer frees the record when it leaves the vector.
        patients.erase(it);
        return true;
    }
    return false;
}

bool ClinicDatabase::removePatient(const std::string& id) {
    std::unique_ptr<PatientRecord> backup;
    if (!removePatientInternal(id, &backup)) {
        return false;
    }

    undoStack.push(AdminAction(AdminAction::Type::DeletePatient,
                               "Delete Patient " + id, id, std::move(backup)));
    return true;
}

void ClinicDatabase::pushPatientSnapshot(std::unique_ptr<PatientRecord> snapshot) {
    if (snapshot == nullptr) {
        return;
    }

    // The identifier is read into a local variable before the snapshot is
    // moved away. The order in which the arguments of a call are evaluated
    // is not fixed by the standard, so the move must not be written in the
    // same call as the reads.
    const std::string patientID = snapshot->getPatientID();

    undoStack.push(AdminAction(AdminAction::Type::UpdatePatient,
                               "Update Patient " + patientID,
                               patientID, std::move(snapshot)));
}

PatientRecord* ClinicDatabase::findPatientByID(const std::string& id) {
    for (int i = 0; i < recentCount; ++i) {
        if (recentPatients[i] != nullptr && recentPatients[i]->getPatientID() == id) {
            return recentPatients[i];
        }
    }

    for (std::unique_ptr<PatientRecord>& patient : patients) {
        if (patient->getPatientID() == id) {
            touchRecentPatient(patient.get());
            return patient.get();
        }
    }
    return nullptr;
}

const PatientRecord* ClinicDatabase::findPatientByID(const std::string& id) const {
    for (const std::unique_ptr<PatientRecord>& patient : patients) {
        if (patient->getPatientID() == id) {
            return patient.get();
        }
    }
    return nullptr;
}

std::vector<PatientRecord*> ClinicDatabase::getAllPatients() const {
    std::vector<PatientRecord*> records;
    records.reserve(patients.size());
    for (const std::unique_ptr<PatientRecord>& patient : patients) {
        records.push_back(patient.get());
    }
    return records;
}

std::vector<Appointment> ClinicDatabase::getAllAppointments() const {
    std::vector<Appointment> all;
    for (const std::unique_ptr<PatientRecord>& patient : patients) {
        const std::vector<Appointment>& owned = patient->getAppointments();
        all.insert(all.end(), owned.begin(), owned.end());
    }
    return all;
}

void ClinicDatabase::enqueuePatient(PatientRecord* patient) {
    if (patient != nullptr) {
        waitingQueue.push(patient);
    }
}

PatientRecord* ClinicDatabase::callNextPatient() {
    if (waitingQueue.empty()) {
        return nullptr;
    }
    PatientRecord* next = waitingQueue.front();
    waitingQueue.pop();
    return next;
}

std::vector<PatientRecord*> ClinicDatabase::getWaitingQueue() const {
    std::vector<PatientRecord*> order;
    std::queue<PatientRecord*> copy = waitingQueue;
    while (!copy.empty()) {
        order.push_back(copy.front());
        copy.pop();
    }
    return order;
}

PatientRecord* ClinicDatabase::findOwnerOfAppointment(const std::string& appointmentID) {
    for (std::unique_ptr<PatientRecord>& patient : patients) {
        if (patient->hasAppointment(appointmentID)) {
            return patient.get();
        }
    }
    return nullptr;
}

bool ClinicDatabase::appointmentIDIsTaken(const std::string& appointmentID) const {
    for (const std::unique_ptr<PatientRecord>& patient : patients) {
        if (patient->hasAppointment(appointmentID)) {
            return true;
        }
    }
    return false;
}

void ClinicDatabase::addAppointment(const Appointment& appointment) {
    addAppointmentInternal(appointment, true);
}

void ClinicDatabase::addAppointmentWithoutUndo(const Appointment& appointment) {
    addAppointmentInternal(appointment, false);
}

void ClinicDatabase::addAppointmentInternal(const Appointment& appointment, bool trackUndo) {
    // The rules of the clinic are checked by the controller before it calls
    // this method. What is left here are the rules that follow from the
    // shape of the data itself.
    if (appointment.getAppointmentID().empty() || appointment.getPatientID().empty() ||
        appointment.getDate().empty() || appointment.getTime().empty()) {
        throw utils::InvalidAppointmentException(
            "An appointment needs an ID, a patient, a date and a time.");
    }

    if (appointmentIDIsTaken(appointment.getAppointmentID())) {
        throw utils::InvalidAppointmentException(
            "Appointment ID [" + appointment.getAppointmentID() + "] is already booked.");
    }

    PatientRecord* patient = findPatientByID(appointment.getPatientID());
    if (patient == nullptr) {
        throw utils::InvalidAppointmentException(
            "No patient with ID [" + appointment.getPatientID() + "] is registered.");
    }

    // Only the categories that answer the Appointable interface can hold a
    // booking. The controller checks this rule first and reports it to the
    // staff, so reaching this point means the booking is allowed.
    Appointable* bookable = dynamic_cast<Appointable*>(patient);
    if (bookable == nullptr) {
        throw utils::InvalidAppointmentException(
            "A patient of the category [" + patient->getTypeLabel() +
            "] is not booked in advance. Please use the waiting room instead.");
    }

    if (!bookable->scheduleAppointment(appointment)) {
        throw utils::InvalidAppointmentException(
            "The appointment [" + appointment.getAppointmentID() + "] could not be booked.");
    }

    if (trackUndo) {
        undoStack.push(AdminAction(AdminAction::Type::AddAppointment,
                                   "Add Appointment " + appointment.getAppointmentID(),
                                   appointment.getPatientID(), appointment));
    }
}

bool ClinicDatabase::cancelAppointment(const std::string& appointmentID) {
    PatientRecord* owner = findOwnerOfAppointment(appointmentID);
    if (owner == nullptr) {
        return false;
    }

    Appointable* bookable = dynamic_cast<Appointable*>(owner);
    if (bookable == nullptr) {
        return false;
    }

    // The state before the cancellation is kept, so that the step can be
    // undone later.
    const Appointment* current = owner->findAppointment(appointmentID);
    if (current == nullptr) {
        return false;
    }
    const Appointment previous = *current;

    if (!bookable->cancelAppointment(appointmentID)) {
        return false;
    }

    undoStack.push(AdminAction(AdminAction::Type::CancelAppointment,
                               "Cancel Appointment " + appointmentID,
                               owner->getPatientID(), previous));
    return true;
}

std::string ClinicDatabase::undoLastAction() {
    if (undoStack.empty()) {
        return "";
    }

    // The entry is moved out of the stack, because it owns a record and
    // cannot be copied.
    AdminAction lastAction = std::move(undoStack.top());
    undoStack.pop();

    switch (lastAction.type) {
        case AdminAction::Type::AddPatient: {
            // The record was added, so undoing removes it again. This does not
            // push a new undo entry.
            if (!lastAction.patientID.empty()) {
                removePatientInternal(lastAction.patientID, nullptr);
            }
            return lastAction.description;
        }

        case AdminAction::Type::DeletePatient: {
            if (lastAction.patientBackup == nullptr) {
                return "";
            }
            // The copy on the stack becomes a live record again.
            PatientRecord* restored = lastAction.patientBackup.get();
            patients.push_back(std::move(lastAction.patientBackup));
            touchRecentPatient(restored);
            return lastAction.description;
        }

        case AdminAction::Type::UpdatePatient: {
            if (lastAction.patientBackup == nullptr) {
                return "";
            }
            PatientRecord* current = findPatientByID(lastAction.patientBackup->getPatientID());
            if (current != nullptr) {
                current->restoreFrom(*lastAction.patientBackup);
            }
            return lastAction.description;
        }

        case AdminAction::Type::AddAppointment: {
            const std::string appointmentID = lastAction.appointmentBackup.getAppointmentID();
            PatientRecord* owner = findOwnerOfAppointment(appointmentID);
            if (owner != nullptr) {
                owner->removeAppointment(appointmentID);
            }
            return lastAction.description;
        }

        case AdminAction::Type::CancelAppointment: {
            const Appointment& previous = lastAction.appointmentBackup;
            PatientRecord* owner = findOwnerOfAppointment(previous.getAppointmentID());
            if (owner != nullptr) {
                owner->restoreAppointment(previous);
            }
            return lastAction.description;
        }
    }
    return "";
}

void ClinicDatabase::clearAll() {
    // The undo entries own their copies of the records, so popping them off
    // the stack frees those copies.
    while (!undoStack.empty()) {
        undoStack.pop();
    }

    // The unique pointers free the records when the vector is cleared.
    patients.clear();

    while (!waitingQueue.empty()) {
        waitingQueue.pop();
    }

    for (int i = 0; i < CACHE_SIZE; ++i) {
        recentPatients[i] = nullptr;
    }
    recentCount = 0;
}

void ClinicDatabase::touchRecentPatient(PatientRecord* patient) {
    if (patient == nullptr) {
        return;
    }

    for (int i = 0; i < recentCount; ++i) {
        if (recentPatients[i] == patient) {
            // Move the record to the front, so that the cache keeps the
            // records that were used most recently.
            for (int j = i; j > 0; --j) {
                recentPatients[j] = recentPatients[j - 1];
            }
            recentPatients[0] = patient;
            return;
        }
    }

    if (recentCount == CACHE_SIZE) {
        for (int i = 0; i < CACHE_SIZE - 1; ++i) {
            recentPatients[i] = recentPatients[i + 1];
        }
        recentPatients[CACHE_SIZE - 1] = patient;
    } else {
        recentPatients[recentCount++] = patient;
    }
}

void ClinicDatabase::removeRecentPatient(PatientRecord* patient) {
    for (int i = 0; i < recentCount; ++i) {
        if (recentPatients[i] == patient) {
            for (int j = i; j < recentCount - 1; ++j) {
                recentPatients[j] = recentPatients[j + 1];
            }
            recentPatients[--recentCount] = nullptr;
            return;
        }
    }
}

void ClinicDatabase::removeFromWaitingQueue(PatientRecord* patient) {
    if (patient == nullptr) {
        return;
    }

    std::queue<PatientRecord*> kept;
    while (!waitingQueue.empty()) {
        PatientRecord* front = waitingQueue.front();
        waitingQueue.pop();
        if (front != patient) {
            kept.push(front);
        }
    }
    waitingQueue = kept;
}

std::vector<PatientRecord*> ClinicDatabase::getRecentPatients() const {
    std::vector<PatientRecord*> ordered;
    for (int i = recentCount - 1; i >= 0; --i) {
        if (recentPatients[i] != nullptr) {
            ordered.push_back(recentPatients[i]);
        }
    }
    return ordered;
}

void ClinicDatabase::displayAllPatients() const {
    std::cout << "\n========== REGISTERED CLINIC PATIENTS ("
              << patients.size() << ") ==========" << std::endl;
    if (patients.empty()) {
        std::cout << "No patients registered in the database." << std::endl;
        return;
    }
    for (const std::unique_ptr<PatientRecord>& patient : patients) {
        // The call goes through the base class pointer, so the right
        // category answers it at run time.
        patient->displayPatient();
    }
}

void ClinicDatabase::displayAllAppointments() const {
    const std::vector<Appointment> all = getAllAppointments();
    std::cout << "\n========== CLINIC APPOINTMENTS SCHEDULE ("
              << all.size() << ") ==========" << std::endl;
    if (all.empty()) {
        std::cout << "No appointments currently scheduled." << std::endl;
        return;
    }
    for (const Appointment& appointment : all) {
        appointment.displayAppointment();
    }
}

} // namespace model
} // namespace scapms
