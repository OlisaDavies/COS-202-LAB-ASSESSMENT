#include "MainWindow.h"
#include <QGroupBox>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <map>
#include <vector>
#include <string>
#include "Appointable.h"
#include "GeneralPatient.h"
#include "EmergencyPatient.h"
#include "FollowUpPatient.h"
#include "SearchEngine.h"
#include "SortEngine.h"
#include "FileHandler.h"

static QString typeLabel(const std::string& type) {
    if (type == "Emergency") return "Emergency Patient";
    if (type == "FollowUp") return "Follow-Up Patient";
    return "General Outpatient";
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    editSnapshot = nullptr;
    nextAppointmentNumber = 1005;

    setupUI();
    setupMenuBar();
    setupTimer();

    loadSampleData();

    refreshPatientTable();
    refreshAppointmentTable();
    refreshPatientDropdowns();
    onGenerateSelectedReport();
}

MainWindow::~MainWindow() {
    delete editSnapshot;
}

void MainWindow::loadSampleData() {
    database.addPatient(new GeneralPatient("GEN-101", "Amaka Okonkwo", 28, "+2348011112222", 45.00));
    database.addPatient(new EmergencyPatient("EMG-999", "Davies Johnson", 22, "+2348034874562", 1, "Acute trauma"));
    database.addPatient(new FollowUpPatient("FOL-205", "Babatunde Alabi", 45, "+2348099998888", "2026-10-20", "Dr. Funke"));
    database.addPatient(new GeneralPatient("GEN-005", "Chioma Eze", 19, "+2348022223333", 50.00));
    database.addPatient(new EmergencyPatient("EMG-102", "Zainab Bello", 32, "+2348055556666", 2, "Fracture"));

    database.addAppointment(Appointment("APT-1001", "GEN-101", "2026-11-15", "10:30 AM", "Dr. Adeyemi", "Pending"));
    database.addAppointment(Appointment("APT-1002", "FOL-205", "2026-11-20", "02:00 PM", "Dr. Funke", "Completed"));
    database.addAppointment(Appointment("APT-1003", "GEN-005", "2026-11-22", "11:00 AM", "Dr. Adeyemi", "Pending"));
    database.addAppointment(Appointment("APT-1004", "EMG-999", "2026-11-10", "01:15 PM", "Dr. Trauma", "Completed"));
}

void MainWindow::setupUI() {
    setWindowTitle("Smart Clinic Appointment & Patient Management System");
    resize(1120, 740);

    setStyleSheet(
        "QMainWindow { background-color: #f8fafc; font-family: 'Segoe UI', Arial, sans-serif; }"
        "QTabWidget::pane { border: 1px solid #e2e8f0; background: #ffffff; border-radius: 8px; margin-top: -1px; }"
        "QTabBar::tab { background: #edf2f7; color: #4a5568; font-weight: 600; padding: 10px 20px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 4px; }"
        "QTabBar::tab:selected { background: #ffffff; color: #2b6cb0; border-top: 3px solid #3182ce; border-bottom: 1px solid #ffffff; }"
        "QTabBar::tab:hover:!selected { background: #e2e8f0; color: #2d3748; }"
        "QGroupBox { font-weight: 600; color: #2d3748; border: 1px solid #e2e8f0; border-radius: 8px; margin-top: 18px; padding-top: 14px; background: #ffffff; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 14px; padding: 0 6px; color: #2b6cb0; }"
        "QLineEdit, QComboBox { background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px; padding: 7px 10px; font-size: 13px; color: #1e293b; selection-background-color: #3b82f6; }"
        "QLineEdit:focus, QComboBox:focus { border: 1.5px solid #3b82f6; background: #f8fafc; }"
        "QTableWidget { background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px; gridline-color: #f1f5f9; font-size: 13px; color: #334155; }"
        "QTableWidget::item { padding: 6px; border-bottom: 1px solid #f1f5f9; }"
        "QTableWidget::item:selected { background-color: #e0f2fe; color: #0369a1; }"
        "QHeaderView::section { background-color: #f8fafc; color: #475569; font-weight: 600; font-size: 12px; padding: 8px; border: none; border-bottom: 2px solid #e2e8f0; }"
        "QListWidget { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px; padding: 6px; font-size: 13px; color: #1e293b; }"
        "QListWidget::item { padding: 8px 12px; margin-bottom: 4px; border-radius: 4px; background: #f8fafc; border-left: 3px solid #3b82f6; }"
        "QStatusBar { background: #f1f5f9; color: #475569; font-weight: 500; font-size: 12px; border-top: 1px solid #e2e8f0; }"
        "QTextEdit { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px; padding: 12px; color: #1e293b; font-family: 'Consolas', monospace; }"
    );

    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    tabWidget->addTab(createPatientRecordsPanel(), "Patient Records");
    tabWidget->addTab(createAppointmentsPanel(), "Appointments");
    tabWidget->addTab(createWaitingQueuePanel(), "Waiting Queue");
    tabWidget->addTab(createSearchSortPanel(), "Search & Sort");
    tabWidget->addTab(createReportsPanel(), "Clinic Reports");

    statusBar()->showMessage("Ready");
}

