#pragma once

#include <exception>
#include <string>

class ClinicException : public std::exception {
protected:
    std::string errorMessage;

public:
    explicit ClinicException(const std::string& msg) : errorMessage(msg) {}

    const char* what() const noexcept override {
        return errorMessage.c_str();
    }
};

class DuplicateIDException : public ClinicException {
public:
    explicit DuplicateIDException(const std::string& id)
        : ClinicException("Patient ID [" + id + "] is already registered.") {}
};

class InvalidPatientDataException : public ClinicException {
public:
    explicit InvalidPatientDataException(const std::string& detail)
        : ClinicException(detail) {}
};

class FileIOException : public ClinicException {
public:
    explicit FileIOException(const std::string& filename, const std::string& reason)
        : ClinicException("Cannot open [" + filename + "]: " + reason) {}
};

class InvalidAppointmentException : public ClinicException {
public:
    explicit InvalidAppointmentException(const std::string& reason)
        : ClinicException(reason) {}
};
