# SCAPMS Class Diagram

```mermaid
classDiagram
    %% Abstract Interface
    class Appointable {
        <<interface>>
        +scheduleAppointment(date: string, time: string)* bool
        +cancelAppointment()* bool
        +hasScheduledAppointment()* bool
        +getAppointmentDetails()* string
    }

    %% Abstract Base Class
    class PatientRecord {
        <<abstract>>
        #patientID: string
        #name: string
        #age: int
        #contactNumber: string
        #patientType: string
        +getPatientID() string
        +getName() string
        +getAge() int
        +getContactNumber() string
        +getPatientType() string
        +setName(newName: string) void
        +setAge(newAge: int) void
        +setContactNumber(newContact: string) void
        +getTypeLabel()* string
        +displayPatient()* void
        +getSpecialDetail()* string
        +clone()* PatientRecord
        +restoreFrom(previous: PatientRecord) void
    }

    %% Intermediate abstract class
    class ScheduledPatient {
        <<abstract>>
        #appointmentBooked: bool
        #appointmentDate: string
        #appointmentTime: string
        +scheduleAppointment(date: string, time: string) bool
        +cancelAppointment() bool
        +hasScheduledAppointment() bool
        +getAppointmentDetails() string
        +restoreFrom(previous: PatientRecord) void
    }

    %% Subclasses (Inheritance)
    class GeneralPatient {
        -consultationFee: double
        +getConsultationFee() double
        +setConsultationFee(fee: double) void
        +getTypeLabel() string
        +displayPatient() void
        +getSpecialDetail() string
        +clone() PatientRecord
        +restoreFrom(previous: PatientRecord) void
    }

    class EmergencyPatient {
        -severityLevel: int
        -triageNote: string
        +getSeverityLevel() int
        +setSeverityLevel(lvl: int) void
        +getTriageNote() string
        +setTriageNote(note: string) void
        +getUrgencyDescription() string
        +getTypeLabel() string
        +displayPatient() void
        +getSpecialDetail() string
        +clone() PatientRecord
        +restoreFrom(previous: PatientRecord) void
    }

    class FollowUpPatient {
        -previousVisitDate: string
        -assignedDoctor: string
        +getPreviousVisitDate() string
        +setPreviousVisitDate(date: string) void
        +getAssignedDoctor() string
        +setAssignedDoctor(doctor: string) void
        +getTypeLabel() string
        +displayPatient() void
        +getSpecialDetail() string
        +clone() PatientRecord
        +restoreFrom(previous: PatientRecord) void
    }

    PatientRecord <|-- ScheduledPatient : Inheritance
    ScheduledPatient <|-- GeneralPatient : Inheritance
    PatientRecord <|-- EmergencyPatient : Inheritance
    ScheduledPatient <|-- FollowUpPatient : Inheritance

    Appointable <|.. ScheduledPatient : Implements Interface
    ScheduledPatient <|-- GeneralPatient : inherits the interface
    ScheduledPatient <|-- FollowUpPatient : inherits the interface

    %% Models
    class Appointment {
        -appointmentID: string
        -patientID: string
        -date: string
        -time: string
        -doctorName: string
        -status: string
        +getAppointmentID() string
        +getPatientID() string
        +getDate() string
        +getTime() string
        +getDoctorName() string
        +getStatus() string
        +setStatus(s: string) void
        +displayAppointment() void
    }

    class AdminAction {
        +type: ActionType
        +description: string
        +patientID: string
        +patientBackup: unique_ptr~PatientRecord~
        +appointmentBackup: Appointment
    }

    %% Database (Composition)
    class ClinicDatabase {
        -patients: vector~PatientRecord*~
        -appointments: vector~Appointment~
        -waitingQueue: queue~PatientRecord*~
        -undoStack: stack~AdminAction~
        -recentPatients: PatientRecord*[10]
        -recentCount: int
        +addPatient(p: PatientRecord*) void
        +addPatients(list: vector~PatientRecord*~) void
        +removePatient(id: string) bool
        +pushPatientSnapshot(snapshot: PatientRecord*) void
        +findPatientByID(id: string) PatientRecord*
        +getAllPatients() vector~PatientRecord*~
        +getPatientCount() size_t
        +getAppointmentsFor(id: string) vector~Appointment~
        +getPendingCountFor(id: string) size_t
        +enqueuePatient(p: PatientRecord*) void
        +callNextPatient() PatientRecord*
        +getQueueSize() size_t
        +addAppointment(apt: Appointment) void
        +cancelAppointment(id: string) bool
        +undoLastAction() string
        +clearAll() void
    }

    ClinicDatabase *-- PatientRecord : Composition (std::vector)
    ClinicDatabase *-- Appointment : Composition (std::vector)
    ClinicDatabase *-- AdminAction : Composition (std::stack)

    %% Controllers
    class SearchEngine {
        <<controller>>
        +linearSearchByID(records, id) int
        +binarySearchByID(sortedRecords, id) int
        +recursiveSearchByName(records, name, idx) int
        +searchByCategory(records, category) vector~int~
    }

    class SortEngine {
        <<controller>>
        +comparePatients(a, b, field) bool
        +selectionSort(records, field) void
        +mergeSort(records, field) void
    }

    class ReportBuilder {
        <<controller>>
        +categoryDistribution(db) string
        +pendingAppointments(db) string
        +dailySummary(db) string
        +frequentPatients(db) string
        +dashboardSummary(db) string
    }

    %% Utilities
    class FileHandler {
        <<utility>>
        +savePatients(filename, patients) void
        +loadPatients(filename, warnings) vector~PatientRecord*~
        +saveAppointments(filename, appointments) void
        +loadAppointments(filename, warnings) vector~Appointment~
    }

    class IDGenerator {
        <<utility>>
        +nextAppointmentId(takenIds) string
        +nextPatientId(prefix, takenIds) string
    }

    class ClinicException {
        <<exception>>
        #errorMessage: string
        +what() const char*
    }

    %% GUI
    class MainWindow {
        <<GUI>>
        -database: ClinicDatabase
        -tabWidget: QTabWidget*
        -dashboardTimer: QTimer*
        -patientPanel: PatientPanel*
        -appointmentPanel: AppointmentPanel*
        -queuePanel: QueuePanel*
        -searchPanel: SearchPanel*
        -reportPanel: ReportPanel*
        +setupUI() void
        +setupMenuBar() void
        +setupTimer() void
        +refreshAll() void
        +onDataChanged() void
        +onUndoAction() void
        +onSaveData() void
        +onLoadData() void
        +onAbout() void
    }

    class PatientPanel {
        <<GUI>>
        -database: ClinicDatabase
        +refresh() void
        +onRegisterPatient() void
        +onEditSelectedPatient() void
        +onUpdatePatient() void
        +onCancelEdit() void
        +onDeleteSelectedPatient() void
    }

    class AppointmentPanel {
        <<GUI>>
        -database: ClinicDatabase
        +refresh() void
        +onScheduleAppointment() void
        +onCancelSelectedAppointment() void
    }

    class QueuePanel {
        <<GUI>>
        -database: ClinicDatabase
        +refresh() void
        +onEnqueuePatient() void
        +onCallNextPatient() void
    }

    class SearchPanel {
        <<GUI>>
        -database: ClinicDatabase
        +refresh() void
        +onExecuteSearch() void
        +onExecuteSort() void
        +onSortAlgorithmChanged(index) void
    }

    class ReportPanel {
        <<GUI>>
        -database: ClinicDatabase
        +refresh() void
        +onGenerateReport() void
    }

    MainWindow *-- PatientPanel : Composition
    MainWindow *-- AppointmentPanel : Composition
    MainWindow *-- QueuePanel : Composition
    MainWindow *-- SearchPanel : Composition
    MainWindow *-- ReportPanel : Composition

    MainWindow --> ClinicDatabase : Uses
    PatientPanel --> ClinicDatabase : Uses
    AppointmentPanel --> ClinicDatabase : Uses
    QueuePanel --> ClinicDatabase : Uses
    SearchPanel --> ClinicDatabase : Uses
    ReportPanel --> ClinicDatabase : Uses

    PatientPanel --> SearchEngine : Calls
    SearchPanel --> SearchEngine : Calls
    SearchPanel --> SortEngine : Calls
    ReportPanel --> ReportBuilder : Calls
    MainWindow --> FileHandler : Calls
    AppointmentPanel --> IDGenerator : Calls
    ClinicDatabase ..> ClinicException : Throws
```
