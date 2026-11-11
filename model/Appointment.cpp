#include "Appointment.h"

#include <iostream>

#include "../utils/DateUtil.h"

namespace scapms {
namespace model {

bool Appointment::happensBefore(const Appointment& first, const Appointment& second) {
    if (first.getDate() != second.getDate()) {
        return first.getDate() < second.getDate();
    }

    const int firstMinutes = utils::timeToMinutes(first.getTime());
    const int secondMinutes = utils::timeToMinutes(second.getTime());
    if (firstMinutes < 0 || secondMinutes < 0) {
        return first.getTime() < second.getTime();
    }
    return firstMinutes < secondMinutes;
}

void Appointment::displayAppointment() const {
    std::cout << "Appointment ID: [" << appointmentID << "] | Patient: " << patientID
              << " | Date: " << date << " " << time
              << " | Doctor: " << doctorName
              << " | Status: " << status << std::endl;
}

} // namespace model
} // namespace scapms
