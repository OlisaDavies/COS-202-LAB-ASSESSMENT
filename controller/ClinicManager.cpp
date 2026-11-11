#include "ClinicManager.h"

#include "../utils/ClinicExceptions.h"
#include "../utils/FileHandler.h"
#include "../utils/IDGenerator.h"
#include "../model/GeneralPatient.h"

#include <cctype>

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

    // The contact number is checked when it is given. It may be left empty,
    // because the clinic does not always have it, but when it is written it
    // has to look like a telephone number.
    const std::string& contact = patient.getContactNumber();
    if (!contact.empty()) {
        bool allowed = false;
        for (char c : contact) {
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '+' || c == '-' ||
                c == ' ' || c == '(' || c == ')') {
                allowed = true;
            } else {
                allowed = false;
                break;
            }
        }
        if (!allowed) {
            throw utils::InvalidPatientDataException(
                "The contact number [" + contact +
                "] may only hold digits and the characters + - ( ) and spaces.");
        }
    }

    // The identifier of a patient carries the category, so that a general
    // patient is GEN-101, an emergency case is EMG-999 and a follow-up
    // patient is FOL-205. A record that breaks the rule is refused, so
    // that the list is not mixed up later on.
    const std::string expectedPrefix = utils::IDGenerator::patientPrefix(patient.getPatientType());
    if (patient.getPatientID().compare(0, expectedPrefix.size(), expectedPrefix) != 0) {
        throw utils::InvalidPatientDataException(
            "The ID of a patient of the category [" + patient.getTypeLabel() +
            "] has to begin with [" + expectedPrefix + "], for example [" +
            expectedPrefix + "001].");
    }

    // The consultation fee has to be a real amount. A value of "nan" or of
    // infinity would pass every comparison, because every comparison with
    // it is false, and it would then be written into the data file.
    const model::GeneralPatient* general =
        dynamic_cast<const model::GeneralPatient*>(&patient);
    if (general != nullptr &&
        !model::GeneralPatient::isValidConsultationFee(general->getConsultationFee())) {
        throw utils::InvalidPatientDataException(
            "The consultation fee of a general patient has to be a real amount "
            "between 0 and 100,000,000 naira.");
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
        if (patient == nullptr) {
            // An empty record is skipped, so that a mistake in the file
            // cannot crash the program.
            if (warnings != nullptr) {
                warnings->push_back("a patient was skipped: the record was empty");
            }
            continue;
        }

        try {
            checkPatientRules(*patient);
            // The load is one administrative step as a whole, so the
            // records are not remembered one by one. Otherwise the undo
            // button would delete the records that were just loaded.
            clinicDatabase.addPatientWithoutUndo(std::move(patient));
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

void ClinicManager::setPatientOrder(const std::vector<model::PatientRecord*>& order) {
    clinicDatabase.setPatientOrder(order);
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

void ClinicManager::scheduleAppointment(const model::Appointment& appointment, bool trackUndo) {
    appointmentController.scheduleAppointment(appointment, trackUndo);
}

bool ClinicManager::cancelAppointment(const std::string& appointmentID) {
    return appointmentController.cancelAppointment(appointmentID);
}

bool ClinicManager::completeAppointment(const std::string& appointmentID) {
    return appointmentController.completeAppointment(appointmentID);
}

std::vector<model::Appointment> ClinicManager::getAllAppointments() const {
    return clinicDatabase.getAllAppointments();
}

void ClinicManager::enqueuePatient(model::PatientRecord* patient) {
    if (patient == nullptr) {
        return;
    }

    // The same patient may not wait twice, otherwise the queue would show
    // the patient twice and the counter would not match the clinic.
    for (const model::PatientRecord* waiting : clinicDatabase.getWaitingQueue()) {
        if (waiting != nullptr && waiting->getPatientID() == patient->getPatientID()) {
            throw utils::InvalidAppointmentException(
                "Patient [" + patient->getPatientID() + "] is already in the waiting room.");
        }
    }

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

    std::vector<model::Appointment> booked;
    try {
        booked = utils::FileHandler::loadAppointments(appointmentFile, warnings);
    } catch (const utils::FileIOException& error) {
        // The patient file is the important one. When the appointment file
        // cannot be read, the patients are still loaded, but the real reason
        // is passed on to the staff instead of being thrown away.
        if (warnings != nullptr) {
            warnings->push_back("the appointment file could not be loaded (" +
                                std::string(error.what()) +
                                "), so only the patients were loaded");
        }
    }

    clinicDatabase.clearAll();
    registerPatients(std::move(patients), warnings);

    // The appointments are added after the patients, because a booking needs
    // a patient to belong to. A booking that comes from a file is not an
    // administrative step of its own, so it is not remembered on the undo
    // stack.
    for (const model::Appointment& appointment : booked) {
        try {
            appointmentController.scheduleAppointment(appointment, false);
        } catch (const utils::ClinicException& error) {
            if (warnings != nullptr) {
                warnings->push_back(std::string("an appointment was skipped: ") + error.what());
            }
        }
    }
}

} // namespace controller
} // namespace scapms
