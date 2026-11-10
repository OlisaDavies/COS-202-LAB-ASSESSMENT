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
        +displayPatient()* void
        +getSpecialDetail()* string
    }

    %% Subclasses (Inheritance)
    class GeneralPatient {
        -consultationFee: double
        -appointmentBooked: bool
        -appointmentDate: string
        -appointmentTime: string
        +getConsultationFee() double
        +setConsultationFee(fee: double) void
        +displayPatient() void
        +getSpecialDetail() string
        +scheduleAppointment(date: string, time: string) bool
        +cancelAppointment() bool
        +hasScheduledAppointment() bool
        +getAppointmentDetails() string
    }

    class EmergencyPatient {
        -severityLevel: int
        -triageNote: string
        +getSeverityLevel() int
        +setSeverityLevel(lvl: int) void
        +getTriageNote() string
        +getUrgencyDescription() string
        +displayPatient() void
        +getSpecialDetail() string
    }

    class FollowUpPatient {
        -previousVisitDate: string
        -assignedDoctor: string
        -appointmentBooked: bool
        -appointmentDate: string
        -appointmentTime: string
        +getPreviousVisitDate() string
        +getAssignedDoctor() string
        +displayPatient() void
        +getSpecialDetail() string
        +scheduleAppointment(date: string, time: string) bool
        +cancelAppointment() bool
        +hasScheduledAppointment() bool
        +getAppointmentDetails() string
    }

    PatientRecord <|-- GeneralPatient : Inheritance
    PatientRecord <|-- EmergencyPatient : Inheritance
    PatientRecord <|-- FollowUpPatient : Inheritance

    Appointable <|.. GeneralPatient : Implements Interface
    Appointable <|.. FollowUpPatient : Implements Interface

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
        +patientBackup: PatientRecord*
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
        +addPatient(p: PatientRecord*, trackUndo: bool) void
        +removePatient(id: string, trackUndo: bool) bool
        +findPatientByID(id: string) PatientRecord*
        +enqueuePatient(p: PatientRecord*) void
        +callNextPatient() PatientRecord*
        +undoLastAction() bool
        +touchRecentPatient(p: PatientRecord*) void
        +addAppointment(apt: Appointment, trackUndo: bool) void
        +getAllPatients() vector~PatientRecord*~
        +getAllAppointments() vector~Appointment~
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
    }

    class SortEngine {
        <<controller>>
        +selectionSort(records, field) void
        +mergeSort(records, field) void
    }

    class FileHandler {
        <<utility>>
        +savePatients(filename, patients) bool
        +loadPatients(filename) vector~PatientRecord*~
        +saveAppointments(filename, appointments) bool
        +loadAppointments(filename) vector~Appointment~
    }

    %% GUI
    class MainWindow {
        <<GUI View/Controller>>
        -database: ClinicDatabase
        -tabWidget: QTabWidget*
        -dashboardTimer: QTimer*
        +setupUI() void
        +setupMenuBar() void
        +onRegisterPatient() void
        +onDeleteSelectedPatient() void
        +onScheduleAppointment() void
        +onEnqueuePatient() void
        +onCallNextPatient() void
        +onExecuteSearch() void
        +onExecuteSort() void
        +onGenerateSelectedReport() void
        +onSaveData() void
        +onLoadData() void
        +onUndoAction() void
    }

    MainWindow --> ClinicDatabase : Uses
    MainWindow --> SearchEngine : Calls
    MainWindow --> SortEngine : Calls
    MainWindow --> FileHandler : Calls
```
