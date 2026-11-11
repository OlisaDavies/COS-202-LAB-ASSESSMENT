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
// day with an empty database.
//
// The panel also shows the dashboard of the clinic and the records that were
// used most recently.
class AdminPanel : public QWidget {
    Q_OBJECT

public:
    explicit AdminPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refresh();

    // Redraws only the statistics of the dashboard. The timer of the window
    // calls this, so that the numbers of the clinic are always the true
    // ones even when nothing was changed.
    void refreshStatistics();

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

    // The numbers of the dashboard. Every one of them is a label, so the
    // dashboard is built out of real widgets and not out of one block of
    // text.
    QLabel* lblPatientCount;
    QLabel* lblWaitingCount;
    QLabel* lblBookedCount;
    QLabel* lblCompletedCount;
    QLabel* lblPendingCount;
    QLabel* lblCancelledCount;

    QListWidget* listRecent;

    // A small label in the colour of one category, used by the table of the
    // categories and their priorities.
    static QLabel* buildCategoryChip(const QString& text, const QString& background,
                                      const QString& foreground, QWidget* parent);
};

} // namespace gui
} // namespace scapms
