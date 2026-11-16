@echo off
REM Rebuilds the submission document from the sources, the screenshots, the
REM diagrams and the report. Needs Python with the markdown package, and
REM Chrome or Edge to print the document.

setlocal
cd /d "%~dp0.."

python tools\build_submission_pdf.py
if errorlevel 1 (
    echo.
    echo The PDF was not built
    exit /b 1
)

endlocal