void MainWindow::setupMenuBar() {
    QMenuBar* bar = menuBar();
    bar->setStyleSheet("QMenuBar { background: #ffffff; border-bottom: 1px solid #e2e8f0; } QMenuBar::item { padding: 6px 12px; } QMenuBar::item:selected { background: #f1f5f9; }");

    QMenu* fileMenu = bar->addMenu("&File");

    QAction* actSave = fileMenu->addAction("&Save As...");
    actSave->setShortcut(QKeySequence("Ctrl+S"));
    actSave->setToolTip("Save patients and appointments to CSV files");
    connect(actSave, &QAction::triggered, this, &MainWindow::onSaveData);

    QAction* actLoad = fileMenu->addAction("&Load...");
    actLoad->setShortcut(QKeySequence("Ctrl+O"));
    actLoad->setToolTip("Load patients and appointments from CSV files");
    connect(actLoad, &QAction::triggered, this, &MainWindow::onLoadData);

    fileMenu->addSeparator();

    QAction* actUndo = fileMenu->addAction("&Undo");
    actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    actUndo->setToolTip("Undo the last change");
    connect(actUndo, &QAction::triggered, this, &MainWindow::onUndoAction);

    fileMenu->addSeparator();

    QAction* actExit = fileMenu->addAction("E&xit");
    actExit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(actExit, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupTimer() {
    dashboardTimer = new QTimer(this);
    connect(dashboardTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    dashboardTimer->start(1000);
}

void MainWindow::onTimerTick() {
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    statusBar()->showMessage(QString("%1   |   Patients: %2   |   Waiting: %3")
        .arg(timeStr)
        .arg(database.getPatientCount())
        .arg(database.getQueueSize()));
}

QWidget* MainWindow::createPatientRecordsPanel() {
    QWidget* panel = new QWidget(this);
    QHBoxLayout* hSplit = new QHBoxLayout(panel);
    hSplit->setContentsMargins(16, 16, 16, 16);
    hSplit->setSpacing(16);

    hSplit->addWidget(createPatientFormBox(panel));
    hSplit->addWidget(createPatientTableBox(panel));

    return panel;
}

QWidget* MainWindow::createPatientFormBox(QWidget* parent) {
    QGroupBox* grpForm = new QGroupBox("Register New Patient", parent);
    QFormLayout* formLayout = new QFormLayout(grpForm);
    formLayout->setContentsMargins(16, 20, 16, 16);
    formLayout->setVerticalSpacing(12);

    txtPatID = new QLineEdit(grpForm);
    txtPatID->setPlaceholderText("e.g. GEN-201, EMG-801");

    txtPatName = new QLineEdit(grpForm);
    txtPatName->setPlaceholderText("Full Patient Name");

    txtPatAge = new QLineEdit(grpForm);
    txtPatAge->setPlaceholderText("Age (0-120)");

    txtPatContact = new QLineEdit(grpForm);
    txtPatContact->setPlaceholderText("+234...");

    cmbPatType = new QComboBox(grpForm);
    cmbPatType->addItem("General Outpatient");
    cmbPatType->addItem("Emergency Patient");
    cmbPatType->addItem("Follow-Up Patient");

    lblTypeField1 = new QLabel("Consultation Fee ($):", grpForm);
    txtTypeField1 = new QLineEdit("45.00", grpForm);

    lblTypeField2 = new QLabel("Doctor / Note:", grpForm);
    txtTypeField2 = new QLineEdit(grpForm);
    lblTypeField2->setVisible(false);
    txtTypeField2->setVisible(false);

    formLayout->addRow("Patient ID *:", txtPatID);
    formLayout->addRow("Full Name *:", txtPatName);
    formLayout->addRow("Age *:", txtPatAge);
    formLayout->addRow("Contact Phone:", txtPatContact);
    formLayout->addRow("Category:", cmbPatType);
    formLayout->addRow(lblTypeField1, txtTypeField1);
    formLayout->addRow(lblTypeField2, txtTypeField2);

    btnRegister = new QPushButton("Register Patient", grpForm);
    btnRegister->setStyleSheet("background-color: #0f766e; color: white; font-weight: 600; padding: 10px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnRegister, &QPushButton::clicked, this, &MainWindow::onRegisterPatient);

    btnSaveChanges = new QPushButton("Save Changes", grpForm);
    btnSaveChanges->setStyleSheet("background-color: #b45309; color: white; font-weight: 600; padding: 10px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnSaveChanges, &QPushButton::clicked, this, &MainWindow::onUpdatePatient);
    btnSaveChanges->setVisible(false);

    btnCancelEdit = new QPushButton("Cancel Edit", grpForm);
    btnCancelEdit->setStyleSheet("background-color: #f1f5f9; color: #475569; font-weight: 600; padding: 8px; border-radius: 6px; border: 1px solid #cbd5e1; font-size: 12px;");
    connect(btnCancelEdit, &QPushButton::clicked, this, &MainWindow::onCancelEdit);
    btnCancelEdit->setVisible(false);

    QPushButton* btnUndo = new QPushButton("Undo (Ctrl+Z)", grpForm);
    btnUndo->setStyleSheet("background-color: #f1f5f9; color: #475569; font-weight: 600; padding: 8px; border-radius: 6px; border: 1px solid #cbd5e1; font-size: 12px;");
    connect(btnUndo, &QPushButton::clicked, this, &MainWindow::onUndoAction);

    formLayout->addRow(btnRegister);
    formLayout->addRow(btnSaveChanges);
    formLayout->addRow(btnCancelEdit);
    formLayout->addRow(btnUndo);

    grpForm->setMaximumWidth(360);
    return grpForm;
}

QWidget* MainWindow::createPatientTableBox(QWidget* parent) {
    QGroupBox* grpTable = new QGroupBox("Registered Patients", parent);
    QVBoxLayout* tableVBox = new QVBoxLayout(grpTable);
    tableVBox->setContentsMargins(16, 20, 16, 16);
    tableVBox->setSpacing(10);

    QLabel* lblLegend = new QLabel(
        "<span style='color:#ad3838;'>&#9679; Emergency Patient</span> &nbsp;&nbsp; "
        "<span style='color:#2c7a52;'>&#9679; General Outpatient</span> &nbsp;&nbsp; "
        "<span style='color:#2a4d72;'>&#9679; Follow-Up Patient</span>", grpTable);
    tableVBox->addWidget(lblLegend);

    tablePatients = new QTableWidget(grpTable);
    tablePatients->setColumnCount(5);
    tablePatients->setHorizontalHeaderLabels({"ID", "Full Name", "Age", "Phone", "Details"});
    tablePatients->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablePatients->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablePatients->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableVBox->addWidget(tablePatients);

    QHBoxLayout* tblBtnRow = new QHBoxLayout();
    btnEdit = new QPushButton("Edit", grpTable);
    btnEdit->setStyleSheet("background-color: #fef3c7; color: #92400e; font-weight: 600; padding: 8px 16px; border-radius: 6px; border: 1px solid #fde68a;");
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEditSelectedPatient);

    QPushButton* btnDelete = new QPushButton("Delete", grpTable);
    btnDelete->setStyleSheet("background-color: #fee2e2; color: #991b1b; font-weight: 600; padding: 8px 16px; border-radius: 6px; border: 1px solid #fecaca;");
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDeleteSelectedPatient);

    tblBtnRow->addWidget(btnEdit);
    tblBtnRow->addStretch();
    tblBtnRow->addWidget(btnDelete);
    tableVBox->addLayout(tblBtnRow);

    return grpTable;
}

void MainWindow::onPatientTypeChanged(int index) {
    if (index == 0) {
        lblTypeField1->setText("Consultation Fee ($):");
        txtTypeField1->setText("45.00");
        lblTypeField2->setVisible(false);
        txtTypeField2->setVisible(false);
    } else if (index == 1) {
        lblTypeField1->setText("Severity (1-3):");
        txtTypeField1->setText("1");
        lblTypeField2->setText("Triage Note:");
        txtTypeField2->setText("Acute distress");
        lblTypeField2->setVisible(true);
        txtTypeField2->setVisible(true);
    } else {
        lblTypeField1->setText("Last Visit Date:");
        txtTypeField1->setText("2026-10-15");
        lblTypeField2->setText("Assigned Doctor:");
        txtTypeField2->setText("Dr. Adeyemi");
        lblTypeField2->setVisible(true);
        txtTypeField2->setVisible(true);
    }
}

void MainWindow::onRegisterPatient() {
    QString id = txtPatID->text().trimmed();
    QString name = txtPatName->text().trimmed();
    QString ageStr = txtPatAge->text().trimmed();
    QString contact = txtPatContact->text().trimmed();
    int typeIdx = cmbPatType->currentIndex();

    if (id.isEmpty() || name.isEmpty() || ageStr.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "ID, name and age are required.");
        return;
    }

    bool ageOk = false;
    int age = ageStr.toInt(&ageOk);
    if (!ageOk || age < 0 || age > 120) {
        QMessageBox::warning(this, "Invalid Input", "Age must be a whole number between 0 and 120.");
        return;
    }

    try {
        PatientRecord* newPatient = nullptr;
        if (typeIdx == 0) {
            double fee = txtTypeField1->text().toDouble();
            newPatient = new GeneralPatient(id.toStdString(), name.toStdString(), age, contact.toStdString(), fee);
        } else if (typeIdx == 1) {
            int severity = txtTypeField1->text().toInt();
            std::string note = txtTypeField2->text().toStdString();
            newPatient = new EmergencyPatient(id.toStdString(), name.toStdString(), age, contact.toStdString(), severity, note);
        } else {
            std::string lastDate = txtTypeField1->text().toStdString();
            std::string doctor = txtTypeField2->text().toStdString();
            newPatient = new FollowUpPatient(id.toStdString(), name.toStdString(), age, contact.toStdString(), lastDate, doctor);
        }

        database.addPatient(newPatient);
        refreshPatientTable();
        refreshPatientDropdowns();

        QMessageBox::information(this, "Registered", name + " added as " + id + ".");
    } catch (const ClinicException& e) {
        QMessageBox::critical(this, "Registration Error", e.what());
    }

    clearPatientForm();
}

void MainWindow::clearPatientForm() {
    txtPatID->clear();
    txtPatName->clear();
    txtPatAge->clear();
    txtPatContact->clear();

    cmbPatType->blockSignals(true);
    cmbPatType->setCurrentIndex(0);
    cmbPatType->blockSignals(false);

    onPatientTypeChanged(0);
}

void MainWindow::loadPatientIntoForm(const PatientRecord* patient) {
    txtPatID->setText(QString::fromStdString(patient->getPatientID()));
    txtPatName->setText(QString::fromStdString(patient->getName()));
    txtPatAge->setText(QString::number(patient->getAge()));
    txtPatContact->setText(QString::fromStdString(patient->getContactNumber()));

    int typeIndex = 0;
    if (patient->getPatientType() == "Emergency") typeIndex = 1;
    else if (patient->getPatientType() == "FollowUp") typeIndex = 2;

    cmbPatType->blockSignals(true);
    cmbPatType->setCurrentIndex(typeIndex);
    cmbPatType->blockSignals(false);

    onPatientTypeChanged(typeIndex);

    if (const GeneralPatient* g = dynamic_cast<const GeneralPatient*>(patient)) {
        txtTypeField1->setText(QString::number(g->getConsultationFee()));
        txtTypeField2->clear();
    } else if (const EmergencyPatient* e = dynamic_cast<const EmergencyPatient*>(patient)) {
        txtTypeField1->setText(QString::number(e->getSeverityLevel()));
        txtTypeField2->setText(QString::fromStdString(e->getTriageNote()));
    } else if (const FollowUpPatient* f = dynamic_cast<const FollowUpPatient*>(patient)) {
        txtTypeField1->setText(QString::fromStdString(f->getPreviousVisitDate()));
        txtTypeField2->setText(QString::fromStdString(f->getAssignedDoctor()));
    }
}

void MainWindow::onEditSelectedPatient() {
    int row = tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Select a patient row first.");
        return;
    }

    QString id = tablePatients->item(row, 0)->text();
    PatientRecord* patient = database.findPatientByID(id.toStdString());
    if (patient == nullptr) {
        QMessageBox::warning(this, "Not Found", "That patient is no longer in the database.");
        return;
    }

    delete editSnapshot;
    editSnapshot = patient->clone();
    editingPatientID = patient->getPatientID();

    loadPatientIntoForm(patient);

    btnRegister->setVisible(false);
    btnSaveChanges->setVisible(true);
    btnCancelEdit->setVisible(true);
    cmbPatType->setEnabled(false);
}

void MainWindow::onUpdatePatient() {
    if (editSnapshot == nullptr || editingPatientID.empty()) {
        return;
    }

    PatientRecord* patient = database.findPatientByID(editingPatientID);
    if (patient == nullptr) {
        QMessageBox::warning(this, "Not Found", "That patient is no longer in the database.");
        onCancelEdit();
        return;
    }

    QString name = txtPatName->text().trimmed();
    QString ageText = txtPatAge->text().trimmed();
    QString contact = txtPatContact->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Name is required.");
        return;
    }

    bool ageOk = false;
    int age = ageText.toInt(&ageOk);
    if (!ageOk || age < 0 || age > 120) {
        QMessageBox::warning(this, "Invalid Input", "Age must be a whole number between 0 and 120.");
        return;
    }

    if (GeneralPatient* g = dynamic_cast<GeneralPatient*>(patient)) {
        g->setName(name.toStdString());
        g->setAge(age);
        g->setContactNumber(contact.toStdString());
        g->setConsultationFee(txtTypeField1->text().toDouble());
    } else if (EmergencyPatient* e = dynamic_cast<EmergencyPatient*>(patient)) {
        e->setName(name.toStdString());
        e->setAge(age);
        e->setContactNumber(contact.toStdString());
        e->setSeverityLevel(txtTypeField1->text().toInt());
        e->setTriageNote(txtTypeField2->text().toStdString());
    } else if (FollowUpPatient* f = dynamic_cast<FollowUpPatient*>(patient)) {
        f->setName(name.toStdString());
        f->setAge(age);
        f->setContactNumber(contact.toStdString());
        f->setPreviousVisitDate(txtTypeField1->text().toStdString());
        f->setAssignedDoctor(txtTypeField2->text().toStdString());
    }

    database.recordPatientUpdate(editSnapshot);
    editSnapshot = nullptr;
    editingPatientID.clear();

    refreshPatientTable();
    refreshPatientDropdowns();
    clearPatientForm();

    btnRegister->setVisible(true);
    btnSaveChanges->setVisible(false);
    btnCancelEdit->setVisible(false);
    cmbPatType->setEnabled(true);

    QMessageBox::information(this, "Updated", "Patient record updated.");
}

