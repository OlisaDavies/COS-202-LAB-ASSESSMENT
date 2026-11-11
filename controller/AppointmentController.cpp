#include "AppointmentController.h"

#include "../model/Appointable.h"
#include "../model/PatientRecord.h"
#include "../utils/ClinicExceptions.h"
#include "../utils/DateUtil.h"
#include "../utils/IDGenerator.h"

namespace scapms {
namespace controller {

AppointmentController::AppointmentController(model::ClinicDatabase& database)
    : database(database) {}

void AppointmentController::checkBookingRules(const model::Appointment& appointment) const {
    if (appointment.getAppointmentID().empty()) {
        throw utils::InvalidAppointmentException("An appointment needs an identifier.");
    }
    if (appointment.getDate().empty()) {
        throw utils::InvalidAppointmentException(
            "An appointment needs a date, written as YYYY-MM-DD.");
    }
    if (appointment.getTime().empty()) {
        throw utils::InvalidAppointmentException(
            "An appointment needs a time, written as HH:MM AM or HH:MM PM.");
    }
    if (!utils::isValidDate(appointment.getDate())) {
        throw utils::InvalidAppointmentException(
            "The date [" + appointment.getDate() + "] is not a valid calendar date.");
    }
    if (utils::timeToMinutes(appointment.getTime()) < 0) {
        throw utils::InvalidAppointmentException(
            "The time [" + appointment.getTime() + "] is not a valid time of the day.");
    }
}

bool AppointmentController::patientCanBeBooked(const model::Appointment& appointment) const {
    const model::PatientRecord* patient = database.findPatientByID(appointment.getPatientID());
    if (patient == nullptr) {
        return false;
    }
    // Only the categories that answer the Appointable interface can be
    // booked. An emergency case is seen in the waiting room instead.
    return dynamic_cast<const model::Appointable*>(patient) != nullptr;
}

void AppointmentController::scheduleAppointment(const model::Appointment& appointment) {
    checkBookingRules(appointment);

    if (appointment.getPatientID().empty()) {
        throw utils::InvalidAppointmentException("An appointment needs a patient.");
    }

    if (database.findPatientByID(appointment.getPatientID()) == nullptr) {
        throw utils::InvalidAppointmentException(
            "No patient with ID [" + appointment.getPatientID() + "] is registered.");
    }

    if (!patientCanBeBooked(appointment)) {
        const model::PatientRecord* patient = database.findPatientByID(appointment.getPatientID());
        throw utils::InvalidAppointmentException(
            "A patient of the category [" + patient->getTypeLabel() +
            "] is not booked in advance. Please use the waiting room instead.");
    }

    // The database stores the booking and remembers it for the undo stack.
    database.addAppointment(appointment);
}

bool AppointmentController::cancelAppointment(const std::string& appointmentID) {
    return database.cancelAppointment(appointmentID);
}

std::string AppointmentController::suggestAppointmentId() const {
    std::vector<std::string> used;
    for (const model::Appointment& appointment : database.getAllAppointments()) {
        used.push_back(appointment.getAppointmentID());
    }
    return utils::IDGenerator::nextAppointmentId(used);
}

} // namespace controller
} // namespace scapms
