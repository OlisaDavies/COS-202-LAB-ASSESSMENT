#pragma once

#include <string>
#include <vector>

#include "ClinicManager.h"

namespace scapms {
namespace controller {

// Builds the clinic reports.
//
// The reports are plain text so that they can be printed by the console
// demonstration and shown in the GUI without the report code depending on the
// user interface.
class ReportBuilder {
public:
    static std::string categoryDistribution(const ClinicManager& manager);

    // Every appointment that has not been completed yet.
    static std::string pendingAppointments(const ClinicManager& manager);

    // How many appointments were scheduled, completed, cancelled or are still
    // pending.
    static std::string dailySummary(const ClinicManager& manager);

    // The patients with the most completed visits, best first.
    static std::string frequentPatients(const ClinicManager& manager);

    // The short version shown on the dashboard.
    static std::string dashboardSummary(const ClinicManager& manager);

private:
    static std::string header(const std::string& title);

    static std::string timestamp();

    static std::vector<std::pair<std::string, int>> completedVisitCounts(
        const ClinicManager& manager);
};

} // namespace controller
} // namespace scapms
