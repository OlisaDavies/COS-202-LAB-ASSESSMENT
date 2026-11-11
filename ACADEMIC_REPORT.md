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
Every class is written as a header file with the declarations and a source file with the
implementation. The Model, Controller and Utility layers do not depend on Qt, so the same code
also runs in the console demonstration.

### 2.1 Abstract Base Class & Inheritance Hierarchy
At the core of the clinical model is the abstract base class `PatientRecord`
(`model/PatientRecord.h`):
- **Encapsulated Attributes:** `patientID`, `name`, `age`, `contactNumber` and `patientType`.
- **Pure Virtual Functions (`= 0`):** `displayPatient()`, `getSpecialDetail()`, `getTypeLabel()`
  and `clone()`. Each derived class has to provide its own version of them.
- **Virtual Destructor:** Ensures safe polymorphic deallocation through base pointers without
  resource leakage.

Three specialized subclasses inherit from `PatientRecord`:
1. `GeneralPatient`: Represents standard outpatient visits. Encapsulates a `consultationFee`
   attribute, which is quoted in naira.
2. `EmergencyPatient`: Models urgent trauma admissions. Encapsulates an urgency `severityLevel`
   (1-3) and a qualitative `triageNote`.
3. `FollowUpPatient`: Facilitates continuity of care. Encapsulates a `previousVisitDate` and
   `assignedDoctor`.

### 2.2 Interface Design (`Appointable`)
C++ does not possess a native `interface` keyword. Consequently, an abstract interface was constructed as a pure abstract class possessing solely pure virtual functions and a virtual destructor:
```cpp
class Appointable {
public:
    virtual ~Appointable() = default;
    virtual bool scheduleAppointment(const std::string& date, const std::string& time) = 0;
    virtual bool cancelAppointment() = 0;
    virtual bool hasScheduledAppointment() const = 0;
    virtual std::string getAppointmentDetails() const = 0;
};
```
`GeneralPatient` and `FollowUpPatient` are both `Appointable`, and because the two of them would
otherwise repeat the same booking code, the shared part is written once in the abstract class
`ScheduledPatient` (`model/ScheduledPatient.h`), which inherits from `PatientRecord` and
implements `Appointable`. The two subclasses then only add their own category fields.

By clinical logic, `EmergencyPatient` deliberately omits `Appointable`, reflecting hospital
protocol where trauma cases cannot be scheduled in advance.

### 2.3 Polymorphism & Composition
- **Runtime Polymorphism:** All patient records are managed through base-class pointers
  (`PatientRecord*`). Virtual function dispatch dynamically routes calls to derived subclass
  implementations via internal virtual method tables (VTables).
- **Composition ("HAS-A"):** The `ClinicDatabase` class encapsulates dynamic collections
  (`std::vector<PatientRecord*>`, `std::vector<Appointment>`). The `Appointment` model links each
  appointment to a patient through `patientID`, so one patient can hold several appointments.
- **Ownership:** The database owns the live records and deletes them in its destructor. The undo
  stack owns one *copy* of the record for each administrative action, so the stack and the roster
  never share the same memory.

## 3. Data Structures & Justification

| Data Structure | C++ Implementation | Complexity | Clinical Justification in SCAPMS |
|---|---|---|---|
| **Vector** | `std::vector<PatientRecord*>` | O(1) amortized append | Manages the dynamic list of registered patients. Guarantees contiguous cache memory for rapid linear scanning. |
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
- **Separation of concerns (Req. 2):** Every tab is its own panel class that owns its widgets and
  talks to the database. `gui/MainWindow.cpp` only builds the tabs, the menu bar and the status
  bar, and it refreshes the panels when one of them reports a change.
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
   - *Challenge:* Storing polymorphic base pointers in `std::vector<PatientRecord*>` risks memory leaks when records are deleted or when the database is cleared.
   - *Resolution:* The database owns every live record and frees them in its destructor. Each undo entry owns its own **copy** of the record, held in a `std::unique_ptr`, so the undo stack and the roster never share memory and no record is freed twice.
2. **Binary Search Precondition:**
   - *Challenge:* Binary search produces incorrect results or infinite loops if executed on unsorted vectors.
   - *Resolution:* The controller sorts a defensive copy with `SortEngine::mergeSort` before executing binary search.
3. **Stability of the Merge Sort:**
   - *Challenge:* The first version of the merge took the record of the right half when two records had the same key, which silently changed the order of equal records.
   - *Resolution:* The merge now takes the record of the **left** half unless the record of the right half is strictly smaller, which is the standard rule for a stable merge sort.
4. **MOC (Meta-Object Compiler) Integration in Multi-File Builds:**
   - *Challenge:* Compiling Qt classes declaring `Q_OBJECT` with MSVC command-line tools requires generating and compiling `moc_*.cpp` files.
   - *Resolution:* Configured a clean Qt Project (`SCAPMS.pro`) and automated script (`build_gui.bat`) running `qmake` and `nmake`.

## 8. Conclusion

The Smart Clinic Appointment & Patient Management System is a working C++ application that covers the lab requirements: object-oriented design, manual algorithms, the required data structures, and a Qt interface.
