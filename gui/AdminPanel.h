#pragma once

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

#include "../controller/ClinicManager.h"

namespace scapms {
namespace gui {

// The administration panel. It holds the operations that belong to the
// staff in charge of the clinic rather than to the treatment of a patient:
// undoing the last step, saving and loading the data, and starting a new
// day with an empty database. It also shows a small summary of the clinic
// and the records that were used most recently.
class AdminPanel : public QWidget {
    Q_OBJECT

public:
    explicit AdminPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refresh();

    // The administrative steps. The window connects its menu entries to
    // these, so that the menu and the panel do the same thing.
    void undoLastOperation();
    void saveData();
    void loadData();
    void newDatabase();

signals:
    void dataChanged();

private slots:
    void onUndoClicked();
    void onSaveClicked();
    void onLoadClicked();
    void onNewClicked();

private:
    controller::ClinicManager& manager;

    QPushButton* btnUndo;
    QPushButton* btnSave;
    QPushButton* btnLoad;
    QPushButton* btnNew;

    // The numbers of the clinic summary. Every one of them is a plain
    // label that is filled in when the panel is redrawn.
    QLabel* lblPatientCount;
    QLabel* lblWaitingCount;
    QLabel* lblAppointmentCount;
    QLabel* lblPendingCount;

    QListWidget* listRecent;
};

} // namespace gui
} // namespace scapms