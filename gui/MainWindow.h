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

    // The size the window is asked for.
    static constexpr int WINDOW_WIDTH = 1080;
    static constexpr int WINDOW_HEIGHT = 640;

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