void MainWindow::onCancelEdit() {
    delete editSnapshot;
    editSnapshot = nullptr;
    editingPatientID.clear();

    clearPatientForm();

    btnRegister->setVisible(true);
    btnSaveChanges->setVisible(false);
    btnCancelEdit->setVisible(false);
    cmbPatType->setEnabled(true);
}

void MainWindow::onDeleteSelectedPatient() {
    int row = tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Select a patient row first.");
        return;
    }

    QString id = tablePatients->item(row, 0)->text();
    QString name = tablePatients->item(row, 1)->text();

    int reply = QMessageBox::question(this, "Delete Patient",
                                      "Delete " + name + " (" + id + ")?\n\nThis can be undone with Ctrl+Z.",
                                      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (editingPatientID == id.toStdString()) {
            onCancelEdit();
        }
        database.removePatient(id.toStdString());
        refreshPatientTable();
        refreshPatientDropdowns();
    }
}

void MainWindow::applyCustomPatientRowFormatting(int row, const std::string& patientType) {
    QColor bgColor;
    QColor fgColor;

    if (patientType == "Emergency") {
        bgColor = QColor("#ffebee");
        fgColor = QColor("#b71c1c");
    } else if (patientType == "General") {
        bgColor = QColor("#e8f5e9");
        fgColor = QColor("#1b5e20");
    } else {
        bgColor = QColor("#e3f2fd");
        fgColor = QColor("#0d47a1");
    }

    for (int col = 0; col < tablePatients->columnCount(); ++col) {
        QTableWidgetItem* itm = tablePatients->item(row, col);
        if (itm) {
            itm->setBackground(bgColor);
            if (col == 4) {
                itm->setForeground(fgColor);
                QFont f = itm->font();
                f.setBold(true);
                itm->setFont(f);
            }
        }
    }
}

