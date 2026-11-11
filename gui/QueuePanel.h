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

    // Redraws the queue list and the patient list of the form.
    void refresh();

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
