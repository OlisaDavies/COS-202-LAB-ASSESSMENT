#pragma once

#include <string>

namespace scapms {
namespace utils {

// Turns a clinic time such as "02:00 PM" into the number of minutes since
// midnight, so that times can be compared instead of sorted as text.
// Returns -1 when the text is not a time.
int timeToMinutes(const std::string& time);

// True when the text looks like a calendar date, "YYYY-MM-DD".
bool isValidDate(const std::string& date);

} // namespace utils
} // namespace scapms