void MainWindow::refreshPatientTable() {
    const std::vector<PatientRecord*>& patients = database.getAllPatients();
    tablePatients->setRowCount(static_cast<int>(patients.size()));

    for (int i = 0; i < static_cast<int>(patients.size()); ++i) {
        const PatientRecord* p = patients[i];
        tablePatients->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(p->getPatientID())));
        tablePatients->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(p->getName())));
        tablePatients->setItem(i, 2, new QTableWidgetItem(QString::number(p->getAge())));
        tablePatients->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(p->getContactNumber())));

        QString detail = typeLabel(p->getPatientType()) + ": " + QString::fromStdString(p->getSpecialDetail());
        if (const Appointable* bookable = dynamic_cast<const Appointable*>(p)) {
            if (bookable->hasScheduledAppointment()) {
                detail += " | Appointment: " + QString::fromStdString(bookable->getAppointmentDetails());
            }
        }
        tablePatients->setItem(i, 4, new QTableWidgetItem(detail));

        applyCustomPatientRowFormatting(i, p->getPatientType());
    }
}

void MainWindow::refreshPatientDropdowns() {
    cmbAptPatientID->clear();
    cmbQueuePatientID->clear();

    const std::vector<PatientRecord*>& patients = database.getAllPatients();
    for (const PatientRecord* p : patients) {
        QString entry = QString::fromStdString(p->getPatientID() + " - " + p->getName())
                        + " (" + typeLabel(p->getPatientType()) + ")";
        cmbAptPatientID->addItem(entry, QString::fromStdString(p->getPatientID()));
        cmbQueuePatientID->addItem(entry, QString::fromStdString(p->getPatientID()));
    }
}

