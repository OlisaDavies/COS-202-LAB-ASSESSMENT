#pragma once

#include <string>
#include <vector>

#include "Appointment.h"

namespace scapms {
namespace model {

// Abstract base class for every patient the clinic knows about. It holds the
// details every category has in common and leaves the rest to the subclasses.
class PatientRecord {
public:
    // The three patient categories. The strings are also the key in the data files.
    static constexpr const char* TYPE_GENERAL   = "General";
    static constexpr const char* TYPE_EMERGENCY = "Emergency";
    static constexpr const char* TYPE_FOLLOWUP  = "FollowUp";

protected:
    std::string patientID;
    std::string name;
    int age;
    std::string contactNumber;
    std::string patientType;

    // Composition: a patient owns the appointments that belong to it, so they
    // are created and destroyed together with the patient that holds them.
    std::vector<Appointment> appointments;

public:
    PatientRecord(std::string id, std::string patientName, int patientAge,
                  std::string contact, std::string type)
        : patientID(id),
          name(patientName),
          age(patientAge),
          contactNumber(contact),
          patientType(type) {}

    // Copying is allowed, and it is deep, so that a record can be cloned.
    PatientRecord(const PatientRecord& other) = default;
    PatientRecord& operator=(const PatientRecord& other) = default;

    // Virtual destructor, so that deleting through a PatientRecord* also
    // runs the destructor of the subclass.
    virtual ~PatientRecord() = default;

    const std::string& getPatientID() const { return patientID; }
    const std::string& getName() const { return name; }
    int getAge() const { return age; }
    const std::string& getContactNumber() const { return contactNumber; }
    const std::string& getPatientType() const { return patientType; }

    void setName(std::string newName) { name = newName; }
    void setAge(int newAge) { age = newAge; }
    void setContactNumber(std::string newContact) { contactNumber = newContact; }

    const std::vector<Appointment>& getAppointments() const { return appointments; }

    std::size_t getAppointmentCount() const { return appointments.size(); }

    const Appointment* findAppointment(const std::string& appointmentID) const;

    bool hasAppointment(const std::string& appointmentID) const;

    const Appointment* getNextPendingAppointment() const;

    std::size_t getPendingAppointmentCount() const;

    // Takes one appointment out of the collection. The database uses this when
    // a booking is undone.
    bool removeAppointment(const std::string& appointmentID);

    // Puts an appointment back the way the backup copy says it was.
    bool restoreAppointment(const Appointment& backup);

    virtual std::string getTypeLabel() const = 0;

    // Every category writes its own version. The polymorphic calls dispatch
    // on this one.
    virtual std::string getPatientReport() const = 0;

    // Prints the report of the category. Concrete, so it works for all of them.
    virtual void displayPatient() const;

    virtual std::string getSpecialDetail() const = 0;

    // Deep copy of the record, used by the undo stack and by the edit
    // snapshot. Each subclass copies its own extra fields.
    virtual PatientRecord* clone() const = 0;

    // Copies the common fields back from a snapshot of the same category. The
    // appointments are not touched, the database undoes them separately.
    virtual void restoreFrom(const PatientRecord& previous);

    // Marks one appointment of this patient as completed. Returns false when
    // it was cancelled already.
    bool completeAppointment(const std::string& appointmentID);

protected:
    // Stores an appointment in this patient's own collection. The copy is
    // stamped with the identifier of its owner, so the two cannot disagree.
    bool addAppointment(const Appointment& appointment);

    bool setAppointmentStatus(const std::string& appointmentID, const std::string& status);

    // A writable pointer to one of the appointments of this patient.
    Appointment* findAppointmentForUpdate(const std::string& appointmentID);
};

} // namespace model
} // namespace scapms
