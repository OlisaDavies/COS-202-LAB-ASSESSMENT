# SCAPMS Class Diagram

```mermaid
classDiagram
    %% Abstract Interface 1
    class Appointable {
        <<interface>>
        +scheduleAppointment(appointment: Appointment) bool
        +cancelAppointment() bool
        +cancelAppointment(appointmentID: string) bool
        +hasScheduledAppointment() bool
        +getAppointmentDetails() string
    }

    %% Abstract Interface 2
    class Billable {
        <<interface>>
        +getBillAmount() double
        +describeBill() string
    }

    %% Abstract Base Class
    class PatientRecord {
        <<abstract>>
        #patientID: string
        #name: string
        #age: int
        #contactNumber: string
        #patientType: string
        #appointments: vector~Appointment~
        +getPatientID() string
        +getName() string
        +getAge() int
        +getContactNumber() string
        +getPatientType() string
        +setName(newName: string) void
        +setAge(newAge: int) void
        +setContactNumber(newContact: string) void
        +getAppointments() vector~Appointment~
        +getAppointmentCount() size_t
        +findAppointment(appointmentID: string) Appointment
        +hasAppointment(appointmentID: string) bool
        +getNextPendingAppointment() Appointment
        +getPendingAppointmentCount() size_t
        +removeAppointment(appointmentID: string) bool
        +restoreAppointment(backup: Appointment) bool
        +getTypeLabel()* string
        +getPatientReport()* string
        +getSpecialDetail()* string
        +clone()* PatientRecord
        +displayPatient() void
        +restoreFrom(previous: PatientRecord) void
    }

    %% Intermediate abstract class
    class ScheduledPatient {
        <<abstract>>
        +scheduleAppointment(appointment: Appointment) bool
        +cancelAppointment() bool
        +cancelAppointment(appointmentID: string) bool
        +hasScheduledAppointment() bool
        +getAppointmentDetails() string
        +getBillAmount() double
        +describeBill() string
    }

    %% Subclasses (Inheritance)
    class GeneralPatient {
        -consultationFee: double
        +getConsultationFee() double
        +setConsultationFee(fee: double) void
        +getTypeLabel() string
        +getPatientReport() string
        +getSpecialDetail() string
        +getBillAmount() double
        +describeBill() string
        +clone() GeneralPatient
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
        +getPatientReport() string
        +getSpecialDetail() string
        +clone() EmergencyPatient
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
        +getPatientReport() string
        +getSpecialDetail() string
        +clone() FollowUpPatient
        +restoreFrom(previous: PatientRecord) void
    }

    PatientRecord <|-- ScheduledPatient : Inheritance
    ScheduledPatient <|-- GeneralPatient : Inheritance
    PatientRecord <|-- EmergencyPatient : Inheritance
    ScheduledPatient <|-- FollowUpPatient : Inheritance

    %% A class may implement more than one interface
    Appointable <|.. ScheduledPatient : Implements Interface
    Billable <|.. ScheduledPatient : Implements Interface
    ScheduledPatient <|-- GeneralPatient : inherits the interfaces
    ScheduledPatient <|-- FollowUpPatient : inherits the interfaces

    %% Composition: a patient owns its appointments
    PatientRecord "1" *-- "0..*" Appointment : Composition (std::vector)

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
        +setPatientID(id: string) void
        +setStatus(s: string) void
        +displayAppointment() void
        +happensBefore(first: Appointment, second: Appointment) bool
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
        -patients: vector~unique_ptr~PatientRecord~~
        -waitingQueue: queue~PatientRecord*~
        -undoStack: stack~AdminAction~
        -recentPatients: PatientRecord*[10]
        -recentCount: int
        +addPatient(p: unique_ptr~PatientRecord~) void
        +addPatients(list: vector~unique_ptr~PatientRecord~~) void
        +removePatient(id: string) bool
        +pushPatientSnapshot(snapshot: unique_ptr~PatientRecord~) void
        +findPatientByID(id: string) PatientRecord*
        +getAllPatients() vector~PatientRecord*~
        +getPatientCount() size_t
        +getAppointmentsFor(id: string) vector~Appointment~
        +getAllAppointments() vector~Appointment~
        +getPendingCountFor(id: string) size_t
        +enqueuePatient(p: PatientRecord*) void
        +callNextPatient() PatientRecord*
        +getQueueSize() size_t
        +addAppointment(apt: Appointment) void
        +addAppointments(list: vector~Appointment~) void
        +cancelAppointment(id: string) bool
        +undoLastAction() string
        +clearAll() void
        +getRecentPatients() vector~PatientRecord*~
    }

    ClinicDatabase *-- PatientRecord : Composition (std::vector)
    ClinicDatabase *-- AdminAction : Composition (std::stack)
    AdminAction *-- PatientRecord : Composition (std::unique_ptr)

    %% Controllers
    class ClinicManager {
        <<controller>>
        -clinicDatabase: ClinicDatabase
        -appointmentController: AppointmentController
        +registerPatient(p: unique_ptr~PatientRecord~) void
        +registerPatients(list: vector~unique_ptr~PatientRecord~~) void
        +deletePatient(patientID: string) bool
        +findPatientByID(patientID: string) PatientRecord*
        +getAllPatients() vector~PatientRecord*~
        +getPatientCount() size_t
        +suggestPatientId(patientType: string) string
        +pushPatientSnapshot(snapshot: unique_ptr~PatientRecord~) void
        +scheduleAppointment(appointment: Appointment) void
        +cancelAppointment(appointmentID: string) bool
        +getAllAppointments() vector~Appointment~
        +enqueuePatient(p: PatientRecord*) void
        +callNextPatient() PatientRecord*
        +getQueueSize() size_t
        +undoLastAction() string
        +clearAll() void
        +getRecentPatients() vector~PatientRecord*~
        +saveToFiles(patientFile: string, appointmentFile: string) void
        +loadFromFiles(patientFile: string, appointmentFile: string) void
    }

    class AppointmentController {
        <<controller>>
        -database: ClinicDatabase
        +scheduleAppointment(appointment: Appointment) void
        +cancelAppointment(appointmentID: string) bool
        +allAppointments() vector~Appointment~
        +appointmentsFor(patientID: string) vector~Appointment~
        +suggestAppointmentId() string
    }

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
        +categoryDistribution(manager) string
        +pendingAppointments(manager) string
        +dailySummary(manager) string
        +frequentPatients(manager) string
        +dashboardSummary(manager) string
    }

    %% Utilities
    class FileHandler {
        <<utility>>
        +savePatients(filename, patients) void
        +loadPatients(filename, warnings) vector~unique_ptr~PatientRecord~~
        +saveAppointments(filename, appointments) void
        +loadAppointments(filename, warnings) vector~Appointment~
    }

    class IDGenerator {
        <<utility>>
        +nextAppointmentId(takenIds) string
        +nextPatientId(prefix, takenIds) string
        +patientPrefix(patientType) string
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
        -editSnapshot: unique_ptr~PatientRecord~
        +refresh() void
        +onRegisterPatient() void
        +onEditSelectedPatient() void
        +onUpdatePatient() void
        +onCancelEdit() void
        +onDeleteSelectedPatient() void
        +onShowReport() void
        +onSuggestPatientID() void
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

    class AdminPanel {
        <<GUI>>
        -manager: ClinicManager
        +refresh() void
        +undoLastOperation() void
        +saveData() void
        +loadData() void
        +newDatabase() void
    }

    MainWindow *-- PatientPanel : Composition
    MainWindow *-- AppointmentPanel : Composition
    MainWindow *-- QueuePanel : Composition
    MainWindow *-- SearchPanel : Composition
    MainWindow *-- ReportPanel : Composition
    MainWindow *-- AdminPanel : Composition
    MainWindow *-- ClinicManager : Composition

    %% The screens talk to the controller, never to the data.
    PatientPanel --> ClinicManager : Uses
    AppointmentPanel --> ClinicManager : Uses
    QueuePanel --> ClinicManager : Uses
    SearchPanel --> ClinicManager : Uses
    ReportPanel --> ClinicManager : Uses
    AdminPanel --> ClinicManager : Uses
    MainWindow --> ClinicManager : Uses

    %% The controller owns the data and the rules.
    ClinicManager *-- ClinicDatabase : Composition
    ClinicManager *-- AppointmentController : Composition
    AppointmentController --> ClinicDatabase : Uses

    PatientPanel --> SearchEngine : Calls
    SearchPanel --> SearchEngine : Calls
    SearchPanel --> SortEngine : Calls
    ReportPanel --> ReportBuilder : Calls
    AdminPanel --> ReportBuilder : Calls
    ClinicManager --> FileHandler : Calls
    ClinicManager --> IDGenerator : Calls
    AppointmentController --> IDGenerator : Calls
    ClinicDatabase ..> ClinicException : Throws
    ClinicManager ..> ClinicException : Throws
    AppointmentController ..> ClinicException : Throws
```
