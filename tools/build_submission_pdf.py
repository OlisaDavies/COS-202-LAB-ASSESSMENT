"""Builds SCAPMS_Submission.pdf.

The whole document is written as one HTML file and then printed to PDF by
headless Chrome, so that the code listings, the screenshots, the diagrams and
the report all keep the layout they are given here.

    python tools\\build_submission_pdf.py

Needs Python with the markdown package, and Google Chrome.
"""

import html
import os
import subprocess
import sys
import tempfile

import markdown

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PDF_PATH = os.path.join(ROOT, "SCAPMS_Submission.pdf")

CHROME_CANDIDATES = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    os.path.expandvars(r"%LOCALAPPDATA%\Google\Chrome\Application\chrome.exe"),
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]

HEADER_FILES = [
    "model/AdminAction.h", "model/Appointable.h", "model/Appointment.h", "model/Billable.h",
    "model/ClinicDatabase.h", "model/EmergencyPatient.h", "model/FollowUpPatient.h",
    "model/GeneralPatient.h", "model/PatientRecord.h", "model/ScheduledPatient.h",
    "controller/AppointmentController.h", "controller/ClinicManager.h",
    "controller/ReportBuilder.h", "controller/SearchEngine.h", "controller/SortEngine.h",
    "utils/ClinicExceptions.h", "utils/DateUtil.h", "utils/FileHandler.h",
    "utils/IDGenerator.h", "utils/StringUtil.h",
    "gui/AdminPanel.h", "gui/AppointmentPanel.h", "gui/MainWindow.h", "gui/PatientPanel.h",
    "gui/QueuePanel.h", "gui/ReportPanel.h", "gui/SearchPanel.h",
]

SOURCE_FILES = [
    "model/AdminAction.cpp", "model/Appointment.cpp", "model/ClinicDatabase.cpp",
    "model/EmergencyPatient.cpp", "model/FollowUpPatient.cpp", "model/GeneralPatient.cpp",
    "model/PatientRecord.cpp", "model/ScheduledPatient.cpp",
    "controller/AppointmentController.cpp", "controller/ClinicManager.cpp",
    "controller/ReportBuilder.cpp", "controller/SearchEngine.cpp", "controller/SortEngine.cpp",
    "utils/DateUtil.cpp", "utils/FileHandler.cpp", "utils/IDGenerator.cpp",
    "main.cpp",
]

GUI_FILES = [
    "gui/AdminPanel.cpp", "gui/AppointmentPanel.cpp", "gui/MainWindow.cpp",
    "gui/PatientPanel.cpp", "gui/QueuePanel.cpp", "gui/ReportPanel.cpp",
    "gui/SearchPanel.cpp", "gui_main.cpp", "gui/clinic.ico", "gui/resources.qrc",
    "SCAPMS.pro", "build_gui.bat", "build.bat",
    ".vscode/tasks.json", ".vscode/launch.json",
]

DATA_FILES = ["patients.csv", "appointments.csv"]

SCREENSHOTS = [
    ("screenshots/1_patient_records.png", "1. Patient Records"),
    ("screenshots/2_appointments.png", "2. Appointments"),
    ("screenshots/3_waiting_queue.png", "3. Waiting Queue"),
    ("screenshots/4_search_and_sort.png", "4. Search & Sort"),
    ("screenshots/5_clinic_reports.png", "5. Clinic Reports"),
    ("screenshots/6_administration.png", "6. Administration"),
]

DIAGRAMS = [
    ("UML_DIAGRAM.png", "Full class diagram (all layers)"),
    ("uml/UML_1_model.png", "Model layer"),
    ("uml/UML_2_controller.png", "Controller layer"),
    ("uml/UML_3_screens.png", "GUI screens"),
    ("uml/UML_4_utilities.png", "Utilities"),
]

STYLE = """
@page { size: A4 portrait; margin: 18mm 16mm 16mm 16mm; }
body { font-family: Calibri, "Segoe UI", sans-serif; font-size: 11pt; color: #111; margin: 0; }
h1 { font-size: 18pt; margin: 0 0 10pt 0; }
h2 { font-size: 14pt; margin: 18pt 0 8pt 0; }
h3 { font-size: 12pt; margin: 14pt 0 6pt 0; }
p { margin: 0 0 7pt 0; }
.cover { text-align: center; margin-top: 90mm; }
.cover .title { font-size: 21pt; font-weight: bold; margin-bottom: 20pt; }
.cover .line { font-size: 11.5pt; margin-bottom: 5pt; }
.contents { margin-top: 30mm; }
.contents h2 { margin-bottom: 12pt; }
.contents p { margin-bottom: 6pt; }
.break { page-break-before: always; }
.label { font-family: Consolas, monospace; font-size: 9pt; font-weight: bold;
         margin: 12pt 0 2pt 0; page-break-after: avoid; }
pre.code { font-family: Consolas, "Courier New", monospace; font-size: 7pt;
           line-height: 1.06; white-space: pre; margin: 0 0 4pt 0;
           font-variant-ligatures: none; }
figure { margin: 0 0 14pt 0; page-break-inside: avoid; text-align: center; }
figure img { max-width: 165mm; max-height: 215mm; }
figcaption { font-size: 10pt; margin-top: 5pt; }
table { border-collapse: collapse; margin: 6pt 0 10pt 0; }
th, td { border: 1px solid #999; padding: 3pt 6pt; text-align: left; vertical-align: top; }
th { background: #eeeeee; }
code { font-family: Consolas, monospace; font-size: 10pt; font-variant-ligatures: none; }
ul, ol { margin: 0 0 8pt 0; padding-left: 20pt; }
li { margin-bottom: 3pt; }
hr { border: 0; border-top: 1px solid #ccc; margin: 12pt 0; }
"""


