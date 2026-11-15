#include "MainWindow.h"

#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include <memory>
#include <vector>

#include "../utils/ClinicExceptions.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"

namespace scapms {
namespace gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    setupMenuBar();

    loadSampleData();
    refreshAll();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("Smart Clinic Appointment & Patient Management System");
    setWindowIcon(QIcon(":/clinic.ico"));

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    patientPanel = new PatientPanel(manager, this);
    appointmentPanel = new AppointmentPanel(manager, this);
    queuePanel = new QueuePanel(manager, this);
    searchPanel = new SearchPanel(manager, this);
    reportPanel = new ReportPanel(manager, this);
    adminPanel = new AdminPanel(manager, this);

    tabWidget->addTab(patientPanel, "Patient Records");
    tabWidget->addTab(appointmentPanel, "Appointments");
    tabWidget->addTab(queuePanel, "Waiting Queue");
    tabWidget->addTab(searchPanel, "Search && Sort");
    tabWidget->addTab(reportPanel, "Clinic Reports");
    tabWidget->addTab(adminPanel, "Administration");

    connect(patientPanel, &PatientPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(appointmentPanel, &AppointmentPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(queuePanel, &QueuePanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(searchPanel, &SearchPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(reportPanel, &ReportPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(adminPanel, &AdminPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(patientPanel, &PatientPanel::undoRequested, this, &MainWindow::onUndoAction);

    // The window is big enough for the tables of the panels.
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenuBar() {
    QMenuBar* bar = menuBar();

    QMenu* fileMenu = bar->addMenu("&File");

    QAction* actNew = fileMenu->addAction("&New");
    actNew->setShortcut(QKeySequence("Ctrl+N"));
    connect(actNew, &QAction::triggered, this, [this]() { adminPanel->newDatabase(); });

    fileMenu->addSeparator();

    QAction* actSave = fileMenu->addAction("&Save as...");
    actSave->setShortcut(QKeySequence("Ctrl+S"));
    connect(actSave, &QAction::triggered, this, [this]() { adminPanel->saveData(); });

    QAction* actLoad = fileMenu->addAction("&Load...");
    actLoad->setShortcut(QKeySequence("Ctrl+O"));
    connect(actLoad, &QAction::triggered, this, [this]() { adminPanel->loadData(); });

    fileMenu->addSeparator();

    QAction* actExit = fileMenu->addAction("E&xit");
    actExit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(actExit, &QAction::triggered, this, &QWidget::close);

    QMenu* editMenu = bar->addMenu("&Edit");

    QAction* actUndo = editMenu->addAction("&Undo last operation");
    actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    connect(actUndo, &QAction::triggered, this, &MainWindow::onUndoAction);

    QAction* actSearch = editMenu->addAction("&Quick search");
    actSearch->setShortcut(QKeySequence("Ctrl+F"));
    connect(actSearch, &QAction::triggered, this, [this]() {
        tabWidget->setCurrentIndex(3);
        tabWidget->setFocus();
    });

    QMenu* helpMenu = bar->addMenu("&Help");

    QAction* actAbout = helpMenu->addAction("&About");
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::loadSampleData() {
    // The sample data is the starting point of the clinic and not a step of
    // the staff, so it is loaded without being remembered and the first
    // Ctrl+Z does not delete it.
    std::vector<std::unique_ptr<model::PatientRecord>> patients;
    patients.push_back(std::make_unique<model::GeneralPatient>(
        "GEN-101", "Amaka Okonkwo", 28, "+2348073451298", 4500.00));
    patients.push_back(std::make_unique<model::EmergencyPatient>(
        "EMG-999", "Daniel Ojukwu", 22, "+2348034874562", 1, "Acute trauma"));
    patients.push_back(std::make_unique<model::FollowUpPatient>(
        "FOL-205", "Babatunde Alabi", 45, "+2348067624513", "2026-10-20", "Dr. Funke"));
    patients.push_back(std::make_unique<model::GeneralPatient>(
        "GEN-005", "Chioma Eze", 19, "+2348091837465", 5000.00));
    patients.push_back(std::make_unique<model::EmergencyPatient>(
        "EMG-102", "Zainab Bello", 32, "+2348019283756", 2, "Fracture"));
    patients.push_back(std::make_unique<model::FollowUpPatient>(
        "FOL-206", "Ngozi Emeka", 61, "+2348075192648", "2026-11-02", "Dr. Okafor"));

    std::vector<std::string> warnings;
    manager.registerPatients(std::move(patients), &warnings);

    // A booking that the sample data cannot make is skipped instead of
    // stopping the program, so that the clinic always begins.
    try {
        manager.scheduleAppointment(model::Appointment("APT-1001", "GEN-101", "2026-11-15",
                                                       "10:30 AM", "Dr. Adeyemi",
                                                       model::Appointment::STATUS_PENDING),
                                    false);
        manager.scheduleAppointment(model::Appointment("APT-1002", "FOL-205", "2026-11-20",
                                                       "02:00 PM", "Dr. Funke",
                                                       model::Appointment::STATUS_COMPLETED),
                                    false);
        manager.scheduleAppointment(model::Appointment("APT-1003", "GEN-005", "2026-11-22",
                                                       "11:00 AM", "Dr. Adeyemi",
                                                       model::Appointment::STATUS_PENDING),
                                    false);
    } catch (const utils::ClinicException& error) {
        warnings.push_back(std::string("a sample appointment was skipped: ") + error.what());
    }

    if (!warnings.empty()) {
        showWarnings(warnings);
    }
}

void MainWindow::showTab(int index) {
    if (tabWidget == nullptr) {
        return;
    }
    // An impossible number leaves the panel where it is, rather than closing
    // the window or opening nothing at all.
    if (index >= 0 && index < tabWidget->count()) {
        tabWidget->setCurrentIndex(index);
    }
}

void MainWindow::refreshAll() {
    patientPanel->refresh();
    appointmentPanel->refresh();
    queuePanel->refresh();
    searchPanel->refresh();
    reportPanel->refresh();
    adminPanel->refresh();
}

void MainWindow::onDataChanged() {
    refreshAll();
}

void MainWindow::onUndoAction() {
    std::string undone;
    try {
        undone = manager.undoLastAction();
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Undo error", error.what());
        return;                                 // the clinic is unchanged
    }

    refreshAll();

    if (undone.empty()) {
        QMessageBox::information(this, "Undo", "There is nothing to undo.");
    } else {
        QMessageBox::information(this, "Undo",
                                 QString::fromStdString(undone) + " was undone.");
    }
}

void MainWindow::showWarnings(const std::vector<std::string>& warnings) {
    if (warnings.empty()) {
        return;
    }

    QString text;
    for (const std::string& warning : warnings) {
        text += "- " + QString::fromStdString(warning) + "\n";
    }

    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle("Some rows could not be read");
    box.setText("The file was loaded, but the rows below were skipped:");
    box.setInformativeText(text);
    box.exec();
}

void MainWindow::onAbout() {
    QMessageBox::about(
        this, "About SCAPMS",
        "<b>Smart Clinic Appointment & Patient Management System</b><br><br>"
        "COS202 Programming II - Lab Assessment<br>"
        "C++17 with the Qt 6 widget library.<br><br>"
        "The searching, sorting and undo routines are written by hand.");
}

} // namespace gui
} // namespace scapms