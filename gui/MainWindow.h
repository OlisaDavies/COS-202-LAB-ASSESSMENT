#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>

#include "PatientPanel.h"
#include "AppointmentPanel.h"
#include "QueuePanel.h"
#include "SearchPanel.h"
#include "ReportPanel.h"
#include "AdminPanel.h"
#include "../controller/ClinicManager.h"

namespace scapms {
namespace gui {

// The main window of the clinic application.
//
// It owns the controller and the panels, puts the panels into the tabs and
// keeps them in step with each other. The panels themselves do the work, so
// this class stays small.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // Opens the given tab, counted from zero, and does nothing when the
    // number is out of range. The command line uses this, so that any panel
    // can be opened, photographed or demonstrated directly.
    void showTab(int index);

    // The two panels the command line also fills in, so that the entry point
    // does not have to reach into the widgets of a panel itself.
    QueuePanel* queuePanelWidget() { return queuePanel; }
    SearchPanel* searchPanelWidget() { return searchPanel; }

protected:
    // Used to put the window in the middle of the screen once it is shown.
    void showEvent(QShowEvent* event) override;

private slots:
    // The clock in the status bar.
    void onTimerTick();

    // Called when a panel changed the data.
    void onDataChanged();

    // The undo of the last administrative operation.
    void onUndoAction();

    void onAbout();

private:
    controller::ClinicManager manager;

    QTabWidget* tabWidget;
    QTimer* dashboardTimer;

    // True once the window has been put in the middle of the screen.
    bool centred;

    // The size the window is asked for. The width leaves room for the
    // roster table, whose name and details columns both hold a long text.
    static constexpr int WINDOW_WIDTH = 1200;
    static constexpr int WINDOW_HEIGHT = 660;

    PatientPanel* patientPanel;
    AppointmentPanel* appointmentPanel;
    QueuePanel* queuePanel;
    SearchPanel* searchPanel;
    ReportPanel* reportPanel;
    AdminPanel* adminPanel;

    void setupUI();
    void setupMenuBar();
    void setupTimer();
    void applyStyleSheet();

    // The records the application starts with.
    void loadSampleData();

    // Redraws every panel.
    void refreshAll();

    // Shows the rows that could not be read from a file.
    void showWarnings(const std::vector<std::string>& warnings);
};

} // namespace gui
} // namespace scapms
