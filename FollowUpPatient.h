#pragma once

#include "PatientRecord.h"
#include "Appointable.h"

class FollowUpPatient : public PatientRecord, public Appointable {
private:
    std::string previousVisitDate;
    std::string assignedDoctor;
    bool appointmentBooked;
    std::string appointmentDate;
    std::string appointmentTime;

public:
    FollowUpPatient(std::string id, std::string patientName, int patientAge,
                    std::string contact, std::string prevDate, std::string doctor)
        : PatientRecord(id, patientName, patientAge, contact, "FollowUp"),
          previousVisitDate(prevDate), assignedDoctor(doctor),
          appointmentBooked(false),
          appointmentDate("None"),
          appointmentTime("None") {}

    std::string getPreviousVisitDate() const { return previousVisitDate; }
    void setPreviousVisitDate(std::string date) { previousVisitDate = date; }

    std::string getAssignedDoctor() const { return assignedDoctor; }
    void setAssignedDoctor(std::string doctor) { assignedDoctor = doctor; }

    void displayPatient() const override {
        std::cout << "-------------------------------------" << std::endl;
        std::cout << "[Category: Follow-Up Consultation]"   << std::endl;
        std::cout << "Patient ID:       " << patientID       << std::endl;
        std::cout << "Name:             " << name             << std::endl;
        std::cout << "Age:              " << age              << std::endl;
        std::cout << "Contact:          " << contactNumber    << std::endl;
        std::cout << "Last Visit Date:  " << previousVisitDate << std::endl;
        std::cout << "Assigned Doctor:  " << assignedDoctor  << std::endl;
        std::cout << "Appointment:      " << getAppointmentDetails() << std::endl;
        std::cout << "-------------------------------------" << std::endl;
    }

    std::string getSpecialDetail() const override {
        return "Follow-up with " + assignedDoctor + " (Last: " + previousVisitDate + ")";
    }

    bool scheduleAppointment(const std::string& date, const std::string& time) override {
        appointmentDate = date;
        appointmentTime = time;
        appointmentBooked = true;
        return true;
    }

    bool cancelAppointment() override {
        if (!appointmentBooked) return false;
        appointmentBooked = false;
        appointmentDate = "None";
        appointmentTime = "None";
        return true;
    }

    bool hasScheduledAppointment() const override {
        return appointmentBooked;
    }

    std::string getAppointmentDetails() const override {
        if (!appointmentBooked) {
            return "No active appointment";
        }
        return appointmentDate + " at " + appointmentTime + " (Dr. " + assignedDoctor + ")";
    }

    PatientRecord* clone() const override {
        FollowUpPatient* copy = new FollowUpPatient(patientID, name, age, contactNumber,
                                                    previousVisitDate, assignedDoctor);
        copy->appointmentBooked = appointmentBooked;
        copy->appointmentDate = appointmentDate;
        copy->appointmentTime = appointmentTime;
        return copy;
    }

    void restoreFrom(const PatientRecord& previous) override {
        const FollowUpPatient& p = static_cast<const FollowUpPatient&>(previous);
        name = p.name;
        age = p.age;
        contactNumber = p.contactNumber;
        previousVisitDate = p.previousVisitDate;
        assignedDoctor = p.assignedDoctor;
        appointmentBooked = p.appointmentBooked;
        appointmentDate = p.appointmentDate;
        appointmentTime = p.appointmentTime;
    }
};
