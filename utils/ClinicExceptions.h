#pragma once

#include <exception>
#include <string>

namespace scapms {
namespace utils {

// Base class for every error the clinic system raises on purpose.
// The GUI catches this one type and turns it into a message box, so a new
// error type never needs a new catch block.
class ClinicException : public std::exception {
protected:
    std::string errorMessage;

public:
    explicit ClinicException(const std::string& message) : errorMessage(message) {}
    const char* what() const noexcept override { return errorMessage.c_str(); }
};

class DuplicateIDException : public ClinicException {
public:
    explicit DuplicateIDException(const std::string& id)
        : ClinicException("Patient ID [" + id + "] is already registered.") {}
};

// Raised for illegal patient details such as a bad age or a blank name.
class InvalidPatientDataException : public ClinicException {
public:
    explicit InvalidPatientDataException(const std::string& detail) : ClinicException(detail) {}
};

// Raised for illegal appointment details such as a missing date or a clash.
class InvalidAppointmentException : public ClinicException {
public:
    explicit InvalidAppointmentException(const std::string& reason) : ClinicException(reason) {}
};

// Raised when a data file cannot be opened, read or written.
class FileIOException : public ClinicException {
public:
    FileIOException(const std::string& filename, const std::string& reason)
        : ClinicException("Cannot use the file [" + filename + "]: " + reason +
                          ". Please check that the path is right and that the file is not "
                          "open in another program.") {}
};

} // namespace utils
} // namespace scapms
