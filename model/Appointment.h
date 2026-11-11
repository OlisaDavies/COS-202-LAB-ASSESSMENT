#pragma once

#include <iostream>
#include <string>
#include <utility>

namespace scapms {
namespace model {

// One booked consultation.
//
// The appointment belongs to a patient through the patient ID, so a patient
// can hold several appointments at the same time.
class Appointment {
public:
    // The three states an appointment can be in. They are kept here so that
    // the same word is used everywhere in the program.
    static constexpr const char* STATUS_PENDING   = "Pending";
    static constexpr const char* STATUS_COMPLETED = "Completed";
    static constexpr const char* STATUS_CANCELLED = "Cancelled";

private:
    std::string appointmentID;
    std::string patientID;
    std::string date;        // YYYY-MM-DD
    std::string time;        // HH:MM AM / PM
    std::string doctorName;
    std::string status;

public:
    Appointment(std::string aptId, std::string patId, std::string aptDate,
                std::string aptTime, std::string docName,
                std::string aptStatus = STATUS_PENDING)
        : appointmentID(aptId),
          patientID(patId),
          date(aptDate),
          time(aptTime),
          doctorName(docName),
          status(aptStatus) {}

    const std::string& getAppointmentID() const { return appointmentID; }
    const std::string& getPatientID() const { return patientID; }
    const std::string& getDate() const { return date; }
    const std::string& getTime() const { return time; }
    const std::string& getDoctorName() const { return doctorName; }
    const std::string& getStatus() const { return status; }

    void setDate(std::string newDate) { date = newDate; }
    void setTime(std::string newTime) { time = newTime; }
    void setDoctorName(std::string newDoctor) { doctorName = newDoctor; }
    void setStatus(std::string newStatus) { status = newStatus; }

    void displayAppointment() const;
};

} // namespace model
} // namespace scapms
