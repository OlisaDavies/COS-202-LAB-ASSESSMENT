#include "ReportPanel.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

#include "../controller/ReportBuilder.h"

namespace scapms {
namespace gui {

ReportPanel::ReportPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager) {
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

    // Choosing another report builds it at once, so that the button does not
    // have to be pressed as well.
    connect(cmbReportType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ReportPanel::onGenerateReport);

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
        report = controller::ReportBuilder::categoryDistribution(manager);
    } else if (reportIndex == 1) {
        report = controller::ReportBuilder::pendingAppointments(manager);
    } else if (reportIndex == 2) {
        report = controller::ReportBuilder::dailySummary(manager);
    } else {
        report = controller::ReportBuilder::frequentPatients(manager);
    }

    txtReportOutput->setPlainText(QString::fromStdString(report));
}

void ReportPanel::refresh() {
    onGenerateReport();
}

} // namespace gui
} // namespace scapms
