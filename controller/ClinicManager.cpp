#include "ClinicManager.h"

#include "../utils/ClinicExceptions.h"
#include "../utils/FileHandler.h"
#include "../utils/IDGenerator.h"

namespace scapms {
namespace controller {

ClinicManager::ClinicManager() : appointmentController(clinicDatabase) {}

ClinicManager::~ClinicManager() = default;

void ClinicManager::checkPatientRules(const model::PatientRecord& patient) const {
    if (patient.getAge() < 0 || patient.getAge() > 120) {
        throw utils::InvalidPatientDataException(
            "Patient age (" + std::to_string(patient.getAge()) +
            ") must be between 0 and 120 years.");
    }
    if (patient.getName().empty()) {
        throw utils::InvalidPatientDataException("Patient name cannot be empty.");
    }
    if (patient.getPatientID().empty()) {
        throw utils::InvalidPatientDataException("Patient ID cannot be empty.");
    }
}

void ClinicManager::registerPatient(std::unique_ptr<model::PatientRecord> patient) {
    if (patient == nullptr) {
        throw utils::InvalidPatientDataException("Cannot register a null patient record.");
    }

    // The rules of the clinic are checked before the record is stored, so
    // the database only ever holds records that are allowed to exist.
    checkPatientRules(*patient);

    clinicDatabase.addPatient(std::move(patient));
}

void ClinicManager::registerPatients(std::vector<std::unique_ptr<model::PatientRecord>> patients,
                                     std::vector<std::string>* warnings) {
    for (std::unique_ptr<model::PatientRecord>& patient : patients) {
        try {
            checkPatientRules(*patient);
            clinicDatabase.addPatient(std::move(patient));
        } catch (const utils::ClinicException& error) {
            if (warnings != nullptr) {
                warnings->push_back(std::string("a patient was skipped: ") + error.what());
            }
        }
    }
}

bool ClinicManager::deletePatient(const std::string& patientID) {
    return clinicDatabase.removePatient(patientID);
}

model::PatientRecord* ClinicManager::findPatientByID(const std::string& patientID) {
    return clinicDatabase.findPatientByID(patientID);
}

const model::PatientRecord* ClinicManager::findPatientByID(const std::string& patientID) const {
    return clinicDatabase.findPatientByID(patientID);
}

std::vector<model::PatientRecord*> ClinicManager::getAllPatients() const {
    return clinicDatabase.getAllPatients();
}

std::size_t ClinicManager::getPatientCount() const {
    return clinicDatabase.getPatientCount();
}

std::string ClinicManager::suggestPatientId(const std::string& patientType) const {
    std::vector<std::string> used;
    for (const model::PatientRecord* patient : clinicDatabase.getAllPatients()) {
        used.push_back(patient->getPatientID());
    }
    return utils::IDGenerator::nextPatientId(utils::IDGenerator::patientPrefix(patientType), used);
}

void ClinicManager::pushPatientSnapshot(std::unique_ptr<model::PatientRecord> snapshot) {
    clinicDatabase.pushPatientSnapshot(std::move(snapshot));
}

void ClinicManager::scheduleAppointment(const model::Appointment& appointment) {
    appointmentController.scheduleAppointment(appointment);
}

bool ClinicManager::cancelAppointment(const std::string& appointmentID) {
    return appointmentController.cancelAppointment(appointmentID);
}

std::vector<model::Appointment> ClinicManager::getAllAppointments() const {
    return clinicDatabase.getAllAppointments();
}

void ClinicManager::enqueuePatient(model::PatientRecord* patient) {
    clinicDatabase.enqueuePatient(patient);
}

model::PatientRecord* ClinicManager::callNextPatient() {
    return clinicDatabase.callNextPatient();
}

std::size_t ClinicManager::getQueueSize() const {
    return clinicDatabase.getQueueSize();
}

std::vector<model::PatientRecord*> ClinicManager::getWaitingQueue() const {
    return clinicDatabase.getWaitingQueue();
}

std::string ClinicManager::undoLastAction() {
    return clinicDatabase.undoLastAction();
}

void ClinicManager::clearAll() {
    clinicDatabase.clearAll();
}

std::vector<model::PatientRecord*> ClinicManager::getRecentPatients() const {
    return clinicDatabase.getRecentPatients();
}

void ClinicManager::saveToFiles(const std::string& patientFile, const std::string& appointmentFile) {
    utils::FileHandler::savePatients(patientFile, clinicDatabase.getAllPatients());
    utils::FileHandler::saveAppointments(appointmentFile, clinicDatabase.getAllAppointments());
}

void ClinicManager::loadFromFiles(const std::string& patientFile, const std::string& appointmentFile,
                                  std::vector<std::string>* warnings) {
    std::vector<std::unique_ptr<model::PatientRecord>> patients =
        utils::FileHandler::loadPatients(patientFile, warnings);

    std::vector<model::Appointment> booked =
        utils::FileHandler::loadAppointments(appointmentFile, warnings);

    clinicDatabase.clearAll();
    registerPatients(std::move(patients), warnings);

    // The appointments are added after the patients, because a booking needs
    // a patient to belong to.
    for (const model::Appointment& appointment : booked) {
        try {
            appointmentController.scheduleAppointment(appointment);
        } catch (const utils::ClinicException& error) {
            if (warnings != nullptr) {
                warnings->push_back(std::string("an appointment was skipped: ") + error.what());
            }
        }
    }
}

} // namespace controller
} // namespace scapms
