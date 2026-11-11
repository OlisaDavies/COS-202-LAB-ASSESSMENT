#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include "../controller/ClinicManager.h"

namespace scapms {
namespace gui {

// The appointment panel. It holds the booking form and the schedule table.
class AppointmentPanel : public QWidget {
    Q_OBJECT

public:
    explicit AppointmentPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refresh();

signals:
    void dataChanged();

private slots:
    void onScheduleAppointment();
    void onCancelSelectedAppointment();
    void onCompleteSelectedAppointment();

private:
    controller::ClinicManager& manager;

    QLineEdit* txtAptID;
    QComboBox* cmbAptPatientID;
    QLineEdit* txtAptDate;
    QLineEdit* txtAptTime;
    QLineEdit* txtAptDoctor;
    QTableWidget* tableAppointments;
    QPushButton* btnComplete;

    // True when the clinic staff has typed an appointment ID of their own.
    bool aptIdEdited;

    QWidget* buildForm(QWidget* parent);
    QWidget* buildSchedule(QWidget* parent);

    // The next free appointment identifier.
    QString nextAppointmentId() const;

    void refreshPatientCombo();
};

} // namespace gui
} // namespace scapms
