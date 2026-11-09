// COS202 Programming II - Lab Assessment
// Smart Clinic Appointment & Patient Management System (SCAPMS)

#include <iostream>

#include "ClinicDatabase.h"
#include "ClinicExceptions.h"
#include "Appointable.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"

int main() {
    ClinicDatabase db;

    try {
        db.addPatient(new GeneralPatient("GEN-101", "Amaka Okonkwo", 28, "+2348011112222", 45.00));
        std::cout << "Registered: Amaka Okonkwo (GEN-101)" << std::endl;
    } catch (const ClinicException& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    try {
        db.addPatient(new GeneralPatient("GEN-102", "Patient Two", -5, "+2348000000000", 30.00));
    } catch (const InvalidPatientDataException& e) {
        std::cout << "Rejected negative age: " << e.what() << std::endl;
    }

    try {
        db.addPatient(new EmergencyPatient("EMG-103", "Patient Three", 185, "+2348000000000", 1, "None"));
    } catch (const InvalidPatientDataException& e) {
        std::cout << "Rejected age above 120: " << e.what() << std::endl;
    }

    try {
        db.addPatient(new GeneralPatient("GEN-101", "Patient Four", 30, "+2348099999999", 50.00));
    } catch (const DuplicateIDException& e) {
        std::cout << "Rejected duplicate ID: " << e.what() << std::endl;
    }

    std::cout << "Total registered patients: " << db.getPatientCount() << std::endl;

    db.displayAllPatients();

    for (PatientRecord* patient : db.getAllPatients()) {
        if (Appointable* bookable = dynamic_cast<Appointable*>(patient)) {
            std::cout << patient->getPatientID() << " has an appointment: "
                      << (bookable->hasScheduledAppointment() ? "yes" : "no") << std::endl;
        }
    }

    db.addAppointment(Appointment("APT-1001", "GEN-101", "2026-11-15", "10:30 AM", "Dr. Adeyemi", "Pending"));
    db.displayAllAppointments();

    return 0;
}
