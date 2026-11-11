// COS202 Programming II - Lab Assessment
// Smart Clinic Appointment & Patient Management System (SCAPMS)

#include <QApplication>
#include <QMessageBox>

#include <exception>
#include <string>

#include "gui/MainWindow.h"

namespace {

// The application of the clinic.
//
// The class exists for one reason: Qt does not catch an exception that
// leaves a slot. Without this class an error that slips past the checks of
// a panel would travel out of the event loop and end the program with
// "terminate called after throwing an instance of ...". The clinic has to
// keep running after a normal mistake, so the exception is caught here and
// shown to the staff as a message.
class ClinicApplication : public QApplication {
public:
    ClinicApplication(int& argc, char** argv) : QApplication(argc, argv) {}

protected:
    // Every event of the program passes through this method, so it is the
    // one place where an exception can still be stopped.
    bool notify(QObject* receiver, QEvent* event) override {
        try {
            return QApplication::notify(receiver, event);
        } catch (const std::exception& error) {
            QMessageBox::critical(nullptr, "Unexpected error", error.what());
            return false;
        } catch (...) {
            QMessageBox::critical(nullptr, "Unexpected error",
                                  "An unknown error stopped this action.");
            return false;
        }
    }
};

} // namespace

int main(int argc, char* argv[]) {
    ClinicApplication app(argc, argv);

    scapms::gui::MainWindow mainWindow;
    mainWindow.show();

    // The panels can be asked for on the command line:
    //
    //   SCAPMS_GUI.exe                      opens the first panel
    //   SCAPMS_GUI.exe 3                    opens the Waiting Queue
    //   SCAPMS_GUI.exe 3 GEN-101            also puts that patient in the queue
    //   SCAPMS_GUI.exe 2 GEN-101 0          also runs a search for GEN-101
    //                                      with the linear search by ID
    //
    // The number is the position of the tab, in the order the tabs are shown.
    // The second and third words only mean something for the two panels they
    // belong to, so an argument that does not fit the panel is ignored. This
    // is how the screenshots of the submission were produced, and it makes a
    // demonstration of any single panel possible without clicking.
    if (argc > 1) {
        bool numberOk = false;
        const int requested = QString::fromLocal8Bit(argv[1]).toInt(&numberOk);
        if (numberOk && requested >= 0 && requested <= 4) {
            mainWindow.showTab(requested);

            if (requested == 2 && argc > 2) {
                mainWindow.queuePanelWidget()->enqueueByID(
                    QString::fromLocal8Bit(argv[2]));
            }
            if (requested == 3 && argc > 3) {
                bool algorithmOk = false;
                const int algorithm =
                    QString::fromLocal8Bit(argv[3]).toInt(&algorithmOk);
                mainWindow.searchPanelWidget()->runQuery(QString::fromLocal8Bit(argv[2]),
                                                         algorithmOk ? algorithm : 0);
            }
        }
    }

    return app.exec();
}
