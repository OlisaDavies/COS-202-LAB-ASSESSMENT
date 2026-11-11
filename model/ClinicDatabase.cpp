#include "ClinicDatabase.h"

#include <algorithm>
#include <iostream>

#include "../utils/ClinicExceptions.h"
#include "../utils/DateUtil.h"

namespace scapms {
namespace model {

namespace {

// True when the first appointment happens before the second one.
bool happensEarlier(const Appointment& first, const Appointment& second) {
    if (first.getDate() != second.getDate()) {
        return first.getDate() < second.getDate();
    }

    int firstMinutes = utils::timeToMinutes(first.getTime());
    int secondMinutes = utils::timeToMinutes(second.getTime());
    if (firstMinutes < 0 || secondMinutes < 0) {
        return first.getTime() < second.getTime();
    }
    return firstMinutes < secondMinutes;
}

} // namespace

ClinicDatabase::ClinicDatabase() : recentCount(0) {
    for (int i = 0; i < CACHE_SIZE; ++i) {
        recentPatients[i] = nullptr;
    }
}

ClinicDatabase::~ClinicDatabase() {
    clearAll();
}

void ClinicDatabase::addPatient(PatientRecord* patient) {
    addPatientInternal(patient, true);
}

void ClinicDatabase::addPatientInternal(PatientRecord* patient, bool trackUndo) {
    if (patient == nullptr) {
        throw utils::InvalidPatientDataException("Cannot register a null patient record.");
    }

    if (patient->getAge() < 0 || patient->getAge() > 120) {
        const int badAge = patient->getAge();
        delete patient;
        throw utils::InvalidPatientDataException(
            "Patient age (" + std::to_string(badAge) + ") must be between 0 and 120 years.");
    }

    if (patient->getName().empty()) {
        delete patient;
        throw utils::InvalidPatientDataException("Patient name cannot be empty.");
    }

    if (patient->getPatientID().empty()) {
        delete patient;
        throw utils::InvalidPatientDataException("Patient ID cannot be empty.");
    }

    if (findPatientByID(patient->getPatientID()) != nullptr) {
        const std::string duplicateID = patient->getPatientID();
        delete patient;
        throw utils::DuplicateIDException(duplicateID);
    }

    patients.push_back(patient);
    touchRecentPatient(patient);

    if (trackUndo) {
        // Undoing this removes the record again, so no copy is needed here.
        undoStack.push(AdminAction(AdminAction::Type::AddPatient,
                                   "Add Patient " + patient->getPatientID(),
                                   patient->getPatientID()));
    }
}

void ClinicDatabase::addPatients(const std::vector<PatientRecord*>& newPatients,
                                 std::vector<std::string>* warnings) {
    for (PatientRecord* patient : newPatients) {
        try {
            addPatientInternal(patient, false);
        } catch (const utils::ClinicException& error) {
            if (warnings != nullptr) {
                warnings->push_back(std::string("a patient was skipped: ") + error.what());
            }
        }
    }
}

bool ClinicDatabase::removePatientInternal(const std::string& id, PatientRecord** backupOut) {
    for (auto it = patients.begin(); it != patients.end(); ++it) {
        if ((*it)->getPatientID() != id) {
            continue;
        }

        PatientRecord* target = *it;

        if (backupOut != nullptr) {
            *backupOut = target->clone();       // the undo stack owns this copy
        }

        removeRecentPatient(target);
        removeFromWaitingQueue(target);

        patients.erase(it);
        delete target;                          // the database owned this record
        return true;
    }
    return false;
}

bool ClinicDatabase::removePatient(const std::string& id) {
    PatientRecord* backup = nullptr;
    if (!removePatientInternal(id, &backup)) {
        return false;
    }

    undoStack.push(AdminAction(AdminAction::Type::DeletePatient,
                               "Delete Patient " + id, id, backup));
    return true;
}

void ClinicDatabase::pushPatientSnapshot(PatientRecord* snapshot) {
    if (snapshot == nullptr) {
        return;
    }
    undoStack.push(AdminAction(AdminAction::Type::UpdatePatient,
                               "Update Patient " + snapshot->getPatientID(),
                               snapshot->getPatientID(), snapshot));
}

PatientRecord* ClinicDatabase::findPatientByID(const std::string& id) {
    for (int i = 0; i < recentCount; ++i) {
        if (recentPatients[i] != nullptr && recentPatients[i]->getPatientID() == id) {
            return recentPatients[i];
        }
    }

    for (PatientRecord* patient : patients) {
        if (patient->getPatientID() == id) {
            touchRecentPatient(patient);
            return patient;
        }
    }
    return nullptr;
}

PatientRecord* ClinicDatabase::findPatientByID(const std::string& id) const {
    for (PatientRecord* patient : patients) {
        if (patient->getPatientID() == id) {
            return patient;
        }
    }
    return nullptr;
}

std::vector<Appointment> ClinicDatabase::getAppointmentsFor(const std::string& patientID) const {
    std::vector<Appointment> result;
    for (const Appointment& appointment : appointments) {
        if (appointment.getPatientID() == patientID) {
            result.push_back(appointment);
        }
    }
    return result;
}

std::size_t ClinicDatabase::getPendingCountFor(const std::string& patientID) const {
    std::size_t pending = 0;
    for (const Appointment& appointment : appointments) {
        if (appointment.getPatientID() == patientID &&
            appointment.getStatus() == Appointment::STATUS_PENDING) {
            ++pending;
        }
    }
    return pending;
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

void ClinicDatabase::addAppointment(const Appointment& appointment) {
    addAppointmentInternal(appointment, true);
}

void ClinicDatabase::addAppointmentInternal(const Appointment& appointment, bool trackUndo) {
    if (appointment.getAppointmentID().empty() || appointment.getPatientID().empty() ||
        appointment.getDate().empty() || appointment.getTime().empty()) {
        throw utils::InvalidAppointmentException(
            "An appointment needs an ID, a patient, a date and a time.");
    }

    for (const Appointment& existing : appointments) {
        if (existing.getAppointmentID() == appointment.getAppointmentID()) {
            throw utils::InvalidAppointmentException(
                "Appointment ID [" + appointment.getAppointmentID() + "] is already booked.");
        }
    }

    if (findPatientByID(appointment.getPatientID()) == nullptr) {
        throw utils::InvalidAppointmentException(
            "No patient with ID [" + appointment.getPatientID() + "] is registered.");
    }

    appointments.push_back(appointment);
    syncPatientBooking(appointment.getPatientID());

    if (trackUndo) {
        undoStack.push(AdminAction(AdminAction::Type::AddAppointment,
                                   "Add Appointment " + appointment.getAppointmentID(),
                                   appointment.getPatientID(), appointment));
    }
}

void ClinicDatabase::addAppointments(const std::vector<Appointment>& newAppointments,
                                     std::vector<std::string>* warnings) {
    for (const Appointment& appointment : newAppointments) {
        try {
            addAppointmentInternal(appointment, false);
        } catch (const utils::ClinicException& error) {
            if (warnings != nullptr) {
                warnings->push_back(std::string("an appointment was skipped: ") + error.what());
            }
        }
    }
}

bool ClinicDatabase::cancelAppointment(const std::string& appointmentID) {
    for (Appointment& appointment : appointments) {
        if (appointment.getAppointmentID() != appointmentID) {
            continue;
        }

        const Appointment previous = appointment;
        appointment.setStatus(Appointment::STATUS_CANCELLED);
        syncPatientBooking(appointment.getPatientID());

        undoStack.push(AdminAction(AdminAction::Type::CancelAppointment,
                                   "Cancel Appointment " + appointmentID,
                                   appointment.getPatientID(), previous));
        return true;
    }
    return false;
}

const Appointment* ClinicDatabase::nextPendingAppointment(const std::string& patientID) const {
    const Appointment* earliest = nullptr;

    for (const Appointment& appointment : appointments) {
        if (appointment.getPatientID() != patientID ||
            appointment.getStatus() != Appointment::STATUS_PENDING) {
            continue;
        }
        if (earliest == nullptr || happensEarlier(appointment, *earliest)) {
            earliest = &appointment;
        }
    }
    return earliest;
}

void ClinicDatabase::syncPatientBooking(const std::string& patientID) {
    PatientRecord* patient = findPatientByID(patientID);
    Appointable* bookable = dynamic_cast<Appointable*>(patient);
    if (bookable == nullptr) {
        return;                                 // this category cannot be booked
    }

    const Appointment* next = nextPendingAppointment(patientID);
    if (next != nullptr) {
        bookable->scheduleAppointment(next->getDate(), next->getTime());
    } else {
        bookable->cancelAppointment();
    }
}

std::string ClinicDatabase::undoLastAction() {
    if (undoStack.empty()) {
        return "";
    }

    AdminAction lastAction = undoStack.top();
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
            PatientRecord* restored = lastAction.patientBackup;
            lastAction.patientBackup = nullptr;
            patients.push_back(restored);
            touchRecentPatient(restored);
            syncPatientBooking(restored->getPatientID());
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
            delete lastAction.patientBackup;
            lastAction.patientBackup = nullptr;
            return lastAction.description;
        }

        case AdminAction::Type::AddAppointment: {
            const std::string appointmentID = lastAction.appointmentBackup.getAppointmentID();
            for (auto it = appointments.begin(); it != appointments.end(); ++it) {
                if (it->getAppointmentID() == appointmentID) {
                    const std::string patientID = it->getPatientID();
                    appointments.erase(it);
                    syncPatientBooking(patientID);
                    return lastAction.description;
                }
            }
            return "";
        }

        case AdminAction::Type::CancelAppointment: {
            const Appointment& previous = lastAction.appointmentBackup;
            for (Appointment& appointment : appointments) {
                if (appointment.getAppointmentID() == previous.getAppointmentID()) {
                    appointment.setStatus(previous.getStatus());
                    syncPatientBooking(previous.getPatientID());
                    return lastAction.description;
                }
            }
            return "";
        }
    }
    return "";
}

void ClinicDatabase::clearAll() {
    while (!undoStack.empty()) {
        AdminAction action = undoStack.top();
        undoStack.pop();

        // The stack holds a copy of every record it can undo.
        if (action.patientBackup != nullptr) {
            delete action.patientBackup;
        }
    }

    for (PatientRecord* patient : patients) {
        delete patient;
    }
    patients.clear();

    appointments.clear();

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
    for (const PatientRecord* patient : patients) {
        patient->displayPatient();
    }
}

void ClinicDatabase::displayAllAppointments() const {
    std::cout << "\n========== CLINIC APPOINTMENTS SCHEDULE ("
              << appointments.size() << ") ==========" << std::endl;
    if (appointments.empty()) {
        std::cout << "No appointments currently scheduled." << std::endl;
        return;
    }
    for (const Appointment& appointment : appointments) {
        appointment.displayAppointment();
    }
}

} // namespace model
} // namespace scapms
