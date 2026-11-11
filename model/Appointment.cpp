#include "Appointment.h"

namespace scapms {
namespace model {

void Appointment::displayAppointment() const {
    std::cout << "Appointment ID: [" << appointmentID << "] | Patient: " << patientID
              << " | Date: " << date << " " << time
              << " | Doctor: " << doctorName
              << " | Status: " << status << std::endl;
}

} // namespace model
} // namespace scapms
