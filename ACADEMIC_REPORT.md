# Academic Project Report

**Course:** COS202 Programming II  
**Project Title:** Smart Clinic Appointment & Patient Management System (SCAPMS)  
**Author / Student:** Davies Johnson  
**Language & Framework:** C++17, Qt 6.5.3 (x64)  
**IDE & Build Environment:** Visual Studio 2022 MSVC Build Tools / Visual Studio Code  

---

## 1. Project Overview & Problem Statement

A clinic still running on paper or a spreadsheet ends up with double bookings, a waiting room nobody can keep track of, deleted records with no way back, and no way to see how the day is going. This project fixes those problems with a desktop application.

The **Smart Clinic Appointment & Patient Management System (SCAPMS)** is a desktop application for running an outpatient clinic. It handles:
1. Patient demographic registration and categorization.
2. Advance consultation scheduling and status tracking.
3. First-In, First-Out (FIFO) consultation waiting queue coordination.
4. Fast manual searching and multi-attribute record sorting.
5. Administrative transaction history with instant rollback (Undo).
6. File persistence via structured Comma-Separated Values (CSV).
7. Diagnostic reporting of clinic attendance trends.

## 2. Object-Oriented System Architecture

The code is split into four layers - Model, Controller, GUI and Utilities - and each layer has
its own C++ namespace (`scapms::model`, `scapms::controller`, `scapms::gui`, `scapms::utils`).
Every class that has behaviour is written as a header file with the declarations and a source file
with the implementation; the two pure interfaces and the small inline helper headers are header
only by design. The Model, Controller and Utility layers do not depend on Qt, so the same code
also runs in the console demonstration.

### 2.0 Separation of Responsibilities (Requirement 2)

The program is built as three layers that only ever depend on the layer below them:

```
   GUI layer          PatientPanel, AppointmentPanel, QueuePanel,
      |                SearchPanel, ReportPanel, AdminPanel, MainWindow
      |  asks the controller to do the work
      v
   Controller layer    ClinicManager, AppointmentController,
      |                SearchEngine, SortEngine, ReportBuilder
      |  applies the rules of the clinic, then stores the result
      v
   Model layer         PatientRecord and its subclasses, Appointment,
                       AdminAction, ClinicDatabase
```

The rule that makes the separation real is that **the screens never call the database directly**.
Every panel holds a `ClinicManager&` and asks it to do the work, in the same way that the console
demonstration does. Before this restructure the panels called `ClinicDatabase` themselves, which
meant the business rules and the data handling had ended up in one class of 488 lines. The rules
now live in the controller layer:

- **`ClinicManager`** is the single point of contact for the GUI. It owns the database and the
  appointment controller, and it applies the rules of a patient record (the age has to be between
  0 and 120, the name may not be empty, the identifier may only be used once).
- **`AppointmentController`** holds the rules of a booking (the date and the time have to be
  written in the format of the clinic, the patient has to be registered, and the category of the
  patient has to be one that can be booked in advance).
- **`ClinicDatabase`** now only stores and finds. The few exceptions it still raises are the ones
  that follow from the shape of the data itself, such as an identifier that is already in use.

The classes named in the brief are all present in their own layer: `ClinicManager`,
`SearchEngine` and `AppointmentController` in the Controller, `MainWindow`, `PatientPanel`,
`AppointmentPanel`, `AdminPanel` and `SearchPanel` in the GUI, `IDGenerator` and `FileHandler` in
the Utilities, and the patient classes and `Appointment` in the Model.

The **`AdminPanel`** is a sixth tab that holds the operations that belong to the person in charge
of the clinic rather than to the treatment of a patient: undoing the last step, saving and loading
the data, and starting a new day with an empty database. It also shows the live dashboard of the
clinic and the records that were used most recently, which are the ten records held in the fixed
size cache.

### 2.1 Abstract Base Class & Inheritance Hierarchy
At the core of the clinical model is the abstract base class `PatientRecord`
(`model/PatientRecord.h`):
- **Encapsulated Attributes:** `patientID`, `name`, `age`, `contactNumber` and `patientType`.
- **Pure Virtual Functions (`= 0`):** `getTypeLabel()`, `getPatientReport()`, `getSpecialDetail()`
  and `clone()`. Each derived class has to provide its own version of them.
