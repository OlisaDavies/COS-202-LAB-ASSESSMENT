#pragma once

#include <string>
#include <vector>

namespace scapms {
namespace utils {

// Produces the identifiers the clinic hands out.
//
// The generator is stateless: it looks at the identifiers that are already in
// use and returns the next free one. That keeps the number generation in one
// place instead of spreading counters across the GUI.
class IDGenerator {
public:
    // Next free "APT-####" identifier.
    static std::string nextAppointmentId(const std::vector<std::string>& takenIds);

    // Next free identifier for a patient category, for example "GEN-106".
    static std::string nextPatientId(const std::string& prefix,
                                     const std::vector<std::string>& takenIds);

    // Prefix used for each patient category.
    static const char* patientPrefix(const std::string& patientType);

private:
    // Highest number already used for the given prefix, or 0 when none.
    static int highestUsedNumber(const std::string& prefix,
                                 const std::vector<std::string>& takenIds);
};

} // namespace utils
} // namespace scapms
