#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include <memory>

#include "../controller/ClinicManager.h"
#include "../model/PatientRecord.h"

namespace scapms {

namespace model {
class GeneralPatient;
class EmergencyPatient;
class FollowUpPatient;
}

namespace gui {

// The patient panel. It holds the registration form and the roster table.
//
// The panel owns the form while a record is being edited. When the change is
// saved the snapshot is handed over to the database, which keeps it on the
// undo stack.
class PatientPanel : public QWidget {
    Q_OBJECT

public:
    explicit PatientPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    // Redraws the roster table.
    void refresh();

signals:
    // Emitted whenever the panel changed the data, so that the other panels
    // can refresh themselves.
    void dataChanged();

    // The window owns the undo stack, so the button asks it to undo.
    void undoRequested();

private slots:
    void onPatientTypeChanged(int index);
    void onRegisterPatient();
    void onEditSelectedPatient();
    void onUpdatePatient();
    void onCancelEdit();
    void onDeleteSelectedPatient();
    void onShowReport();
    void onSuggestPatientID();

private:
    // The order of the entries in the category combo box.
    static constexpr int GENERAL_INDEX = 0;
    static constexpr int EMERGENCY_INDEX = 1;
    static constexpr int FOLLOWUP_INDEX = 2;

    // The two fields that change with the patient category. The panel reads
    // them through the functions below, so it never has to ask what kind of
    // patient it is dealing with.
    struct TypeFields {
        QString label1;
        QString value1;
        QString label2;
        QString value2;
        bool hasSecondField = false;
    };

    controller::ClinicManager& manager;

    // Form.
    QLineEdit* txtPatID;
    QLineEdit* txtPatName;
    QLineEdit* txtPatAge;
    QLineEdit* txtPatContact;
    QComboBox* cmbPatType;
    QLabel* lblTypeField1;
    QLineEdit* txtTypeField1;
    QLabel* lblTypeField2;
    QLineEdit* txtTypeField2;

    // Roster.
    QTableWidget* tablePatients;

    // Buttons.
    QPushButton* btnRegister;
    QPushButton* btnSaveChanges;
    QPushButton* btnCancelEdit;
    QPushButton* btnUndo;
    QPushButton* btnEdit;
    QPushButton* btnDelete;
    QPushButton* btnReport;
    QPushButton* btnSuggestID;

    // The record that is being edited and the copy taken before the edit.
    // The copy is owned by the panel, so it is released even when the
    // window is closed in the middle of an edit.
    std::unique_ptr<model::PatientRecord> editSnapshot;
    std::string editingPatientID;

    QWidget* buildForm(QWidget* parent);
    QWidget* buildRoster(QWidget* parent);

    void clearPatientForm();
    void loadPatientIntoForm(const model::PatientRecord* patient);
    void showCategoryFields(int index);
    void applyRowFormatting(int row, const std::string& patientType);
    void setEditingMode(bool editing);

    // The combo box entry of a record and of a category name.
    static int categoryIndex(const model::PatientRecord* patient);
    static int categoryIndex(const std::string& patientType);

    // The empty form of a category, the values of a record, and writing the
    // values back into a record.
    static TypeFields defaultFields(int index);
    static TypeFields readFields(const model::PatientRecord* patient);
    static void writeFields(model::PatientRecord* patient, const TypeFields& fields);

    static void writeGeneral(model::GeneralPatient* patient, const TypeFields& fields);
    static void writeEmergency(model::EmergencyPatient* patient, const TypeFields& fields);
    static void writeFollowUp(model::FollowUpPatient* patient, const TypeFields& fields);
};

} // namespace gui
} // namespace scapms
