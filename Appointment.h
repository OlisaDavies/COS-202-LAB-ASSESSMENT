#pragma once

#include <iostream>
#include <string>

class Appointment {
private:
    std::string appointmentID;
    std::string patientID;
    std::string date;        // YYYY-MM-DD
    std::string time;        // HH:MM AM/PM
    std::string doctorName;
    std::string status;      // Pending, Completed or Cancelled

public:
    Appointment(std::string aptId, std::string patId, std::string aptDate,
                std::string aptTime, std::string docName, std::string aptStatus = "Pending")
        : appointmentID(aptId), patientID(patId), date(aptDate),
          time(aptTime), doctorName(docName), status(aptStatus) {}

    std::string getAppointmentID() const { return appointmentID; }
    std::string getPatientID() const { return patientID; }
    std::string getDate() const { return date; }
    std::string getTime() const { return time; }
    std::string getDoctorName() const { return doctorName; }
    std::string getStatus() const { return status; }

    void setDate(const std::string& newDate) { date = newDate; }
    void setTime(const std::string& newTime) { time = newTime; }
    void setDoctorName(const std::string& newDoc) { doctorName = newDoc; }
    void setStatus(const std::string& newStatus) { status = newStatus; }

    void displayAppointment() const {
        std::cout << "Appointment ID: [" << appointmentID << "] | Patient: " << patientID
                  << " | Date: " << date << " " << time
                  << " | Doctor: " << doctorName
                  << " | Status: " << status << std::endl;
    }
};
