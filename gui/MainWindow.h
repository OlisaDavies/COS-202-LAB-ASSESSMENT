#pragma once

#include <QMainWindow>
#include <QTabWidget>

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
// It owns the controller and the panels, and keeps the panels in step with each
// other. The panels themselves do the work.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // Opens the given tab, counted from zero. The command line uses this.
    void showTab(int index);

    // The two panels the command line also fills in.
    QueuePanel* queuePanelWidget() { return queuePanel; }
    SearchPanel* searchPanelWidget() { return searchPanel; }

private slots:
    void onDataChanged();

    // The undo of the last administrative operation.
    void onUndoAction();

    void onAbout();

private:
    controller::ClinicManager manager;

    QTabWidget* tabWidget;

    // The size the window is asked for.
    static constexpr int WINDOW_WIDTH = 1100;
    static constexpr int WINDOW_HEIGHT = 620;

    PatientPanel* patientPanel;
    AppointmentPanel* appointmentPanel;
    QueuePanel* queuePanel;
    SearchPanel* searchPanel;
    ReportPanel* reportPanel;
    AdminPanel* adminPanel;

    void setupUI();
    void setupMenuBar();

    // The records the application starts with.
    void loadSampleData();

    // Redraws every panel.
    void refreshAll();

    // Shows the rows that could not be read from a file.
    void showWarnings(const std::vector<std::string>& warnings);
};

} // namespace gui
} // namespace scapms