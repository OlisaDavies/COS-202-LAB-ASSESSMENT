#include "SearchPanel.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

#include "../controller/SearchEngine.h"
#include "../controller/SortEngine.h"

namespace scapms {
namespace gui {

SearchPanel::SearchPanel(controller::ClinicManager& manager, QWidget* parent)
    : QWidget(parent), manager(manager) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(14);

    QHBoxLayout* controlRow = new QHBoxLayout();
    controlRow->setSpacing(16);
    controlRow->addWidget(buildSearchBox(this));
    controlRow->addWidget(buildSortBox(this));
    layout->addLayout(controlRow);

    lblAlgorithmExplanation = new QLabel(
        "<b>Selection sort:</b> O(n^2) comparisons, sorts in place.<br>"
        "<b>Merge sort:</b> O(n log n) comparisons, needs an extra array.", this);
    lblAlgorithmExplanation->setWordWrap(true);
    layout->addWidget(lblAlgorithmExplanation);

    tableSearchResults = new QTableWidget(this);
    tableSearchResults->setColumnCount(5);
    tableSearchResults->setHorizontalHeaderLabels(
        {"ID", "Full Name", "Age", "Phone", "Category"});

    // The column widths are set by hand, so that the table never asks for
    // more room than the window can give.
    QHeaderView* header = tableSearchResults->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setMinimumSectionSize(45);
    header->resizeSection(0, 70);
    header->resizeSection(1, 120);
    header->resizeSection(2, 45);
    header->resizeSection(3, 110);
    header->setStretchLastSection(true);
    tableSearchResults->verticalHeader()->setVisible(false);
    tableSearchResults->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableSearchResults->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(tableSearchResults);

    onSearchAlgorithmChanged(SEARCH_LINEAR_ID);
    onSortAlgorithmChanged(0);
}

QWidget* SearchPanel::buildSearchBox(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Search", parent);
    QFormLayout* form = new QFormLayout(group);
    form->setContentsMargins(16, 20, 16, 16);
    form->setVerticalSpacing(10);

    txtSearchQuery = new QLineEdit(group);
    txtSearchQuery->setPlaceholderText("ID or name");
    txtSearchQuery->setToolTip("What the search should look for.");

    cmbSearchAlgorithm = new QComboBox(group);
    cmbSearchAlgorithm->addItem("Linear search by ID (O(n))");
    cmbSearchAlgorithm->addItem("Binary search by ID (O(log n))");
    cmbSearchAlgorithm->addItem("Recursive search by name (O(n))");
    cmbSearchAlgorithm->addItem("Linear search by category (O(n))");
    cmbSearchAlgorithm->setToolTip("The searching routine that is used.");

    cmbSearchCategory = new QComboBox(group);
    cmbSearchCategory->addItem("General Outpatient");
    cmbSearchCategory->addItem("Emergency Patient");
    cmbSearchCategory->addItem("Follow-Up Patient");
    cmbSearchCategory->setToolTip("The category that is looked for.");
    cmbSearchCategory->setVisible(false);

    lblSearchCategory = new QLabel("Category:", group);
    lblSearchCategory->setVisible(false);

    QPushButton* btnSearch = new QPushButton("Search", group);
    btnSearch->setToolTip("Run the chosen searching routine.");
    connect(btnSearch, &QPushButton::clicked, this, &SearchPanel::onExecuteSearch);

    form->addRow("Query:", txtSearchQuery);
    form->addRow(lblSearchCategory, cmbSearchCategory);
    form->addRow("Algorithm:", cmbSearchAlgorithm);
    form->addRow(btnSearch);

    connect(cmbSearchAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SearchPanel::onSearchAlgorithmChanged);

    return group;
}

QWidget* SearchPanel::buildSortBox(QWidget* parent) {
    QGroupBox* group = new QGroupBox("Sort", parent);
    QFormLayout* form = new QFormLayout(group);
    form->setContentsMargins(16, 20, 16, 16);
    form->setVerticalSpacing(10);

    cmbSortAlgorithm = new QComboBox(group);
    cmbSortAlgorithm->addItem("Selection sort (O(n^2))");
    cmbSortAlgorithm->addItem("Merge sort (O(n log n))");
    cmbSortAlgorithm->setToolTip("The sorting routine that is used.");

    cmbSortField = new QComboBox(group);
    cmbSortField->addItem("Patient ID");
    cmbSortField->addItem("Patient Name");
    cmbSortField->addItem("Patient Age");
    cmbSortField->setToolTip("The field the records are ordered by.");

    QPushButton* btnSort = new QPushButton("Sort", group);
    btnSort->setToolTip("Order the records with the chosen routine.");
    connect(btnSort, &QPushButton::clicked, this, &SearchPanel::onExecuteSort);

    form->addRow("Algorithm:", cmbSortAlgorithm);
    form->addRow("Sort by:", cmbSortField);
    form->addRow(btnSort);

    connect(cmbSortAlgorithm, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SearchPanel::onSortAlgorithmChanged);

    return group;
}

void SearchPanel::onSearchAlgorithmChanged(int index) {
    const bool byCategory = (index == SEARCH_CATEGORY);

    cmbSearchCategory->setVisible(byCategory);
    lblSearchCategory->setVisible(byCategory);
    txtSearchQuery->setEnabled(!byCategory);
}

void SearchPanel::onSortAlgorithmChanged(int index) {
    if (index == 0) {
        lblAlgorithmExplanation->setText(
            "<b>Selection sort:</b> O(n^2) comparisons, sorts in place and needs no "
            "extra memory. It is a good choice for the short lists of a clinic.");
    } else {
        lblAlgorithmExplanation->setText(
            "<b>Merge sort:</b> O(n log n) comparisons, splits the list in half and "
            "joins it back together. It is stable and keeps the order of equal records.");
    }
}

void SearchPanel::showResults(const std::vector<model::PatientRecord*>& results) {
    tableSearchResults->setRowCount(static_cast<int>(results.size()));

    for (int i = 0; i < static_cast<int>(results.size()); ++i) {
        const model::PatientRecord* patient = results[i];
        tableSearchResults->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(patient->getPatientID())));
        tableSearchResults->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(patient->getName())));
        tableSearchResults->setItem(i, 2, new QTableWidgetItem(QString::number(patient->getAge())));
        tableSearchResults->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(patient->getContactNumber())));
        tableSearchResults->setItem(i, 4, new QTableWidgetItem(
            QString::fromStdString(patient->getTypeLabel())));
    }
}

