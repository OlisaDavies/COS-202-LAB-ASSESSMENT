# Smart Clinic Appointment & Patient Management System (SCAPMS)

**Course:** COS202 Programming II - Lab Assessment Project  
**Author:** OlisaEmeka Davies Chinedu-Eze  
**Technologies:** C++17, Qt 6.5.3 (x64), MSVC 2022

## Project Overview

A desktop application for running an outpatient clinic. Patient records, appointments and the
waiting room are all managed from a Qt widgets interface, backed by `std::vector`, `std::queue`
and `std::stack`. The search and sort algorithms are written by hand rather than taken from the
standard library.

The program is split into four layers, and every layer has its own namespace:

| Layer | Folder | Namespace | What it holds |
| --- | --- | --- | --- |
| Model | `model/` | `scapms::model` | The patient classes, the appointment, the database |
| Controller | `controller/` | `scapms::controller` | The searching, sorting and reporting routines |
| GUI | `gui/` | `scapms::gui` | The main window and one panel per tab |
| Utilities | `utils/` | `scapms::utils` | The exception classes, the file reader, the ID generator |

The model, controller and utility layers do not use Qt at all, so the same code runs in the
console demonstration (`main.cpp`) and in the graphical program (`gui_main.cpp`).

## Features

- Patient records for the general, emergency and follow-up categories, with add, edit and delete
- Appointment scheduling and cancellation
- Waiting room queue, served first in first out
- Undo for the last change to a patient or an appointment
- Patient search by linear, binary, recursive and category search, and sorting by selection sort
  and merge sort
- CSV import and export for patients and appointments
- Reports on category distribution, pending appointments, the daily schedule and frequently
  attended patients

## Folder layout

```
model/       PatientRecord, Appointable, Billable, ScheduledPatient, GeneralPatient,
             EmergencyPatient, FollowUpPatient, Appointment, AdminAction, ClinicDatabase
controller/  ClinicManager, AppointmentController, SearchEngine, SortEngine, ReportBuilder
gui/         MainWindow, PatientPanel, AppointmentPanel, QueuePanel,
             SearchPanel, ReportPanel, AdminPanel, style.qss, clinic.ico,
             resources.qrc
utils/       ClinicExceptions, DateUtil, FileHandler, IDGenerator, StringUtil
main.cpp     the console demonstration
gui_main.cpp the entry point of the graphical program
```

### The source code under the headings of the submission

The submission asks for the complete source code under four headings. The files of the project
belong to them like this:

**i. Header files**
```
model/PatientRecord.h          model/Appointable.h        model/Billable.h
model/ScheduledPatient.h       model/GeneralPatient.h      model/EmergencyPatient.h
model/FollowUpPatient.h        model/Appointment.h         model/AdminAction.h
model/ClinicDatabase.h         controller/ClinicManager.h  controller/AppointmentController.h
controller/SearchEngine.h      controller/SortEngine.h     controller/ReportBuilder.h
utils/ClinicExceptions.h       utils/DateUtil.h            utils/FileHandler.h
utils/IDGenerator.h            utils/StringUtil.h          gui/MainWindow.h
gui/PatientPanel.h             gui/AppointmentPanel.h      gui/QueuePanel.h
gui/SearchPanel.h              gui/ReportPanel.h           gui/AdminPanel.h
```

**ii. Source files**
```
model/PatientRecord.cpp        model/ScheduledPatient.cpp  model/GeneralPatient.cpp
model/EmergencyPatient.cpp     model/FollowUpPatient.cpp   model/Appointment.cpp
model/AdminAction.cpp          model/ClinicDatabase.cpp    controller/ClinicManager.cpp
controller/AppointmentController.cpp                       controller/SearchEngine.cpp
controller/SortEngine.cpp      controller/ReportBuilder.cpp utils/DateUtil.cpp
utils/FileHandler.cpp          utils/IDGenerator.cpp       main.cpp
```

**iii. GUI files and resources**
```
gui/MainWindow.cpp              gui/PatientPanel.cpp        gui/AppointmentPanel.cpp
gui/QueuePanel.cpp              gui/SearchPanel.cpp         gui/ReportPanel.cpp
gui/AdminPanel.cpp              gui_main.cpp                gui/style.qss
gui/clinic.ico                  gui/resources.qrc           SCAPMS.pro
build_gui.bat                   build.bat                  .vscode/tasks.json
.vscode/launch.json
```

