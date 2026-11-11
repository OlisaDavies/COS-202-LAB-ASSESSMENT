// COS202 Programming II - Lab Assessment
// Smart Clinic Appointment & Patient Management System (SCAPMS)
//
// A short run through the business layer without the graphical interface.
// It shows that the exception handling, the searching and sorting routines,
// the waiting room and the undo stack all work on their own.
//
// The console demonstration talks to the controller, exactly like the
// graphical interface does. The data itself is never reached directly.

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "controller/ClinicManager.h"
#include "model/GeneralPatient.h"
#include "model/EmergencyPatient.h"
#include "model/FollowUpPatient.h"
#include "model/Appointable.h"
#include "model/Billable.h"
#include "utils/ClinicExceptions.h"
#include "controller/SearchEngine.h"
#include "controller/SortEngine.h"

using namespace scapms::model;
using namespace scapms::controller;
using namespace scapms::utils;

int main() {
    ClinicManager manager;

    // Registering the patients.
    try {
        manager.registerPatient(
            std::make_unique<GeneralPatient>("GEN-101", "Amaka Okonkwo", 28,
                                             "+2348073451298", 4500.00));
        manager.registerPatient(
            std::make_unique<EmergencyPatient>("EMG-999", "Davies Johnson", 22,
                                               "+2348034874562", 1, "Acute trauma"));
        manager.registerPatient(
            std::make_unique<FollowUpPatient>("FOL-205", "Babatunde Alabi", 45,
                                              "+2348067624513",
                                              "2026-10-20", "Dr. Funke"));
        manager.registerPatient(
            std::make_unique<GeneralPatient>("GEN-005", "Chioma Eze", 19,
                                             "+2348091837465", 5000.00));
        std::cout << "Four patients registered." << std::endl;
    } catch (const ClinicException& error) {
        std::cout << "Error: " << error.what() << std::endl;
    }

    // The rules of the clinic raise their own exception types.
    try {
        manager.registerPatient(
            std::make_unique<GeneralPatient>("GEN-102", "Patient Two", -5,
                                             "+2348000000000", 3000.00));
    } catch (const InvalidPatientDataException& error) {
        std::cout << "Rejected negative age: " << error.what() << std::endl;
    }

    try {
        manager.registerPatient(
            std::make_unique<GeneralPatient>("GEN-101", "Patient Four", 30,
                                             "+2348099999999", 5000.00));
    } catch (const DuplicateIDException& error) {
        std::cout << "Rejected duplicate ID: " << error.what() << std::endl;
    }

    // The records are shown through a PatientRecord pointer, so the version
    // of displayPatient() that runs is chosen at run time.
    manager.database().displayAllPatients();

    // Only the bookable categories answer the Appointable interface.
    for (PatientRecord* patient : manager.getAllPatients()) {
        Appointable* bookable = dynamic_cast<Appointable*>(patient);
        if (bookable != nullptr) {
            std::cout << patient->getPatientID() << " (" << patient->getTypeLabel()
                      << ") can be booked: "
                      << (bookable->hasScheduledAppointment() ? "yes" : "no") << std::endl;
        } else {
            std::cout << patient->getPatientID() << " (" << patient->getTypeLabel()
                      << ") cannot be booked." << std::endl;
        }
    }

    // A class may implement more than one interface: the scheduled patients
    // answer both Appointable and Billable.
    for (const PatientRecord* patient : manager.getAllPatients()) {
        const Billable* billable = dynamic_cast<const Billable*>(patient);
        if (billable != nullptr) {
            std::cout << patient->getPatientID() << " bill: "
                      << billable->describeBill() << std::endl;
        }
    }

    try {
        manager.scheduleAppointment(
            Appointment("APT-1001", "GEN-101", "2026-11-15", "10:30 AM", "Dr. Adeyemi"));
        manager.scheduleAppointment(
            Appointment("APT-1002", "FOL-205", "2026-11-20", "02:00 PM", "Dr. Funke",
                        Appointment::STATUS_COMPLETED));
        std::cout << "Two appointments booked." << std::endl;
    } catch (const ClinicException& error) {
        std::cout << "Error: " << error.what() << std::endl;
    }

    // The whole schedule is shown, gathered from the patients that hold it.
    manager.database().displayAllAppointments();

    // An emergency patient is never booked in advance, so the controller
    // refuses the booking and explains why.
    try {
        manager.scheduleAppointment(
            Appointment("APT-1003", "EMG-999", "2026-11-18", "09:00 AM", "Dr. Trauma"));
    } catch (const ClinicException& error) {
        std::cout << "Rejected emergency booking: " << error.what() << std::endl;
    }

    const std::vector<PatientRecord*> records = manager.getAllPatients();

    // Linear search works on any list.
    int index = SearchEngine::linearSearchByID(records, "GEN-005");
    std::cout << "Linear search for GEN-005 found index " << index << std::endl;

    // Binary search needs a sorted list, so a sorted copy is made first.
    std::vector<PatientRecord*> sorted = records;
    SortEngine::mergeSort(sorted, SortField::PatientID);
    index = SearchEngine::binarySearchByID(sorted, "FOL-205");
    std::cout << "Binary search on the sorted list found index " << index << std::endl;

    // The recursive search walks the list itself and collects every patient
    // whose name holds the text.
    const std::vector<int> nameMatches = SearchEngine::recursiveSearchByName(records, "Chioma Eze");
    index = nameMatches.empty() ? -1 : nameMatches.front();
    std::cout << "Recursive search for Chioma Eze found index " << index << std::endl;

    // The category search returns every match.
    std::vector<int> emergencies = SearchEngine::searchByCategory(records, PatientRecord::TYPE_EMERGENCY);
    std::cout << "Category search for Emergency found " << emergencies.size() << " patient(s)"
              << std::endl;

    // Both sorting routines are chosen by hand.
    std::vector<PatientRecord*> byAge = records;
    SortEngine::selectionSort(byAge, SortField::PatientAge);
    std::cout << "Selection sort by age: ";
    for (const PatientRecord* patient : byAge) {
        std::cout << patient->getName() << " (" << patient->getAge() << ")  ";
    }
    std::cout << std::endl;

    std::vector<PatientRecord*> byName = records;
    SortEngine::mergeSort(byName, SortField::PatientName);
    std::cout << "Merge sort by name:     ";
    for (const PatientRecord* patient : byName) {
        std::cout << patient->getName() << "  ";
    }
    std::cout << std::endl;

    // The waiting room is first in, first out.
    manager.enqueuePatient(manager.findPatientByID("GEN-101"));
    manager.enqueuePatient(manager.findPatientByID("EMG-999"));

    PatientRecord* next = manager.callNextPatient();
    std::cout << "Called next: " << (next != nullptr ? next->getName() : "nobody") << std::endl;

    // The last administrative operation can be undone.
    manager.deletePatient("GEN-005");
    std::cout << "After the delete there are " << manager.getPatientCount() << " patients." << std::endl;

    std::string undone = manager.undoLastAction();
    std::cout << "Undo reported: " << undone << std::endl;
    std::cout << "After the undo there are " << manager.getPatientCount() << " patients." << std::endl;

    return 0;
}
