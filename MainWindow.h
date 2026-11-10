#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <string>

#include "ClinicDatabase.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    ClinicDatabase database;
    QTimer* dashboardTimer;
    QTabWidget* tabWidget;

    // patient records
    QLineEdit* txtPatID;
    QLineEdit* txtPatName;
    QLineEdit* txtPatAge;
    QLineEdit* txtPatContact;
    QComboBox* cmbPatType;
    QLineEdit* txtTypeField1;
    QLineEdit* txtTypeField2;
    QLabel* lblTypeField1;
    QLabel* lblTypeField2;
    QTableWidget* tablePatients;
    QPushButton* btnRegister;
    QPushButton* btnSaveChanges;
    QPushButton* btnCancelEdit;
    QPushButton* btnEdit;

    // patient being edited, plus a copy of it taken before the edit
    PatientRecord* editSnapshot;
    std::string editingPatientID;
    int nextAppointmentNumber;

    // appointments
    QLineEdit* txtAptID;
    QComboBox* cmbAptPatientID;
    QLineEdit* txtAptDate;
    QLineEdit* txtAptTime;
    QLineEdit* txtAptDoctor;
    QTableWidget* tableAppointments;

    // waiting queue
    QComboBox* cmbQueuePatientID;
    QListWidget* listQueueView;
    QLabel* lblQueueCount;

    // search and sort
    QLineEdit* txtSearchQuery;
    QComboBox* cmbSearchAlgorithm;
    QComboBox* cmbSortAlgorithm;
    QComboBox* cmbSortField;
    QTableWidget* tableSearchResults;
    QLabel* lblAlgorithmExplanation;

    // reports
    QComboBox* cmbReportType;
    QTextEdit* txtReportOutput;

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onTimerTick();

    void onSaveData();
    void onLoadData();
    void onUndoAction();

    void onPatientTypeChanged(int index);
    void onRegisterPatient();
    void onEditSelectedPatient();
    void onUpdatePatient();
    void onCancelEdit();
    void onDeleteSelectedPatient();

    void onScheduleAppointment();
    void onCancelSelectedAppointment();

    void onEnqueuePatient();
    void onCallNextPatient();

    void onExecuteSearch();
    void onExecuteSort();
    void onSortAlgorithmChanged(int index);

    void onGenerateSelectedReport();

private:
    void setupUI();
    void setupMenuBar();
    void setupTimer();
    void loadSampleData();

    QWidget* createPatientRecordsPanel();
    QWidget* createPatientFormBox(QWidget* parent);
    QWidget* createPatientTableBox(QWidget* parent);
    QWidget* createAppointmentsPanel();
    QWidget* createWaitingQueuePanel();
    QWidget* createSearchSortPanel();
    QWidget* createReportsPanel();

    void refreshPatientTable();
    void refreshAppointmentTable();
    void refreshPatientDropdowns();
    void refreshQueueView();

    void clearPatientForm();
    void loadPatientIntoForm(const PatientRecord* patient);

    QString generateAppointmentId();
    void setSearchResultRow(int row, const PatientRecord* patient);

    QString reportHeader(const QString& title) const;
    QString buildCategoryReport() const;
    QString buildPendingAppointmentsReport();
    QString buildDailySummaryReport() const;
    QString buildFrequentPatientsReport();

    void applyCustomPatientRowFormatting(int row, const std::string& patientType);
};