**iv. Data files**
```
patients.csv                    appointments.csv
```

`patients.csv` and `appointments.csv` hold the records that the import and the export dialogs read
and write. The program starts with the same records built into it, so it also runs without them.

The program has two entry points: `gui_main.cpp` builds the graphical program and `main.cpp` builds
the console demonstration. They cannot be in one binary because each of them has its own `main()`,
so the console version is built by `build.bat` with the compiler directly and is not part of
`SCAPMS.pro`.

### Opening a panel from the command line

The panels of the graphical program can also be asked for on the command line, which is how the
screenshots in the submission were produced and which makes a demonstration of any single panel
possible without clicking:

```
SCAPMS_GUI.exe                      opens the first panel
SCAPMS_GUI.exe 3                    opens the Waiting Queue
SCAPMS_GUI.exe 3 GEN-101            also puts that patient in the queue
SCAPMS_GUI.exe 2 GEN-101 0          also runs a search for GEN-101 with the
                                    linear search by ID
```

The number is the position of the tab, in the order the tabs are shown: 0 Patient Records,
1 Appointments, 2 Waiting Queue, 3 Search &amp; Sort, 4 Clinic Reports, 5 Administration. The second
and the third words only mean something for the two panels they belong to, so an argument that does
not fit the panel is ignored.

## The submission document

`SCAPMS_Submission.pdf` is the single document the assessment asks for. It holds, in the order the
brief lists them: the two to three page report, the six screenshots, the UML class diagrams, and
the complete source code under the four headings the brief names. `SUBMISSION_REPORT.md` is the
report on its own, and the four `UML_*.png` pictures are the diagrams, because a full diagram of
Twenty five classes do not read on one page when they are printed.

## Where each requirement is implemented

| Requirement | Implementation |
| --- | --- |
| 1.a Abstract base class | `model/PatientRecord.h` - `getTypeLabel()`, `getPatientReport()`, `getSpecialDetail()` and `clone()` are pure virtual; `displayPatient()` and `restoreFrom()` are concrete methods shared by every category |
| 1.b Subclasses | `model/GeneralPatient.h`, `model/EmergencyPatient.h`, `model/FollowUpPatient.h` |
| 1.c Interface | `model/Appointable.h` - `scheduleAppointment()`, `cancelAppointment()` and the overloaded `cancelAppointment(id)`; `model/Billable.h` is a second interface, so a class can implement more than one |
| 1.d Runtime polymorphism | `main.cpp` walks the records through a `PatientRecord*` and calls `displayPatient()`, which dispatches to the version of the category; `gui/PatientPanel.cpp` shows the same report through the base pointer |
| 1.e Composition | `model/PatientRecord.h` owns the `std::vector<Appointment> appointments` collection, so a patient holds one or more appointments; `model/ClinicDatabase.h` owns the patient records |
| 1.f / 2 Organisation | The four layers above, each with its own namespace; every class with behaviour is split into a header and a source file |
| 2 Separation of responsibilities | `controller/ClinicManager.h` is the only class the GUI talks to. `controller/AppointmentController.h` holds the rules of a booking. `model/ClinicDatabase.h` only stores and finds. The GUI never calls the database directly. |
| 3.a Vector | `model/PatientRecord.h` - `std::vector<Appointment> appointments`; `model/ClinicDatabase.h` - `std::vector<std::unique_ptr<PatientRecord>> patients` |
| 3.b Queue | `model/ClinicDatabase.h` - `std::queue<PatientRecord*> waitingQueue`. The patient who joins first is served first |
| 3.c Stack | `model/ClinicDatabase.h` - `std::stack<AdminAction> undoStack`, with `model/AdminAction.h` holding the copy of the record that is put back. Undoing a delete also gives the record its place in the waiting queue again |
| 3.d Fixed-size array | `model/ClinicDatabase.h` - `PatientRecord* recentPatients[10]`, shown in the Administration panel. The array is held from the oldest record at the front to the newest at the back, the most recently used record is shown first, and when the cache is full the record that was used the longest time ago makes room for a new one |
| 4 Searching | `controller/SearchEngine.h` - linear by ID or name, binary by ID on sorted records, recursive by name, linear by category. The text is matched without regard to letter case, and a part of the identifier or the name is enough |
| 5 Sorting | `controller/SortEngine.h` - selection sort and merge sort, chosen from a combo box on the Search & Sort panel. The chosen order is kept by the controller, so the roster and the results show the same order. The names and the identifiers are compared without regard to letter case |
| 6 Events | `gui/MainWindow.cpp` and the panels - button clicks, menu actions, combo box changes, text field changes and a `QTimer` that refreshes the status bar and the dashboard. The File menu holds New, Save as, Load and Exit |
| 7 Panels | Patient Records, Appointments, Waiting Queue, Search & Sort, Clinic Reports, Administration |
| 8 Layout | `QFormLayout` for the forms, `QHBoxLayout` and `QVBoxLayout` for the panels, `QGridLayout` for the tables of the categories and their priorities |
| 9 Advanced GUI | 9.a colour coded rows with a table of the categories and their priorities, 9.b fields that change with the patient type, 9.c file dialogs, 9.d status bar timer, 9.e validation messages, 9.f shortcuts, 9.g tooltips |
| 10 Exceptions | `utils/ClinicExceptions.h` - `DuplicateIDException`, `InvalidPatientDataException`, `InvalidAppointmentException`, `FileIOException`. `gui_main.cpp` also catches anything that escapes a slot, so that the program never stops by itself |
| 11 Persistence | `utils/FileHandler.h` - CSV read and write for both patients and appointments. A file is written to a temporary name first, so a failed save cannot destroy the data that is already on the disk |