QWidget* MainWindow::createAppointmentsPanel() {
    QWidget* panel = new QWidget(this);
    QHBoxLayout* hSplit = new QHBoxLayout(panel);
    hSplit->setContentsMargins(16, 16, 16, 16);
    hSplit->setSpacing(16);

    QGroupBox* grpForm = new QGroupBox("Schedule New Appointment", panel);
    QFormLayout* form = new QFormLayout(grpForm);
    form->setContentsMargins(16, 20, 16, 16);
    form->setVerticalSpacing(12);

    txtAptID = new QLineEdit(generateAppointmentId(), grpForm);
    cmbAptPatientID = new QComboBox(grpForm);
    txtAptDate = new QLineEdit("2026-11-25", grpForm);
    txtAptTime = new QLineEdit("10:00 AM", grpForm);
    txtAptDoctor = new QLineEdit("Dr. Adeyemi", grpForm);

    form->addRow("Appointment ID:", txtAptID);
    form->addRow("Patient:", cmbAptPatientID);
    form->addRow("Date:", txtAptDate);
    form->addRow("Time:", txtAptTime);
    form->addRow("Doctor:", txtAptDoctor);

    QPushButton* btnBook = new QPushButton("Book Appointment", grpForm);
    btnBook->setStyleSheet("background-color: #35618f; color: white; font-weight: 600; padding: 10px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnBook, &QPushButton::clicked, this, &MainWindow::onScheduleAppointment);
    form->addRow(btnBook);

    grpForm->setMaximumWidth(360);
    hSplit->addWidget(grpForm);

    QGroupBox* grpTable = new QGroupBox("Scheduled Appointments", panel);
    QVBoxLayout* tableVBox = new QVBoxLayout(grpTable);
    tableVBox->setContentsMargins(16, 20, 16, 16);
    tableVBox->setSpacing(10);

    tableAppointments = new QTableWidget(grpTable);
    tableAppointments->setColumnCount(6);
    tableAppointments->setHorizontalHeaderLabels({"ID", "Patient ID", "Date", "Time", "Doctor", "Status"});
    tableAppointments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableAppointments->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableAppointments->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableVBox->addWidget(tableAppointments);

    QHBoxLayout* actRow = new QHBoxLayout();
    QPushButton* btnCancel = new QPushButton("Cancel Appointment", grpTable);
    btnCancel->setStyleSheet("background-color: #f9ebeb; color: #ad3838; font-weight: 600; padding: 8px 16px; border-radius: 6px; border: 1px solid #ecc6c6;");
    connect(btnCancel, &QPushButton::clicked, this, &MainWindow::onCancelSelectedAppointment);
    actRow->addStretch();
    actRow->addWidget(btnCancel);
    tableVBox->addLayout(actRow);

    hSplit->addWidget(grpTable);
    return panel;
}

QString MainWindow::generateAppointmentId() {
    for (int attempt = 0; attempt < 1000; ++attempt) {
        QString candidate = "APT-" + QString::number(nextAppointmentNumber++);
        bool taken = false;
        for (const Appointment& a : database.getAllAppointments()) {
            if (a.getAppointmentID() == candidate.toStdString()) {
                taken = true;
                break;
            }
        }
        if (!taken) {
            return candidate;
        }
    }
    return "APT-" + QString::number(nextAppointmentNumber++);
}

void MainWindow::onScheduleAppointment() {
    if (cmbAptPatientID->count() == 0) {
        QMessageBox::warning(this, "No Patients", "Register a patient before booking an appointment.");
        return;
    }

    QString aptID = txtAptID->text().trimmed();
    QString patID = cmbAptPatientID->currentData().toString();
    QString date = txtAptDate->text().trimmed();
    QString time = txtAptTime->text().trimmed();
    QString doc = txtAptDoctor->text().trimmed();

    try {
        Appointment apt(aptID.toStdString(), patID.toStdString(), date.toStdString(), time.toStdString(), doc.toStdString(), "Pending");
        database.addAppointment(apt);
    } catch (const ClinicException& e) {
        QMessageBox::critical(this, "Booking Error", e.what());
        return;
    }

    refreshAppointmentTable();
    refreshPatientTable();
    QMessageBox::information(this, "Booked", aptID + " booked for " + patID + ".");
    txtAptID->setText(generateAppointmentId());
}

void MainWindow::onCancelSelectedAppointment() {
    int row = tableAppointments->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No Selection", "Select an appointment row first.");
        return;
    }

    QString aptID = tableAppointments->item(row, 0)->text();
    QString patID = tableAppointments->item(row, 1)->text();

    if (!database.cancelAppointment(aptID.toStdString())) {
        QMessageBox::warning(this, "Not Found", "That appointment is no longer scheduled.");
        return;
    }

    refreshAppointmentTable();
    refreshPatientTable();
    QMessageBox::information(this, "Cancelled", aptID + " cancelled.");
}

void MainWindow::refreshAppointmentTable() {
    const std::vector<Appointment>& appointments = database.getAllAppointments();
    tableAppointments->setRowCount(static_cast<int>(appointments.size()));

    for (int i = 0; i < static_cast<int>(appointments.size()); ++i) {
        const Appointment& a = appointments[i];
        tableAppointments->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(a.getAppointmentID())));
        tableAppointments->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(a.getPatientID())));
        tableAppointments->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(a.getDate())));
        tableAppointments->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(a.getTime())));
        tableAppointments->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(a.getDoctorName())));
        tableAppointments->setItem(i, 5, new QTableWidgetItem(QString::fromStdString(a.getStatus())));

        if (a.getStatus() == "Completed") {
            tableAppointments->item(i, 5)->setBackground(QColor("#e9f3ed"));
            tableAppointments->item(i, 5)->setForeground(QColor("#2c7a52"));
        } else if (a.getStatus() == "Cancelled") {
            tableAppointments->item(i, 5)->setBackground(QColor("#f9ebeb"));
            tableAppointments->item(i, 5)->setForeground(QColor("#ad3838"));
        } else {
            tableAppointments->item(i, 5)->setBackground(QColor("#fbf3e3"));
            tableAppointments->item(i, 5)->setForeground(QColor("#96701d"));
        }
    }
}

