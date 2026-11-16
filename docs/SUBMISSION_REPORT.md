# Academic Project Report

**Course:** COS202 Programming II – Lab Assessment Project  
**Project:** Smart Clinic Appointment & Patient Management System (SCAPMS)  
**Student:** OlisaEmeka Davies Chinedu-Eze  
**Language & Framework:** C++17, Qt 6.5.3 (Qt Widgets)  
**Build Environment:** Visual Studio 2022 MSVC Build Tools, qmake

## 1. Project Description

A clinic running on paper records suffers double bookings, a waiting room nobody can track, and deleted records with no way back. SCAPMS is a desktop application that fixes those problems. It keeps patients in three categories — general, emergency and follow-up — and lets the staff register, edit and delete a record; book, cancel and complete an appointment; run a first-in first-out waiting room; search and sort the records with the algorithm of their choice; undo the last administrative step; and read four clinic reports. The data is written to and read from CSV files, so the clinic survives between runs.

## 2. System Features

| Feature | Where it lives |
|---|---|
| Register, edit and delete a patient | Patient Records panel |
| Schedule, cancel and complete an appointment | Appointments panel |
| Waiting room: add a patient, call the next one | Waiting Queue panel |
| Search by ID, keyword, name or category; sort by ID, name or age | Search & Sort panel |
| Undo the last change to a patient or an appointment | Administration panel, or Ctrl+Z |
| Save and load the clinic files | Administration panel, or Ctrl+S / Ctrl+O |
| Four clinic reports | Clinic Reports panel |
| Clinic summary and recently accessed records | Administration panel (the dashboard) |

## 3. Class Hierarchy

The core of the model is the **abstract base class `PatientRecord`**, which holds what every patient has in common: ID, name, age, contact number and category. It declares four pure virtual functions — `getTypeLabel()`, `getPatientReport()`, `getSpecialDetail()` and `clone()` — so it can never be created on its own, and its destructor is virtual, so deleting through a `PatientRecord*` frees the subclass.

**Three subclasses** give each category its own behaviour: **`GeneralPatient`** (an outpatient visit with a consultation fee), **`EmergencyPatient`** (a severity level of 1–3 and a triage note) and **`FollowUpPatient`** (the date of the last visit and the assigned doctor).

**Interfaces.** C++ has no `interface` keyword, so **`Appointable`** is an abstract class that only declares `scheduleAppointment()`, `cancelAppointment()`, `hasScheduledAppointment()` and `getAppointmentDetails()`. It holds no data. `EmergencyPatient` deliberately does not implement it, because a trauma case is never booked in advance — and since the database routes every booking through `Appointable`, that rule is enforced by the type system rather than by a test on the category name. A second interface, **`Billable`**, is implemented at the same time through the intermediate abstract class `ScheduledPatient`, showing that a class may implement more than one interface.

**Composition.** A patient owns its appointments in a `std::vector<Appointment>`, so they are created and destroyed together, and `ClinicDatabase` owns the records in a `std::vector<std::unique_ptr<PatientRecord>>`, so each record is freed exactly once. The fields of each category are private and reached through an accessor; the five fields every patient shares are protected.
## 4. Data Structures Used

| Structure | Where | Why this one |
|---|---|---|
| `std::vector` | the patient roster; the appointments of each patient | O(1) append, and the list grows and shrinks all day |
| `std::queue` | the consultation waiting room | First-in first-out: the patient who joined first is served first |
| `std::stack` | the administrative undo history | Last-in first-out, exactly what "undo the last step" needs |
| `PatientRecord* recentPatients[10]` | the recently accessed cache | A fixed-size array of ten slots, oldest at the front and newest at the back; a cache hit moves a record to the newest place and the record used longest ago makes room when it is full |

## 5. Searching Algorithms Used

All five searches are written by hand. The program never calls `std::find`, `std::binary_search` or any other library search.

1. **Linear search (O(n))** — `linearSearchByID` scans the roster and returns the position of the patient whose ID is *exactly* the text typed, in a list of any order.
2. **Linear keyword search (O(n))** — `linearSearchByText` finds any patient whose ID *or* name holds the text, ignoring letter case, so "GEN" finds every general outpatient.
3. **Binary search (O(log n))** — `binarySearchByID` halves the search space on every step. It needs a sorted list, so the panel makes a sorted copy before calling it, which guarantees the precondition.
4. **Recursive search** — `recursiveSearchByName` calls itself with the next position until it runs off the end of the list.
5. **Category search (O(n))** — `searchByCategory` returns every record of one category.

The routine is chosen from a combo box, so the search that runs matches the state of the data.

## 6. Sorting Algorithms Used

Both sorts are written by hand as well; the program never calls `std::sort` or `std::stable_sort`.

