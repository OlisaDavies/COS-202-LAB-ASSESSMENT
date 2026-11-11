#include "PatientPanel.h"

#include <QColor>
#include <QFile>
#include <QFormLayout>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

#include <memory>
#include <vector>

#include "../model/Appointable.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include "../utils/ClinicExceptions.h"
#include "../utils/DateUtil.h"
#include "../utils/IDGenerator.h"

namespace scapms {
namespace gui {

PatientPanel::PatientPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);

    layout->addWidget(buildForm(this));
    layout->addWidget(buildRoster(this));

    clearPatientForm();
    setEditingMode(false);
    refresh();
}

QWidget* PatientPanel::buildForm(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Register New Patient", parent);
    QFormLayout* form = new QFormLayout(group);
    form->setContentsMargins(16, 20, 16, 16);
    form->setVerticalSpacing(12);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    txtPatID = new QLineEdit(group);
    txtPatID->setPlaceholderText("e.g. GEN-201, EMG-801");
    txtPatID->setToolTip("The identifier of the patient. It has to begin with the prefix "
                          "of the chosen category, and it may only be used once.");

    // The program answers while the identifier is being typed, so that the
    // staff find out straight away that the identifier is already taken.
    connect(txtPatID, &QLineEdit::textChanged, this, &PatientPanel::onPatientIDChanged);

    lblIDHint = new QLabel("The identifier must be unique.", group);
    lblIDHint->setToolTip("Tells you whether the identifier is free or already in use.");
    lblIDHint->setWordWrap(true);
    lblIDHint->setStyleSheet("color: #777; font-size: 11px;");
    // The hint always carries a line, so that the fields below it keep the
    // same place while the staff type.
    lblIDHint->setMinimumHeight(16);

    btnSuggestID = new QPushButton("Suggest ID", group);
    btnSuggestID->setToolTip("Fill in the next free identifier for the chosen category.");
    connect(btnSuggestID, &QPushButton::clicked, this, &PatientPanel::onSuggestPatientID);

    txtPatName = new QLineEdit(group);
    txtPatName->setPlaceholderText("Full Patient Name");

    txtPatAge = new QLineEdit(group);
    txtPatAge->setPlaceholderText("Age (0-120)");

    txtPatContact = new QLineEdit(group);
    txtPatContact->setPlaceholderText("+234...");

    cmbPatType = new QComboBox(group);
    cmbPatType->addItem("General Outpatient");
    cmbPatType->addItem("Emergency Patient");
    cmbPatType->addItem("Follow-Up Patient");
    cmbPatType->setToolTip("The category decides which extra details the form asks for.");

    lblTypeField1 = new QLabel(group);
    txtTypeField1 = new QLineEdit(group);
    lblTypeField2 = new QLabel(group);
    txtTypeField2 = new QLineEdit(group);

    // The inputs keep a fixed width, so that the form never squeezes the
    // roster table on a small screen.
    const int inputWidth = 190;
    for (QLineEdit* input : {txtPatID, txtPatName, txtPatAge, txtPatContact,
                              txtTypeField1, txtTypeField2}) {
        input->setMaximumWidth(inputWidth);
    }
    cmbPatType->setMaximumWidth(inputWidth);
    form->addRow("Patient ID *:", txtPatID);
    form->addRow(lblIDHint);
    form->addRow(btnSuggestID);
    form->addRow("Full Name *:", txtPatName);
    form->addRow("Age *:", txtPatAge);
    form->addRow("Contact Phone:", txtPatContact);
    form->addRow("Category:", cmbPatType);
    form->addRow(lblTypeField1, txtTypeField1);
    form->addRow(lblTypeField2, txtTypeField2);

    btnRegister = new QPushButton("Register Patient", group);
    btnRegister->setToolTip("Add the patient to the clinic roster.");
    connect(btnRegister, &QPushButton::clicked, this, &PatientPanel::onRegisterPatient);

    btnSaveChanges = new QPushButton("Save Changes", group);
    btnSaveChanges->setToolTip("Store the changes that were made to the record.");
    connect(btnSaveChanges, &QPushButton::clicked, this, &PatientPanel::onUpdatePatient);

    btnCancelEdit = new QPushButton("Cancel Edit", group);
    btnCancelEdit->setToolTip("Throw the changes away and close the record.");
    connect(btnCancelEdit, &QPushButton::clicked, this, &PatientPanel::onCancelEdit);

    btnUndo = new QPushButton("Undo (Ctrl+Z)", group);
    btnUndo->setToolTip("Reverse the last change to a patient or an appointment.");
    connect(btnUndo, &QPushButton::clicked, this, [this]() { emit undoRequested(); });

    form->addRow(btnRegister);
    form->addRow(btnSaveChanges);
    form->addRow(btnCancelEdit);
    form->addRow(btnUndo);

    connect(cmbPatType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PatientPanel::onPatientTypeChanged);

    // The form keeps a minimum width, so that the roster table always has
    // room on a small screen, and it may grow when the window is wide.
    group->setMinimumWidth(330);
    group->setMaximumWidth(420);
    group->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    return group;
}

