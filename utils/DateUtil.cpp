#include "DateUtil.h"

#include <cstdio>
#include <cstdlib>

namespace scapms {
namespace utils {

int timeToMinutes(const std::string& time) {
    // Expected shape: "HH:MM AM" or "HH:MM PM".
    int hour = 0;
    int minute = 0;
    char meridiem[3] = {0, 0, 0};

    if (std::sscanf(time.c_str(), "%d:%d %2s", &hour, &minute, meridiem) != 3) {
        return -1;
    }
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
        return -1;
    }

    // The marker has to be a real AM or PM. Without this check a time such
    // as "10:30 XM" would be accepted and stored as bad data.
    if (meridiem[0] != 'A' && meridiem[0] != 'P') {
        return -1;
    }
    if (meridiem[1] != 'M') {
        return -1;
    }

    // With a marker the hour runs from 1 to 12.
    if (hour < 1 || hour > 12) {
        return -1;
    }

    int total = hour * 60 + minute;
    if (meridiem[0] == 'P' && hour < 12) {
        total += 12 * 60;                      // afternoon
    } else if (meridiem[0] == 'A' && hour == 12) {
        total -= 12 * 60;                      // midnight
    }
    return total;
}

bool isValidDate(const std::string& date) {
    int year = 0;
    int month = 0;
    int day = 0;

    if (std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }
    if (year < 1900 || year > 2200 || month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    // Reject the impossible days such as 31 April.
    static const int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int limit = daysInMonth[month - 1];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
        limit = 29;                            // leap year
    }
    return day <= limit;
}

} // namespace utils
} // namespace scapms
