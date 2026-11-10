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
> All searching routines (Linear, Binary, Recursive) and sorting algorithms (Selection Sort and Merge Sort) are **implemented manually from scratch**. The entire application is backed by CSV file persistence and a custom exception hierarchy and handles invalid input through the exception classes."*

---

## 2. Likely Lecturer Oral Defense Questions & Model Answers

### Question 1: "Why did you make `PatientRecord` an abstract class instead of a regular class?"
**Model Answer:**
> *"Because in a real hospital, nobody is just a generic 'Patient'. A patient who walks into a clinic is specifically a General Outpatient, an Emergency Case, or a Follow-Up Consultation. 
> 
> By declaring `displayPatient()` and `getSpecialDetail()` as **pure virtual functions (`= 0`)**, I made `PatientRecord` an **abstract class**. This prevents anyone from instantiating an incomplete generic patient object directly, while enforcing that every subclass must implement its own specific display and detail logic."*

---

### Question 2: "What is the difference between an Abstract Class and an Interface in C++?"
**Model Answer:**
> *"C++ does not have a native `interface` keyword like Java. In C++, an **Interface** is achieved using an abstract class that contains **no member variables** and **only pure virtual functions**. 
> 
> In my project, `PatientRecord` is an **Abstract Base Class** because it holds shared member variables (`name`, `age`, `patientID`) and common getters. In contrast, `Appointable` is a **pure Interface** because it only defines the operations for scheduling and cancelling appointments (`scheduleAppointment()`, `cancelAppointment()`), without storing any data variables."*

---

### Question 3: "Why does `GeneralPatient` inherit from `Appointable`, but `EmergencyPatient` does not?"
**Model Answer:**
> *"This was a deliberate clinical design decision. General and Follow-Up patients can book advance scheduled consultations with doctors. However, by medical protocol, Emergency trauma cases are rushed into the triage ward immediately and cannot be scheduled for future appointments. 
> 
> Therefore, using **Multiple Inheritance**, only `GeneralPatient` and `FollowUpPatient` implement the `Appointable` interface."*

---

### Question 4: "Why did you implement both Selection Sort and Merge Sort? How do their time complexities compare?"
**Model Answer:**
> *"I implemented both to demonstrate the contrast between basic and optimal sorting paradigms:
> 
> 1. **Selection Sort** is an in-place O(N^2) algorithm. While its comparison count is quadratic, it performs at most N-1 swaps, which makes it useful when memory write operations are expensive on small datasets.
> 2. **Merge Sort** is a Divide-and-Conquer algorithm with an optimal O(N log N) time complexity in all cases (best, average, and worst). It recursively divides the patient vector in half and merges the sorted subsets. It is stable, so equal records keep their original order."*

---

### Question 5: "What is the golden rule / precondition before running Binary Search?"
**Model Answer:**
> *"The collection **must be sorted** prior to execution! If the records are not sorted, Binary Search will discard the wrong half of the array and fail to find the patient. 
> 
> In my GUI implementation, before executing `SearchEngine::binarySearchByID`, the program defensively sorts the records by Patient ID using Merge Sort."*

---

### Question 6: "Explain how your Undo feature works using `std::stack`."
**Model Answer:**
> *"A stack operates on the **LIFO (Last-In, First-Out)** principle. Whenever an administrator deletes a patient or adds a record, an `AdminAction` object containing the operation type and a backup pointer is pushed onto `std::stack<AdminAction> undoStack`.
> 
> When the user presses the 'Undo' button or `Ctrl+Z`, the system calls `undoStack.top()`, reverses that specific action (for example, re-inserting the deleted patient pointer back into the vector), and then calls `undoStack.pop()`. This allows multi-level undo operations."*

---

### Question 7: "Why did you declare `virtual ~PatientRecord()` with a virtual destructor?"
**Model Answer:**
> *"This is critical for preventing memory leaks in C++ when using polymorphism. When we store pointers as `PatientRecord*` and later call `delete patientPtr`, if the destructor was not virtual, C++ would only execute the base class destructor and fail to call the derived class destructor (like `GeneralPatient` or `EmergencyPatient`). 
> 
> Making the destructor `virtual` ensures the entire inheritance chain is properly cleaned up from the derived class down to the base class."*

---

### Question 8: "Why did you use layout managers (`QVBoxLayout`, `QHBoxLayout`, `QFormLayout`) instead of fixed pixel positioning?"
**Model Answer:**
> *"Fixed pixel coordinates break whenever the window is resized, maximized, or opened on monitors with different DPI scalings. 
> 
> Layout managers like `QVBoxLayout`, `QHBoxLayout`, and `QFormLayout` automatically calculate widget sizes, manage spacing dynamically, and keep the user interface responsive and visually aligned across all screen resolutions."*

---

## 3. Live Defense Modification Exercises

Sometimes a lecturer will say: *"Make a small change right now in front of me to prove you understand your code."*
Here are 3 common requests and exactly how to solve them:

### Exercise 1: Change the age validation range from `0-120` to `0-130`
**Where to go:** Open `ClinicDatabase.h`.  
Find lines 58-63:
```cpp
// Change:
if (patient->getAge() < 0 || patient->getAge() > 120)
// To:
if (patient->getAge() < 0 || patient->getAge() > 130)
```

---

### Exercise 2: Add a new sort order (e.g. descending order instead of ascending)
**Where to go:** Open `SortEngine.h`.  
Look at `comparePatients`:
```cpp
// To sort by Age Oldest-to-Youngest:
case SORT_BY_AGE:
    return a->getAge() > b->getAge(); // Simply flip '<' to '>'!
```

---

### Exercise 3: Add a new shortcut to the Menu Bar
**Where to go:** Open `MainWindow.cpp`.  
Inside `setupMenuBar()`:
```cpp
QAction* actSearch = fileMenu->addAction("&Quick Search");
actSearch->setShortcut(QKeySequence("Ctrl+F"));
connect(actSearch, &QAction::triggered, [this]() {
    tabWidget->setCurrentIndex(3); // Switches directly to Search/Sort tab!
});
```
