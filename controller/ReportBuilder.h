#pragma once

#include <string>
#include <vector>

#include "../model/ClinicDatabase.h"

namespace scapms {
namespace controller {

// Builds the clinic reports.
//
// The reports are plain text so that they can be printed by the console
// demonstration and shown in the GUI without the report code depending on the
// user interface.
class ReportBuilder {
public:
    // How many patients there are in each category.
    static std::string categoryDistribution(const model::ClinicDatabase& database);

    // Every appointment that has not been completed yet.
    static std::string pendingAppointments(const model::ClinicDatabase& database);

    // How many appointments were scheduled, completed, cancelled or are still
    // pending.
    static std::string dailySummary(const model::ClinicDatabase& database);

    // The patients with the most completed visits, best first.
    static std::string frequentPatients(const model::ClinicDatabase& database);

    // The short version shown on the dashboard.
    static std::string dashboardSummary(const model::ClinicDatabase& database);

private:
    // The title block that every report starts with.
    static std::string header(const std::string& title);

    // The current date and time, written as text.
    static std::string timestamp();

    // Counts the completed visits of every patient.
    static std::vector<std::pair<std::string, int>> completedVisitCounts(
        const model::ClinicDatabase& database);
};

} // namespace controller
} // namespace scapms
