cmake_minimum_required(VERSION 3.21)

project(SCAPMS
    VERSION 1.0
    DESCRIPTION "Smart Clinic Appointment & Patient Management System"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Both programs are written to build\bin, so that the path of the binary does
# not change with the build configuration.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
foreach(SCAPMS_CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${SCAPMS_CONFIG} SCAPMS_CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${SCAPMS_CONFIG_UPPER}
        ${CMAKE_BINARY_DIR}/bin)
endforeach()

if(MSVC)
    add_compile_options(/W3 /D_CRT_SECURE_NO_WARNINGS)
endif()

# The model, controller and utility layers.
#
# These three layers do not use Qt at all, so they are listed once here and
# used by both programs.
set(CORE_SOURCES
    model/PatientRecord.cpp
    model/ScheduledPatient.cpp
    model/GeneralPatient.cpp
    model/EmergencyPatient.cpp
    model/FollowUpPatient.cpp
    model/Appointment.cpp
    model/AdminAction.cpp
    model/ClinicDatabase.cpp
    controller/ClinicManager.cpp
    controller/AppointmentController.cpp
    controller/SearchEngine.cpp
    controller/SortEngine.cpp
    controller/ReportBuilder.cpp
    utils/DateUtil.cpp
    utils/FileHandler.cpp
    utils/IDGenerator.cpp)

set(CORE_HEADERS
    model/PatientRecord.h
    model/Appointable.h
    model/Billable.h
    model/ScheduledPatient.h
    model/GeneralPatient.h
    model/EmergencyPatient.h
    model/FollowUpPatient.h
    model/Appointment.h
    model/AdminAction.h
    model/ClinicDatabase.h
    controller/ClinicManager.h
    controller/AppointmentController.h
    controller/SearchEngine.h
    controller/SortEngine.h
    controller/ReportBuilder.h
    utils/ClinicExceptions.h
    utils/DateUtil.h
    utils/FileHandler.h
    utils/IDGenerator.h
    utils/StringUtil.h)

# The console demonstration of the clinic, which needs no Qt at all.
add_executable(SCAPMS main.cpp ${CORE_SOURCES} ${CORE_HEADERS})

# The graphical program.
#
# It needs Qt, so it is only built when Qt was found. The build scripts point
# CMake at the Qt folder with -DCMAKE_PREFIX_PATH.
find_package(Qt6 COMPONENTS Widgets QUIET)

if(Qt6_FOUND)
    set(GUI_SOURCES
        gui_main.cpp
        gui/MainWindow.cpp
        gui/PatientPanel.cpp
        gui/AppointmentPanel.cpp
        gui/QueuePanel.cpp
        gui/SearchPanel.cpp
        gui/ReportPanel.cpp
        gui/AdminPanel.cpp
        gui/resources.qrc)

    set(GUI_HEADERS
        gui/MainWindow.h
        gui/PatientPanel.h
        gui/AppointmentPanel.h
        gui/QueuePanel.h
        gui/SearchPanel.h
        gui/ReportPanel.h
        gui/AdminPanel.h)

    add_executable(SCAPMS_GUI ${GUI_SOURCES} ${GUI_HEADERS}
        ${CORE_SOURCES} ${CORE_HEADERS})
    target_link_libraries(SCAPMS_GUI PRIVATE Qt6::Widgets)
else()
    message(STATUS
        "Qt6 was not found, so only the console program will be built. "
        "Point CMake at the Qt folder with -DCMAKE_PREFIX_PATH=<path> "
        "to build the graphical program as well.")
endif()
