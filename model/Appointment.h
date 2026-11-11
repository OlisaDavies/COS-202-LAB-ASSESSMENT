#pragma once

#include <string>

namespace scapms {
namespace model {

// One booked consultation.
//
// The appointment belongs to a patient. The patient holds the appointment in
// its own collection, so a patient can have several appointments at the
// same time. The identifier of the patient is kept in the appointment as
// well, because the flat schedule that is written to the data file needs it.
class Appointment {
public:
    // The three states an appointment can be in. They are kept here so that
    // the same word is used everywhere in the program.
    static constexpr const char* STATUS_PENDING   = "Pending";
    static constexpr const char* STATUS_COMPLETED = "Completed";
    static constexpr const char* STATUS_CANCELLED = "Cancelled";

    // True when the first appointment happens before the second one. The
    // times are compared as minutes since midnight, so that "09:00 AM" is
    // correctly placed before "10:30 AM".
    static bool happensBefore(const Appointment& first, const Appointment& second);

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

    // The patient that owns the appointment. It is set by the patient itself
    // when the appointment is stored, so that the two always agree.
    void setPatientID(std::string id) { patientID = id; }

    void setDate(std::string newDate) { date = newDate; }
    void setTime(std::string newTime) { time = newTime; }
    void setDoctorName(std::string newDoctor) { doctorName = newDoctor; }
    void setStatus(std::string newStatus) { status = newStatus; }

    void displayAppointment() const;
};

} // namespace model
} // namespace scapms