1. **Selection sort (O(n²))** — finds the smallest remaining record and does one swap per pass, so it performs at most n − 1 swaps, sorts in place and needs no extra memory.
2. **Merge sort (O(n log n))** — splits the list in half and rejoins it. The merge takes the record of the left half first unless the right one is strictly smaller, which makes the sort **stable**: records with the same key keep the order they had.

Records are ordered by patient ID, patient name or patient age. Names and IDs are compared without regard to letter case, and the sort and the binary search share that one comparison rule, so the list stays sorted the way the binary search expects.

## 7. Recursive Algorithm Used

`recursiveSearchByName` walks the records one position at a time. The base case is reached when the position runs off the end of the list; the recursive case tries the next position and joins what it found. The recursion solves a genuine subproblem — the same search on the remainder of the list — rather than being a loop dressed as recursion. Merge sort recurses in the same way: it splits the list and defers to itself on each half.

## 8. GUI Design

The interface is built in Qt Widgets and split into six tabs, one class per tab: Patient Records, Appointments, Waiting Queue, Search & Sort, Clinic Reports and Administration. The Administration tab is the dashboard of the program: it shows the clinic summary and the records used most recently, and it carries the undo, save, load and new-database actions.

- **Events.** Button clicks, menu actions, combo box changes and text field changes all go through signals and slots. The File menu holds New, Save as…, Load… and Exit; the Edit menu holds Undo last operation and Quick search.
- **Layouts.** No pixel positioning is used. `QFormLayout` keeps a label with its field, and `QHBoxLayout` and `QVBoxLayout` divide the panels into rows and columns.
- **Advanced techniques.** The form fields change with the chosen patient type, file dialogs choose the import and export files, every invalid entry raises a message naming the field, and shortcuts cover Ctrl+N, Ctrl+S, Ctrl+O, Ctrl+Z, Ctrl+F and Ctrl+Q.

## 9. Exception Handling Approach

`ClinicException` derives from `std::exception` and overrides `what()`. Four derived types name the failure precisely: `DuplicateIDException` for an ID already registered, `InvalidPatientDataException` for an age outside 0–120 or a blank name, `InvalidAppointmentException` for a booking without a date or an unregistered patient, and `FileIOException` for a file that cannot be opened, read or written.

Every panel wraps the work it asks the controller to do in `try` / `catch` and shows the message as a dialog, so a mistake never closes the program. There is also a last line of defence: the application class overrides `notify()`, the one method every event passes through, so an exception that escapes any slot is caught and shown rather than ending the program. Rows in a data file that cannot be read are skipped and listed, and a save writes to a temporary file first, so a failed write cannot destroy the data already on disk.
## 10. Algorithms Chosen and Reasons

- **Two sorts, not one.** Selection sort suits the handful of records a small clinic holds, because it makes the fewest swaps and needs no scratch space; merge sort is there for a large import, because it stays at O(n log n) whatever the order of the data and keeps equal records in order.
- **Binary search with a defensive copy.** Binary search is only correct on a sorted list, so a sorted copy is made before it is called, and the panel works with the records it finds and not with their positions.
- **One comparison rule everywhere.** The search ignores letter case, so the sort and the binary search do too.
- **One cache order.** Oldest at the front and newest at the back, and a cache hit records the access.

## 11. Challenges Encountered

1. **One class that did too much.** The panels first called the database directly, so every rule of the clinic ended up in `ClinicDatabase`, which grew to almost 500 lines and mixed the data handling with the business logic — the very thing the brief forbids.
2. **The binary search showed the wrong patient.** The search returned a position, and the panel used that position in the *unsorted* roster.
3. **The cache did not behave like a cache.** Three different ideas of the order were used at once, so the panel showed the records in registration order rather than use order.
4. **A failed save could destroy the data that was already saved.** The file was opened for writing straight away, so the stream emptied it before the writing had begun.
5. **Loading a file threw the appointments away.** The save asked for two files but the load asked for one.
6. **One mistake could end the program.** Qt does not catch an exception that leaves a slot.

## 12. How the Challenges Were Resolved

1. The rules were moved up into `ClinicManager` and `AppointmentController`, and the panels were repointed at the manager. `ClinicDatabase` now only stores and finds, and no file in `gui/` refers to it.
2. The search returns the records that matched, not their positions, so the record shown is always the record that was found.
3. One order is used everywhere: a record used again moves to the newest place and a new record is added at the back.
4. The records are written to a temporary file first, and the real file is only replaced when the writing has finished.
5. The load asks for the appointment file as well, offers the file next to the patient file when the staff cancels, and asks before the clinic is replaced.
6. The application class in `gui_main.cpp` overrides `notify()`, so anything that escapes a slot is caught and shown as a message.
---

*The complete source code is listed in the Source Code section of this document.*