- **Concrete Methods:** `displayPatient()` and `restoreFrom()` are implemented in the base class and
  shared by every category. `displayPatient()` writes the report that the category produced, so it
  is also the method that the console demonstration calls through a base-class pointer.
- **Virtual Destructor:** Ensures safe polymorphic deallocation through base pointers without
  resource leakage.

Three specialized subclasses inherit from `PatientRecord`:
1. `GeneralPatient`: Represents standard outpatient visits. Encapsulates a `consultationFee`
   attribute, which is quoted in naira.
2. `EmergencyPatient`: Models urgent trauma admissions. Encapsulates an urgency `severityLevel`
   (1-3) and a qualitative `triageNote`.
3. `FollowUpPatient`: Facilitates continuity of care. Encapsulates a `previousVisitDate` and
   `assignedDoctor`.

### 2.2 Interface Design (`Appointable` and `Billable`)
C++ does not possess a native `interface` keyword. Consequently, abstract interfaces were constructed as pure abstract classes possessing solely pure virtual functions and a virtual destructor:
```cpp
class Appointable {
public:
    virtual ~Appointable() = default;
    virtual bool scheduleAppointment(const model::Appointment& appointment) = 0;
    virtual bool cancelAppointment() = 0;
    virtual bool cancelAppointment(const std::string& appointmentID) = 0;
    virtual bool hasScheduledAppointment() const = 0;
    virtual std::string getAppointmentDetails() const = 0;
};
```
`Appointable` also demonstrates **compile-time polymorphism** through method overloading: the two
`cancelAppointment()` methods share a name and differ only in their parameter list, so the choice
between cancelling the next booking and cancelling a named one is made by the compiler.

A second interface, `Billable` (`model/Billable.h`), declares `getBillAmount()` and
`describeBill()`. The intermediate abstract class `ScheduledPatient` inherits from the base class
**and from both interfaces at the same time**, which demonstrates that a class may implement more
than one interface:
```cpp
class ScheduledPatient : public PatientRecord, public Appointable, public Billable { ... };
```
`GeneralPatient` and `FollowUpPatient` therefore answer both interfaces, while `GeneralPatient`
overrides the billing methods with its consultation fee. The shared booking code is written once in
`ScheduledPatient`, so the two subclasses only add their own category fields.

By clinical logic, `EmergencyPatient` deliberately omits `Appointable`, reflecting hospital
protocol where trauma cases cannot be scheduled in advance. Because the database routes every
booking through the `Appointable` interface, this rule is enforced by the type system rather than
by a check on the category name: an attempt to book an emergency case is rejected with a
`InvalidAppointmentException`, and the emergency case reaches the doctor through the waiting room
instead.

### 2.3 Polymorphism & Composition
- **Runtime Polymorphism:** All patient records are managed through base-class pointers
  (`PatientRecord*`). Virtual function dispatch dynamically routes calls to derived subclass
  implementations via internal virtual method tables (VTables). The console demonstration calls
  `displayPatient()` through a `PatientRecord*`, and the graphical panel builds the same report
  through `getPatientReport()`, so the polymorphic call is exercised by both programs.
- **Composition ("HAS-A"):** A patient owns its appointments. `PatientRecord` holds a
  `std::vector<Appointment> appointments` collection, so a patient is associated with one or more
  appointments and the appointments are created and destroyed together with their owner. The
  `ClinicDatabase` owns the patient records in a `std::vector<std::unique_ptr<PatientRecord>>` and
  gathers the appointments of all patients on demand for the schedule table and the reports.
- **Ownership:** The database owns the live records and deletes them in its destructor. The undo
  stack owns one *copy* of the record for each administrative action, so the stack and the roster
  never share the same memory. The undo entry holds the copy in a `std::unique_ptr`, which makes it
  movable but not copyable, and frees the copy automatically when the entry is destroyed.

## 3. Data Structures & Justification

| Data Structure | C++ Implementation | Complexity | Clinical Justification in SCAPMS |
|---|---|---|---|
| **Vector** | `std::vector<std::unique_ptr<PatientRecord>>` | O(1) amortized append | Manages the dynamic list of registered patients. The unique pointers keep the ownership of each record explicit, so a record is freed exactly once. |
| **Vector** | `std::vector<Appointment>` | O(1) amortized append | The appointments a patient holds. The patient owns them, so they are created and destroyed together with the record. |
| **Queue** | `std::queue<PatientRecord*>` | O(1) push / pop | Enforces First-In, First-Out (FIFO) triage order in the consultation waiting room. |
| **Stack** | `std::stack<AdminAction>` | O(1) push / pop | Powers the administrative **Undo** engine (LIFO). Accidental record deletions or modifications are popped and reversed in reverse chronological order. |
| **Array** | `PatientRecord* recentPatients[10]` | O(1) indexed access | A fixed-size 10-slot cache storing the Most Recently Accessed records, reducing database scan frequency for active patients. |

