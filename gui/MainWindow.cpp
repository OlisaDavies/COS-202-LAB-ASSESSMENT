#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScreen>
#include <QShowEvent>
#include <QStatusBar>

#include <memory>
#include <vector>

#include "../utils/ClinicExceptions.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"

namespace scapms {
namespace gui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), centred(false) {
    applyStyleSheet();
    setupUI();
    setupMenuBar();
    setupTimer();

    loadSampleData();
    refreshAll();
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    // The window is put in the middle of the screen once Windows has placed
    // it, so that it never hangs over the edge of the display.
    if (!centred) {
        centred = true;
        QTimer::singleShot(300, this, [this]() {
            if (QScreen* screen = QGuiApplication::primaryScreen()) {
                const QRect available = screen->availableGeometry();
                move(available.center() - QPoint(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2));
            }
        });
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::applyStyleSheet() {
    // The look and feel is kept in a resource file, so it is always the same.
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }
}

void MainWindow::setupUI() {
    setWindowTitle("Smart Clinic Appointment & Patient Management System");

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

    // Every panel tells the window when it changed the data.
    connect(patientPanel, &PatientPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(appointmentPanel, &AppointmentPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(queuePanel, &QueuePanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(searchPanel, &SearchPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(reportPanel, &ReportPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(adminPanel, &AdminPanel::dataChanged, this, &MainWindow::onDataChanged);
    connect(patientPanel, &PatientPanel::undoRequested, this, &MainWindow::onUndoAction);

    // The window is sized after the panels are in place, so that it is big
    // enough for the tables.
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);

    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenuBar() {
    QMenuBar* bar = menuBar();

    QMenu* fileMenu = bar->addMenu("&File");

    QAction* actSave = fileMenu->addAction("&Save as...");
    actSave->setShortcut(QKeySequence("Ctrl+S"));
    actSave->setToolTip("Write the patients and the appointments to CSV files.");
    connect(actSave, &QAction::triggered, this, [this]() { adminPanel->saveData(); });

    QAction* actLoad = fileMenu->addAction("&Load...");
    actLoad->setShortcut(QKeySequence("Ctrl+O"));
    actLoad->setToolTip("Read the patients and the appointments from CSV files.");
    connect(actLoad, &QAction::triggered, this, [this]() { adminPanel->loadData(); });

    fileMenu->addSeparator();

    QAction* actExit = fileMenu->addAction("E&xit");
    actExit->setShortcut(QKeySequence("Ctrl+Q"));
    actExit->setToolTip("Close the application.");
    connect(actExit, &QAction::triggered, this, &QWidget::close);

    QMenu* editMenu = bar->addMenu("&Edit");

    QAction* actUndo = editMenu->addAction("&Undo last operation");
    actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    actUndo->setToolTip("Reverse the last change to a patient or an appointment.");
    connect(actUndo, &QAction::triggered, this, &MainWindow::onUndoAction);

    QAction* actSearch = editMenu->addAction("&Quick search");
    actSearch->setShortcut(QKeySequence("Ctrl+F"));
    actSearch->setToolTip("Go to the search and sort panel.");
    connect(actSearch, &QAction::triggered, this, [this]() {
        tabWidget->setCurrentIndex(3);
        tabWidget->setFocus();
    });

    QMenu* helpMenu = bar->addMenu("&Help");

    QAction* actAbout = helpMenu->addAction("&About");
    actAbout->setToolTip("Show the information about this program.");
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupTimer() {
    dashboardTimer = new QTimer(this);
    connect(dashboardTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    dashboardTimer->start(1000);
}

void MainWindow::onTimerTick() {
    const QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    statusBar()->showMessage(QString("%1   |   Patients: %2   |   Waiting: %3")
                             .arg(time)
                             .arg(manager.getPatientCount())
                             .arg(manager.getQueueSize()));
}

void MainWindow::loadSampleData() {
    manager.registerPatient(
        std::make_unique<model::GeneralPatient>("GEN-101", "Amaka Okonkwo", 28,
                                                "+2348073451298", 4500.00));
    manager.registerPatient(
        std::make_unique<model::EmergencyPatient>("EMG-999", "Davies Johnson", 22,
                                                  "+2348034874562", 1, "Acute trauma"));
    manager.registerPatient(
        std::make_unique<model::FollowUpPatient>("FOL-205", "Babatunde Alabi", 45,
                                                 "+2348067624513",
                                                 "2026-10-20", "Dr. Funke"));
    manager.registerPatient(
        std::make_unique<model::GeneralPatient>("GEN-005", "Chioma Eze", 19,
                                                "+2348091837465", 5000.00));
    manager.registerPatient(
        std::make_unique<model::EmergencyPatient>("EMG-102", "Zainab Bello", 32,
                                                  "+2348019283756", 2, "Fracture"));
    manager.registerPatient(
        std::make_unique<model::FollowUpPatient>("FOL-206", "Ngozi Emeka", 61,
                                                 "+2348075192648",
                                                 "2026-11-02", "Dr. Okafor"));

    manager.scheduleAppointment(model::Appointment("APT-1001", "GEN-101", "2026-11-15",
                                                   "10:30 AM", "Dr. Adeyemi",
                                                   model::Appointment::STATUS_PENDING));
    manager.scheduleAppointment(model::Appointment("APT-1002", "FOL-205", "2026-11-20",
                                                   "02:00 PM", "Dr. Funke",
                                                   model::Appointment::STATUS_COMPLETED));
    manager.scheduleAppointment(model::Appointment("APT-1003", "GEN-005", "2026-11-22",
                                                   "11:00 AM", "Dr. Adeyemi",
                                                   model::Appointment::STATUS_PENDING));
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
    const std::string undone = manager.undoLastAction();

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
