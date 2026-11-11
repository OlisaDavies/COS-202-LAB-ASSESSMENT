# Lecturer Defense Preparation Guide

**Course:** COS202 Programming II  
**Project:** Smart Clinic Appointment & Patient Management System (SCAPMS)  
**Student:** Davies Johnson  

This guide covers the questions you are likely to be asked in your project defense. Lecturers don't just want to see code that works - they want to verify that **you wrote it, understand every line, and know why every design choice was made**.

---

## 1. Executive Summary (The 60-Second Elevator Pitch)

When the lecturer asks: *"Tell me about your project in one minute"*, answer:

> *"Good day, Sir/Ma'am. My project is the **Smart Clinic Appointment & Patient Management System (SCAPMS)**. It is an interactive C++ desktop application built with Qt to digitalize university outpatient clinic workflows.
> 
> The system manages three categories of patients - General, Emergency, and Follow-Up - using an **Abstract Base Class (`PatientRecord`)** and an **Interface (`Appointable`)** demonstrating runtime polymorphism. 
> 
> For data storage and flow control, it implements four distinct data structures: **`std::vector`** for main patient storage, **`std::queue`** for a First-In, First-Out waiting room, **`std::stack`** for an administrative Undo rollback engine, and a **fixed-size array** for caching recently accessed records. 
> 
> All searching routines (Linear, Binary, Recursive, Category) and sorting algorithms (Selection Sort and Merge Sort) are **implemented manually from scratch**. The entire application is backed by CSV file persistence and a custom exception hierarchy and handles invalid input through the exception classes."*

---

## 2. Likely Lecturer Oral Defense Questions & Model Answers

### Question 1: "Why did you make `PatientRecord` an abstract class instead of a regular class?"
**Model Answer:**
> *"Because in a real hospital, nobody is just a generic 'Patient'. A patient who walks into a clinic is specifically a General Outpatient, an Emergency Case, or a Follow-Up Consultation. 
> 
> By declaring `getTypeLabel()`, `getPatientReport()`, `getSpecialDetail()` and `clone()` as **pure virtual functions (`= 0`)**, I made `PatientRecord` an **abstract class**. This prevents anyone from instantiating an incomplete generic patient object directly, while enforcing that every subclass must implement its own specific display and detail logic.
> 
> The class is not empty of behaviour, though. `displayPatient()` and `restoreFrom()` are **concrete methods** implemented in the base class and shared by all three categories. `displayPatient()` writes the report that the category produced, so it is also the method that proves the run-time polymorphism."*

---

### Question 2: "What is the difference between an Abstract Class and an Interface in C++?"
**Model Answer:**
> *"C++ does not have a native `interface` keyword like Java. In C++, an **Interface** is achieved using an abstract class that contains **no member variables** and **only pure virtual functions**. 
> 
> In my project, `PatientRecord` is an **Abstract Base Class** because it holds shared member variables (`name`, `age`, `patientID`) and common getters. In contrast, `Appointable` is a **pure Interface** because it only defines the operations for scheduling and cancelling appointments (`scheduleAppointment()`, `cancelAppointment()`), without storing any data variables.
> 
> I actually have **two** interfaces. The second one is `Billable`, with `getBillAmount()` and `describeBill()`. The point of having two is that a class can implement more than one interface: `ScheduledPatient` inherits from `PatientRecord`, from `Appointable` **and** from `Billable` at the same time, and `GeneralPatient` then overrides the billing methods with its own consultation fee."*

---

### Question 3: "Why does `GeneralPatient` inherit from `Appointable`, but `EmergencyPatient` does not?"
**Model Answer:**
> *"This was a deliberate clinical design decision. General and Follow-Up patients can book advance scheduled consultations with doctors. However, by medical protocol, Emergency trauma cases are rushed into the triage ward immediately and cannot be scheduled for future appointments. 
> 
> Therefore, using **Multiple Inheritance**, only `GeneralPatient` and `FollowUpPatient` implement the `Appointable` interface.
> 
> The important part is that this is not only a comment in the code - it is **enforced by the type system**. The booking goes through the `AppointmentController`, which routes it through the `Appointable` interface, so it checks with a `dynamic_cast` whether the patient answers that interface. An emergency patient does not, so the booking is refused with a clear `InvalidAppointmentException` that tells the staff to use the waiting room instead. The emergency case still reaches the doctor, but through the FIFO waiting queue."*

