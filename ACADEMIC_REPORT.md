# Academic Project Report

**Course:** COS202 Programming II  
**Project Title:** Smart Clinic Appointment & Patient Management System (SCAPMS)  
**Author / Student:** OlisaEmeka Davies Chinedu-Eze  
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
size cache. The dashboard is built out of real widgets rather than out of one block of text: a
table of the six figures of the clinic, and a table of the three categories with the priority that
goes with each of them.

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
| **Array** | `PatientRecord* recentPatients[10]` | O(1) indexed access | A fixed-size 10-slot cache storing the Most Recently Accessed records, reducing database scan frequency for active patients. The array is held from the oldest record at the front to the newest at the back; a record that is used again moves to the newest place, and when the cache is full the record that was used the longest time ago is the one that is dropped. |

## 4. Manual Algorithm Implementation

The brief rules out the STL algorithms, so every search and sort below is written by hand:

### 4.1 Searching Algorithms
1. **Linear Search (O(N)):** Implemented in `SearchEngine::linearSearchByText`. Scans arbitrary, unsorted rosters element-by-element. It matches the identifier **and** the name, without regard to letter case, and a part of either is enough - so the staff can type "chi" and find "Chioma Eze". It returns **every** match, not only the first one.
2. **Binary Search (O(log N)):** Implemented in `SearchEngine::binarySearchByID`. Operates on sorted rosters by calculating the midpoint `mid = low + (high - low) / 2` and halving the search space each cycle. The GUI makes a sorted copy of the records before calling it, so the precondition always holds.
3. **Recursive Search:** Implemented in `SearchEngine::recursiveSearchByName`. Solves patient lookup by recursively advancing an index pointer until hitting either base case (match located or end of array reached). It collects every patient whose name holds the text.
4. **Category Search:** Implemented in `SearchEngine::searchByCategory`. Returns every record of a category, which is what the clinic staff needs when they want to see, for example, all emergency cases.

### 4.2 Sorting Algorithms
1. **Selection Sort (O(N^2)):** Iteratively identifies the minimum element in the unsorted partition and performs a single swap per outer iteration. Highly suitable for small datasets where write operations must be minimized.
2. **Merge Sort (O(N log N)):** A divide-and-conquer algorithm that recursively bisects the patient vector into singletons and merges them into sorted temporary buffers. Guarantees optimal O(N log N) execution regardless of initial data distribution. The merge takes the record of the left half first when both records have the same key, so the sort is **stable** and equal records keep the order they had.
3. **Multi-Field Comparator:** Sorting algorithms accept an abstract criteria parameter (`SortField::PatientID`, `SortField::PatientName`, `SortField::PatientAge`), permitting dynamic user-driven reordering from the GUI.
4. **Case-Insensitive Comparison:** The name and the identifier are compared without regard to letter case, using the same rule as the search. A name that was typed in small letters therefore sits in its proper place in the list instead of being pushed to the end by the ASCII order, and the binary search keeps working, because it uses the very same comparison that the sort uses.

## 5. Graphical User Interface (GUI) & Event Architecture

The graphical frontend was built natively using **Qt 6.5.3 (C++17)**:
- **Layout Management (Req. 8):** Hardcoded pixel coordinates were strictly avoided. Responsive layouts adapt dynamically to window resizing using `QVBoxLayout`, `QHBoxLayout`, `QFormLayout` and `QGridLayout`. The forms use `QFormLayout`, so a label and its field stay together and the form grows with its longest label; the panels use `QHBoxLayout` and `QVBoxLayout`, so the columns and the rows share the space that is left; and the three tables of figures and of categories and priorities use `QGridLayout`, so the entries line up under each other in both directions.
- **Event-Driven Programming (Req. 6):** User actions trigger Qt Signals connected to C++ Member Slots.
  - *Button events (6.a):* Add, Edit, Delete, Undo, Book, Mark as Completed, Cancel, Search, Sort, Save, Load and New Database are all wired to a slot.
  - *Menu events (6.b):* The File menu holds **New**, **Save as...** (`Ctrl+S`), **Load...** (`Ctrl+O`) and **Exit** (`Ctrl+Q`); the Edit menu holds **Undo last operation** (`Ctrl+Z`) and **Quick search** (`Ctrl+F`); the Help menu holds **About**.
  - *Text field events (6.c):* the appointment identifier field records that the staff typed it, so that the free identifier is not written over their work; and the patient identifier field answers while it is being typed, telling the staff whether the identifier is free or already in the clinic.
  - *Timer events (6.d):* a one-second `QTimer` keeps the clock and the live numbers of the clinic in the status bar, shows a reminder of the bookings that are still pending, and refreshes the statistics of the dashboard.
- **Multi-Panel Navigation (Req. 7):** Organized via a central `QTabWidget` containing:
  - *Panel 1:* Patient Registration form and color-coded Roster table (`gui/PatientPanel.cpp`).
  - *Panel 2:* Appointment booking form and schedule grid (`gui/AppointmentPanel.cpp`).
  - *Panel 3:* Waiting room line manager with FIFO calling dialogs (`gui/QueuePanel.cpp`).
  - *Panel 4:* Search and sort engine with real-time algorithm selection dropdowns (`gui/SearchPanel.cpp`).
  - *Panel 5:* Clinic Reports generator (`gui/ReportPanel.cpp`).
  - *Panel 6:* Administration panel with the administrative steps, the live dashboard of the clinic and the recently accessed records (`gui/AdminPanel.cpp`).