QWidget* MainWindow::createWaitingQueuePanel() {
    QWidget* panel = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    QGroupBox* grpQueueCtrl = new QGroupBox("Waiting Room", panel);
    QVBoxLayout* vCtrl = new QVBoxLayout(grpQueueCtrl);
    vCtrl->setContentsMargins(16, 20, 16, 16);
    vCtrl->setSpacing(12);

    vCtrl->addWidget(new QLabel("Patient:"));
    cmbQueuePatientID = new QComboBox(grpQueueCtrl);
    vCtrl->addWidget(cmbQueuePatientID);

    QPushButton* btnEnqueue = new QPushButton("Add to Queue", grpQueueCtrl);
    btnEnqueue->setStyleSheet("background-color: #1d6a85; color: white; font-weight: 600; padding: 10px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnEnqueue, &QPushButton::clicked, this, &MainWindow::onEnqueuePatient);
    vCtrl->addWidget(btnEnqueue);

    vCtrl->addSpacing(20);

    QPushButton* btnCallNext = new QPushButton("Call Next", grpQueueCtrl);
    btnCallNext->setStyleSheet("background-color: #2a4d72; color: white; font-weight: 700; padding: 14px; border-radius: 8px; font-size: 14px; border: none;");
    connect(btnCallNext, &QPushButton::clicked, this, &MainWindow::onCallNextPatient);
    vCtrl->addWidget(btnCallNext);

    lblQueueCount = new QLabel("<b>Waiting: 0</b>", grpQueueCtrl);
    lblQueueCount->setStyleSheet("color: #7e8794; font-size: 13px; padding-top: 8px;");
    vCtrl->addWidget(lblQueueCount);
    vCtrl->addStretch();

    grpQueueCtrl->setMaximumWidth(360);
    layout->addWidget(grpQueueCtrl);

    QGroupBox* grpQueueList = new QGroupBox("Queue", panel);
    QVBoxLayout* vList = new QVBoxLayout(grpQueueList);
    vList->setContentsMargins(16, 20, 16, 16);

    listQueueView = new QListWidget(grpQueueList);
    vList->addWidget(listQueueView);

    layout->addWidget(grpQueueList);
    return panel;
}

void MainWindow::refreshQueueView() {
    listQueueView->clear();

    const std::vector<PatientRecord*>& queue = database.getWaitingQueue();
    for (const PatientRecord* p : queue) {
        listQueueView->addItem(QString::fromStdString(p->getPatientID() + " - " + p->getName())
                               + " (" + typeLabel(p->getPatientType()) + ")");
    }

    lblQueueCount->setText("<b>Waiting: " + QString::number(database.getQueueSize()) + "</b>");
}

void MainWindow::onEnqueuePatient() {
    if (cmbQueuePatientID->count() == 0) {
        QMessageBox::warning(this, "No Patients", "Register a patient before adding them to the queue.");
        return;
    }

    QString patID = cmbQueuePatientID->currentData().toString();
    PatientRecord* p = database.findPatientByID(patID.toStdString());
    if (p == nullptr) {
        return;
    }

    database.enqueuePatient(p);
    refreshQueueView();
}

void MainWindow::onCallNextPatient() {
    PatientRecord* next = database.callNextPatient();
    if (next == nullptr) {
        refreshQueueView();
        QMessageBox::information(this, "Queue Empty", "No patients waiting.");
        return;
    }

    refreshQueueView();

    QStringList lines;
    lines << "<b>" + QString::fromStdString(next->getName()) + "</b>"
          << "ID: " + QString::fromStdString(next->getPatientID())
          << "Category: " + typeLabel(next->getPatientType())
          << "Details: " + QString::fromStdString(next->getSpecialDetail());
    QMessageBox::information(this, "Next Patient", lines.join("<br>"));
}

QWidget* MainWindow::createSearchSortPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* mainVBox = new QVBoxLayout(panel);
    mainVBox->setContentsMargins(16, 16, 16, 16);
    mainVBox->setSpacing(14);

    QHBoxLayout* ctrlRow = new QHBoxLayout();
    ctrlRow->setSpacing(16);

    QGroupBox* grpSearch = new QGroupBox("Search", panel);
    QFormLayout* fSearch = new QFormLayout(grpSearch);
    fSearch->setContentsMargins(16, 20, 16, 16);
    fSearch->setVerticalSpacing(10);

    txtSearchQuery = new QLineEdit(grpSearch);
    txtSearchQuery->setPlaceholderText("ID or name");

    cmbSearchAlgorithm = new QComboBox(grpSearch);
    cmbSearchAlgorithm->addItem("Linear search by ID (O(n))");
    cmbSearchAlgorithm->addItem("Binary search by ID (O(log n))");
    cmbSearchAlgorithm->addItem("Recursive search by name (O(n))");

    QPushButton* btnSearch = new QPushButton("Search", grpSearch);
    btnSearch->setStyleSheet("background-color: #1d6a85; color: white; font-weight: 600; padding: 8px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onExecuteSearch);

    fSearch->addRow("Query:", txtSearchQuery);
    fSearch->addRow("Algorithm:", cmbSearchAlgorithm);
    fSearch->addRow(btnSearch);
    ctrlRow->addWidget(grpSearch);

    QGroupBox* grpSort = new QGroupBox("Sort", panel);
    QFormLayout* fSort = new QFormLayout(grpSort);
    fSort->setContentsMargins(16, 20, 16, 16);
    fSort->setVerticalSpacing(10);

    cmbSortAlgorithm = new QComboBox(grpSort);
    cmbSortAlgorithm->addItem("Selection sort (O(n^2))");
    cmbSortAlgorithm->addItem("Merge sort (O(n log n))");

    cmbSortField = new QComboBox(grpSort);
    cmbSortField->addItem("Patient ID");
    cmbSortField->addItem("Patient Name");
    cmbSortField->addItem("Patient Age");

    QPushButton* btnSort = new QPushButton("Sort", grpSort);
    btnSort->setStyleSheet("background-color: #96701d; color: white; font-weight: 600; padding: 8px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnSort, &QPushButton::clicked, this, &MainWindow::onExecuteSort);

    fSort->addRow("Algorithm:", cmbSortAlgorithm);
    fSort->addRow("Sort by:", cmbSortField);
    fSort->addRow(btnSort);
    ctrlRow->addWidget(grpSort);

    mainVBox->addLayout(ctrlRow);

    lblAlgorithmExplanation = new QLabel(
        "<b>Selection sort:</b> O(n^2) comparisons, sorts in place.<br>"
        "<b>Merge sort:</b> O(n log n) comparisons, needs an extra array.", panel);
    lblAlgorithmExplanation->setStyleSheet("background-color: #f7f8fa; color: #5b636e; padding: 10px 14px; border: 1px solid #dde1e7; border-radius: 6px; font-size: 12px;");
    mainVBox->addWidget(lblAlgorithmExplanation);

    connect(cmbSortAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortAlgorithmChanged);

    tableSearchResults = new QTableWidget(panel);
    tableSearchResults->setColumnCount(5);
    tableSearchResults->setHorizontalHeaderLabels({"ID", "Full Name", "Age", "Phone", "Category"});
    tableSearchResults->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableSearchResults->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableSearchResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainVBox->addWidget(tableSearchResults);

    return panel;
}

void MainWindow::onSortAlgorithmChanged(int index) {
    if (index == 0) {
        lblAlgorithmExplanation->setText("<b>Selection sort:</b> O(n^2) comparisons, sorts in place, no extra memory.");
    } else {
        lblAlgorithmExplanation->setText("<b>Merge sort:</b> O(n log n) comparisons, splits the list in half and merges it back, needs an extra array.");
    }
}

void MainWindow::onExecuteSort() {
    std::vector<PatientRecord*> sorted = database.getAllPatients();
    if (sorted.empty()) {
        QMessageBox::information(this, "No Records", "There are no patient records to sort.");
        return;
    }

    SortField field = SORT_BY_ID;
    if (cmbSortField->currentIndex() == 1) field = SORT_BY_NAME;
    else if (cmbSortField->currentIndex() == 2) field = SORT_BY_AGE;

    int algo = cmbSortAlgorithm->currentIndex();
    if (algo == 0) {
        SortEngine::selectionSort(sorted, field);
    } else {
        SortEngine::mergeSort(sorted, field);
    }

    tableSearchResults->setRowCount(static_cast<int>(sorted.size()));
    for (int i = 0; i < static_cast<int>(sorted.size()); ++i) {
        setSearchResultRow(i, sorted[i]);
    }
}

void MainWindow::setSearchResultRow(int row, const PatientRecord* patient) {
    tableSearchResults->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(patient->getPatientID())));
    tableSearchResults->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(patient->getName())));
    tableSearchResults->setItem(row, 2, new QTableWidgetItem(QString::number(patient->getAge())));
    tableSearchResults->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(patient->getContactNumber())));
    tableSearchResults->setItem(row, 4, new QTableWidgetItem(typeLabel(patient->getPatientType())));
}

