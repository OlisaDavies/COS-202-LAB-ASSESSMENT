@echo off
REM Builds the console demonstration of SCAPMS. It needs no Qt, and it
REM proves that the searching, sorting, queue, undo and reporting features
REM work on their own.
REM
REM The binary is written to build\bin\SCAPMS.exe and is run at the end.

setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d "%~dp0.."

echo [1/2] Looking for CMake...
call "%~dp0find_cmake.bat"
if errorlevel 1 goto :failed

echo [2/2] Building the program...
%CMAKE% -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto :failed

%CMAKE% --build build --config Release --target SCAPMS
if errorlevel 1 goto :failed

echo.
build\bin\SCAPMS.exe
exit /b 0

:failed
echo.
echo Build FAILED
exit /b 1