- **Separation of concerns (Req. 2):** Every tab is its own panel class that owns its widgets and
  talks to the `ClinicManager`, never to the database. `gui/MainWindow.cpp` only builds the tabs,
  the menu bar and the status bar, and it refreshes the panels when one of them reports a change.
- **Advanced GUI Techniques (Req. 9):**
  - Custom table cell presentation using soft colors (`#ffebee` for Emergency, `#e8f5e9` for General, `#e3f2fd` for Follow-Up), with a table of the categories and their priorities beside the roster and beside the dashboard.
  - Fields that appear and disappear with the selected patient type.
  - Background `QTimer` maintaining live clock synchronization, queue size updates, a reminder of the pending appointments and a live refresh of the dashboard statistics.
  - Standard file dialog integration via `QFileDialog` for custom file import/export.
  - Accessible keyboard shortcuts (`Ctrl+N`, `Ctrl+S`, `Ctrl+O`, `Ctrl+Z`, `Ctrl+F`, `Ctrl+Q`).
  - A live hint under the patient identifier field that says whether the identifier can be used.
  - A tooltip on every button, every field and every list, which says what the control does in one short sentence.

## 6. Exception Handling

A custom exception class hierarchy inheriting from standard `std::exception` guarantees error tolerance without abrupt application termination:
- `ClinicException`: Base exception class providing overridden `what() const noexcept`.
- `DuplicateIDException`: Thrown when attempting to register an ID already present in the database.
- `InvalidPatientDataException`: Thrown for illegal ages (< 0 or > 120) or blank mandatory name fields.
- `InvalidAppointmentException`: Thrown for an appointment without an ID, patient, date or time, for a duplicated appointment ID and for a patient that is not registered.
- `FileIOException`: Thrown during file stream access failures.

All GUI event slots wrap database mutations inside `try { ... } catch (const ClinicException& e) { ... }` blocks, presenting descriptive `QMessageBox::critical` alerts to users. Rows in a data file that cannot be read are skipped and listed for the user instead of silently turning into empty records.

**The safety net.** Qt does not catch an exception that leaves a slot: it travels out of the event loop and ends the program with *"terminate called after throwing an instance of ..."*. Because requirement 11 asks for a program that keeps running after a normal mistake, `gui_main.cpp` uses a small `QApplication` subclass whose `notify()` method catches anything that escapes an event and shows it to the staff as a message. The rules of the clinic are checked in the controller layer, so the panels usually never let an exception through, but the program still has a last line of defence.

**The rules are checked on every way in.** A record is checked when it is registered, when it is edited and when it is read from a file. The contact number rule, for example, was at first only applied on registration, so an edit could put an illegal contact number into a record. The edit path now runs the same `ClinicManager::validatePatient` check as the registration path.

## 7. Challenges and Fixes

1. **One Class That Did Too Much:**
   - *Challenge:* The graphical panels called the database directly, so every rule of the clinic - a valid age, a date written correctly, a patient that may be booked - ended up inside `ClinicDatabase`, which grew to 488 lines and mixed the data handling with the business logic.
   - *Resolution:* The rules were moved up into `ClinicManager` and `AppointmentController` in the controller layer, and the panels were repointed from the database to the manager. `ClinicDatabase` now only stores and finds.

2. **The Binary Search Showed The Wrong Patient:**
   - *Challenge:* The panel made a sorted copy of the records and searched in that copy. The routine returned the **place** of the record it found, and the panel then used that place to look the record up in the unsorted roster, so it showed a patient that had nothing to do with the search.
   - *Resolution:* The search returns the records that matched, not their positions, so the record shown is always the record that was found.

3. **The Cache Of Frequently Accessed Records Did Not Work:**
   - *Challenge:* The fixed-size array of ten records was written with three different ideas of the order at the same time, so it kept the order in which the records were *registered* rather than the order in which they were *used*. A cache hit did not even record the access.
   - *Resolution:* One order is used everywhere now: the array is held from the oldest record at the front to the newest at the back, using a record moves it to the newest place, and a new record is added at the back.

4. **A Failed Save Could Destroy The Data That Was Already Saved:**
   - *Challenge:* The file was opened for writing straight away, and an ofstream empties the file the moment it is opened. If the writing then failed, a half written file was left behind and the program still said "The data was written".
   - *Resolution:* The records are written to a temporary file first, and the real file is only replaced when the writing has finished, so a failed save leaves the old file untouched.

5. **Loading A File Could Silently Throw The Appointments Away:**
   - *Challenge:* The save asked for two file names, but the load asked for one and then looked for a file called "appointments.csv" next to the patient file. Appointments saved under any other name were silently lost, and loading emptied the whole clinic without asking.
   - *Resolution:* The load asks for the appointment file as well, offers the one next to the patient file when the staff cancels, and asks before the clinic is replaced.

6. **One Mistake Could End The Whole Program:**
   - *Challenge:* Qt does not catch an exception that leaves a slot. The exception travels out of the event loop and the program ends with "terminate called after throwing an instance of ...", and the windows disappear.
   - *Resolution:* The application class in gui_main.cpp overrides `notify()`, the one place every event passes through, so anything that escapes a slot is caught and shown as a message.

## 8. Conclusion

The Smart Clinic Appointment & Patient Management System is a working C++ application that covers the lab requirements: object-oriented design, manual algorithms, the required data structures, and a Qt interface.
