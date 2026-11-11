#pragma once

#include <string>

namespace scapms {
namespace model {

// Interface for the patient types that the clinic can send a bill to.
//
// Like Appointable it is a pure abstract class: it declares the operations
// and holds no member variables. A class may implement more than one
// interface, and ScheduledPatient implements both of them.
class Billable {
public:
    virtual ~Billable() = default;

    // The amount that has to be paid for the consultation, in naira.
    virtual double getBillAmount() const = 0;

    // A short description of the bill, for the screen and for the reports.
    virtual std::string describeBill() const = 0;
};

} // namespace model
} // namespace scapms
