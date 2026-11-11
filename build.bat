@echo off
REM Builds the console demonstration of SCAPMS (no Qt required).
REM The console build proves that the searching, sorting, queue, undo and
REM reporting features work on their own.

setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d "%~dp0"

if not exist build mkdir build

echo [1/2] Compiling the console demonstration...
cl /nologo /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /Fe:build\SCAPMS.exe ^
   main.cpp ^
   model\PatientRecord.cpp ^
   model\ScheduledPatient.cpp ^
   model\GeneralPatient.cpp ^
   model\EmergencyPatient.cpp ^
   model\FollowUpPatient.cpp ^
   model\Appointment.cpp ^
   model\ClinicDatabase.cpp ^
   controller\SearchEngine.cpp ^
   controller\SortEngine.cpp ^
   controller\ReportBuilder.cpp ^
   utils\DateUtil.cpp ^
   utils\FileHandler.cpp ^
   utils\IDGenerator.cpp ^
   /Fo:build\ /Fd:build\vc.pdb

if errorlevel 1 (
    echo.
    echo Build FAILED
    exit /b 1
)

echo [2/2] Running the demonstration...
echo.
build\SCAPMS.exe

endlocal
