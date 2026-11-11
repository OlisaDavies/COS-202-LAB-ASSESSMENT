#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include "../controller/ClinicManager.h"

namespace scapms {
namespace gui {

// The search and sort panel.
//
// The user chooses the algorithm, so the program shows which routine was used
// and how the records were ordered.
class SearchPanel : public QWidget {
    Q_OBJECT

public:
    explicit SearchPanel(controller::ClinicManager& manager, QWidget* parent = nullptr);

    void refresh();

signals:
    void dataChanged();

private slots:
    void onExecuteSearch();
    void onExecuteSort();
    void onSearchAlgorithmChanged(int index);
    void onSortAlgorithmChanged(int index);

private:
    controller::ClinicManager& manager;

    QLineEdit* txtSearchQuery;
    QComboBox* cmbSearchAlgorithm;
    QLabel* lblSearchCategory;
    QComboBox* cmbSearchCategory;
    QComboBox* cmbSortAlgorithm;
    QComboBox* cmbSortField;

    QTableWidget* tableSearchResults;
    QLabel* lblAlgorithmExplanation;

    // The entries of the two algorithm combo boxes.
    enum SearchAlgorithm {
        SEARCH_LINEAR_ID = 0,
        SEARCH_BINARY_ID = 1,
        SEARCH_RECURSIVE_NAME = 2,
        SEARCH_CATEGORY = 3
    };

    QWidget* buildSearchBox(QWidget* parent);
    QWidget* buildSortBox(QWidget* parent);

    void showResults(const std::vector<model::PatientRecord*>& results);
};

} // namespace gui
} // namespace scapms
