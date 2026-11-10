@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
set PATH=C:\Qt\6.5.3\msvc2019_64\bin;%PATH%
cd /d "%~dp0"

echo [1/3] Generating Makefile with qmake...
C:\Qt\6.5.3\msvc2019_64\bin\qmake.exe SCAPMS.pro -spec win32-msvc "CONFIG+=release"

echo [2/3] Compiling Qt Application with nmake...
nmake /NOLOGO release

echo [3/3] Deploying Qt runtime DLLs...
C:\Qt\6.5.3\msvc2019_64\bin\windeployqt.exe release\SCAPMS_GUI.exe --no-translations --compiler-runtime >nul 2>&1

echo.
echo Build Complete! Binary created at: release\SCAPMS_GUI.exe