void SearchPanel::onExecuteSearch() {
    const std::vector<model::PatientRecord*> patients = manager.getAllPatients();
    const int algorithm = cmbSearchAlgorithm->currentIndex();

    std::vector<model::PatientRecord*> results;

    if (algorithm == SEARCH_CATEGORY) {
        const int categoryIndex = cmbSearchCategory->currentIndex();
        const std::string category = (categoryIndex == 1)
            ? model::PatientRecord::TYPE_EMERGENCY
            : (categoryIndex == 2 ? model::PatientRecord::TYPE_FOLLOWUP
                                  : model::PatientRecord::TYPE_GENERAL);

        const std::vector<int> matches = controller::SearchEngine::searchByCategory(patients, category);
        for (int index : matches) {
            results.push_back(patients[index]);
        }

        if (results.empty()) {
            QMessageBox::information(this, "No match",
                                     "No patient is in that category.");
        } else {
            QMessageBox::information(this, "Match found",
                                     QString::number(results.size()) + " patient(s) found.");
        }
        showResults(results);
        return;
    }

    const QString query = txtSearchQuery->text().trimmed();
    if (query.isEmpty()) {
        QMessageBox::warning(this, "Invalid input", "Please enter an ID or a name to look for.");
        return;
    }

    int foundIndex = -1;
    std::vector<model::PatientRecord*> sortedCopy;

    if (algorithm == SEARCH_BINARY_ID) {
        // Binary search only works on a sorted list, so a sorted copy of the
        // records is made first.
        sortedCopy = patients;
        controller::SortEngine::mergeSort(sortedCopy, controller::SortField::PatientID);
        foundIndex = controller::SearchEngine::binarySearchByID(sortedCopy, query.toStdString());
        if (foundIndex != -1) {
            results.push_back(sortedCopy[foundIndex]);
        }
    } else if (algorithm == SEARCH_RECURSIVE_NAME) {
        foundIndex = controller::SearchEngine::recursiveSearchByName(patients, query.toStdString());
        if (foundIndex != -1) {
            results.push_back(patients[foundIndex]);
        }
    } else {
        foundIndex = controller::SearchEngine::linearSearchByID(patients, query.toStdString());
        if (foundIndex != -1) {
            results.push_back(patients[foundIndex]);
        }
    }

    if (results.empty()) {
        showResults(results);
        QMessageBox::information(this, "No match", "No patient matched \"" + query + "\".");
        return;
    }

    showResults(results);
    QMessageBox::information(this, "Match found",
                             QString::fromStdString(results.front()->getPatientID()) + " matched.");
}

void SearchPanel::onExecuteSort() {
    std::vector<model::PatientRecord*> sorted = manager.getAllPatients();
    if (sorted.empty()) {
        QMessageBox::information(this, "No records",
                                 "There are no patient records to sort.");
        return;
    }

    controller::SortField field = controller::SortField::PatientID;
    const int fieldIndex = cmbSortField->currentIndex();
    if (fieldIndex == 1) {
        field = controller::SortField::PatientName;
    } else if (fieldIndex == 2) {
        field = controller::SortField::PatientAge;
    }

    const QString algorithmName = cmbSortAlgorithm->currentText();
    if (cmbSortAlgorithm->currentIndex() == 0) {
        controller::SortEngine::selectionSort(sorted, field);
    } else {
        controller::SortEngine::mergeSort(sorted, field);
    }

    showResults(sorted);

    QMessageBox::information(this, "Sorted",
                             "The records were ordered with " + algorithmName + ".");
}

void SearchPanel::refresh() {
    // Nothing to redraw: the table holds the result of the last search.
}

} // namespace gui
} // namespace scapms
