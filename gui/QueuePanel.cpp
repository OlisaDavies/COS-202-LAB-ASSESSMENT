#include "QueuePanel.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include "../utils/ClinicExceptions.h"

namespace scapms {
namespace gui {

QueuePanel::QueuePanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    layout->addWidget(buildControls(this));
    layout->addWidget(buildList(this));

    refresh();
}

QWidget* QueuePanel::buildControls(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Waiting room", parent);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(16, 20, 16, 16);
    layout->setSpacing(12);

    layout->addWidget(new QLabel("Patient:", group));

    cmbQueuePatientID = new QComboBox(group);
    cmbQueuePatientID->setToolTip("The patient that should join the queue.");
    cmbQueuePatientID->setMaximumWidth(190);
    layout->addWidget(cmbQueuePatientID);

    QPushButton* btnEnqueue = new QPushButton("Add to Queue", group);
    btnEnqueue->setToolTip("Put the patient at the end of the waiting room.");
    connect(btnEnqueue, &QPushButton::clicked, this, &QueuePanel::onEnqueuePatient);
    layout->addWidget(btnEnqueue);

    layout->addSpacing(20);

    QPushButton* btnCallNext = new QPushButton("Call Next", group);
    btnCallNext->setToolTip("Call the patient that has been waiting the longest.");
    connect(btnCallNext, &QPushButton::clicked, this, &QueuePanel::onCallNextPatient);
    layout->addWidget(btnCallNext);

    lblQueueCount = new QLabel("<b>Waiting: 0</b>", group);
    layout->addWidget(lblQueueCount);
    layout->addStretch();

    // The controls keep a fixed width, so that the queue list always has room.
    group->setFixedWidth(360);
    group->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    return group;
}

QWidget* QueuePanel::buildList(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Queue", parent);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(16, 20, 16, 16);

    listQueueView = new QListWidget(group);
    listQueueView->setToolTip("The patients that are waiting, the longest wait first.");
    layout->addWidget(listQueueView);

    return group;
}

void QueuePanel::refreshPatientCombo() {
    const QString previous = cmbQueuePatientID->currentData().toString();

    cmbQueuePatientID->clear();
    for (const model::PatientRecord* patient : manager.getAllPatients()) {
        const QString entry = QString::fromStdString(patient->getPatientID() + " - " +
                                                      patient->getName()) +
                               " (" + QString::fromStdString(patient->getTypeLabel()) + ")";
        cmbQueuePatientID->addItem(entry, QString::fromStdString(patient->getPatientID()));
    }

    const int index = cmbQueuePatientID->findData(previous);
    if (index >= 0) {
        cmbQueuePatientID->setCurrentIndex(index);
    }
}

void QueuePanel::onEnqueuePatient() {
    if (cmbQueuePatientID->count() == 0) {
        QMessageBox::warning(this, "No patients",
                             "Please register a patient before adding one to the queue.");
        return;
    }

    const QString patientID = cmbQueuePatientID->currentData().toString();
    model::PatientRecord* patient = manager.findPatientByID(patientID.toStdString());
    if (patient == nullptr) {
        QMessageBox::warning(this, "Not found", "That patient is no longer in the clinic.");
        return;
    }

    try {
        manager.enqueuePatient(patient);
    } catch (const utils::ClinicException& error) {
        QMessageBox::warning(this, "Cannot add to the queue", error.what());
        return;
    }

    refresh();
    emit dataChanged();
}

void QueuePanel::onCallNextPatient() {
    model::PatientRecord* next = manager.callNextPatient();
    if (next == nullptr) {
        refresh();
        QMessageBox::information(this, "Queue empty", "There are no patients waiting.");
        return;
    }

    refresh();
    emit dataChanged();

    QMessageBox::information(this, "Next patient",
                             "<b>" + QString::fromStdString(next->getName()) + "</b><br>" +
                             "ID: " + QString::fromStdString(next->getPatientID()) + "<br>" +
                             "Category: " + QString::fromStdString(next->getTypeLabel()) + "<br>" +
                             "Details: " + QString::fromStdString(next->getSpecialDetail()));
}

void QueuePanel::refresh() {
    refreshPatientCombo();

    listQueueView->clear();
    for (const model::PatientRecord* patient : manager.getWaitingQueue()) {
        listQueueView->addItem(QString::fromStdString(patient->getPatientID() + " - " +
                                                      patient->getName()) +
                               " (" + QString::fromStdString(patient->getTypeLabel()) + ")");
    }

    lblQueueCount->setText("<b>Waiting: " + QString::number(manager.getQueueSize()) + "</b>");
}

} // namespace gui
} // namespace scapms
