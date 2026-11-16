@echo off
REM Builds the graphical program of SCAPMS with CMake and Qt 6.
REM The binary is written to build\bin\SCAPMS_GUI.exe.

setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d "%~dp0.."

if "%QT_DIR%"=="" set "QT_DIR=C:\Qt\6.5.3\msvc2019_64"

echo [1/4] Looking for CMake...
call "%~dp0find_cmake.bat"
if errorlevel 1 goto :failed

echo [2/4] Configuring...
%CMAKE% -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_DIR%"
if errorlevel 1 goto :failed

echo [3/4] Building the program...
%CMAKE% --build build --config Release --target SCAPMS_GUI
if errorlevel 1 goto :failed

echo [4/4] Putting the Qt runtime next to the binary...
"%QT_DIR%\bin\windeployqt.exe" build\bin\SCAPMS_GUI.exe --no-translations --compiler-runtime >nul 2>&1

echo.
echo Build Complete! Binary created at: build\bin\SCAPMS_GUI.exe
exit /b 0

:failed
echo.
echo Build FAILED
exit /b 1
