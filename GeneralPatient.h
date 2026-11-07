#pragma once

#include "PatientRecord.h"
#include "Appointable.h"
#include <iomanip>
#include <sstream>

class GeneralPatient : public PatientRecord, public Appointable {
private:
    double consultationFee;
    bool appointmentBooked;
    std::string appointmentDate;
    std::string appointmentTime;

public:
    GeneralPatient(std::string id, std::string patientName, int patientAge,
                   std::string contact, double fee)
        : PatientRecord(id, patientName, patientAge, contact, "General"),
          consultationFee(fee),
          appointmentBooked(false),
          appointmentDate("None"),
          appointmentTime("None") {}

    double getConsultationFee() const { return consultationFee; }
    void setConsultationFee(double fee) { consultationFee = fee; }

    void displayPatient() const override {
        std::cout << "-------------------------------------" << std::endl;
        std::cout << "[Category: General Outpatient]"        << std::endl;
        std::cout << "Patient ID:       " << patientID       << std::endl;
        std::cout << "Name:             " << name             << std::endl;
        std::cout << "Age:              " << age              << std::endl;
        std::cout << "Contact:          " << contactNumber    << std::endl;
        std::cout << "Consultation Fee: $" << std::fixed << std::setprecision(2) << consultationFee << std::endl;
        std::cout << "Appointment:      " << getAppointmentDetails() << std::endl;
        std::cout << "-------------------------------------" << std::endl;
    }

    std::string getSpecialDetail() const override {
        std::stringstream ss;
        ss << "Fee: $" << std::fixed << std::setprecision(2) << consultationFee;
        return ss.str();
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
        std::stringstream ss;
        ss << appointmentDate << " at " << appointmentTime
           << " (Fee: $" << std::fixed << std::setprecision(2) << consultationFee << ")";
        return ss.str();
    }

    PatientRecord* clone() const override {
        GeneralPatient* copy = new GeneralPatient(patientID, name, age, contactNumber, consultationFee);
        copy->appointmentBooked = appointmentBooked;
        copy->appointmentDate = appointmentDate;
        copy->appointmentTime = appointmentTime;
        return copy;
    }

    void restoreFrom(const PatientRecord& previous) override {
        const GeneralPatient& p = static_cast<const GeneralPatient&>(previous);
        name = p.name;
        age = p.age;
        contactNumber = p.contactNumber;
        consultationFee = p.consultationFee;
        appointmentBooked = p.appointmentBooked;
        appointmentDate = p.appointmentDate;
        appointmentTime = p.appointmentTime;
    }
};