---

### Question 4: "What is `ScheduledPatient` and why did you add another class?"
**Model Answer:**
> *"Both `GeneralPatient` and `FollowUpPatient` have to answer the `Appointable` interface in exactly the same way, because a booking is a date, a time and a booking that can be cancelled. Instead of writing that code twice, I put it once in the abstract class **`ScheduledPatient`**, which inherits from `PatientRecord` and implements `Appointable`. The two subclasses then only add what makes them different: the consultation fee, and the previous visit with the assigned doctor. It removes duplicated code and it shows that I can design an inheritance hierarchy, not just copy a class three times.
> 
> `ScheduledPatient` is also where the second interface, `Billable`, is implemented, which is what gives one class two interfaces at the same time."*

---

### Question 4b: "How is an appointment connected to its patient?"
**Model Answer:**
> *"By **composition**. `PatientRecord` holds a `std::vector<Appointment> appointments`, so the patient owns the appointments that belong to it and a patient is genuinely associated with one or more appointments. The appointments are stored by value, so they are created and destroyed together with the record they belong to.
> 
> There is only **one** copy of the truth. Earlier I had a separate flat list of appointments in the database *and* a small cache inside each patient holding the current booking, and the two could disagree. Now the patient holds the appointments and everything else - the schedule table, the reports, the pending count - is derived from that single collection. `ClinicDatabase::getAllAppointments()` gathers them from all the patients when the screen needs the whole schedule."*

---

### Question 5: "Why did you implement both Selection Sort and Merge Sort? How do their time complexities compare?"
**Model Answer:**
> *"I implemented both to demonstrate the contrast between basic and optimal sorting paradigms:
> 
> 1. **Selection Sort** is an in-place O(N^2) algorithm. While its comparison count is quadratic, it performs at most N-1 swaps, which makes it useful when memory write operations are expensive on small datasets.
> 2. **Merge Sort** is a Divide-and-Conquer algorithm with an optimal O(N log N) time complexity in all cases (best, average, and worst). It recursively divides the patient vector in half and merges the sorted subsets. It is stable, so equal records keep their original order."*

---

### Question 6: "What is the golden rule / precondition before running Binary Search?"
**Model Answer:**
> *"The collection **must be sorted** prior to execution! If the records are not sorted, Binary Search will discard the wrong half of the array and fail to find the patient. 
> 
> In my GUI implementation, before executing `SearchEngine::binarySearchByID`, the program defensively sorts the records by Patient ID using Merge Sort on a copy of the list."*

---

### Question 7: "Explain how your Undo feature works using `std::stack`."
**Model Answer:**
> *"A stack operates on the **LIFO (Last-In, First-Out)** principle. Whenever an administrator deletes a patient, changes a record or books an appointment, an `AdminAction` object containing the operation type and a **copy** of the record is pushed onto `std::stack<AdminAction> undoStack`.
> 
> When the user presses the 'Undo' button or `Ctrl+Z`, the program takes the top entry, reverses that specific action (for example, putting the deleted patient back into the vector), and then calls `undoStack.pop()`. This allows multi-level undo operations.
> 
> The important detail is that the stack owns a **copy**, so the copy is never the same memory as the live record. That is what keeps the program from freeing the same record twice. The copy is held in a `std::unique_ptr`, which is why the undo entry can be moved but not copied - it follows the rule of five, because it owns a resource."*

---

### Question 7b: "How do you make sure the memory is freed exactly once?"
**Model Answer:**
> *"Ownership is written down explicitly. The database owns the live records in a `std::vector<std::unique_ptr<PatientRecord>>`, the undo entry owns its copy in a `std::unique_ptr`, and the edit panel owns the snapshot it takes before an edit in a `std::unique_ptr` as well.
> 
> Because every owner is a `unique_ptr`, the record is freed exactly once by whoever owns it, and the compiler checks it. There is no raw `delete` left in the model or the GUI.
> 
> The undo entry needed one more step: it holds a `unique_ptr`, so its copy constructor is deleted and only the move constructor is left. That is why the stack entry is moved out of the stack when it is undone."*

---