## The rules of the clinic

The rules live in the controller layer, so that the screens and the data always agree on them.

| Rule | Where it is checked |
| --- | --- |
| The age has to be between 0 and 120 | `ClinicManager::checkPatientRules` |
| The name may not be empty | `ClinicManager::checkPatientRules` |
| The identifier has to begin with the prefix of the category: `GEN-`, `EMG-` or `FOL-` | `ClinicManager::checkPatientRules` |
| A contact number that is given has to look like a telephone number | `ClinicManager::checkPatientRules` |
| The same patient may not wait in the queue twice | `ClinicManager::enqueuePatient` |
| The date has to be written as `YYYY-MM-DD` and the time as `HH:MM AM` or `HH:MM PM` | `AppointmentController::checkBookingRules` |
| The same doctor cannot be booked twice in the same slot | `AppointmentController::scheduleAppointment` |
| An emergency patient is not booked in advance, it goes to the waiting room | `AppointmentController::scheduleAppointment` |
| A visit that was completed cannot be cancelled, and a cancelled visit cannot be completed | `ScheduledPatient::cancelAppointment` |

## Screenshots

The `screenshots/` folder holds the six views of the graphical program, in the order in which the
submission asks for them:

| File | View |
| --- | --- |
| `1_main_dashboard.png` | The dashboard of the clinic, with the live figures and the categories |
| `2_patient_registration.png` | The registration form and the roster of the patients |
| `3_appointment_management.png` | The booking form and the schedule of the clinic |
| `4_search_and_sort.png` | The search and the sort, with the result of a search |
| `5_waiting_queue.png` | The waiting room and the patients that are waiting |
| `6_clinic_reports.png` | The report of the clinic |

## Build & Run

### Prerequisites
- Windows 10/11 (x64)
- Visual Studio 2022 C++ Build Tools
- Qt 6.5+

### Compilation
Run the build script for the graphical program:

```cmd
build_gui.bat
```

The binary is generated at `release\SCAPMS_GUI.exe`.

`build.bat` builds the console version (`build\SCAPMS.exe`), which walks through the exception
handling, the polymorphic calls, the searching and sorting routines, the waiting room, the undo
stack, the summary of the clinic and the four reports without needing the GUI.

### Data files
`patients.csv` and `appointments.csv` hold the same records the application starts with, and are
the files the import and export dialogs read and write. The program starts with the same records
built into it, so it also runs when these two files are not there.
