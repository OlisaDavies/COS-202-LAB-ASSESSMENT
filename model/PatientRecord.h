#pragma once

#include <string>
#include <vector>

#include "Appointment.h"

namespace scapms {
namespace model {

// Abstract base class for every patient the clinic knows about.
//
// It holds the details that every category has in common and leaves the
// category specific behaviour to the subclasses. Because it declares pure
// virtual functions it can never be created on its own, which is what makes
// the polymorphic calls through PatientRecord* safe.
//
// The class also shows the other half of an abstract class: besides the pure
// virtual functions it carries concrete methods that every category shares,
// such as the accessors, restoreFrom() and displayPatient().
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

    // Composition: a patient holds the appointments that belong to it, so a
    // patient is associated with one or more appointments. The appointments
    // are stored by value, so they are created and destroyed together with
    // the patient that owns them.
    std::vector<Appointment> appointments;

public:
    PatientRecord(std::string id, std::string patientName, int patientAge,
                  std::string contact, std::string type)
        : patientID(id),
          name(patientName),
          age(patientAge),
          contactNumber(contact),
          patientType(type) {}

    // Copying is allowed so that a record can be cloned safely for the undo
    // stack and for the edit snapshot. The copy is deep: the appointment
    // collection is copied as well.
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

    // The appointments the patient holds. This is the association that the
    // UML diagram shows as a composition.
    const std::vector<Appointment>& getAppointments() const { return appointments; }

    std::size_t getAppointmentCount() const { return appointments.size(); }

    const Appointment* findAppointment(const std::string& appointmentID) const;

    bool hasAppointment(const std::string& appointmentID) const;

    const Appointment* getNextPendingAppointment() const;

    std::size_t getPendingAppointmentCount() const;

    // Takes one of the patient's appointments out of the collection. The
    // database uses this when a booking is undone.
    bool removeAppointment(const std::string& appointmentID);

    // Puts an appointment back the way the backup copy says it was. The
    // database uses this when a cancellation is undone.
    bool restoreAppointment(const Appointment& backup);

    virtual std::string getTypeLabel() const = 0;

    // Every category writes its own version, so this is one of the pure
    // virtual functions that the polymorphic calls dispatch on.
    virtual std::string getPatientReport() const = 0;

    // Category specific behaviour. These are the pure virtual functions that
    // make PatientRecord an abstract class.
    virtual void displayPatient() const;
    virtual std::string getSpecialDetail() const = 0;

    // Deep copy of the record, used by the undo stack and by the edit
    // snapshot. Each subclass copies its own extra fields. The return type
    // is covariant, so a GeneralPatient clone comes back as a GeneralPatient.
    // The caller takes over the memory and should wrap the result in a
    // std::unique_ptr.
    virtual PatientRecord* clone() const = 0;

    // Copies the common fields back from a snapshot of the same category.
    // Subclasses override this and restore their own extra fields. The
    // appointments are not touched, because they are managed by the
    // database and are undone by their own entries on the undo stack.
    virtual void restoreFrom(const PatientRecord& previous);

    // Marks one of the appointments of this patient as completed. Returns
    // false when there is no such appointment or when it was cancelled.
    bool completeAppointment(const std::string& appointmentID);

protected:
    // Stores an appointment in this patient's own collection. The
    // appointment is copied, and the copy is stamped with the identifier of
    // its owner, so the two can never disagree. Returns false when the
    // patient already holds an appointment with that identifier.
    bool addAppointment(const Appointment& appointment);

    bool setAppointmentStatus(const std::string& appointmentID, const std::string& status);

    // A writable pointer to one of the appointments of this patient.
    Appointment* findAppointmentForUpdate(const std::string& appointmentID);
};

} // namespace model
} // namespace scapms
