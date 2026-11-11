# Academic Project Report

**Course:** COS202 Programming II – Lab Assessment Project
**Project:** Smart Clinic Appointment & Patient Management System (SCAPMS)
**Student:** OlisaEmeka Davies Chinedu-Eze
**Language & Framework:** C++17, Qt 6.5.3 (Qt Widgets)
**Build Environment:** Visual Studio 2022 MSVC Build Tools, qmake

---

## 1. Project Description

A clinic running on paper records suffers double bookings, a waiting room nobody can track, and deleted records with no way back. SCAPMS is a desktop application that fixes those problems. It manages patients in three categories — general, emergency and follow-up — and lets a member of staff register, edit and delete a record; book, cancel and complete an appointment; manage a first-in first-out waiting queue; search and sort the records with the algorithm of their choice; undo the last administrative step; and read four clinic reports. All patient and appointment data is written to and read from CSV files, so the clinic survives between runs.

## 2. System Features

| Feature | Where it lives in the interface |
|---|---|
| Register, edit and delete a patient | Patient Records panel |
| Schedule, cancel and complete an appointment | Appointments panel |
| Waiting room: add a patient, call the next one | Waiting Queue panel |
| Search by ID, keyword, name or category; sort by ID, name or age | Search & Sort panel |
| Undo the last change to a patient or an appointment | Administration panel, or Ctrl+Z |
| Save and load the clinic files | Administration panel, or Ctrl+S / Ctrl+O |
| Four clinic reports | Clinic Reports panel |
| Live dashboard and the recently accessed records | Administration panel |

## 3. Class Hierarchy

The core of the model is the **abstract base class `PatientRecord`**, which holds the details every patient has in common: patient ID, name, age, contact number and category. It declares four pure virtual functions — `getTypeLabel()`, `getPatientReport()`, `getSpecialDetail()` and `clone()` — so it can never be created on its own. It carries concrete methods that every category shares, and its destructor is virtual, so deleting through a `PatientRecord*` frees the subclass.

**Three subclasses** give each category its own behaviour: **`GeneralPatient`** (a standard outpatient visit with a consultation fee), **`EmergencyPatient`** (an urgent case with a severity level 1–3 and a triage note), and **`FollowUpPatient`** (a returning patient with the date of the last visit and the assigned doctor).

**Interface.** C++ has no `interface` keyword, so **`Appointable`** is a pure abstract class that only declares `scheduleAppointment()`, `cancelAppointment()` (with and without an ID), `hasScheduledAppointment()` and `getAppointmentDetails()`. It holds no data, so any class that can be booked may implement it. `EmergencyPatient` deliberately does not: a trauma case is never booked in advance, and because the database routes every booking through `Appointable`, the rule is enforced by the type system rather than by a check on the category name. A second interface, **`Billable`**, is implemented at the same time through the intermediate abstract class `ScheduledPatient`, showing that a class may implement more than one interface.

**Composition and association.** A patient owns its appointments: `PatientRecord` holds a `std::vector<Appointment>`, so a patient is associated with one or more appointments and they are created and destroyed together. `ClinicDatabase` owns the records in a `std::vector<std::unique_ptr<PatientRecord>>`, so each record is freed exactly once.

**Encapsulation.** Every patient field is private and reached through an accessor. The rules of the clinic — a valid age, a unique ID, a date in the right format — are checked in the controller layer, so the screens and the data can never disagree about them.

## 4. Data Structures Used

| Structure | Where | Why this one |
|---|---|---|
| `std::vector` | the patient roster; the appointments of each patient | O(1) append, and the record list grows and shrinks all day |
| `std::queue` | the consultation waiting room | First-in first-out: the patient who joined first is served first |
| `std::stack` | the administrative undo history | Last-in first-out, exactly what "undo the last step" needs |
| `PatientRecord* recentPatients[10]` | the recently accessed cache | A fixed-size array of ten slots, held oldest at the front and newest at the back. Using a record moves it to the newest place; when it is full the record used longest ago makes room |

## 5. Searching Algorithms Used

Every search is written by hand; the program never calls `std::find` or `std::binary_search`.

1. **Linear search (O(n))** — `linearSearchByID` scans the roster and returns the position of the patient whose ID is *exactly* the text typed, in a list of any order.
2. **Linear keyword search (O(n))** — `linearSearchByText` finds any patient whose ID *or* name holds the text, ignoring letter case, so "GEN" finds every general outpatient.
3. **Binary search (O(log n))** — `binarySearchByID` halves the search space on each step. It needs a sorted list, so the panel makes a sorted copy before calling it, which guarantees the precondition.
4. **Recursive search** — `recursiveSearchByName` calls itself with the next position until it runs off the end of the list, collecting every patient whose name holds the text.
5. **Category search (O(n))** — `searchByCategory` returns every record of one category.

The routine is chosen from a combo box, so the search that runs matches the state of the data: binary search for a sorted list, linear search otherwise.

## 6. Sorting Algorithms Used

Both sorts are written by hand and chosen from a combo box.

1. **Selection sort (O(n²))** — finds the smallest remaining record and performs one swap per pass, so it does at most n − 1 swaps. It sorts in place and needs no extra memory, which suits the short lists of a clinic.
2. **Merge sort (O(n log n))** — splits the list in half and rejoins it. The merge takes the record of the left half first unless the record of the right half is strictly smaller, which makes the sort **stable**: records with the same key keep the order they had.

