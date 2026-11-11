#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>

#include "../controller/ClinicManager.h"
#include "../controller/SortEngine.h"

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

    // Runs the given search without showing a message box, and redraws the
    // results table. Returns the number of records that matched. The command
    // line uses this to open the panel with a search already shown.
    int runQuery(const QString& query, int algorithm);

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

    // The last search that was run, so that it can be run again when the
    // data changed. An empty text means that no search was run yet.
    QString lastQuery;
    int lastAlgorithm;

    // The entries of the two algorithm combo boxes.
    //
    // The linear search has two entries, because the brief asks for a linear
    // search by patient ID, and a keyword search across the identifier and
    // the name is what the staff actually type. Both are linear scans, but
    // they compare different fields, so they are kept apart.
    enum SearchAlgorithm {
        SEARCH_LINEAR_ID = 0,
        SEARCH_LINEAR_KEYWORD = 1,
        SEARCH_BINARY_ID = 2,
        SEARCH_RECURSIVE_NAME = 3,
        SEARCH_CATEGORY = 4
    };

    // The entries of the field combo box of the sort.
    enum SortFieldChoice {
        SORT_BY_ID = 0,
        SORT_BY_NAME = 1,
        SORT_BY_AGE = 2
    };

    QWidget* buildSearchBox(QWidget* parent);
    QWidget* buildSortBox(QWidget* parent);

    void showResults(const std::vector<model::PatientRecord*>& results);

    // Runs the chosen routine and returns the records that matched.
    //
    // The records themselves are returned and not only their positions,
    // because the binary search works on a sorted copy of the list. A
    // position in that copy would point at a different record in the
    // roster, and the wrong patient would be shown.
    std::vector<model::PatientRecord*> runSearch(const std::vector<model::PatientRecord*>& patients,
                                                 const QString& query,
                                                 int algorithm);

    // The sort field that is chosen in the combo box.
    controller::SortField selectedSortField() const;

    // Runs the search again, without showing a message box. This is used
    // when the data changed, so that the results stay true.
    void repeatLastSearch();
};

} // namespace gui
} // namespace scapms
