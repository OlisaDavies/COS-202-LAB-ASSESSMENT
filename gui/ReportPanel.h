#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>

#include "../model/ClinicDatabase.h"

namespace scapms {
namespace gui {

// The report panel. The reports are built by the report controller, so this
// panel only shows them.
class ReportPanel : public QWidget {
    Q_OBJECT

public:
    explicit ReportPanel(model::ClinicDatabase& database, QWidget* parent = nullptr);

    void refresh();

signals:
    void dataChanged();

private slots:
    void onGenerateReport();

private:
    model::ClinicDatabase& database;

    QComboBox* cmbReportType;
    QTextEdit* txtReportOutput;
};

} // namespace gui
} // namespace scapms
