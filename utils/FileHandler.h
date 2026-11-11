#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "ClinicExceptions.h"
#include "../model/PatientRecord.h"
#include "../model/Appointment.h"

namespace scapms {
namespace utils {

// Reads and writes the clinic data files.
//
// Patients are stored as "type,id,name,age,contact,<category fields>" and
// appointments as "id,patient,date,time,doctor,status". Fields that contain a
// comma are written inside double quotes.
class FileHandler {
public:
    // Writes the patients. The caller keeps ownership of the records.
    static void savePatients(const std::string& filename,
                             const std::vector<model::PatientRecord*>& patients);

    // Writes the appointments.
    static void saveAppointments(const std::string& filename,
                                 const std::vector<model::Appointment>& appointments);

    // Reads the patients. The returned records are owned by the caller, so a
    // file that is only half read never leaks them. Rows that cannot be read
    // are skipped and described in the warnings list.
    static std::vector<std::unique_ptr<model::PatientRecord>> loadPatients(
        const std::string& filename, std::vector<std::string>* warnings = nullptr);

    // Reads the appointments. Rows that cannot be read are skipped and
    // described in the warnings list.
    static std::vector<model::Appointment> loadAppointments(const std::string& filename,
                                                            std::vector<std::string>* warnings = nullptr);

private:
    // Splits one record into its fields. A field inside double quotes may
    // contain a comma, and a doubled quote is read as one quote character.
    static std::vector<std::string> splitRow(const std::string& line);

    // Wraps a field in double quotes when it contains a comma, a quote or a
    // newline, so that reading the file back gives the original value.
    static std::string quoteField(const std::string& value);

    // Writes the category specific columns of one patient.
    static void writeCategoryFields(std::ostream& out, const model::PatientRecord* patient);

    // Reads the category specific columns and builds the matching subclass.
    static std::unique_ptr<model::PatientRecord> readPatient(
        const std::vector<std::string>& fields, std::string* warning);
};

} // namespace utils
} // namespace scapms
