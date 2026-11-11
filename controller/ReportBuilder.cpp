#include "ReportBuilder.h"

#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>

namespace scapms {
namespace controller {

namespace {

const char* RULE = "------------------------------------------------------------";

// Pads a value to the given width. A negative width keeps the value on the
// left and fills the rest with spaces.
std::string pad(const std::string& value, int width) {
    const std::size_t target = (width < 0) ? static_cast<std::size_t>(-width)
                                            : static_cast<std::size_t>(width);

    std::string padded = value;
    while (padded.size() < target) {
        padded += " ";
    }
    return padded;
}

std::string padNumber(int value, int width) {
    std::ostringstream out;
    out << value;
    return pad(out.str(), width);
}

// One line of a report: a label and a number.
std::string line(const std::string& label, int value) {
    return pad(label, 28) + padNumber(value, 5) + "\n";
}

} // namespace

std::string ReportBuilder::timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm parts = {};
    localtime_s(&parts, &now);

    char buffer[32] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &parts);
    return std::string(buffer);
}

std::string ReportBuilder::header(const std::string& title) {
    std::string out = "CLINIC REPORT\n";
    out += title + "\n";
    out += "Generated: " + timestamp() + "\n";
    out += RULE;
    out += "\n";
    return out;
}

std::string ReportBuilder::categoryDistribution(const ClinicManager& manager) {
    int generalCount = 0;
    int emergencyCount = 0;
    int followUpCount = 0;

    for (const model::PatientRecord* patient : manager.getAllPatients()) {
        if (patient == nullptr) {
            continue;
        }
        if (patient->getPatientType() == model::PatientRecord::TYPE_GENERAL) {
            ++generalCount;
        } else if (patient->getPatientType() == model::PatientRecord::TYPE_EMERGENCY) {
            ++emergencyCount;
        } else if (patient->getPatientType() == model::PatientRecord::TYPE_FOLLOWUP) {
            ++followUpCount;
        }
    }

    const int total = static_cast<int>(manager.getPatientCount());

    std::string out = header("Patient Category Distribution");
    out += line("General Outpatient:", generalCount);
    out += line("Emergency Patient:", emergencyCount);
    out += line("Follow-Up Patient:", followUpCount);
    out += RULE;
    out += "\n";
    out += line("Total patients:", total);
    return out;
}

std::string ReportBuilder::pendingAppointments(const ClinicManager& manager) {
    std::string out = header("Patients with Pending Appointments");
    out += pad("ID", -12) + pad("Patient", -24) + pad("Date / Time", -24) + pad("Doctor", -16) + "\n";
    out += RULE;
    out += "\n";

    int pendingCount = 0;
    for (const model::Appointment& appointment : manager.getAllAppointments()) {
        if (appointment.getStatus() != model::Appointment::STATUS_PENDING) {
            continue;
        }

        const model::PatientRecord* patient = manager.findPatientByID(appointment.getPatientID());
        const std::string patientName = (patient != nullptr) ? patient->getName() : "Unknown";

        out += pad(appointment.getAppointmentID(), -12)
             + pad(patientName, -24)
             + pad(appointment.getDate() + " " + appointment.getTime(), -24)
             + pad(appointment.getDoctorName(), -16)
             + "\n";
        ++pendingCount;
    }

    out += RULE;
    out += "\n";
    out += line("Pending appointments:", pendingCount);
    return out;
}

std::string ReportBuilder::dailySummary(const ClinicManager& manager) {
    int completed = 0;
    int pending = 0;
    int cancelled = 0;

    for (const model::Appointment& appointment : manager.getAllAppointments()) {
        if (appointment.getStatus() == model::Appointment::STATUS_COMPLETED) {
            ++completed;
        } else if (appointment.getStatus() == model::Appointment::STATUS_PENDING) {
            ++pending;
        } else if (appointment.getStatus() == model::Appointment::STATUS_CANCELLED) {
            ++cancelled;
        }
    }

    const std::size_t total = manager.getAllAppointments().size();
    const double completionRate = (total == 0) ? 0.0 : (completed * 100.0 / total);

    std::string out = header("Daily Appointment Summary");
    out += line("Scheduled appointments:", static_cast<int>(total));
    out += line("  Completed:", completed);
    out += line("  Pending:", pending);
    out += line("  Cancelled:", cancelled);
    out += RULE;
    out += "\n";

    std::ostringstream rate;
    rate << std::fixed << std::setprecision(1) << completionRate;
    out += pad("Completion rate:", 28) + pad(rate.str() + "%", 5) + "\n";
    return out;
}

std::vector<std::pair<std::string, int>> ReportBuilder::completedVisitCounts(
    const ClinicManager& manager) {
    std::map<std::string, int> counts;

    for (const model::Appointment& appointment : manager.getAllAppointments()) {
        if (appointment.getStatus() == model::Appointment::STATUS_COMPLETED) {
            ++counts[appointment.getPatientID()];
        }
    }

    // The map is turned into a list, because the list can be ranked.
    std::vector<std::pair<std::string, int>> ranked;
    for (const auto& entry : counts) {
        ranked.push_back(std::make_pair(entry.first, entry.second));
    }
    return ranked;
}

std::string ReportBuilder::frequentPatients(const ClinicManager& manager) {
    std::vector<std::pair<std::string, int>> ranked = completedVisitCounts(manager);

    // Selection sort on the visit count, best first. The list is short, so the
    // simple routine is enough.
    for (int i = 0; i < static_cast<int>(ranked.size()) - 1; ++i) {
        int bestIndex = i;
        for (int j = i + 1; j < static_cast<int>(ranked.size()); ++j) {
            if (ranked[j].second > ranked[bestIndex].second) {
                bestIndex = j;
            }
        }
        if (bestIndex != i) {
            std::swap(ranked[i], ranked[bestIndex]);
        }
    }

    std::string out = header("Frequently Attended Patients");
    out += pad("Rank", -8) + pad("Patient ID", -16) + pad("Patient Name", -28) +
           pad("Completed Visits", -18) + "\n";
    out += RULE;
    out += "\n";

    int rank = 1;
    for (const auto& entry : ranked) {
        const model::PatientRecord* patient = manager.findPatientByID(entry.first);
        const std::string patientName = (patient != nullptr) ? patient->getName() : "Unknown";

        out += padNumber(rank++, -8)
             + pad(entry.first, -16)
             + pad(patientName, -28)
             + padNumber(entry.second, -18)
             + "\n";
    }

    if (ranked.empty()) {
        out += "No completed visits recorded yet.\n";
    }
    return out;
}

std::string ReportBuilder::dashboardSummary(const ClinicManager& manager) {
    int completed = 0;
    int pending = 0;
    int cancelled = 0;

    for (const model::Appointment& appointment : manager.getAllAppointments()) {
        if (appointment.getStatus() == model::Appointment::STATUS_COMPLETED) {
            ++completed;
        } else if (appointment.getStatus() == model::Appointment::STATUS_PENDING) {
            ++pending;
        } else if (appointment.getStatus() == model::Appointment::STATUS_CANCELLED) {
            ++cancelled;
        }
    }

    std::string out;
    out += line("Registered patients:", static_cast<int>(manager.getPatientCount()));
    out += line("Patients waiting:", static_cast<int>(manager.getQueueSize()));
    out += line("Appointments booked:", static_cast<int>(manager.getAllAppointments().size()));
    out += line("  Completed:", completed);
    out += line("  Pending:", pending);
    out += line("  Cancelled:", cancelled);
    return out;
}

} // namespace controller
} // namespace scapms
