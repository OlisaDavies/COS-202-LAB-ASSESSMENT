# Smart Clinic Appointment & Patient Management System (SCAPMS)

**Course:** COS202 Programming II - Lab Assessment Project  
**Author:** Chinedu-Eze Olisa Davies  
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
model/       PatientRecord, Appointable, ScheduledPatient, GeneralPatient,
             EmergencyPatient, FollowUpPatient, Appointment, AdminAction, ClinicDatabase
controller/  SearchEngine, SortEngine, ReportBuilder
gui/         MainWindow, PatientPanel, AppointmentPanel, QueuePanel,
             SearchPanel, ReportPanel, style.qss
utils/       ClinicExceptions, DateUtil, FileHandler, IDGenerator, StringUtil
main.cpp     the console demonstration
gui_main.cpp the entry point of the graphical program
```

## Where each requirement is implemented

| Requirement | Implementation |
| --- | --- |
| 1.a Abstract base class | `model/PatientRecord.h` - `displayPatient()`, `getSpecialDetail()`, `getTypeLabel()` and `clone()` are pure virtual |
| 1.b Subclasses | `model/GeneralPatient.h`, `model/EmergencyPatient.h`, `model/FollowUpPatient.h` |
| 1.c Interface | `model/Appointable.h` - `scheduleAppointment()` and `cancelAppointment()`, called through a `dynamic_cast` in `model/ClinicDatabase.cpp` |
| 1.d Runtime polymorphism | `main.cpp` walks the records through a `PatientRecord*` and calls `displayPatient()`; `utils/FileHandler.cpp` picks the type specific fields with `dynamic_cast` |
| 1.e Composition | `model/ClinicDatabase.h` holds the patient and appointment collections |
| 1.f / 2 Organisation | The four layers above, each with its own namespace, every class split into a header and a source file |
| 3.a Vector | `model/ClinicDatabase.h` - `std::vector<PatientRecord*> patients` |
| 3.b Queue | `model/ClinicDatabase.h` - `std::queue<PatientRecord*> waitingQueue` |
| 3.c Stack | `model/ClinicDatabase.h` - `std::stack<AdminAction> undoStack`, with `model/AdminAction.h` holding the copy of the record that is put back |
| 3.d Fixed-size array | `model/ClinicDatabase.h` - `PatientRecord* recentPatients[10]` |
| 4 Searching | `controller/SearchEngine.h` - linear by ID, binary by ID on sorted records, recursive by name, linear by category |
| 5 Sorting | `controller/SortEngine.h` - selection sort and merge sort, chosen from a combo box on the Search & Sort panel |
| 6 Events | `gui/MainWindow.cpp` and the panels - button clicks, menu actions, combo box changes and a `QTimer` that refreshes the status bar |
| 7 Panels | Patient Records, Appointments, Waiting Queue, Search & Sort, Clinic Reports |
| 8 Layout | `QFormLayout` for the forms, `QHBoxLayout` and `QVBoxLayout` for the panels |
| 9 Advanced GUI | 9.a colour coded rows, 9.b fields that change with the patient type, 9.c file dialogs, 9.d status bar timer, 9.e validation messages, 9.f shortcuts, 9.g tooltips |
| 10 Exceptions | `utils/ClinicExceptions.h` - `DuplicateIDException`, `InvalidPatientDataException`, `InvalidAppointmentException`, `FileIOException` |
| 11 Persistence | `utils/FileHandler.h` - CSV read and write for both patients and appointments |

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
