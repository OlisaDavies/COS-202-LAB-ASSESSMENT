#include "AdminPanel.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include <vector>

#include "../controller/ReportBuilder.h"
#include "../utils/ClinicExceptions.h"

namespace scapms {
namespace gui {

AdminPanel::AdminPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // ---- the administrative steps ----
    QGroupBox* steps = new QGroupBox("Administrative Operations", this);
    QHBoxLayout* stepRow = new QHBoxLayout(steps);
    stepRow->setSpacing(10);

    btnUndo = new QPushButton("Undo Last Operation", steps);
    btnUndo->setToolTip("Reverse the last change to a patient or an appointment.");
    connect(btnUndo, &QPushButton::clicked, this, &AdminPanel::onUndoClicked);

    btnSave = new QPushButton("Save Data", steps);
    btnSave->setToolTip("Write the patients and the appointments to CSV files.");
    connect(btnSave, &QPushButton::clicked, this, &AdminPanel::onSaveClicked);

    btnLoad = new QPushButton("Load Data", steps);
    btnLoad->setToolTip("Read the patients and the appointments from CSV files.");
    connect(btnLoad, &QPushButton::clicked, this, &AdminPanel::onLoadClicked);

    btnNew = new QPushButton("New Database", steps);
    btnNew->setToolTip("Clear every record and start again with an empty clinic.");
    connect(btnNew, &QPushButton::clicked, this, &AdminPanel::onNewClicked);

    stepRow->addWidget(btnUndo);
    stepRow->addWidget(btnSave);
    stepRow->addWidget(btnLoad);
    stepRow->addStretch();
    stepRow->addWidget(btnNew);

    layout->addWidget(steps);

    // ---- the dashboard and the recent records ----
    QHBoxLayout* lowerRow = new QHBoxLayout();
    lowerRow->setSpacing(12);

    QGroupBox* dashboardBox = new QGroupBox("Clinic Dashboard", this);
    QFormLayout* dashboardForm = new QFormLayout(dashboardBox);
    dashboardForm->setContentsMargins(12, 16, 12, 12);

    txtDashboard = new QTextEdit(dashboardBox);
    txtDashboard->setReadOnly(true);
    txtDashboard->setFontFamily("Consolas");
    txtDashboard->setFontPointSize(10);
    txtDashboard->setMinimumHeight(220);
    dashboardForm->addRow(txtDashboard);
    lowerRow->addWidget(dashboardBox, 1);

    QGroupBox* recentBox = new QGroupBox("Recently Accessed Records", this);
    QVBoxLayout* recentLayout = new QVBoxLayout(recentBox);
    recentLayout->setContentsMargins(12, 16, 12, 12);

    listRecent = new QListWidget(recentBox);
    listRecent->setToolTip("The records that were used most recently.");
    listRecent->setMinimumHeight(220);
    recentLayout->addWidget(listRecent);
    lowerRow->addWidget(recentBox, 1);

    layout->addLayout(lowerRow);
    layout->addStretch();

    refresh();
}

void AdminPanel::refresh() {
    txtDashboard->setPlainText(
        QString::fromStdString(controller::ReportBuilder::dashboardSummary(manager)));

    listRecent->clear();
    for (const model::PatientRecord* patient : manager.getRecentPatients()) {
        listRecent->addItem(QString::fromStdString(
            patient->getPatientID() + " - " + patient->getName() + " (" +
            patient->getTypeLabel() + ")"));
    }
}

void AdminPanel::undoLastOperation() {
    const std::string undone = manager.undoLastAction();

    refresh();
    emit dataChanged();

    if (undone.empty()) {
        QMessageBox::information(this, "Undo", "There is nothing to undo.");
    } else {
        QMessageBox::information(this, "Undo",
                                 QString::fromStdString(undone) + " was undone.");
    }
}

void AdminPanel::saveData() {
    const QString patientFile = QFileDialog::getSaveFileName(
        this, "Save the patients", "patients.csv", "CSV files (*.csv)");
    if (patientFile.isEmpty()) {
        return;
    }

    const QString appointmentFile = QFileDialog::getSaveFileName(
        this, "Save the appointments", "appointments.csv", "CSV files (*.csv)");
    if (appointmentFile.isEmpty()) {
        return;
    }

    try {
        manager.saveToFiles(patientFile.toStdString(), appointmentFile.toStdString());
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Save error", error.what());
        return;
    }

    QMessageBox::information(this, "Saved",
                             "The patient and appointment data was written to CSV files.");
}

void AdminPanel::loadData() {
    const QString patientFile = QFileDialog::getOpenFileName(
        this, "Open the patient file", "", "CSV files (*.csv)");
    if (patientFile.isEmpty()) {
        return;
    }

    // The appointment file is expected to sit next to the patient file.
    const QString appointmentFile =
        QDir(QFileInfo(patientFile).absolutePath()).filePath("appointments.csv");

    std::vector<std::string> warnings;

    try {
        manager.loadFromFiles(patientFile.toStdString(), appointmentFile.toStdString(),
                              &warnings);
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Load error", error.what());
        return;
    }

    refresh();
    emit dataChanged();

    if (!warnings.empty()) {
        QString text;
        for (const std::string& warning : warnings) {
            text += "- " + QString::fromStdString(warning) + "\n";
        }

        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle("Some rows could not be read");
        box.setText("The file was loaded, but the rows below were skipped:");
        box.setInformativeText(text);
        box.exec();
    }

    QMessageBox::information(
        this, "Loaded",
        QString::number(manager.getPatientCount()) + " patients and " +
        QString::number(manager.getAllAppointments().size()) + " appointments were loaded.");
}

void AdminPanel::newDatabase() {
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this, "New database",
        "Remove every patient and every appointment?\n\nThis cannot be undone, because it also clears the undo history.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    manager.clearAll();

    refresh();
    emit dataChanged();
}

void AdminPanel::onUndoClicked() {
    undoLastOperation();
}

void AdminPanel::onSaveClicked() {
    saveData();
}

void AdminPanel::onLoadClicked() {
    loadData();
}

void AdminPanel::onNewClicked() {
    newDatabase();
}

} // namespace gui
} // namespace scapms