QWidget* PatientPanel::buildRoster(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Registered Patients", parent);
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(16, 20, 16, 16);
    layout->setSpacing(10);

    // The legend shows the colour of each category and the priority that goes
    // with it, so that a row can be read without looking up the category.
    QGridLayout* legend = new QGridLayout();
    legend->setHorizontalSpacing(14);
    legend->setVerticalSpacing(6);
    legend->addWidget(buildLegendChip("Emergency", "#ffebee", "#b71c1c", group), 0, 0);
    legend->addWidget(new QLabel("High Priority", group), 0, 1);
    legend->addWidget(buildLegendChip("General", "#e8f5e9", "#1b5e20", group), 1, 0);
    legend->addWidget(new QLabel("Normal", group), 1, 1);
    legend->addWidget(buildLegendChip("Follow-Up", "#e3f2fd", "#0d47a1", group), 2, 0);
    legend->addWidget(new QLabel("Scheduled", group), 2, 1);
    legend->setColumnStretch(1, 1);
    layout->addLayout(legend);

    tablePatients = new QTableWidget(group);
    tablePatients->setColumnCount(5);
    tablePatients->setHorizontalHeaderLabels(
        {"ID", "Full Name", "Age", "Phone", "Details"});

    // The identifier, the name, the age and the telephone number take the
    // width of their content, so that none of them is cut in half. The
    // details column takes the rest of the table and lets the long text
    // end in an ellipsis.
    QHeaderView* header = tablePatients->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
    header->setMinimumSectionSize(45);
    tablePatients->verticalHeader()->setVisible(false);
    tablePatients->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablePatients->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tablePatients->setTextElideMode(Qt::ElideRight);
    layout->addWidget(tablePatients);

    QHBoxLayout* buttonRow = new QHBoxLayout();

    btnEdit = new QPushButton("Edit", group);
    btnEdit->setToolTip("Put the selected patient into the form so that it can be changed.");
    connect(btnEdit, &QPushButton::clicked, this, &PatientPanel::onEditSelectedPatient);

    btnDelete = new QPushButton("Delete", group);
    btnDelete->setToolTip("Remove the selected patient. The step can be undone.");
    connect(btnDelete, &QPushButton::clicked, this, &PatientPanel::onDeleteSelectedPatient);

    btnReport = new QPushButton("Full Report", group);
    btnReport->setToolTip("Show the full report of the selected patient.");
    connect(btnReport, &QPushButton::clicked, this, &PatientPanel::onShowReport);

    buttonRow->addWidget(btnEdit);
    buttonRow->addWidget(btnReport);
    buttonRow->addStretch();
    buttonRow->addWidget(btnDelete);
    layout->addLayout(buttonRow);

    return group;
}

QLabel* PatientPanel::buildLegendChip(const QString& text, const QString& background,
                                       const QString& foreground, QWidget* parent) {
    QLabel* chip = new QLabel(text, parent);
    chip->setStyleSheet(
        QString("background-color: %1; color: %2; border-radius: 9px;"
                "padding: 3px 12px; font-size: 12px; font-weight: 600;").arg(background, foreground));
    chip->setAlignment(Qt::AlignCenter);
    chip->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    return chip;
}

void PatientPanel::showCategoryFields(int index) {
    const TypeFields fields = defaultFields(index);

    lblTypeField1->setText(fields.label1);
    txtTypeField1->setText(fields.value1);

    lblTypeField2->setVisible(fields.hasSecondField);
    txtTypeField2->setVisible(fields.hasSecondField);
    if (fields.hasSecondField) {
        lblTypeField2->setText(fields.label2);
        txtTypeField2->setText(fields.value2);
    }
}

