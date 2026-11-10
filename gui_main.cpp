// COS202 Programming II - Lab Assessment
// Smart Clinic Appointment & Patient Management System (SCAPMS)

#include <QApplication>

#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
