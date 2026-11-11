#include "AdminPanel.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
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
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

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

    QHBoxLayout* lowerRow = new QHBoxLayout();
    lowerRow->setSpacing(12);

    QGroupBox* dashboardBox = new QGroupBox("Clinic Dashboard", this);
    QVBoxLayout* dashboardLayout = new QVBoxLayout(dashboardBox);
    dashboardLayout->setContentsMargins(16, 20, 16, 16);
    dashboardLayout->setSpacing(14);

    // The numbers of the clinic are put in a grid, so that the dashboard is
    // made of real widgets and the figures line up under each other.
    QGridLayout* figures = new QGridLayout();
    figures->setHorizontalSpacing(18);
    figures->setVerticalSpacing(10);

    lblPatientCount = new QLabel("0", dashboardBox);
    lblWaitingCount = new QLabel("0", dashboardBox);
    lblBookedCount = new QLabel("0", dashboardBox);
    lblCompletedCount = new QLabel("0", dashboardBox);
    lblPendingCount = new QLabel("0", dashboardBox);
    lblCancelledCount = new QLabel("0", dashboardBox);

    for (QLabel* value : {lblPatientCount, lblWaitingCount, lblBookedCount,
                           lblCompletedCount, lblPendingCount, lblCancelledCount}) {
        value->setStyleSheet("font-size: 22px; font-weight: 600; color: #25292f;");
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    lblCompletedCount->setStyleSheet("font-size: 22px; font-weight: 600; color: #2c7a52;");
    lblPendingCount->setStyleSheet("font-size: 22px; font-weight: 600; color: #96701d;");
    lblCancelledCount->setStyleSheet("font-size: 22px; font-weight: 600; color: #ad3838;");

    figures->addWidget(new QLabel("Registered patients", dashboardBox), 0, 0);
    figures->addWidget(lblPatientCount, 0, 1);
    figures->addWidget(new QLabel("Patients waiting", dashboardBox), 1, 0);
    figures->addWidget(lblWaitingCount, 1, 1);
    figures->addWidget(new QLabel("Appointments booked", dashboardBox), 2, 0);
    figures->addWidget(lblBookedCount, 2, 1);
    figures->addWidget(new QLabel("Completed visits", dashboardBox), 3, 0);
    figures->addWidget(lblCompletedCount, 3, 1);
    figures->addWidget(new QLabel("Pending appointments", dashboardBox), 4, 0);
    figures->addWidget(lblPendingCount, 4, 1);
    figures->addWidget(new QLabel("Cancelled appointments", dashboardBox), 5, 0);
    figures->addWidget(lblCancelledCount, 5, 1);
    figures->setColumnStretch(0, 1);
    dashboardLayout->addLayout(figures);

    // The categories of the clinic with the priority that goes with them.
    QGridLayout* categories = new QGridLayout();
    categories->setHorizontalSpacing(14);
    categories->setVerticalSpacing(6);

    categories->addWidget(new QLabel("Category", dashboardBox), 0, 0);
    categories->addWidget(new QLabel("Priority", dashboardBox), 0, 1);
    categories->addWidget(buildCategoryChip("Emergency", "#ffebee", "#b71c1c", dashboardBox), 1, 0);
    categories->addWidget(new QLabel("High Priority", dashboardBox), 1, 1);
    categories->addWidget(buildCategoryChip("General", "#e8f5e9", "#1b5e20", dashboardBox), 2, 0);
    categories->addWidget(new QLabel("Normal", dashboardBox), 2, 1);
    categories->addWidget(buildCategoryChip("Follow-Up", "#e3f2fd", "#0d47a1", dashboardBox), 3, 0);
    categories->addWidget(new QLabel("Scheduled", dashboardBox), 3, 1);
    categories->setColumnStretch(1, 1);
    dashboardLayout->addLayout(categories);

    dashboardLayout->addStretch();
    lowerRow->addWidget(dashboardBox, 1);

    QGroupBox* recentBox = new QGroupBox("Recently Accessed Records", this);
    QVBoxLayout* recentLayout = new QVBoxLayout(recentBox);
    recentLayout->setContentsMargins(12, 16, 12, 12);

    listRecent = new QListWidget(recentBox);
    listRecent->setToolTip("The records that were used most recently, the most recently "
                            "used one first. The list holds at most ten records.");
    recentLayout->addWidget(listRecent);
    lowerRow->addWidget(recentBox, 1);

    // The dashboard and the recent records share the rest of the window, so
    // that neither of them ends in a band of empty space.
    layout->addLayout(lowerRow, 1);

    refresh();
}

QLabel* AdminPanel::buildCategoryChip(const QString& text, const QString& background,
                                       const QString& foreground, QWidget* parent) {
    QLabel* chip = new QLabel(text, parent);
    chip->setStyleSheet(
        QString("background-color: %1; color: %2; border-radius: 9px;"
                "padding: 3px 12px; font-size: 12px; font-weight: 600;").arg(background, foreground));
    chip->setAlignment(Qt::AlignCenter);
    chip->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return chip;
}

void AdminPanel::refresh() {
    refreshStatistics();

    listRecent->clear();
    for (const model::PatientRecord* patient : manager.getRecentPatients()) {
        listRecent->addItem(QString::fromStdString(
            patient->getPatientID() + " - " + patient->getName() + " (" +
            patient->getTypeLabel() + ")"));
    }
}

void AdminPanel::refreshStatistics() {
    // The figures of the dashboard are counted from the data of the clinic,
    // so that they are always the true ones.
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

    lblPatientCount->setText(QString::number(manager.getPatientCount()));
    lblWaitingCount->setText(QString::number(manager.getQueueSize()));
    lblBookedCount->setText(QString::number(manager.getAllAppointments().size()));
    lblCompletedCount->setText(QString::number(completed));
    lblPendingCount->setText(QString::number(pending));
    lblCancelledCount->setText(QString::number(cancelled));
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

    // The appointment file is asked for as well, because the staff may have
    // saved it under any name. When it is not wanted, the file that sits
    // next to the patient file is used.
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
