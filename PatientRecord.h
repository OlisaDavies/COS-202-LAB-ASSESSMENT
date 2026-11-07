#pragma once

#include <iostream>
#include <string>

class PatientRecord {
protected:
    std::string patientID;
    std::string name;
    int age;
    std::string contactNumber;
    std::string patientType;

public:
    PatientRecord(std::string id, std::string patientName, int patientAge,
                  std::string contact, std::string type)
        : patientID(id), name(patientName), age(patientAge),
          contactNumber(contact), patientType(type) {}

    virtual ~PatientRecord() {}

    std::string getPatientID() const { return patientID; }
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getContactNumber() const { return contactNumber; }
    std::string getPatientType() const { return patientType; }

    void setName(std::string newName) { name = newName; }
    void setAge(int newAge) { age = newAge; }
    void setContactNumber(std::string newContact) { contactNumber = newContact; }

    virtual void displayPatient() const = 0;
    virtual std::string getSpecialDetail() const = 0;

    virtual PatientRecord* clone() const = 0;
    virtual void restoreFrom(const PatientRecord& previous) = 0;
};
