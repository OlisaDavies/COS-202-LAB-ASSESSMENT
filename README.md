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
             SearchPanel, ReportPanel, AdminPanel, style.qss, clinic.ico
utils/       ClinicExceptions, DateUtil, FileHandler, IDGenerator, StringUtil
main.cpp     the console demonstration
gui_main.cpp the entry point of the graphical program
```

The program is built in three layers. The **model** layer holds the data and
the rules that keep it consistent, the **controller** layer holds the rules of
the clinic and coordinates the work, and the **GUI** layer draws the screens
and asks the controller to do the work. The screens never touch the database
directly: every panel holds a `ClinicManager&` and asks it to do the work.

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
stack and the reports without needing the GUI.

### Data files
`patients.csv` and `appointments.csv` hold the same records the application starts with, and are
the files the import and export dialogs read and write.
