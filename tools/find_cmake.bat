@echo off
REM Prints the command that runs CMake, in the variable CMAKE.
REM It fails when CMake cannot be found at all.
REM
REM CMake is looked for on the PATH first, and then as a Python package,
REM because pip install cmake puts it there.

where cmake >nul 2>&1
if not errorlevel 1 (
    set "CMAKE=cmake"
    goto :found
)

where python >nul 2>&1
if not errorlevel 1 (
    python -m cmake --version >nul 2>&1
    if not errorlevel 1 (
        set "CMAKE=python -m cmake"
        goto :found
    )
)

echo CMake was not found. Install it with "pip install cmake" or put it on the PATH.
exit /b 1

:found
endlocal & set "CMAKE=%CMAKE%"
