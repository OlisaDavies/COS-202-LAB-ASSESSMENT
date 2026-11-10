# Smart Clinic Appointment & Patient Management System (SCAPMS)

**Course:** COS202 Programming II — Lab Assessment Project  
**Author:** Chinedu-Eze Olisa Davies  
**Technologies:** C++17, Qt 6.5.3 (x64), MSVC 2022

## Project Overview

A desktop application for running an outpatient clinic. Patient records, appointments and the
waiting room are all managed from a Qt widgets interface, backed by `std::vector`, `std::queue`
and `std::stack`. The search and sort algorithms are written by hand rather than taken from the
standard library.

## Features

- Patient records for the general, emergency and follow-up categories, with add, edit and delete
- Appointment scheduling and cancellation
- Waiting room queue, served first in first out
- Undo for the last change to a patient or an appointment
- Patient search by linear, binary and recursive search, and sorting by selection sort and merge sort
- CSV import and export for patients and appointments
- Reports on category distribution, pending appointments, the daily schedule and frequently
  attended patients

## Where each requirement is implemented

| Requirement | Implementation |
| --- | --- |
| 1.a Abstract base class | `PatientRecord.h` — `displayPatient()`, `getSpecialDetail()`, `clone()` and `restoreFrom()` are pure virtual |
| 1.b Subclasses | `GeneralPatient.h`, `EmergencyPatient.h`, `FollowUpPatient.h` |
| 1.c Interface | `Appointable.h` — `scheduleAppointment()` and `cancelAppointment()`, called through a `dynamic_cast` in `ClinicDatabase.h` |
| 1.d Runtime polymorphism | `main.cpp` walks the records through a `PatientRecord*` and calls `displayPatient()`; `FileHandler.h` picks the type specific fields with `dynamic_cast` |
| 1.e Composition | `ClinicDatabase.h` holds the patient and appointment collections |
| 1.f / 2 Organisation | Model headers, `SearchEngine.h`, `SortEngine.h`, `FileHandler.h` and `MainWindow.cpp` are kept apart, and the headers do not import namespaces into other files |
| 3.a Vector | `ClinicDatabase.h` — `std::vector<PatientRecord*> patients` |
| 3.b Queue | `ClinicDatabase.h` — `std::queue<PatientRecord*> waitingQueue` |
| 3.c Stack | `ClinicDatabase.h` — `std::stack<AdminAction> undoStack`, with `AdminAction.h` holding the record to put back |
| 3.d Fixed-size array | `ClinicDatabase.h` — `PatientRecord* recentPatients[10]` |
| 4 Searching | `SearchEngine.h` — linear by ID, binary by ID on sorted records, recursive by name |
| 5 Sorting | `SortEngine.h` — selection sort and merge sort, chosen from a combo box on the Search & Sort panel |
| 6 Events | `MainWindow.cpp` — button clicks, menu actions, combo box changes and a `QTimer` that refreshes the status bar |
| 7 Panels | Patient Records, Appointments, Waiting Queue, Search & Sort, Clinic Reports |
| 8 Layout | `MainWindow.cpp` — `QFormLayout` for the forms, `QHBoxLayout` and `QVBoxLayout` for the panels |
| 9 Advanced GUI | 9.a colour coded rows, 9.b fields that change with the patient type, 9.c file dialogs, 9.d status bar timer, 9.e validation messages, 9.f shortcuts, 9.g tooltips |
| 10 Exceptions | `ClinicExceptions.h` — `DuplicateIDException`, `InvalidPatientDataException`, `FileIOException`, `InvalidAppointmentException` |
| 11 Persistence | `FileHandler.h` — CSV read and write for both patients and appointments |

## Build & Run

### Prerequisites
- Windows 10/11 (x64)
- Visual Studio 2022 C++ Build Tools
- Qt 6.5+

### Compilation
Run the build script:

```cmd
build_gui.bat
```

The binary is generated at `release\SCAPMS_GUI.exe`.

`build.bat` builds the console version (`SCAPMS.exe`), which walks through the exception handling
and the polymorphic calls without needing the GUI.

### Data files
`patients.csv` and `appointments.csv` hold the same records the application starts with, and are
the files the import and export dialogs read and write.
