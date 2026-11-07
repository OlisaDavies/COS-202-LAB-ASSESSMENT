#pragma once

#include <vector>
#include <queue>
#include <stack>
#include <iostream>
#include <string>

#include "PatientRecord.h"
#include "Appointment.h"
#include "Appointable.h"
#include "AdminAction.h"
#include "ClinicExceptions.h"

class ClinicDatabase {
private:
    std::vector<PatientRecord*> patients;
    std::vector<Appointment> appointments;
    std::queue<PatientRecord*> waitingQueue;
    std::stack<AdminAction> undoStack;

    static const int CACHE_SIZE = 10;
    PatientRecord* recentPatients[CACHE_SIZE];
    int recentCount;

public:
    ClinicDatabase() : recentCount(0) {
        for (int i = 0; i < CACHE_SIZE; i++) {
            recentPatients[i] = nullptr;
        }
    }

    ~ClinicDatabase() {
        clearAll();
    }

    void addPatient(PatientRecord* patient, bool trackUndo = true) {
        if (patient == nullptr) {
            throw InvalidPatientDataException("Cannot register a null patient record.");
        }

        if (patient->getAge() < 0 || patient->getAge() > 120) {
            int badAge = patient->getAge();
            delete patient;
            throw InvalidPatientDataException("Patient age (" + std::to_string(badAge) + ") must be between 0 and 120 years.");
        }

        if (patient->getName().empty()) {
            delete patient;
            throw InvalidPatientDataException("Patient name cannot be empty.");
        }

        if (findPatientByID(patient->getPatientID()) != nullptr) {
            std::string dupID = patient->getPatientID();
            delete patient;
            throw DuplicateIDException(dupID);
        }

        patients.push_back(patient);
        touchRecentPatient(patient);

        if (trackUndo) {
            undoStack.push(AdminAction(ACTION_ADD_PATIENT, "Add Patient " + patient->getPatientID(), patient));
        }
    }

    PatientRecord* findPatientByID(const std::string& id) {
        for (int i = 0; i < recentCount; i++) {
            if (recentPatients[i] != nullptr && recentPatients[i]->getPatientID() == id) {
                return recentPatients[i];
            }
        }

        for (PatientRecord* p : patients) {
            if (p->getPatientID() == id) {
                touchRecentPatient(p);
                return p;
            }
        }
        return nullptr;
    }

    bool removePatient(const std::string& id, bool trackUndo = true) {
        for (auto it = patients.begin(); it != patients.end(); ++it) {
            if ((*it)->getPatientID() == id) {
                PatientRecord* target = *it;

                removeRecentPatient(target);
                removeFromWaitingQueue(target);

                if (trackUndo) {
                    undoStack.push(AdminAction(ACTION_DELETE_PATIENT, "Delete Patient " + id, target));
                }

                patients.erase(it);

                if (!trackUndo) {
                    delete target;
                }
                return true;
            }
        }
        return false;
    }

    void recordPatientUpdate(PatientRecord* previousState) {
        undoStack.push(AdminAction(ACTION_UPDATE_PATIENT,
                                   "Update Patient " + previousState->getPatientID(),
                                   previousState));
    }

    const std::vector<PatientRecord*>& getAllPatients() const { return patients; }
    std::size_t getPatientCount() const { return patients.size(); }

    void enqueuePatient(PatientRecord* patient) {
        if (patient != nullptr) {
            waitingQueue.push(patient);
        }
    }

    PatientRecord* callNextPatient() {
        if (waitingQueue.empty()) {
            return nullptr;
        }
        PatientRecord* next = waitingQueue.front();
        waitingQueue.pop();
        return next;
    }

    std::size_t getQueueSize() const {
        return waitingQueue.size();
    }

    std::vector<PatientRecord*> getWaitingQueue() const {
        std::vector<PatientRecord*> order;
        std::queue<PatientRecord*> copy = waitingQueue;
        while (!copy.empty()) {
            order.push_back(copy.front());
            copy.pop();
        }
        return order;
    }

    std::string undoLastAction() {
        if (undoStack.empty()) {
            return "";
        }

        AdminAction lastAction = undoStack.top();
        undoStack.pop();

        switch (lastAction.type) {
            case ACTION_ADD_PATIENT: {
                removePatient(lastAction.patientBackup->getPatientID(), false);
                return lastAction.description;
            }
            case ACTION_DELETE_PATIENT: {
                patients.push_back(lastAction.patientBackup);
                touchRecentPatient(lastAction.patientBackup);
                return lastAction.description;
            }
            case ACTION_UPDATE_PATIENT: {
                PatientRecord* current = findPatientByID(lastAction.patientBackup->getPatientID());
                if (current != nullptr) {
                    current->restoreFrom(*lastAction.patientBackup);
                }
                delete lastAction.patientBackup;
                return lastAction.description;
            }
            case ACTION_ADD_APPOINTMENT: {
                for (auto it = appointments.begin(); it != appointments.end(); ++it) {
                    if (it->getAppointmentID() == lastAction.appointmentBackup.getAppointmentID()) {
                        appointments.erase(it);
                        syncPatientBooking(lastAction.appointmentBackup.getPatientID());
                        return lastAction.description;
                    }
                }
                return "";
            }
            case ACTION_CANCEL_APPOINTMENT: {
                const Appointment& previous = lastAction.appointmentBackup;
                for (Appointment& apt : appointments) {
                    if (apt.getAppointmentID() == previous.getAppointmentID()) {
                        apt.setStatus(previous.getStatus());
                        syncPatientBooking(previous.getPatientID());
                        return lastAction.description;
                    }
                }
                return "";
            }
            default:
                return "";
        }
    }

