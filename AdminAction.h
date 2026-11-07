#pragma once

#include <string>

#include "PatientRecord.h"
#include "Appointment.h"

enum ActionType {
    ACTION_ADD_PATIENT,
    ACTION_DELETE_PATIENT,
    ACTION_UPDATE_PATIENT,
    ACTION_ADD_APPOINTMENT,
    ACTION_CANCEL_APPOINTMENT
};

class AdminAction {
public:
    ActionType type;
    std::string description;
    PatientRecord* patientBackup;
    Appointment appointmentBackup;

    AdminAction(ActionType t, const std::string& desc, PatientRecord* p = nullptr)
        : type(t), description(desc), patientBackup(p),
          appointmentBackup("", "", "", "", "") {}

    AdminAction(ActionType t, const std::string& desc, const Appointment& apt)
        : type(t), description(desc), patientBackup(nullptr),
          appointmentBackup(apt) {}
};
