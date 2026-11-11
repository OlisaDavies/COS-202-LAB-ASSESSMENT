#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

#include "../controller/ClinicManager.h"

namespace scapms {
namespace gui {

// The waiting room panel. The queue is first in, first out, so the patient
// that joined first is the one that is called first.
class QueuePanel : public QWidget {
    Q_OBJECT

public:
    explicit QueuePanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refresh();

    // Puts the given patient into the waiting room and redraws the panel.
    // Returns false when that patient is not registered or is already waiting.
    // The command line uses this to open the panel with a patient in it.
    bool enqueueByID(const QString& patientID);

signals:
    void dataChanged();

private slots:
    void onEnqueuePatient();
    void onCallNextPatient();

private:
    controller::ClinicManager& manager;

    QComboBox* cmbQueuePatientID;
    QListWidget* listQueueView;
    QLabel* lblQueueCount;

    QWidget* buildControls(QWidget* parent);
    QWidget* buildList(QWidget* parent);

    void refreshPatientCombo();
};

} // namespace gui
} // namespace scapms