## 4. Manual Algorithm Implementation

The brief rules out the STL algorithms, so every search and sort below is written by hand:

### 4.1 Searching Algorithms
1. **Linear Search (O(N)):** Implemented in `SearchEngine::linearSearchByID`. Scans arbitrary, unsorted rosters element-by-element. Used when records have not undergone pre-sorting.
2. **Binary Search (O(log N)):** Implemented in `SearchEngine::binarySearchByID`. Operates on sorted rosters by calculating the midpoint `mid = low + (high - low) / 2` and halving the search space each cycle. Reduces search time dramatically from 1,000 operations to ~10 operations.
3. **Recursive Search:** Implemented in `SearchEngine::recursiveSearchByName`. Solves patient lookup by recursively advancing an index pointer until hitting either base case (match located or end of array reached).
4. **Category Search:** Implemented in `SearchEngine::searchByCategory`. Returns every record of a category, which is what the clinic staff needs when they want to see, for example, all emergency cases.

### 4.2 Sorting Algorithms
1. **Selection Sort (O(N^2)):** Iteratively identifies the minimum element in the unsorted partition and performs a single swap per outer iteration. Highly suitable for small datasets where write operations must be minimized.
2. **Merge Sort (O(N log N)):** A divide-and-conquer algorithm that recursively bisects the patient vector into singletons and merges them into sorted temporary buffers. Guarantees optimal O(N log N) execution regardless of initial data distribution. The merge takes the record of the left half first when both records have the same key, so the sort is **stable** and equal records keep the order they had.
3. **Multi-Field Comparator:** Sorting algorithms accept an abstract criteria parameter (`SortField::PatientID`, `SortField::PatientName`, `SortField::PatientAge`), permitting dynamic user-driven reordering from the GUI.

## 5. Graphical User Interface (GUI) & Event Architecture

The graphical frontend was built natively using **Qt 6.5.3 (C++17)**:
- **Layout Management (Req. 8):** Hardcoded pixel coordinates were strictly avoided. Responsive layouts adapt dynamically to window resizing using `QVBoxLayout`, `QHBoxLayout`, and `QFormLayout`.
- **Event-Driven Programming (Req. 6):** User actions trigger Qt Signals connected to C++ Member Slots.
- **Multi-Panel Navigation (Req. 7):** Organized via a central `QTabWidget` containing:
  - *Panel 1:* Patient Registration form and color-coded Roster table (`gui/PatientPanel.cpp`).
  - *Panel 2:* Appointment booking form and schedule grid (`gui/AppointmentPanel.cpp`).
  - *Panel 3:* Waiting room line manager with FIFO calling dialogs (`gui/QueuePanel.cpp`).
  - *Panel 4:* Search and sort engine with real-time algorithm selection dropdowns (`gui/SearchPanel.cpp`).
  - *Panel 5:* Clinic Reports generator (`gui/ReportPanel.cpp`).
  - *Panel 6:* Administration panel with the administrative steps, the live dashboard and the recently accessed records (`gui/AdminPanel.cpp`).
- **Separation of concerns (Req. 2):** Every tab is its own panel class that owns its widgets and
  talks to the `ClinicManager`, never to the database. `gui/MainWindow.cpp` only builds the tabs,
  the menu bar and the status bar, and it refreshes the panels when one of them reports a change.
- **Advanced GUI Techniques (Req. 9):**
  - Custom table cell presentation using soft colors (`#ffebee` for Emergency, `#e8f5e9` for General, `#e3f2fd` for Follow-Up).
  - Fields that appear and disappear with the selected patient type.
  - Background `QTimer` maintaining live clock synchronization and queue size updates.
  - Standard file dialog integration via `QFileDialog` for custom file import/export.
  - Accessible keyboard shortcuts (`Ctrl+S`, `Ctrl+O`, `Ctrl+Z`, `Ctrl+F`, `Ctrl+Q`).

