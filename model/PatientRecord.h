#pragma once

#include <string>
#include <utility>

namespace scapms {
namespace model {

// Abstract base class for every patient the clinic knows about.
//
// It holds the details that every category has in common and leaves the
// category specific behaviour to the subclasses. Because it declares pure
// virtual functions it can never be created on its own, which is what makes
// the polymorphic calls through PatientRecord* safe.
class PatientRecord {
public:
    // The three patient categories. The strings are also the key used in the
    // data files, so they are kept in one place.
    static constexpr const char* TYPE_GENERAL   = "General";
    static constexpr const char* TYPE_EMERGENCY = "Emergency";
    static constexpr const char* TYPE_FOLLOWUP  = "FollowUp";

protected:
    std::string patientID;
    std::string name;
    int age;
    std::string contactNumber;
    std::string patientType;

public:
    PatientRecord(std::string id, std::string patientName, int patientAge,
                  std::string contact, std::string type)
        : patientID(id),
          name(patientName),
          age(patientAge),
          contactNumber(contact),
          patientType(type) {}

    // Copying is allowed so that a record can be cloned safely for the undo
    // stack and for the edit snapshot.
    PatientRecord(const PatientRecord& other) = default;
    PatientRecord& operator=(const PatientRecord& other) = default;

    // Virtual destructor, so that deleting a record through a PatientRecord*
    // also runs the destructor of the subclass.
    virtual ~PatientRecord() = default;

    const std::string& getPatientID() const { return patientID; }
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
    const std::string& getContactNumber() const { return contactNumber; }
    const std::string& getPatientType() const { return patientType; }

    void setName(std::string newName) { name = newName; }
    void setAge(int newAge) { age = newAge; }
    void setContactNumber(std::string newContact) { contactNumber = newContact; }

    // The name shown to the clinic staff, for example "Emergency Patient".
    virtual std::string getTypeLabel() const = 0;

    // Category specific behaviour. These are the pure virtual functions that
    // make PatientRecord an abstract class.
    virtual void displayPatient() const = 0;
    virtual std::string getSpecialDetail() const = 0;

    // Deep copy of the record, used by the undo stack and by the edit
    // snapshot. Each subclass copies its own extra fields.
    virtual PatientRecord* clone() const = 0;

    // Copies the common fields back from a snapshot of the same category.
    // Subclasses override this and restore their own extra fields.
    virtual void restoreFrom(const PatientRecord& previous);
};

} // namespace model
} // namespace scapms