void MainWindow::onExecuteSearch() {
    QString q = txtSearchQuery->text().trimmed();
    if (q.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Enter an ID or name to search for.");
        return;
    }

    const std::vector<PatientRecord*>& patients = database.getAllPatients();
    int algo = cmbSearchAlgorithm->currentIndex();
    int foundIdx = -1;

    if (algo == 1) {
        std::vector<PatientRecord*> sortedCopy = patients;
        SortEngine::mergeSort(sortedCopy, SORT_BY_ID);
        foundIdx = SearchEngine::binarySearchByID(sortedCopy, q.toStdString());
        if (foundIdx != -1) {
            tableSearchResults->setRowCount(1);
            setSearchResultRow(0, sortedCopy[foundIdx]);
            QMessageBox::information(this, "Match Found",
                                     QString::fromStdString(sortedCopy[foundIdx]->getPatientID()) + " matched.");
            return;
        }
    } else if (algo == 0) {
        foundIdx = SearchEngine::linearSearchByID(patients, q.toStdString());
    } else {
        foundIdx = SearchEngine::recursiveSearchByName(patients, q.toStdString());
    }

    if (foundIdx != -1) {
        tableSearchResults->setRowCount(1);
        setSearchResultRow(0, patients[foundIdx]);
        QMessageBox::information(this, "Match Found",
                                 QString::fromStdString(patients[foundIdx]->getPatientID()) + " matched.");
    } else {
        tableSearchResults->setRowCount(0);
        QMessageBox::information(this, "No Match", "No patient matched \"" + q + "\".");
    }
}

QWidget* MainWindow::createReportsPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addWidget(new QLabel("Report:"));

    cmbReportType = new QComboBox(panel);
    cmbReportType->addItem("Patient Category Distribution");
    cmbReportType->addItem("Patients with Pending Appointments");
    cmbReportType->addItem("Daily Appointment Summary");
    cmbReportType->addItem("Frequently Attended Patients");
    topRow->addWidget(cmbReportType, 1);

    QPushButton* btnGenerate = new QPushButton("Generate", panel);
    btnGenerate->setStyleSheet("background-color: #35618f; color: white; font-weight: 600; padding: 8px 16px; border-radius: 6px; border: none; font-size: 13px;");
    connect(btnGenerate, &QPushButton::clicked, this, &MainWindow::onGenerateSelectedReport);
    topRow->addWidget(btnGenerate);

    layout->addLayout(topRow);

    txtReportOutput = new QTextEdit(panel);
    txtReportOutput->setReadOnly(true);
    txtReportOutput->setFontFamily("Consolas");
    txtReportOutput->setFontPointSize(10);
    layout->addWidget(txtReportOutput);

    return panel;
}

QString MainWindow::reportHeader(const QString& title) const {
    QString out = "CLINIC REPORT\n";
    out += title + "\n";
    out += "Generated: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n";
    out += "------------------------------------------------------------\n";
    return out;
}

QString MainWindow::buildCategoryReport() const {
    const std::vector<PatientRecord*>& patients = database.getAllPatients();

    int genCount = 0, emgCount = 0, folCount = 0;
    for (const PatientRecord* p : patients) {
        if (p->getPatientType() == "General") genCount++;
        else if (p->getPatientType() == "Emergency") emgCount++;
        else if (p->getPatientType() == "FollowUp") folCount++;
    }

    int total = static_cast<int>(patients.size());

    QString out = reportHeader("Patient Category Distribution");
    out += QString("General Outpatient:  %1 (%2%)\n")
        .arg(genCount, 3)
        .arg(total > 0 ? (genCount * 100.0 / total) : 0, 5, 'f', 1);
    out += QString("Emergency Patient:   %1 (%2%)\n")
        .arg(emgCount, 3)
        .arg(total > 0 ? (emgCount * 100.0 / total) : 0, 5, 'f', 1);
    out += QString("Follow-Up Patient:   %1 (%2%)\n")
        .arg(folCount, 3)
        .arg(total > 0 ? (folCount * 100.0 / total) : 0, 5, 'f', 1);
    out += "------------------------------------------------------------\n";
    out += QString("Total patients:      %1\n").arg(total, 3);
    return out;
}

