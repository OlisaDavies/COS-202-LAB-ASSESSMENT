#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "PatientRecord.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"
#include "Appointment.h"
#include "ClinicExceptions.h"

class FileHandler {
public:
    static void savePatients(const std::string& filename, const std::vector<PatientRecord*>& patients) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            throw FileIOException(filename, "could not be opened for writing");
        }

        for (const PatientRecord* p : patients) {
            if (!p) continue;

            std::string type = p->getPatientType();
            outFile << type << ","
                    << p->getPatientID() << ","
                    << p->getName() << ","
                    << p->getAge() << ","
                    << p->getContactNumber() << ",";

            if (type == "General") {
                const GeneralPatient* gp = dynamic_cast<const GeneralPatient*>(p);
                if (gp) {
                    outFile << gp->getConsultationFee() << "\n";
                }
            } else if (type == "Emergency") {
                const EmergencyPatient* ep = dynamic_cast<const EmergencyPatient*>(p);
                if (ep) {
                    outFile << ep->getSeverityLevel() << "," << ep->getTriageNote() << "\n";
                }
            } else if (type == "FollowUp") {
                const FollowUpPatient* fp = dynamic_cast<const FollowUpPatient*>(p);
                if (fp) {
                    outFile << fp->getPreviousVisitDate() << "," << fp->getAssignedDoctor() << "\n";
                }
            }
        }

        outFile.close();
    }

    static std::vector<PatientRecord*> loadPatients(const std::string& filename) {
        std::vector<PatientRecord*> loadedPatients;
        std::ifstream inFile(filename);

        if (!inFile.is_open()) {
            throw FileIOException(filename, "could not be opened for reading");
        }

        std::string line;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string type, id, name, ageStr, contact;

            std::getline(ss, type, ',');
            std::getline(ss, id, ',');
            std::getline(ss, name, ',');
            std::getline(ss, ageStr, ',');
            std::getline(ss, contact, ',');

            int age = 0;
            try {
                age = std::stoi(ageStr);
            } catch (const std::exception&) {
                age = 0;
            }

            if (type == "General") {
                std::string feeStr;
                std::getline(ss, feeStr, ',');
                double fee = 0.0;
                try {
                    fee = std::stod(feeStr);
                } catch (const std::exception&) {
                    fee = 0.0;
                }
                loadedPatients.push_back(new GeneralPatient(id, name, age, contact, fee));
            } else if (type == "Emergency") {
                std::string severityStr, note;
                std::getline(ss, severityStr, ',');
                std::getline(ss, note, ',');
                int severity = 1;
                try {
                    severity = std::stoi(severityStr);
                } catch (const std::exception&) {
                    severity = 1;
                }
                loadedPatients.push_back(new EmergencyPatient(id, name, age, contact, severity, note));
            } else if (type == "FollowUp") {
                std::string prevDate, doctor;
                std::getline(ss, prevDate, ',');
                std::getline(ss, doctor, ',');
                loadedPatients.push_back(new FollowUpPatient(id, name, age, contact, prevDate, doctor));
            }
        }

        inFile.close();
        return loadedPatients;
    }

    static void saveAppointments(const std::string& filename, const std::vector<Appointment>& appointments) {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            throw FileIOException(filename, "could not be opened for writing");
        }

        for (const Appointment& apt : appointments) {
            outFile << apt.getAppointmentID() << ","
                    << apt.getPatientID() << ","
                    << apt.getDate() << ","
                    << apt.getTime() << ","
                    << apt.getDoctorName() << ","
                    << apt.getStatus() << "\n";
        }

        outFile.close();
    }

    static std::vector<Appointment> loadAppointments(const std::string& filename) {
        std::vector<Appointment> loaded;
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            throw FileIOException(filename, "could not be opened for reading");
        }

        std::string line;
        while (std::getline(inFile, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string aptID, patID, date, time, doc, status;

            std::getline(ss, aptID, ',');
            std::getline(ss, patID, ',');
            std::getline(ss, date, ',');
            std::getline(ss, time, ',');
            std::getline(ss, doc, ',');
            std::getline(ss, status, ',');

            loaded.push_back(Appointment(aptID, patID, date, time, doc, status));
        }

        inFile.close();
        return loaded;
    }
};
