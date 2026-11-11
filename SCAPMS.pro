QT += core gui widgets

TARGET = SCAPMS_GUI
TEMPLATE = app

CONFIG += c++17
DEFINES += _CRT_SECURE_NO_WARNINGS

# The graphical user interface.
#
# The program has two entry points: gui_main.cpp builds the graphical
# program, and main.cpp builds the console demonstration of the same model,
# controller and utility layers. They cannot be in one binary, because each
# of them has its own main(), so the console version is built by build.bat
# with the compiler directly and is not part of this project file.
SOURCES += \
    gui_main.cpp \
    gui/MainWindow.cpp \
    gui/PatientPanel.cpp \
    gui/AppointmentPanel.cpp \
    gui/QueuePanel.cpp \
    gui/SearchPanel.cpp \
    gui/ReportPanel.cpp \
    gui/AdminPanel.cpp \

HEADERS += \
    gui/MainWindow.h \
    gui/PatientPanel.h \
    gui/AppointmentPanel.h \
    gui/QueuePanel.h \
    gui/SearchPanel.h \
    gui/ReportPanel.h \
    gui/AdminPanel.h \

# The model layer.
SOURCES += \
    model/PatientRecord.cpp \
    model/ScheduledPatient.cpp \
    model/GeneralPatient.cpp \
    model/EmergencyPatient.cpp \
    model/FollowUpPatient.cpp \
    model/Appointment.cpp \
    model/AdminAction.cpp \
    model/ClinicDatabase.cpp

HEADERS += \
    model/PatientRecord.h \
    model/Appointable.h \
    model/Billable.h \
    model/ScheduledPatient.h \
    model/GeneralPatient.h \
    model/EmergencyPatient.h \
    model/FollowUpPatient.h \
    model/Appointment.h \
    model/AdminAction.h \
    model/ClinicDatabase.h

# The controller layer.
SOURCES += \
    controller/ClinicManager.cpp \
    controller/AppointmentController.cpp \
    controller/SearchEngine.cpp \
    controller/SortEngine.cpp \
    controller/ReportBuilder.cpp

HEADERS += \
    controller/ClinicManager.h \
    controller/AppointmentController.h \
    controller/SearchEngine.h \
    controller/SortEngine.h \
    controller/ReportBuilder.h

# The utility layer.
SOURCES += \
    utils/DateUtil.cpp \
    utils/FileHandler.cpp \
    utils/IDGenerator.cpp

HEADERS += \
    utils/ClinicExceptions.h \
    utils/DateUtil.h \
    utils/FileHandler.h \
    utils/IDGenerator.h \
    utils/StringUtil.h

# The look and feel of the program.
RESOURCES += gui/resources.qrc