### Question 8: "Why did you declare `virtual ~PatientRecord()` with a virtual destructor?"
**Model Answer:**
> *"This is critical for preventing memory leaks in C++ when using polymorphism. When we store pointers as `PatientRecord*` and later call `delete patientPtr`, if the destructor was not virtual, C++ would only execute the base class destructor and fail to call the derived class destructor (like `GeneralPatient` or `EmergencyPatient`). 
> 
> Making the destructor `virtual` ensures the entire inheritance chain is properly cleaned up from the derived class down to the base class."*

---

### Question 9: "Why did you use layout managers (`QVBoxLayout`, `QHBoxLayout`, `QFormLayout`) instead of fixed pixel positioning?"
**Model Answer:**
> *"Fixed pixel coordinates break whenever the window is resized, maximized, or opened on monitors with different DPI scalings. 
> 
> Layout managers like `QVBoxLayout`, `QHBoxLayout`, and `QFormLayout` automatically calculate widget sizes, manage spacing dynamically, and keep the user interface responsive and visually aligned across all screen resolutions."*

---

### Question 10: "How is your program organised?"
**Model Answer:**
> *"It is split into four layers, and each layer has its own namespace so that the names cannot clash: `scapms::model`, `scapms::controller`, `scapms::gui` and `scapms::utils`. Every class that has behaviour is split into a header file with the declarations and a source file with the implementation; the two pure interfaces and the small inline helper headers are header-only by design.
>
> The **model** layer holds the patient classes and the database, the **controller** layer holds the searching, sorting and reporting routines, the **GUI** layer has one panel class per tab, and the **utilities** hold the exception classes, the file reader and the ID generator.
>
> The model, controller and utility layers do not use Qt at all, so the same code also runs in the console demonstration, which is how I tested the algorithms."*

---

### Question 10b: "What does `ClinicManager` do, and why does it exist?"
**Model Answer:**
> *"The `ClinicManager` is the class that the graphical interface talks to. It is the only class the screens are allowed to call, and it owns the database and the appointment controller.
>
> It exists because of a problem I ran into. In the first version the panels called the database themselves, so every rule of the clinic - the age has to be between 0 and 120, the date has to be written as YYYY-MM-DD, an emergency patient may not be booked in advance - was written inside `ClinicDatabase`. That class grew to 488 lines and mixed the business logic with the data handling, which is exactly what requirement 2 asks me not to do.
>
> So I moved the rules up into the controller layer. `ClinicManager` now checks the rules of a patient record, `AppointmentController` checks the rules of a booking, and `ClinicDatabase` only stores and finds. The panels were repointed from the database to the manager, and **no file in the `gui/` folder refers to the database any more**. That is the check I would point a marker to: the separation is not a claim in the report, it is a fact in the source tree."*

---

## 3. Live Defense Modification Exercises

Sometimes a lecturer will say: *"Make a small change right now in front of me to prove you understand your code."*

Here are 3 common requests and exactly how to solve them:

### Exercise 1: Change the age validation range from `0-120` to `0-130`
**Where to go:** Open `controller/ClinicManager.cpp`.  
Find `checkPatientRules`:
```cpp
// Change:
if (patient.getAge() < 0 || patient.getAge() > 120)
// To:
if (patient.getAge() < 0 || patient.getAge() > 130)
```
*Note for the examiner: the same range is also checked in `gui/PatientPanel.cpp` in
`onRegisterPatient()`, so both places have to be changed for the rule to be consistent.*

---

### Exercise 2: Add a new sort order (e.g. descending order instead of ascending)
**Where to go:** Open `controller/SortEngine.cpp`.  
Look at `comparePatients` (line 6 onwards):
```cpp
case SortField::PatientAge:
    return a->getAge() < b->getAge(); // Simply flip '<' to '>'!
```

---

### Exercise 3: Add a new shortcut to the Menu Bar
**Where to go:** Open `gui/MainWindow.cpp`.  
Inside `setupMenuBar()` (line 101):
```cpp
QAction* actSearch = editMenu->addAction("&Quick search");
actSearch->setShortcut(QKeySequence("Ctrl+F"));
connect(actSearch, &QAction::triggered, this, [this]() {
    tabWidget->setCurrentIndex(3); // Switches directly to Search/Sort tab!
});
```