void PatientPanel::onPatientTypeChanged(int index) {
    showCategoryFields(index);

    // The identifier carries the category, so the placeholder shows the
    // prefix that the chosen category needs.
    const std::string type = selectedPatientType();
    const std::string prefix = utils::IDGenerator::patientPrefix(type);
    txtPatID->setPlaceholderText(QString::fromStdString(
        "e.g. " + prefix + "201, " + prefix + "801"));
}

void PatientPanel::clearPatientForm() {
    txtPatID->clear();
    txtPatName->clear();
    txtPatAge->clear();
    txtPatContact->clear();

    cmbPatType->blockSignals(true);
    cmbPatType->setCurrentIndex(GENERAL_INDEX);
    cmbPatType->blockSignals(false);

    showCategoryFields(GENERAL_INDEX);
}

void PatientPanel::loadPatientIntoForm(const model::PatientRecord* patient) {
    txtPatID->setText(QString::fromStdString(patient->getPatientID()));
    txtPatName->setText(QString::fromStdString(patient->getName()));
    txtPatAge->setText(QString::number(patient->getAge()));
    txtPatContact->setText(QString::fromStdString(patient->getContactNumber()));

    const int index = categoryIndex(patient);

    cmbPatType->blockSignals(true);
    cmbPatType->setCurrentIndex(index);
    cmbPatType->blockSignals(false);

    const TypeFields fields = readFields(patient);
    lblTypeField1->setText(fields.label1);
    txtTypeField1->setText(fields.value1);
    lblTypeField2->setVisible(fields.hasSecondField);
    txtTypeField2->setVisible(fields.hasSecondField);
    if (fields.hasSecondField) {
        lblTypeField2->setText(fields.label2);
        txtTypeField2->setText(fields.value2);
    }
}

void PatientPanel::setEditingMode(bool editing) {
    btnRegister->setVisible(!editing);
    btnSaveChanges->setVisible(editing);
    btnCancelEdit->setVisible(editing);
    cmbPatType->setEnabled(!editing);
    txtPatID->setEnabled(!editing);
}

void PatientPanel::onRegisterPatient() {
    const QString id = txtPatID->text().trimmed();
    const QString name = txtPatName->text().trimmed();
    const QString ageText = txtPatAge->text().trimmed();
    const QString contact = txtPatContact->text().trimmed();

    if (id.isEmpty() || name.isEmpty() || ageText.isEmpty()) {
        QMessageBox::warning(this, "Invalid input",
                             "The patient ID, the name and the age are required.");
        return;                                 // the form is kept as it is
    }

    bool ageOk = false;
    const int age = ageText.toInt(&ageOk);
    if (!ageOk || age < 0 || age > 120) {
        QMessageBox::warning(this, "Invalid input",
                             "The age must be a whole number between 0 and 120.");
        return;
    }

    const int typeIndex = cmbPatType->currentIndex();

    TypeFields fields;
    fields.label1 = lblTypeField1->text();
    fields.value1 = txtTypeField1->text().trimmed();
    fields.label2 = lblTypeField2->text();
    fields.value2 = txtTypeField2->text().trimmed();
    fields.hasSecondField = txtTypeField2->isVisible();

    // The unique pointer owns the record until it is handed over to the
    // database, so it is released even when the registration is refused.
    std::unique_ptr<model::PatientRecord> owned;
    if (typeIndex == EMERGENCY_INDEX) {
        owned = std::make_unique<model::EmergencyPatient>(id.toStdString(), name.toStdString(),
                                                           age, contact.toStdString(), 1, "");
    } else if (typeIndex == FOLLOWUP_INDEX) {
        owned = std::make_unique<model::FollowUpPatient>(id.toStdString(), name.toStdString(),
                                                          age, contact.toStdString(), "", "");
    } else {
        owned = std::make_unique<model::GeneralPatient>(id.toStdString(), name.toStdString(),
                                                         age, contact.toStdString(), 0.0);
    }

    try {
        // The category fields are checked before the record is stored.
        writeFields(owned.get(), fields);
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Registration error", error.what());
        return;                                 // the form is kept as it is
    }

    try {
        // From here on the manager owns the record, also when it refuses it.
        manager.registerPatient(std::move(owned));
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Registration error", error.what());
        return;                                 // the form is kept as it is
    }
    clearPatientForm();
    refresh();
    emit dataChanged();

    QMessageBox::information(this, "Registered", name + " was added as " + id + ".");
}

