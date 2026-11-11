#include "AppointmentPanel.h"

#include <QColor>
#include <QDate>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

#include "../model/Appointment.h"
#include "../model/Appointable.h"
#include "../utils/ClinicExceptions.h"
#include "../utils/DateUtil.h"

namespace scapms {
namespace gui {

AppointmentPanel::AppointmentPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager), aptIdEdited(false) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    layout->addWidget(buildForm(this));
    layout->addWidget(buildSchedule(this));

    refresh();
}

QWidget* AppointmentPanel::buildForm(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Schedule New Appointment", parent);
    QFormLayout* form = new QFormLayout(group);
    form->setContentsMargins(16, 20, 16, 16);
    form->setVerticalSpacing(12);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    txtAptID = new QLineEdit(group);
    txtAptID->setToolTip("The identifier of the booking. A free one is filled in for you.");

    cmbAptPatientID = new QComboBox(group);
    cmbAptPatientID->setToolTip("The patient that the appointment is for.");

    txtAptDate = new QLineEdit("2026-11-25", group);
    txtAptDate->setPlaceholderText("YYYY-MM-DD");

    txtAptTime = new QLineEdit("10:00 AM", group);
    txtAptTime->setPlaceholderText("HH:MM AM / PM");

    txtAptDoctor = new QLineEdit("Dr. Adeyemi", group);

    // The inputs keep a fixed width, so that the form never squeezes the
    // schedule table on a small screen.
    const int inputWidth = 190;
    for (QLineEdit* input : {txtAptID, txtAptDate, txtAptTime, txtAptDoctor}) {
        input->setMaximumWidth(inputWidth);
    }
    cmbAptPatientID->setMaximumWidth(inputWidth);

    form->addRow("Appointment ID:", txtAptID);
    form->addRow("Patient:", cmbAptPatientID);
    form->addRow("Date:", txtAptDate);
    form->addRow("Time:", txtAptTime);
    form->addRow("Doctor:", txtAptDoctor);

    // A free identifier is filled in, unless the staff types one of their own.
    connect(txtAptID, &QLineEdit::textEdited, this, [this]() { aptIdEdited = true; });

    QPushButton* btnBook = new QPushButton("Book Appointment", group);
    btnBook->setToolTip("Store the booking in the schedule.");
    connect(btnBook, &QPushButton::clicked, this, &AppointmentPanel::onScheduleAppointment);
    form->addRow(btnBook);

    // The form keeps a fixed width, so that the schedule table always has
    // room on a small screen.
    group->setFixedWidth(360);
    group->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    return group;
}

QWidget* AppointmentPanel::buildSchedule(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Scheduled Appointments", parent);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(16, 20, 16, 16);
    layout->setSpacing(10);

    tableAppointments = new QTableWidget(group);
    tableAppointments->setColumnCount(6);
    tableAppointments->setHorizontalHeaderLabels(
        {"ID", "Patient ID", "Date", "Time", "Doctor", "Status"});

    // The column widths are set by hand, so that the table never asks for
    // more room than the window can give.
    QHeaderView* header = tableAppointments->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setMinimumSectionSize(45);
    header->resizeSection(0, 80);    // ID
    header->resizeSection(1, 90);    // Patient ID
    header->resizeSection(2, 90);    // Date
    header->resizeSection(3, 80);    // Time
    header->resizeSection(4, 90);    // Doctor
    header->setStretchLastSection(true);
    tableAppointments->verticalHeader()->setVisible(false);
    tableAppointments->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableAppointments->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(tableAppointments);

    QHBoxLayout* buttonRow = new QHBoxLayout();
    QPushButton* btnCancel = new QPushButton("Cancel Appointment", group);
    btnCancel->setToolTip("Cancel the selected booking. The step can be undone.");
    connect(btnCancel, &QPushButton::clicked,
            this, &AppointmentPanel::onCancelSelectedAppointment);
    buttonRow->addStretch();
    buttonRow->addWidget(btnCancel);
    layout->addLayout(buttonRow);

    return group;
}

QString AppointmentPanel::nextAppointmentId() const {
    // The controller knows which identifiers are already in use.
    return QString::fromStdString(manager.appointments().suggestAppointmentId());
}