Records are ordered by patient ID, patient name or patient age. Names and IDs are compared without regard to letter case, so a name typed in small letters sits in its proper place rather than at the end. A label under the sort controls explains the chosen routine and why it fits, and the sort and the binary search share one comparison rule, so the list stays sorted the way the binary search expects.

## 7. Recursive Algorithm Used

`recursiveSearchByName` walks the records one position at a time. The base case is reached when the position runs off the end of the list; the recursive case tries the next position and joins what it found. The recursion solves a genuine subproblem — the same search on the remainder of the list — rather than being a loop dressed as recursion. Merge sort is recursive in the same way: it splits the list and defers to itself on each half.

## 8. GUI Design

The interface is built in Qt Widgets and split into six tabs, one class per tab: Patient Records, Appointments, Waiting Queue, Search & Sort, Clinic Reports and Administration.

- **Events.** Button clicks, menu actions, combo box changes, text field changes and a one-second timer all go through signals and slots. The File menu holds New, Save as…, Load… and Exit; the Edit menu holds Undo last operation and Quick search.
- **Layouts.** No pixel positioning is used. `QFormLayout` keeps a label with its field, `QHBoxLayout` and `QVBoxLayout` divide the panels into rows and columns, and `QGridLayout` lines up the tables of figures and of category priorities. Using four techniques rather than one means each part of the window is laid out by the one that suits its shape.
- **Advanced techniques.** Rows are colour-coded by category (red for emergency, green for general, blue for follow-up) with a table of categories and priorities beside the roster; the form fields change with the chosen patient type; file dialogs choose the import and export files; the timer keeps the status bar and the dashboard live; every invalid entry raises a message naming the field; shortcuts cover Ctrl+N, Ctrl+S, Ctrl+O, Ctrl+Z, Ctrl+F and Ctrl+Q; and every control carries a tooltip.

## 9. Exception Handling Approach

`ClinicException`, the base of the hierarchy, derives from `std::exception` and overrides `what()`. Four derived types name the failure precisely: `DuplicateIDException` for an ID already registered, `InvalidPatientDataException` for an age outside 0–120 or a blank name, `InvalidAppointmentException` for a booking without a date or a patient that is not registered, and `FileIOException` for a file that cannot be opened, read or written.

Every panel wraps the work it asks the controller to do in `try` / `catch` and shows the message as a dialog, so a mistake never closes the program. There is also a last line of defence: the application class overrides `notify()`, the one method every event passes through, so an exception that escapes any slot is caught and shown rather than ending the program. Rows in a data file that cannot be read are skipped and listed, and a save writes to a temporary file first, so a failed write cannot destroy the data already on disk.

## 10. Algorithms Chosen and Reasons

- **Two sorts, not one.** Selection sort suits the handful of records a small clinic holds, because it makes the fewest swaps and needs no scratch space; merge sort is there for a large import, because it stays at O(n log n) whatever the order of the data and preserves the order of equal records.
- **Binary search with a defensive copy.** Binary search is only correct on a sorted list, so a sorted copy is made before it is called. The panel works with the records it finds and not with their positions, so the record shown is always the record that was found.
- **One comparison rule everywhere.** The search ignores letter case, so the sort and the binary search do too, which keeps the list sorted the way the binary search expects.
- **One cache order.** The recently accessed records use a single order — oldest at the front, newest at the back — and a cache hit records the access, so the panel really shows the most recently used records.

## 11. Challenges Encountered

1. **One class that did too much.** The panels first called the database directly, so every rule of the clinic ended up in `ClinicDatabase`, which grew to 488 lines and mixed the data handling with the business logic — the very thing the brief forbids.
2. **The binary search showed the wrong patient.** The search returned a position, and the panel used that position in the *unsorted* roster, so the wrong patient appeared.
3. **The cache did not behave like a cache.** Three different ideas of the order were used at once, so the panel showed the records in registration order rather than use order.
4. **A failed save could destroy the data that was already saved.** The file was opened for writing straight away, so an ofstream emptied it before the writing had even begun.
5. **Loading a file threw the appointments away.** The save asked for two files but the load asked for one, so appointments saved under another name were silently lost.
6. **One mistake could end the program.** Qt does not catch an exception that leaves a slot.

## 12. How the Challenges Were Resolved

1. The rules were moved up into `ClinicManager` and `AppointmentController` in the controller layer, and the panels were repointed at the manager. `ClinicDatabase` now only stores and finds, and no file in `gui/` refers to it.
2. The search returns the records that matched, not their positions, so the record shown is always the record that was found.
3. One order is used everywhere: a record used again moves to the newest place, a new record is added at the back, and a cache hit records the access.
4. The records are written to a temporary file first, and the real file is only replaced when the writing has finished, so a failed save leaves the old file untouched.
5. The load asks for the appointment file as well, offers the file next to the patient file when the staff cancels, and asks before the clinic is replaced.
6. The application class in `gui_main.cpp` overrides `notify()`, the one place every event passes through, so anything that escapes a slot is caught and shown as a message.

---

*The complete source code is at `https://github.com/OlisaDavies/COS-202-LAB-ASSESSMENT` and is also listed in the Source Code section of this document.*