QString MainWindow::buildPendingAppointmentsReport() {
    const std::vector<Appointment>& appointments = database.getAllAppointments();

    QString out = reportHeader("Patients with Pending Appointments");
    out += QString("%1 %2 %3 %4\n")
        .arg("ID", -10)
        .arg("Patient", -22)
        .arg("Date / Time", -22)
        .arg("Doctor", -15);
    out += "------------------------------------------------------------\n";

    int pendingCount = 0;
    for (const Appointment& a : appointments) {
        if (a.getStatus() != "Pending") {
            continue;
        }
        PatientRecord* p = database.findPatientByID(a.getPatientID());
        QString pName = p ? QString::fromStdString(p->getName()) : "Unknown";
        QString dt = QString::fromStdString(a.getDate() + " " + a.getTime());
        out += QString("%1 %2 %3 %4\n")
            .arg(QString::fromStdString(a.getAppointmentID()), -10)
            .arg(pName, -22)
            .arg(dt, -22)
            .arg(QString::fromStdString(a.getDoctorName()), -15);
        pendingCount++;
    }

    out += "------------------------------------------------------------\n";
    out += QString("Pending appointments: %1\n").arg(pendingCount);
    return out;
}

QString MainWindow::buildDailySummaryReport() const {
    const std::vector<Appointment>& appointments = database.getAllAppointments();

    int completed = 0, pending = 0, cancelled = 0;
    for (const Appointment& a : appointments) {
        if (a.getStatus() == "Completed") completed++;
        else if (a.getStatus() == "Pending") pending++;
        else if (a.getStatus() == "Cancelled") cancelled++;
    }

    QString out = reportHeader("Daily Appointment Summary");
    out += QString("Scheduled: %1\n").arg(appointments.size());
    out += QString("  Completed: %1\n").arg(completed);
    out += QString("  Pending:   %1\n").arg(pending);
    out += QString("  Cancelled: %1\n").arg(cancelled);
    out += "------------------------------------------------------------\n";

    double completionRate = appointments.empty() ? 0.0 : (completed * 100.0 / appointments.size());
    out += QString("Completion rate: %1%\n").arg(completionRate, 5, 'f', 1);
    return out;
}

QString MainWindow::buildFrequentPatientsReport() {
    const std::vector<Appointment>& appointments = database.getAllAppointments();

    std::map<std::string, int> visitCounts;
    for (const Appointment& a : appointments) {
        if (a.getStatus() == "Completed") {
            visitCounts[a.getPatientID()]++;
        }
    }

    QString out = reportHeader("Frequently Attended Patients");
    out += QString("%1 %2 %3 %4\n")
        .arg("Rank", -6)
        .arg("Patient ID", -14)
        .arg("Patient Name", -26)
        .arg("Completed Visits", -18);
    out += "------------------------------------------------------------\n";

    int rank = 1;
    for (const auto& entry : visitCounts) {
        PatientRecord* p = database.findPatientByID(entry.first);
        QString pName = p ? QString::fromStdString(p->getName()) : "Unknown";
        out += QString("%1 %2 %3 %4\n")
            .arg(rank++, -6)
            .arg(QString::fromStdString(entry.first), -14)
            .arg(pName, -26)
            .arg(entry.second, -18);
    }

    if (visitCounts.empty()) {
        out += "No completed visits recorded yet.\n";
    }
    return out;
}

void MainWindow::onGenerateSelectedReport() {
    int reportIdx = cmbReportType->currentIndex();

    QString out;
    if (reportIdx == 0) {
        out = buildCategoryReport();
    } else if (reportIdx == 1) {
        out = buildPendingAppointmentsReport();
    } else if (reportIdx == 2) {
        out = buildDailySummaryReport();
    } else {
        out = buildFrequentPatientsReport();
    }

    txtReportOutput->setPlainText(out);
}

void MainWindow::onSaveData() {
    QString patFile = QFileDialog::getSaveFileName(this, "Save Patients CSV", "patients.csv", "CSV (*.csv)");
    if (patFile.isEmpty()) return;

    QString aptFile = QFileDialog::getSaveFileName(this, "Save Appointments CSV", "appointments.csv", "CSV (*.csv)");
    if (aptFile.isEmpty()) return;

    try {
        FileHandler::savePatients(patFile.toStdString(), database.getAllPatients());
        FileHandler::saveAppointments(aptFile.toStdString(), database.getAllAppointments());
    } catch (const ClinicException& e) {
        QMessageBox::critical(this, "Save Error", e.what());
        return;
    }

    QMessageBox::information(this, "Saved", "Patient and appointment data written to CSV.");
}

void MainWindow::onLoadData() {
    QString patFile = QFileDialog::getOpenFileName(this, "Open Patients CSV", "", "CSV (*.csv)");
    if (patFile.isEmpty()) return;

    try {
        std::vector<PatientRecord*> loadedP = FileHandler::loadPatients(patFile.toStdString());
        if (loadedP.empty()) {
            QMessageBox::warning(this, "Empty File", "No patient records in that file.");
            return;
        }

        // the appointments file sits beside the patients file
        QString aptFile = QDir(QFileInfo(patFile).absolutePath()).filePath("appointments.csv");
        std::vector<Appointment> loadedA;
        if (QFileInfo::exists(aptFile)) {
            loadedA = FileHandler::loadAppointments(aptFile.toStdString());
        }

        database.clearAll();
        for (PatientRecord* p : loadedP) database.addPatient(p, false);
        for (const Appointment& a : loadedA) database.addAppointment(a, false);

        refreshPatientTable();
        refreshAppointmentTable();
        refreshPatientDropdowns();
        refreshQueueView();

        QMessageBox::information(this, "Loaded",
            QString::number(database.getPatientCount()) + " patients and "
            + QString::number(database.getAllAppointments().size()) + " appointments loaded.");
    } catch (const ClinicException& e) {
        QMessageBox::critical(this, "Load Error", e.what());
    }
}

void MainWindow::onUndoAction() {
    if (!editingPatientID.empty()) {
        onCancelEdit();
    }

    std::string undone = database.undoLastAction();
    if (!undone.empty()) {
        refreshPatientTable();
        refreshAppointmentTable();
        refreshPatientDropdowns();
        refreshQueueView();
        QMessageBox::information(this, "Undo", QString::fromStdString(undone) + " undone.");
    } else {
        QMessageBox::information(this, "Undo", "Nothing to undo.");
    }
}