def file_uri(relative):
    return "file:///" + os.path.join(ROOT, relative).replace("\\", "/").replace(" ", "%20")


def code_block(relative):
    path = os.path.join(ROOT, relative)

    if relative.lower().endswith(".ico"):
        size = os.path.getsize(path)
        body = ("This file is the icon of the program, a small bitmap of %d bytes.\n"
                "It is attached to the window in the resources file below." % size)
    else:
        with open(path, encoding="utf-8") as handle:
            body = handle.read().replace("\r\n", "\n").rstrip("\n")

    return ('<div class="label">=== %s ===</div><pre class="code">%s</pre>'
            % (relative, html.escape(body)))


def image_block(relative, caption):
    return '<figure><img src="%s" alt="%s"><figcaption>%s</figcaption></figure>' % (
        file_uri(relative),
        html.escape(caption),
        html.escape(caption),
    )


def build_html(path):
    parts = [
        "<!DOCTYPE html><html><head><meta charset='utf-8'><title>SCAPMS</title>",
        "<style>%s</style></head><body>" % STYLE,
        "<div class='cover'>",
        "<div class='title'>Smart Clinic Appointment &amp; Patient Management System (SCAPMS)</div>",
        "<div class='line'>Course: COS202 Programming II - Lab Assessment Project</div>",
        "<div class='line'>Student: OlisaEmeka Davies Chinedu-Eze</div>",
        "<div class='line'>Language: C++17, Qt 6.5.3 (Qt Widgets), MSVC 2022</div>",
        "<div class='line'>Build: qmake + nmake (build_gui.bat) / cl (build.bat) - both verified</div>",
        "</div>",
        "<div class='contents'>",
        "<h2>Contents</h2>",
        "<p>1. Source Code - i. Header files, ii. Source files, iii. GUI files and resources, "
        "iv. Data files</p>",
        "<p>2. Screenshots of the major GUI parts</p>",
        "<p>3. UML / Class Diagrams (full diagram and four layer sections)</p>",
        "<p>4. Project Report</p>",
        "</div>",
        "<div class='break'><h1>1. Source Code</h1>",
        "<h2>i. Header files</h2>",
    ]

    for relative in HEADER_FILES:
        parts.append(code_block(relative))

    parts.append("<h2>ii. Source files</h2>")
    for relative in SOURCE_FILES:
        parts.append(code_block(relative))

    parts.append("<h2>iii. GUI files and resources</h2>")
    for relative in GUI_FILES:
        parts.append(code_block(relative))

    parts.append("<h2>iv. Data files</h2>")
    for relative in DATA_FILES:
        parts.append(code_block(relative))

    parts.append("</div>")

    parts.append("<div class='break'><h1>2. Screenshots of the major GUI parts</h1>")
    for relative, caption in SCREENSHOTS:
        parts.append(image_block(relative, caption))
    parts.append("</div>")

    parts.append("<div class='break'><h1>3. UML / Class Diagrams "
                 "(full diagram and four layer sections)</h1>")
    for relative, caption in DIAGRAMS:
        parts.append(image_block(relative, caption))
    parts.append("</div>")

    with open(os.path.join(ROOT, "SUBMISSION_REPORT.md"), encoding="utf-8") as handle:
        report = handle.read()
    report_html = markdown.markdown(report, extensions=["tables", "sane_lists"])

    parts.append("<div class='break'><h1>4. Project Report</h1>%s</div>" % report_html)
    parts.append("</body></html>")

    document = "\n".join(parts)
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(document)

    return len(document)


def find_chrome():
    for candidate in CHROME_CANDIDATES:
        if candidate and os.path.exists(candidate):
            return candidate
    return None


def main():
    chrome = find_chrome()
    if chrome is None:
        print("Chrome or Edge was not found, so the PDF cannot be printed.")
        return 1

    work = tempfile.mkdtemp(prefix="scapms_submission_")
    html_path = os.path.join(work, "submission.html")

    size = build_html(html_path)
    print("html written: %s  (%.1f MB)" % (html_path, size / 1048576.0))

    if os.path.exists(PDF_PATH):
        os.remove(PDF_PATH)

    command = [
        chrome,
        "--headless=new",
        "--disable-gpu",
        "--no-sandbox",
        "--no-pdf-header-footer",
        "--run-all-compositor-stages-before-draw",
        "--virtual-time-budget=30000",
        "--print-to-pdf=" + PDF_PATH,
        "file:///" + html_path.replace("\\", "/").replace(" ", "%20"),
    ]
    result = subprocess.run(command, capture_output=True, text=True, timeout=600)
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)

    if not os.path.exists(PDF_PATH):
        print("the PDF was not produced")
        return 1

    print("pdf written: %s  (%.1f MB)"
          % (PDF_PATH, os.path.getsize(PDF_PATH) / 1048576.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
