#pragma once

#include <string>

class Appointable {
public:
    virtual ~Appointable() {}

    virtual bool scheduleAppointment(const std::string& date, const std::string& time) = 0;
    virtual bool cancelAppointment() = 0;
    virtual bool hasScheduledAppointment() const = 0;
    virtual std::string getAppointmentDetails() const = 0;
};
