#include "ReportPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "../controller/ReportBuilder.h"

namespace scapms {
namespace gui {

ReportPanel::ReportPanel(model::ClinicDatabase& database, QWidget* parent)
    : QWidget(parent), database(database) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Report:", this));

    cmbReportType = new QComboBox(this);
    cmbReportType->addItem("Patient category distribution");
    cmbReportType->addItem("Patients with pending appointments");
    cmbReportType->addItem("Daily appointment summary");
    cmbReportType->addItem("Frequently attended patients");
    cmbReportType->setToolTip("The report that is built.");
    topRow->addWidget(cmbReportType, 1);

    QPushButton* btnGenerate = new QPushButton("Generate", this);
    btnGenerate->setToolTip("Build the chosen report from the clinic data.");
    connect(btnGenerate, &QPushButton::clicked, this, &ReportPanel::onGenerateReport);
    topRow->addWidget(btnGenerate);

    layout->addLayout(topRow);

    txtReportOutput = new QTextEdit(this);
    txtReportOutput->setReadOnly(true);
    txtReportOutput->setFontFamily("Consolas");
    txtReportOutput->setFontPointSize(10);
    layout->addWidget(txtReportOutput);

    onGenerateReport();
}

void ReportPanel::onGenerateReport() {
    const int reportIndex = cmbReportType->currentIndex();

    std::string report;
    if (reportIndex == 0) {
        report = controller::ReportBuilder::categoryDistribution(database);
    } else if (reportIndex == 1) {
        report = controller::ReportBuilder::pendingAppointments(database);
    } else if (reportIndex == 2) {
        report = controller::ReportBuilder::dailySummary(database);
    } else {
        report = controller::ReportBuilder::frequentPatients(database);
    }

    txtReportOutput->setPlainText(QString::fromStdString(report));
}

void ReportPanel::refresh() {
    onGenerateReport();
}

} // namespace gui
} // namespace scapms
