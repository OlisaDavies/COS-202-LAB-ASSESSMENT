#include "FileHandler.h"

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <memory>
#include <ostream>
#include <sstream>

#include "StringUtil.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"

namespace scapms {
namespace utils {

namespace {

// Reads a whole number. Returns false when the text is not a number.
bool parseInt(const std::string& text, int* value) {
    if (text.empty()) {
        return false;
    }
    try {
        std::size_t used = 0;
        int parsed = std::stoi(text, &used);
        if (used != text.size()) {
            return false;
        }
        *value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Reads a decimal number. Returns false when the text is not a number.
bool parseDouble(const std::string& text, double* value) {
    if (text.empty()) {
        return false;
    }
    try {
        std::size_t used = 0;
        double parsed = std::stod(text, &used);
        if (used != text.size()) {
            return false;
        }
        *value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace

std::vector<std::string> FileHandler::splitRow(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool insideQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (insideQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i;                          // skip the escaped quote
                } else {
                    insideQuotes = false;
                }
            } else {
                current += c;
            }
        } else if (c == '"') {
            insideQuotes = true;
        } else if (c == ',') {
            fields.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }

    fields.push_back(current);

    // A row whose quote was never closed was cut in half by a line break
    // inside a quoted field. The broken row is dropped, so that it cannot
    // turn into a patient with nonsense fields.
    if (insideQuotes) {
        fields.clear();
    }
    return fields;
}

std::string FileHandler::quoteField(const std::string& value) {
    // Only a comma and a quote have to be protected. A line break is not
    // protected, because the reader reads the file line by line and would
    // not be able to join a field that is spread over two lines.
    if (value.find_first_of(",\"") == std::string::npos) {
        return value;                           // nothing to protect
    }

    std::string quoted = "\"";
    for (char c : value) {
        if (c == '"') {
            quoted += "\"\"";                    // escape the quote
        } else if (c == '\n' || c == '\r') {
            quoted += ' ';                       // a line break would break the file
        } else {
            quoted += c;
        }
    }
    quoted += "\"";
    return quoted;
}

void FileHandler::writeCategoryFields(std::ostream& out, const model::PatientRecord* patient) {
    const std::string type = patient->getPatientType();

    if (type == model::PatientRecord::TYPE_GENERAL) {
        const model::GeneralPatient* general =
            dynamic_cast<const model::GeneralPatient*>(patient);
        if (general != nullptr) {
            // The fee is written with two decimals. Without that the
            // stream would round the number to six digits, and a fee such
            // as 123456.78 would come back as 123457.
            out << std::fixed << std::setprecision(2) << general->getConsultationFee();
        }
    } else if (type == model::PatientRecord::TYPE_EMERGENCY) {
        const model::EmergencyPatient* emergency =
            dynamic_cast<const model::EmergencyPatient*>(patient);
        if (emergency != nullptr) {
            out << emergency->getSeverityLevel() << ","
                << quoteField(emergency->getTriageNote());
        }
    } else if (type == model::PatientRecord::TYPE_FOLLOWUP) {
        const model::FollowUpPatient* followUp =
            dynamic_cast<const model::FollowUpPatient*>(patient);
        if (followUp != nullptr) {
            out << quoteField(followUp->getPreviousVisitDate()) << ","
                << quoteField(followUp->getAssignedDoctor());
        }
    }
}

void FileHandler::savePatients(const std::string& filename,
                               const std::vector<model::PatientRecord*>& patients) {
    // The records are written to a temporary file first. The real file is
    // only replaced when the writing was finished, so a disk that is full
    // or a drive that is removed cannot leave a half written file behind.
    const std::string temporary = filename + ".tmp";

    {
        std::ofstream outFile(temporary);
        if (!outFile.is_open()) {
            throw FileIOException(filename, "could not be opened for writing");
        }

        for (const model::PatientRecord* patient : patients) {
            if (patient == nullptr) {
                continue;
            }

            outFile << patient->getPatientType() << ","
                    << quoteField(patient->getPatientID()) << ","
                    << quoteField(patient->getName()) << ","
                    << patient->getAge() << ","
                    << quoteField(patient->getContactNumber()) << ",";

            writeCategoryFields(outFile, patient);
            outFile << "\n";
        }

        outFile.close();

        // The state of the stream is checked, because a stream that has
        // failed does not say so by itself.
        if (outFile.fail()) {
            std::remove(temporary.c_str());
            throw FileIOException(filename,
                                  "could not be written completely. The disk may be full "
                                  "or the folder may not be writable");
        }
    }

    replaceFile(temporary, filename);
}

void FileHandler::saveAppointments(const std::string& filename,
                                   const std::vector<model::Appointment>& appointments) {
    // The same two step writing as for the patients.
    const std::string temporary = filename + ".tmp";

    {
        std::ofstream outFile(temporary);
        if (!outFile.is_open()) {
            throw FileIOException(filename, "could not be opened for writing");
        }

        for (const model::Appointment& appointment : appointments) {
            outFile << quoteField(appointment.getAppointmentID()) << ","
                    << quoteField(appointment.getPatientID()) << ","
                    << quoteField(appointment.getDate()) << ","
                    << quoteField(appointment.getTime()) << ","
                    << quoteField(appointment.getDoctorName()) << ","
                    << appointment.getStatus() << "\n";
        }

        outFile.close();

        if (outFile.fail()) {
            std::remove(temporary.c_str());
            throw FileIOException(filename,
                                  "could not be written completely. The disk may be full "
                                  "or the folder may not be writable");
        }
    }

    replaceFile(temporary, filename);
}

void FileHandler::replaceFile(const std::string& temporary, const std::string& filename) {
    // The old file is only removed when the new one is ready, so the clinic
    // never loses its data because of a failed save.
    std::remove(filename.c_str());

    if (std::rename(temporary.c_str(), filename.c_str()) != 0) {
        std::remove(temporary.c_str());
        throw FileIOException(filename, "could not be replaced by the new file");
    }
}

std::unique_ptr<model::PatientRecord> FileHandler::readPatient(
    const std::vector<std::string>& fields, std::string* warning) {
    if (fields.size() < 5) {
        *warning = "a row has fewer than five columns and was skipped";
        return nullptr;
    }

    const std::string type = utils::trim(fields[0]);
    const std::string id = utils::trim(fields[1]);
    const std::string name = utils::trim(fields[2]);
    const std::string ageText = utils::trim(fields[3]);
    const std::string contact = utils::trim(fields[4]);

    if (id.empty()) {
        *warning = "a row has no patient ID and was skipped";
        return nullptr;
    }
    if (name.empty()) {
        *warning = "patient [" + id + "] has no name and was skipped";
        return nullptr;
    }

    int age = 0;
    if (!parseInt(ageText, &age)) {
        *warning = "patient [" + id + "] has an unreadable age [" + ageText + "] and was skipped";
        return nullptr;
    }
    if (age < 0 || age > 120) {
        *warning = "patient [" + id + "] has an age outside 0-120 and was skipped";
        return nullptr;
    }

    if (type == model::PatientRecord::TYPE_GENERAL) {
        double fee = 0.0;
        if (fields.size() <= 5) {
            // A general row without a fee is incomplete, so it is skipped
            // instead of being loaded with a fee of zero.
            *warning = "patient [" + id + "] has no consultation fee and was skipped";
            return nullptr;
        }
        const std::string feeText = utils::trim(fields[5]);
        if (!feeText.empty() && !parseDouble(feeText, &fee)) {
            *warning = "patient [" + id + "] has an unreadable fee [" + feeText + "], using 0";
            fee = 0.0;
        }
        // A fee of "nan" or of infinity is not a real amount. It is skipped,
        // because a broken fee would be carried through every report.
        if (!model::GeneralPatient::isValidConsultationFee(fee)) {
            *warning = "patient [" + id + "] has a fee of [" + feeText +
                       "] that is not a real amount, and was skipped";
            return nullptr;
        }
        return std::make_unique<model::GeneralPatient>(id, name, age, contact, fee);
    }

    if (type == model::PatientRecord::TYPE_EMERGENCY) {
        int severity = 1;
        std::string severityText = fields.size() > 5 ? utils::trim(fields[5]) : "";
        std::string note = fields.size() > 6 ? utils::trim(fields[6]) : "";
        if (!severityText.empty() && !parseInt(severityText, &severity)) {
            *warning = "patient [" + id + "] has an unreadable severity, using 1";
            severity = 1;
        }
        return std::make_unique<model::EmergencyPatient>(id, name, age, contact, severity, note);
    }

    if (type == model::PatientRecord::TYPE_FOLLOWUP) {
        std::string previousVisit = fields.size() > 5 ? utils::trim(fields[5]) : "";
        std::string doctor = fields.size() > 6 ? utils::trim(fields[6]) : "";
        return std::make_unique<model::FollowUpPatient>(id, name, age, contact,
                                                         previousVisit, doctor);
    }

    *warning = "patient [" + id + "] has an unknown category [" + type + "] and was skipped";
    return nullptr;
}

std::vector<std::unique_ptr<model::PatientRecord>> FileHandler::loadPatients(
    const std::string& filename, std::vector<std::string>* warnings) {
    std::vector<std::unique_ptr<model::PatientRecord>> loaded;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        throw FileIOException(filename, "could not be opened for reading");
    }

    std::string line;
    int rowNumber = 0;
    while (std::getline(inFile, line)) {
        ++rowNumber;
        if (utils::trim(line).empty()) {
            continue;
        }

        std::vector<std::string> fields = splitRow(line);
        std::string warning;
        std::unique_ptr<model::PatientRecord> patient = readPatient(fields, &warning);
        if (patient == nullptr) {
            if (warnings != nullptr) {
                warnings->push_back("row " + std::to_string(rowNumber) + ": " + warning);
            }
            continue;
        }
        loaded.push_back(std::move(patient));
    }

    inFile.close();
    return loaded;
}

std::vector<model::Appointment> FileHandler::loadAppointments(const std::string& filename,
                                                              std::vector<std::string>* warnings) {
    std::vector<model::Appointment> loaded;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        throw FileIOException(filename, "could not be opened for reading");
    }

    std::string line;
    int rowNumber = 0;
    while (std::getline(inFile, line)) {
        ++rowNumber;
        if (utils::trim(line).empty()) {
            continue;
        }

        std::vector<std::string> fields = splitRow(line);
        if (fields.size() < 6) {
            if (warnings != nullptr) {
                warnings->push_back("row " + std::to_string(rowNumber) +
                                    ": an appointment needs six columns and was skipped");
            }
            continue;
        }

        std::string status = utils::trim(fields[5]);
        if (status != model::Appointment::STATUS_PENDING &&
            status != model::Appointment::STATUS_COMPLETED &&
            status != model::Appointment::STATUS_CANCELLED) {
            status = model::Appointment::STATUS_PENDING;
            if (warnings != nullptr) {
                warnings->push_back("row " + std::to_string(rowNumber) +
                                    ": unknown status, treated as Pending");
            }
        }

        loaded.push_back(model::Appointment(utils::trim(fields[0]), utils::trim(fields[1]),
                                           utils::trim(fields[2]), utils::trim(fields[3]),
                                           utils::trim(fields[4]), status));
    }

    inFile.close();
    return loaded;
}

} // namespace utils
} // namespace scapms
