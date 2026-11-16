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

#include "../model/Appointment.h"
#include "../utils/ClinicExceptions.h"

namespace scapms {
namespace gui {

AdminPanel::AdminPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(12);

    QGroupBox* steps = new QGroupBox("Administrative Operations", this);
    QHBoxLayout* stepRow = new QHBoxLayout(steps);

    btnUndo = new QPushButton("Undo Last Operation", steps);
    connect(btnUndo, &QPushButton::clicked, this, &AdminPanel::onUndoClicked);

    btnSave = new QPushButton("Save Data", steps);
    connect(btnSave, &QPushButton::clicked, this, &AdminPanel::onSaveClicked);

    btnLoad = new QPushButton("Load Data", steps);
    connect(btnLoad, &QPushButton::clicked, this, &AdminPanel::onLoadClicked);

    btnNew = new QPushButton("New Database", steps);
    connect(btnNew, &QPushButton::clicked, this, &AdminPanel::onNewClicked);

    stepRow->addWidget(btnUndo);
    stepRow->addWidget(btnSave);
    stepRow->addWidget(btnLoad);
    stepRow->addSpacing(24);
    stepRow->addWidget(btnNew);
    stepRow->addStretch();

    layout->addWidget(steps);

    QHBoxLayout* lowerRow = new QHBoxLayout();

    // A small summary of the clinic. The figures are counted from the data
    // when the panel is redrawn, so they are always the true ones.
    QGroupBox* summaryBox = new QGroupBox("Clinic Summary", this);
    QFormLayout* summary = new QFormLayout(summaryBox);

    lblPatientCount = new QLabel("0", summaryBox);
    lblWaitingCount = new QLabel("0", summaryBox);
    lblAppointmentCount = new QLabel("0", summaryBox);
    lblPendingCount = new QLabel("0", summaryBox);

    summary->addRow("Registered patients:", lblPatientCount);
    summary->addRow("Patients waiting:", lblWaitingCount);
    summary->addRow("Appointments booked:", lblAppointmentCount);
    summary->addRow("Appointments still pending:", lblPendingCount);

    lowerRow->addWidget(summaryBox);

    QGroupBox* recentBox = new QGroupBox("Recently Accessed Records", this);
    QVBoxLayout* recentLayout = new QVBoxLayout(recentBox);

    listRecent = new QListWidget(recentBox);
    recentLayout->addWidget(listRecent);
    lowerRow->addWidget(recentBox);

    layout->addLayout(lowerRow);

    refresh();
}

void AdminPanel::refresh() {
    int pending = 0;
    for (const model::Appointment& appointment : manager.getAllAppointments()) {
        if (appointment.getStatus() == model::Appointment::STATUS_PENDING) {
            ++pending;
        }
    }

    lblPatientCount->setText(QString::number(manager.getPatientCount()));
    lblWaitingCount->setText(QString::number(manager.getQueueSize()));
    lblAppointmentCount->setText(QString::number(manager.getAllAppointments().size()));
    lblPendingCount->setText(QString::number(pending));

    listRecent->clear();
    for (const model::PatientRecord* patient : manager.getRecentPatients()) {
        listRecent->addItem(QString::fromStdString(
            patient->getPatientID() + " - " + patient->getName() + " (" +
            patient->getTypeLabel() + ")"));
    }
}

void AdminPanel::undoLastOperation() {
    std::string undone;
    try {
        undone = manager.undoLastAction();
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Undo error", error.what());
        return;                                 // the clinic is unchanged
    }

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

    // The appointment file is asked for as well, because the staff may have
    // saved it under any name. When it is not wanted, the file next to the
    // patient file is used.
    QString appointmentFile = QFileDialog::getOpenFileName(
        this, "Open the appointment file (cancel to use the one next to the patient file)",
        QFileInfo(patientFile).absolutePath(), "CSV files (*.csv)");
    if (appointmentFile.isEmpty()) {
        appointmentFile =
            QDir(QFileInfo(patientFile).absolutePath()).filePath("appointments.csv");
    }

    // Loading replaces the whole clinic, so the staff are asked first.
    if (manager.getPatientCount() > 0) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Load data",
            "Loading a file replaces every patient and every appointment, and it clears "
            "the undo history.\n\nContinue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (reply != QMessageBox::Yes) {
            return;
        }
    }

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

    try {
        manager.clearAll();
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "New database error", error.what());
        return;                                 // the clinic is unchanged
    }

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