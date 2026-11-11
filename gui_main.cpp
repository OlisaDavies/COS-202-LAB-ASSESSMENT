// COS202 Programming II - Lab Assessment
// Smart Clinic Appointment & Patient Management System (SCAPMS)

#include <QApplication>
#include <QMessageBox>

#include <exception>

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

    return app.exec();
}
