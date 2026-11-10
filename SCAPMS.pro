QT += core gui widgets

TARGET = SCAPMS_GUI
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    gui_main.cpp \
    MainWindow.cpp

HEADERS += \
    MainWindow.h \
    PatientRecord.h \
    GeneralPatient.h \
    EmergencyPatient.h \
    FollowUpPatient.h \
    Appointable.h \
    Appointment.h \
    AdminAction.h \
    ClinicDatabase.h \
    ClinicExceptions.h \
    SearchEngine.h \
    SortEngine.h \
    FileHandler.h