    void touchRecentPatient(PatientRecord* patient) {
        if (patient == nullptr) return;

        for (int i = 0; i < recentCount; i++) {
            if (recentPatients[i] == patient) {
                return;
            }
        }

        if (recentCount == CACHE_SIZE) {
            for (int i = 0; i < CACHE_SIZE - 1; i++) {
                recentPatients[i] = recentPatients[i + 1];
            }
            recentPatients[CACHE_SIZE - 1] = patient;
        } else {
            recentPatients[recentCount++] = patient;
        }
    }

    void removeRecentPatient(PatientRecord* patient) {
        for (int i = 0; i < recentCount; i++) {
            if (recentPatients[i] == patient) {
                for (int j = i; j < recentCount - 1; j++) {
                    recentPatients[j] = recentPatients[j + 1];
                }
                recentPatients[--recentCount] = nullptr;
                return;
            }
        }
    }

    void removeFromWaitingQueue(PatientRecord* patient) {
        if (patient == nullptr) return;

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

    void addAppointment(const Appointment& apt, bool trackUndo = true) {
        if (apt.getAppointmentID().empty() || apt.getPatientID().empty() ||
            apt.getDate().empty() || apt.getTime().empty()) {
            throw InvalidAppointmentException("An appointment needs an ID, a patient, a date and a time.");
        }

        for (const Appointment& existing : appointments) {
            if (existing.getAppointmentID() == apt.getAppointmentID()) {
                throw InvalidAppointmentException("Appointment ID [" + apt.getAppointmentID() + "] is already booked.");
            }
        }

        appointments.push_back(apt);
        syncPatientBooking(apt.getPatientID());

        if (trackUndo) {
            undoStack.push(AdminAction(ACTION_ADD_APPOINTMENT, "Add Appointment " + apt.getAppointmentID(), apt));
        }
    }

    bool cancelAppointment(const std::string& appointmentID) {
        for (Appointment& apt : appointments) {
            if (apt.getAppointmentID() == appointmentID) {
                Appointment previous = apt;
                apt.setStatus("Cancelled");
                syncPatientBooking(apt.getPatientID());
                undoStack.push(AdminAction(ACTION_CANCEL_APPOINTMENT, "Cancel Appointment " + appointmentID, previous));
                return true;
            }
        }
        return false;
    }

    const std::vector<Appointment>& getAllAppointments() const { return appointments; }

    void displayAllPatients() const {
        std::cout << "\n========== REGISTERED CLINIC PATIENTS (" << patients.size() << ") ==========" << std::endl;
        if (patients.empty()) {
            std::cout << "No patients registered in the database." << std::endl;
            return;
        }
        for (const PatientRecord* p : patients) {
            p->displayPatient();
        }
    }

    void displayAllAppointments() const {
        std::cout << "\n========== CLINIC APPOINTMENTS SCHEDULE (" << appointments.size() << ") ==========" << std::endl;
        if (appointments.empty()) {
            std::cout << "No appointments currently scheduled." << std::endl;
            return;
        }
        for (const Appointment& apt : appointments) {
            apt.displayAppointment();
        }
    }

    void clearAll() {
        while (!undoStack.empty()) {
            AdminAction a = undoStack.top();
            undoStack.pop();
            if ((a.type == ACTION_DELETE_PATIENT || a.type == ACTION_UPDATE_PATIENT) &&
                a.patientBackup != nullptr) {
                delete a.patientBackup;
            }
        }

        for (PatientRecord* p : patients) {
            delete p;
        }
        patients.clear();
        appointments.clear();
        while (!waitingQueue.empty()) {
            waitingQueue.pop();
        }
        recentCount = 0;
    }

private:
    void syncPatientBooking(const std::string& patientID) {
        PatientRecord* p = findPatientByID(patientID);
        Appointable* bookable = dynamic_cast<Appointable*>(p);
        if (bookable == nullptr) {
            return;
        }

        const Appointment* pending = nullptr;
        for (const Appointment& apt : appointments) {
            if (apt.getPatientID() == patientID && apt.getStatus() == "Pending") {
                pending = &apt;
                break;
            }
        }

        if (pending != nullptr) {
            bookable->scheduleAppointment(pending->getDate(), pending->getTime());
        } else {
            bookable->cancelAppointment();
        }
    }
};