## 6. Exception Handling

A custom exception class hierarchy inheriting from standard `std::exception` guarantees error tolerance without abrupt application termination:
- `ClinicException`: Base exception class providing overridden `what() const noexcept`.
- `DuplicateIDException`: Thrown when attempting to register an ID already present in the database.
- `InvalidPatientDataException`: Thrown for illegal ages (< 0 or > 120) or blank mandatory name fields.
- `InvalidAppointmentException`: Thrown for an appointment without an ID, patient, date or time, for a duplicated appointment ID and for a patient that is not registered.
- `FileIOException`: Thrown during file stream access failures.

All GUI event slots wrap database mutations inside `try { ... } catch (const ClinicException& e) { ... }` blocks, presenting descriptive `QMessageBox::critical` alerts to users. Rows in a data file that cannot be read are skipped and listed for the user instead of silently turning into empty records.

## 7. Challenges and Fixes

1. **Memory Management with Polymorphic Pointers:**
   - *Challenge:* Storing polymorphic base pointers in a collection risks memory leaks when records are deleted or when the database is cleared.
   - *Resolution:* The database owns every live record in a `std::vector<std::unique_ptr<PatientRecord>>` and frees them in its destructor. Each undo entry owns its own **copy** of the record, held in a `std::unique_ptr`, so the undo stack and the roster never share memory and no record is freed twice. Because the undo entry owns a record, it follows the rule of five and can be moved but not copied.
2. **A Partially Applied Edit:**
   - *Challenge:* The edit form writes the name, the age and the category fields in turn. If a category field turns out to be invalid, the record was left half changed, and no undo entry had been created for it.
   - *Resolution:* The edit is now tried out on a **copy** of the record first. Only when the copy has passed every validation is the undo entry created and the values copied onto the live record, so an invalid entry can never leave the record in a mixed state.
3. **The Order of Evaluation of Function Arguments:**
   - *Challenge:* The undo entry was created in a single call that both read the identifier of the record and moved the snapshot into the entry. The C++ standard does not fix the order in which the arguments of a call are evaluated, and the compiler evaluated the move first, which emptied the pointer before the identifier was read.
   - *Resolution:* The identifier is read into a local variable **before** the entry is created, so the move is never written in the same call as the reads.
4. **Binary Search Precondition:**
   - *Challenge:* Binary search produces incorrect results or infinite loops if executed on unsorted vectors.
   - *Resolution:* The controller sorts a defensive copy with `SortEngine::mergeSort` before executing binary search.
5. **Stability of the Merge Sort:**
   - *Challenge:* The first version of the merge took the record of the right half when two records had the same key, which silently changed the order of equal records.
   - *Resolution:* The merge now takes the record of the **left** half unless the record of the right half is strictly smaller, which is the standard rule for a stable merge sort.
6. **MOC (Meta-Object Compiler) Integration in Multi-File Builds:**
   - *Challenge:* Compiling Qt classes declaring `Q_OBJECT` with MSVC command-line tools requires generating and compiling `moc_*.cpp` files.
   - *Resolution:* Configured a clean Qt Project (`SCAPMS.pro`) and automated script (`build_gui.bat`) running `qmake` and `nmake`.
7. **One Class That Did Too Much:**
   - *Challenge:* The first version of the program had the graphical panels calling the database directly. Every rule of the clinic - a valid age, a date that is written correctly, a patient that may be booked - was therefore written inside `ClinicDatabase`, which grew to 488 lines and mixed the data handling with the business logic. That is exactly what requirement 2 forbids.
   - *Resolution:* The rules were moved up into a new `ClinicManager` and an `AppointmentController` in the controller layer, and the panels were repointed from the database to the manager. `ClinicDatabase` now only stores and finds, and no file in the `gui/` folder refers to it any more.
8. **Code That Was Written But Never Used:**
   - *Challenge:* The dashboard summary and the list of the recently accessed records were written for the reports but nothing ever asked for them, so they were dead code.
   - *Resolution:* Both were given a real purpose in the new `AdminPanel`, which shows the live dashboard of the clinic and the ten records of the fixed size cache.

## 8. Conclusion

The Smart Clinic Appointment & Patient Management System is a working C++ application that covers the lab requirements: object-oriented design, manual algorithms, the required data structures, and a Qt interface.