void PatientPanel::onEditSelectedPatient() {
    const int row = tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No selection", "Please select a patient row first.");
        return;
    }

    const QString id = tablePatients->item(row, 0)->text();
    model::PatientRecord* patient = manager.findPatientByID(id.toStdString());
    if (patient == nullptr) {
        QMessageBox::warning(this, "Not found", "That patient is no longer in the clinic.");
        return;
    }

    // The copy taken before the edit. The unique pointer owns it, so it is
    // released even when the window is closed in the middle of an edit.
    editSnapshot.reset(patient->clone());
    editingPatientID = patient->getPatientID();

    loadPatientIntoForm(patient);
    setEditingMode(true);
}

void PatientPanel::onUpdatePatient() {
    if (editSnapshot == nullptr || editingPatientID.empty()) {
        return;
    }

    model::PatientRecord* patient = manager.findPatientByID(editingPatientID);
    if (patient == nullptr) {
        QMessageBox::warning(this, "Not found", "That patient is no longer in the clinic.");
        onCancelEdit();
        return;
    }

    const QString name = txtPatName->text().trimmed();
    const QString ageText = txtPatAge->text().trimmed();
    const QString contact = txtPatContact->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid input", "The name is required.");
        return;
    }

    bool ageOk = false;
    const int age = ageText.toInt(&ageOk);
    if (!ageOk || age < 0 || age > 120) {
        QMessageBox::warning(this, "Invalid input",
                             "The age must be a whole number between 0 and 120.");
        return;
    }

    TypeFields fields;
    fields.label1 = lblTypeField1->text();
    fields.value1 = txtTypeField1->text().trimmed();
    fields.label2 = lblTypeField2->text();
    fields.value2 = txtTypeField2->text().trimmed();
    fields.hasSecondField = txtTypeField2->isVisible();

    // The change is first tried out on a copy of the record. That way an
    // invalid value cannot leave the live record half changed.
    std::unique_ptr<model::PatientRecord> candidate(patient->clone());

    try {
        candidate->setName(name.toStdString());
        candidate->setAge(age);
        candidate->setContactNumber(contact.toStdString());
        writeFields(candidate.get(), fields);

        // The rules of the clinic are checked on the copy as well, so that an
        // edit cannot put an illegal contact number or an illegal age into
        // a record that was registered properly.
        manager.validatePatient(*candidate);
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Update error", error.what());
        return;                                 // the live record was not touched
    }

    // The change is good, so the state from before the edit is kept on the
    // undo stack before the live record is changed.
    manager.pushPatientSnapshot(std::move(editSnapshot));
    editingPatientID.clear();

    // The values that were checked on the copy are now copied onto the live
    // record.
    patient->restoreFrom(*candidate);

    setEditingMode(false);
    clearPatientForm();
    refresh();
    emit dataChanged();

    QMessageBox::information(this, "Updated", "The patient record was updated.");
}

void PatientPanel::onCancelEdit() {
    editSnapshot.reset();
    editingPatientID.clear();

    setEditingMode(false);
    clearPatientForm();
}

void PatientPanel::onDeleteSelectedPatient() {
    const int row = tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No selection", "Please select a patient row first.");
        return;
    }

    const QString id = tablePatients->item(row, 0)->text();
    const QString name = tablePatients->item(row, 1)->text();

    const int reply = QMessageBox::question(
        this, "Delete patient",
        "Delete " + name + " (" + id + ")?\n\nThe step can be undone with Ctrl+Z.",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }

    if (editingPatientID == id.toStdString()) {
        onCancelEdit();
    }

    try {
        if (!manager.deletePatient(id.toStdString())) {
            QMessageBox::warning(this, "Not found", "That patient is no longer in the clinic.");
            return;
        }
    } catch (const utils::ClinicException& error) {
        QMessageBox::critical(this, "Delete error", error.what());
        return;                                 // the patient is still in the clinic
    }

    refresh();
    emit dataChanged();
}

void PatientPanel::onShowReport() {
    const int row = tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "No selection", "Please select a patient row first.");
        return;
    }

    const QString id = tablePatients->item(row, 0)->text();
    const model::PatientRecord* patient = manager.findPatientByID(id.toStdString());
    if (patient == nullptr) {
        QMessageBox::warning(this, "Not found", "That patient is no longer in the clinic.");
        return;
    }

    // The call goes through the base class pointer, so the report of the
    // category of the patient is built at run time.
    QMessageBox box(this);
    box.setWindowTitle("Patient report - " + id);
    box.setText(QString::fromStdString(patient->getPatientReport()));
    box.exec();
}

