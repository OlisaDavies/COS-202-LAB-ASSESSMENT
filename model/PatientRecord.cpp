#include "PatientRecord.h"

#include <iostream>

namespace scapms {
namespace model {

const Appointment* PatientRecord::findAppointment(const std::string& appointmentID) const {
    for (const Appointment& appointment : appointments) {
        if (appointment.getAppointmentID() == appointmentID) {
            return &appointment;
        }
    }
    return nullptr;
}

bool PatientRecord::hasAppointment(const std::string& appointmentID) const {
    return findAppointment(appointmentID) != nullptr;
}

Appointment* PatientRecord::findAppointmentForUpdate(const std::string& appointmentID) {
    for (Appointment& appointment : appointments) {
        if (appointment.getAppointmentID() == appointmentID) {
            return &appointment;
        }
    }
    return nullptr;
}

const Appointment* PatientRecord::getNextPendingAppointment() const {
    const Appointment* earliest = nullptr;

    for (const Appointment& appointment : appointments) {
        if (appointment.getStatus() != Appointment::STATUS_PENDING) {
            continue;
        }
        if (earliest == nullptr || Appointment::happensBefore(appointment, *earliest)) {
            earliest = &appointment;
        }
    }
    return earliest;
}

std::size_t PatientRecord::getPendingAppointmentCount() const {
    std::size_t pending = 0;
    for (const Appointment& appointment : appointments) {
        if (appointment.getStatus() == Appointment::STATUS_PENDING) {
            ++pending;
        }
    }
    return pending;
}

bool PatientRecord::removeAppointment(const std::string& appointmentID) {
    for (std::size_t i = 0; i < appointments.size(); ++i) {
        if (appointments[i].getAppointmentID() == appointmentID) {
            appointments.erase(appointments.begin() + static_cast<std::ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

bool PatientRecord::restoreAppointment(const Appointment& backup) {
    for (Appointment& appointment : appointments) {
        if (appointment.getAppointmentID() == backup.getAppointmentID()) {
            appointment.setStatus(backup.getStatus());
            return true;
        }
    }

    // The appointment is not in the collection any more, so it is put back.
    Appointment restored = backup;
    restored.setPatientID(patientID);
    appointments.push_back(restored);
    return true;
}

bool PatientRecord::addAppointment(const Appointment& appointment) {
    if (appointment.getAppointmentID().empty()) {
        return false;
    }
    if (hasAppointment(appointment.getAppointmentID())) {
        return false;                          // the identifier is already in use
    }

    // The copy is stamped with the identifier of its owner, so that the
    // appointment and the patient always point at each other.
    Appointment owned = appointment;
    owned.setPatientID(patientID);
    appointments.push_back(owned);
    return true;
}

bool PatientRecord::setAppointmentStatus(const std::string& appointmentID, const std::string& status) {
    for (Appointment& appointment : appointments) {
        if (appointment.getAppointmentID() == appointmentID) {
            appointment.setStatus(status);
            return true;
        }
    }
    return false;
}

void PatientRecord::displayPatient() const {
    // A concrete method of the abstract class: it writes the report that the
    // category produced, and the call dispatches to the right subclass.
    std::cout << getPatientReport() << std::endl;
}

void PatientRecord::restoreFrom(const PatientRecord& previous) {
    name = previous.name;
    age = previous.age;
    contactNumber = previous.contactNumber;
}

} // namespace model
} // namespace scapms