void AppointmentPanel::refreshPatientCombo() {
    const QString previous = cmbAptPatientID->currentData().toString();

    cmbAptPatientID->clear();
    for (const model::PatientRecord* patient : manager.getAllPatients()) {
        // Only the categories that answer the Appointable interface can be
        // booked. An emergency case is seen in the waiting room instead.
        if (dynamic_cast<const model::Appointable*>(patient) == nullptr) {
            continue;
        }

        const QString entry = QString::fromStdString(patient->getPatientID() + " - " +
                                                      patient->getName()) +
                               " (" + QString::fromStdString(patient->getTypeLabel()) + ")";
        cmbAptPatientID->addItem(entry, QString::fromStdString(patient->getPatientID()));
    }

    const int index = cmbAptPatientID->findData(previous);
    if (index >= 0) {
        cmbAptPatientID->setCurrentIndex(index);
    }
}

void AppointmentPanel::onScheduleAppointment() {
    if (cmbAptPatientID->count() == 0) {
        QMessageBox::warning(this, "No patients",
                             "Please register a patient before booking an appointment.");
        return;
    }

    const QString appointmentID = txtAptID->text().trimmed();
    const QString patientID = cmbAptPatientID->currentData().toString();
    const QString date = txtAptDate->text().trimmed();
    const QString time = txtAptTime->text().trimmed();
    const QString doctor = txtAptDoctor->text().trimmed();

    if (appointmentID.isEmpty() || patientID.isEmpty() || date.isEmpty() || time.isEmpty()) {
        QMessageBox::warning(this, "Invalid input",
                             "The appointment ID, the patient, the date and the time are required.");
        return;
    }

    if (!utils::isValidDate(date.toStdString())) {
        QMessageBox::warning(this, "Invalid input",
                             "The date must be written as YYYY-MM-DD, for example 2026-11-25.");
        return;
    }

    if (utils::timeToMinutes(time.toStdString()) < 0) {
        QMessageBox::warning(this, "Invalid input",
                             "The time must be written as HH:MM AM or HH:MM PM.");
        return;
    }

    try {
        const model::Appointment appointment(appointmentID.toStdString(), patientID.toStdString(),
                                              date.toStdString(), time.toStdString(),
                                              doctor.toStdString(),
                                              model::Appointment::STATUS_PENDING);
        manager.scheduleAppointment(appointment);
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Booking error", error.what());
        return;
    }

    txtAptID->setText(nextAppointmentId());
    aptIdEdited = false;
    refresh();
    emit dataChanged();

    QMessageBox::information(this, "Booked",
                             appointmentID + " was booked for " + patientID + ".");
}

void AppointmentPanel::onCancelSelectedAppointment() {
    const int row = tableAppointments->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No selection", "Please select an appointment row first.");
        return;
    }

    const QString appointmentID = tableAppointments->item(row, 0)->text();

    if (!manager.cancelAppointment(appointmentID.toStdString())) {
        QMessageBox::warning(this, "Not found",
                             "That appointment is no longer scheduled.");
        return;
    }

    refresh();
    emit dataChanged();

    QMessageBox::information(this, "Cancelled", appointmentID + " was cancelled.");
}

void AppointmentPanel::refresh() {
    refreshPatientCombo();

    // A free identifier is offered as long as the staff did not type one.
    if (!aptIdEdited) {
        txtAptID->setText(nextAppointmentId());
    }

    const std::vector<model::Appointment> appointments = manager.getAllAppointments();
    tableAppointments->setRowCount(static_cast<int>(appointments.size()));

    for (int i = 0; i < static_cast<int>(appointments.size()); ++i) {
        const model::Appointment& appointment = appointments[i];

        tableAppointments->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(appointment.getAppointmentID())));
        tableAppointments->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(appointment.getPatientID())));
        tableAppointments->setItem(i, 2, new QTableWidgetItem(
            QString::fromStdString(appointment.getDate())));
        tableAppointments->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(appointment.getTime())));
        tableAppointments->setItem(i, 4, new QTableWidgetItem(
            QString::fromStdString(appointment.getDoctorName())));

        QTableWidgetItem* status = new QTableWidgetItem(
            QString::fromStdString(appointment.getStatus()));
        tableAppointments->setItem(i, 5, status);

        // The colour of the status cell shows at a glance what happened.
        if (appointment.getStatus() == model::Appointment::STATUS_COMPLETED) {
            status->setBackground(QColor("#e9f3ed"));
            status->setForeground(QColor("#2c7a52"));
        } else if (appointment.getStatus() == model::Appointment::STATUS_CANCELLED) {
            status->setBackground(QColor("#f9ebeb"));
            status->setForeground(QColor("#ad3838"));
        } else {
            status->setBackground(QColor("#fbf3e3"));
            status->setForeground(QColor("#96701d"));
        }
    }
}

} // namespace gui
} // namespace scapms