void PatientPanel::onPatientIDChanged(const QString& text) {
    // The label under the identifier field tells the staff at once whether
    // the identifier can be used.
    const QString id = text.trimmed();

    if (id.isEmpty()) {
        lblIDHint->setText("The identifier is required.");
        lblIDHint->setStyleSheet("color: #777; font-size: 11px;");
        return;
    }

    const std::string prefix = utils::IDGenerator::patientPrefix(selectedPatientType());
    if (!id.startsWith(QString::fromStdString(prefix))) {
        lblIDHint->setText("An identifier of this category begins with [" +
                           QString::fromStdString(prefix) + "].");
        lblIDHint->setStyleSheet("color: #b06000; font-size: 11px;");
        return;
    }

    if (manager.findPatientByID(id.toStdString()) != nullptr) {
        lblIDHint->setText("This identifier is already in the clinic.");
        lblIDHint->setStyleSheet("color: #b00020; font-size: 11px;");
        return;
    }

    lblIDHint->setText("This identifier is free.");
    lblIDHint->setStyleSheet("color: #1b7a2f; font-size: 11px;");
}

void PatientPanel::onSuggestPatientID() {
    // The controller knows which identifiers are already in use.
    const std::string suggestion = manager.suggestPatientId(selectedPatientType());
    txtPatID->setText(QString::fromStdString(suggestion));
    txtPatID->setFocus();
}

std::string PatientPanel::selectedPatientType() const {
    switch (cmbPatType->currentIndex()) {
        case EMERGENCY_INDEX:
            return model::PatientRecord::TYPE_EMERGENCY;
        case FOLLOWUP_INDEX:
            return model::PatientRecord::TYPE_FOLLOWUP;
        case GENERAL_INDEX:
        default:
            return model::PatientRecord::TYPE_GENERAL;
    }
}

int PatientPanel::categoryIndex(const model::PatientRecord* patient) {
    if (patient == nullptr) {
        return GENERAL_INDEX;
    }
    return categoryIndex(patient->getPatientType());
}

int PatientPanel::categoryIndex(const std::string& patientType) {
    if (patientType == model::PatientRecord::TYPE_EMERGENCY) {
        return EMERGENCY_INDEX;
    }
    if (patientType == model::PatientRecord::TYPE_FOLLOWUP) {
        return FOLLOWUP_INDEX;
    }
    return GENERAL_INDEX;
}

PatientPanel::TypeFields PatientPanel::defaultFields(int index) {
    TypeFields fields;

    if (index == EMERGENCY_INDEX) {
        fields.label1 = "Severity (1-3):";
        fields.value1 = "1";
        fields.label2 = "Triage Note:";
        fields.value2 = "Acute distress";
        fields.hasSecondField = true;
    } else if (index == FOLLOWUP_INDEX) {
        fields.label1 = "Last Visit Date:";
        fields.value1 = "2026-10-15";
        fields.label2 = "Assigned Doctor:";
        fields.value2 = "Dr. Adeyemi";
        fields.hasSecondField = true;
    } else {
        fields.label1 = "Consultation Fee (N):";
        fields.value1 = "4500.00";
        fields.hasSecondField = false;
    }
    return fields;
}

PatientPanel::TypeFields PatientPanel::readFields(const model::PatientRecord* patient) {
    if (patient == nullptr) {
        return defaultFields(GENERAL_INDEX);
    }

    const int index = categoryIndex(patient);
    TypeFields fields = defaultFields(index);

    if (const model::GeneralPatient* general =
            dynamic_cast<const model::GeneralPatient*>(patient)) {
        fields.value1 = QString::number(general->getConsultationFee());
    } else if (const model::EmergencyPatient* emergency =
                   dynamic_cast<const model::EmergencyPatient*>(patient)) {
        fields.value1 = QString::number(emergency->getSeverityLevel());
        fields.value2 = QString::fromStdString(emergency->getTriageNote());
    } else if (const model::FollowUpPatient* followUp =
                   dynamic_cast<const model::FollowUpPatient*>(patient)) {
        fields.value1 = QString::fromStdString(followUp->getPreviousVisitDate());
        fields.value2 = QString::fromStdString(followUp->getAssignedDoctor());
    }
    return fields;
}

