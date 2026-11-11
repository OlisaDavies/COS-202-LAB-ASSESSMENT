#include "PatientRecord.h"

namespace scapms {
namespace model {

void PatientRecord::restoreFrom(const PatientRecord& previous) {
    name = previous.name;
    age = previous.age;
    contactNumber = previous.contactNumber;
}

} // namespace model
} // namespace scapms