void PatientPanel::writeGeneral(model::GeneralPatient* patient, const TypeFields& fields) {
    bool ok = false;
    const double fee = fields.value1.trimmed().toDouble(&ok);

    // A number that is not a number at all, a value that is infinite and a
    // value that is far too large are all refused. Without this check a
    // fee of "nan" would be accepted, because every comparison with nan is
    // false, and it would then be written into the data file.
    if (!ok || !model::GeneralPatient::isValidConsultationFee(fee)) {
        throw utils::InvalidPatientDataException(
            "The consultation fee must be a number of naira between 0 and 100,000,000, "
            "for example 4500.00.");
    }
    patient->setConsultationFee(fee);
}

void PatientPanel::writeEmergency(model::EmergencyPatient* patient, const TypeFields& fields) {
    bool ok = false;
    const int severity = fields.value1.trimmed().toInt(&ok);
    if (!ok || severity < 1 || severity > 3) {
        throw utils::InvalidPatientDataException(
            "The severity must be 1 (critical), 2 (urgent) or 3 (moderate).");
    }
    patient->setSeverityLevel(severity);
    patient->setTriageNote(fields.value2.trimmed().toStdString());
}

void PatientPanel::writeFollowUp(model::FollowUpPatient* patient, const TypeFields& fields) {
    const std::string lastVisit = fields.value1.trimmed().toStdString();
    if (!utils::isValidDate(lastVisit)) {
        throw utils::InvalidPatientDataException(
            "The last visit date must be written as YYYY-MM-DD.");
    }
    if (fields.value2.trimmed().isEmpty()) {
        throw utils::InvalidPatientDataException("Please enter the doctor that is assigned.");
    }
    patient->setPreviousVisitDate(lastVisit);
    patient->setAssignedDoctor(fields.value2.trimmed().toStdString());
}

void PatientPanel::writeFields(model::PatientRecord* patient, const TypeFields& fields) {
    if (patient == nullptr) {
        throw utils::InvalidPatientDataException("There is no patient record to update.");
    }

    if (model::GeneralPatient* general = dynamic_cast<model::GeneralPatient*>(patient)) {
        writeGeneral(general, fields);
    } else if (model::EmergencyPatient* emergency = dynamic_cast<model::EmergencyPatient*>(patient)) {
        writeEmergency(emergency, fields);
    } else if (model::FollowUpPatient* followUp = dynamic_cast<model::FollowUpPatient*>(patient)) {
        writeFollowUp(followUp, fields);
    } else {
        throw utils::InvalidPatientDataException("Unknown patient category.");
    }
}

void PatientPanel::applyRowFormatting(int row, const std::string& patientType) {
    QColor background;
    QColor foreground;

    if (patientType == model::PatientRecord::TYPE_EMERGENCY) {
        background = QColor("#ffebee");
        foreground = QColor("#b71c1c");
    } else if (patientType == model::PatientRecord::TYPE_GENERAL) {
        background = QColor("#e8f5e9");
        foreground = QColor("#1b5e20");
    } else {
        background = QColor("#e3f2fd");
        foreground = QColor("#0d47a1");
    }

    for (int column = 0; column < tablePatients->columnCount(); ++column) {
        QTableWidgetItem* item = tablePatients->item(row, column);
        if (item == nullptr) {
            continue;
        }
        item->setBackground(background);
        if (column == 4) {
            item->setForeground(foreground);
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
    }
}

void PatientPanel::refresh() {
    const std::vector<model::PatientRecord*> patients = manager.getAllPatients();
    tablePatients->setRowCount(static_cast<int>(patients.size()));

    for (int i = 0; i < static_cast<int>(patients.size()); ++i) {
        const model::PatientRecord* patient = patients[i];

        tablePatients->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(patient->getPatientID())));
        tablePatients->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(patient->getName())));
        tablePatients->setItem(i, 2, new QTableWidgetItem(QString::number(patient->getAge())));
        tablePatients->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(patient->getContactNumber())));

        QString details = QString::fromStdString(patient->getTypeLabel()) + ": " +
                          QString::fromStdString(patient->getSpecialDetail());
        if (const model::Appointable* bookable =
                dynamic_cast<const model::Appointable*>(patient)) {
            if (bookable->hasScheduledAppointment()) {
                details += " | Appointment: " +
                           QString::fromStdString(bookable->getAppointmentDetails());
            }
        }

        // A patient can hold more than one appointment, so the number of
        // them is shown as well.
        const std::size_t total = patient->getAppointmentCount();
        if (total > 1) {
            details += " | " + QString::number(total) + " appointments";
        }

        tablePatients->setItem(i, 4, new QTableWidgetItem(details));

        applyRowFormatting(i, patient->getPatientType());
    }
}

} // namespace gui
} // namespace scapms
