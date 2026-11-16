"""Builds the submission document of the project.

The document follows the four parts of the brief:

    Part 1  Source Files/Code  - the file index and the link to the repository
    Part 2  Project Report
    Part 3  Screenshots of the program
    Part 4  UML/Class Diagram

The complete source code is not printed out, because it is long and the
repository holds it. The first part lists every file with a short description
and a link that opens the file on GitHub.

    python tools\\build_submission_pdf.py

Needs Python with the markdown package, and Google Chrome.
"""

import html
import os
import re
import subprocess
import sys
import tempfile

import markdown

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PDF_PATH = os.path.join(ROOT, "COS202 Project Report Olisaemeka Chinedu-Eze.pdf")
WORK = os.path.join(ROOT, "docs", "SUBMISSION_OVERVIEW.png")

REPOSITORY = "https://github.com/OlisaDavies/COS-202-LAB-ASSESSMENT"
BRANCH = "main"

# --------------------------------------------------------------- the details
NAME = "OlisaEmeka Davies Chinedu-Eze"
EMAIL = "c.olisaemeka1477@miva.edu.ng"
DEPARTMENT = "Cyber Security"
MATRIC_NUMBER = "2025/A/CYB/0260"
DATE = "November 15, 2026"

# ------------------------------------------------------------- the file index
SOURCE_INDEX = [
    ("i. Header Files", [
        ("model/PatientRecord.h", "Abstract base class for every patient category."),
        ("model/Appointable.h", "Interface for the patient types that can be booked."),
        ("model/Billable.h", "Interface for the patient types that can be billed."),
        ("model/ScheduledPatient.h", "Abstract class for the categories that can be booked."),
        ("model/GeneralPatient.h", "General outpatient class with a consultation fee."),
        ("model/EmergencyPatient.h", "Emergency triage patient class."),
        ("model/FollowUpPatient.h", "Follow-up patient class."),
        ("model/Appointment.h", "Appointment data class and its three states."),
        ("model/AdminAction.h", "One entry on the administrative undo stack."),
        ("model/ClinicDatabase.h", "In-memory clinic database, waiting room and undo history."),
        ("controller/ClinicManager.h", "Core clinic workflow manager and the rules of the clinic."),
        ("controller/AppointmentController.h", "The rules that govern a booking."),
        ("controller/SearchEngine.h", "Linear, keyword, binary, recursive and category search."),
        ("controller/SortEngine.h", "Selection sort and merge sort."),
        ("controller/ReportBuilder.h", "The four clinic reports and the clinic summary."),
        ("utils/ClinicExceptions.h", "The custom exception classes."),
        ("utils/DateUtil.h", "Date and time helpers for the booking rules."),
        ("utils/FileHandler.h", "CSV saving and loading of the clinic files."),
        ("utils/IDGenerator.h", "The next free patient and appointment identifiers."),
        ("utils/StringUtil.h", "Case-insensitive comparison and trimming helpers."),
        ("gui/MainWindow.h", "Main window and tab navigation."),
        ("gui/PatientPanel.h", "Patient registration form and the roster table."),
        ("gui/AppointmentPanel.h", "Appointment booking form and the schedule table."),
        ("gui/QueuePanel.h", "The waiting room and the queue list."),
        ("gui/SearchPanel.h", "Search and sort controls with the result table."),
        ("gui/ReportPanel.h", "Clinic report viewer."),
        ("gui/AdminPanel.h", "Administrative actions, clinic summary and recent records."),
    ]),
    ("ii. Source Files", [
        ("model/PatientRecord.cpp", "Base patient implementation."),
        ("model/ScheduledPatient.cpp", "Booking and cancellation for the bookable categories."),
        ("model/GeneralPatient.cpp", "General patient implementation."),
        ("model/EmergencyPatient.cpp", "Emergency patient implementation."),
        ("model/FollowUpPatient.cpp", "Follow-up patient implementation."),
        ("model/Appointment.cpp", "Appointment implementation."),
        ("model/AdminAction.cpp", "Undo stack entry implementation."),
        ("model/ClinicDatabase.cpp", "Storage, waiting room, undo history and the recent record cache."),
        ("controller/ClinicManager.cpp", "Clinic manager logic and the checks on every record."),
        ("controller/AppointmentController.cpp", "Booking rules and the availability of a doctor."),
        ("controller/SearchEngine.cpp", "Search algorithm implementations."),
        ("controller/SortEngine.cpp", "Sorting algorithm implementations."),
        ("controller/ReportBuilder.cpp", "Report generation logic."),
        ("utils/DateUtil.cpp", "Date and time parsing implementation."),
        ("utils/FileHandler.cpp", "CSV reading and writing implementation."),
        ("utils/IDGenerator.cpp", "Identifier generation implementation."),
        ("main.cpp", "The console demonstration of the clinic."),
    ]),
    ("iii. GUI Files and Resources", [
        ("gui/MainWindow.cpp", "Main window and tab navigation."),
        ("gui/PatientPanel.cpp", "Patient registration form and the roster table."),
        ("gui/AppointmentPanel.cpp", "Appointment booking form and the schedule table."),
        ("gui/QueuePanel.cpp", "The waiting room and the queue list."),
        ("gui/SearchPanel.cpp", "Search and sort controls with the result table."),
        ("gui/ReportPanel.cpp", "Clinic report viewer."),
        ("gui/AdminPanel.cpp", "Administrative actions, clinic summary and recent records."),
        ("gui_main.cpp", "Application entry point of the graphical program."),
        ("gui/clinic.ico", "Application icon."),
        ("gui/resources.qrc", "Application icon resource bundle."),
        ("CMakeLists.txt", "The build of both programs."),
        ("tools/build_gui.bat", "Builds the graphical program."),
        ("tools/build.bat", "Builds the console demonstration."),
        (".vscode/tasks.json", "Build tasks for Visual Studio Code."),
        (".vscode/launch.json", "Run configuration for Visual Studio Code."),
    ]),
    ("iv. Data Files Required to Run the Application", [
        ("data/patients.csv", "Sample patient records."),
        ("data/appointments.csv", "Sample appointment records."),
    ]),
]

BUILD_INSTRUCTIONS = """# Configure the project
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=C:\\Qt\\6.5.3\\msvc2019_64

# Build the graphical program
cmake --build build --config Release --target SCAPMS_GUI

# Run it
build\\bin\\SCAPMS_GUI.exe

# Or let the script do all of it
tools\\build_gui.bat"""

SCREENSHOTS = [
    ("docs/screenshots/1_main_dashboard.png", "i.", "Main dashboard"),
    ("docs/screenshots/2_patient_registration.png", "ii.", "Patient registration"),
    ("docs/screenshots/3_appointment_management.png", "iii.", "Appointment management"),
    ("docs/screenshots/4_search_sort_interface.png", "iv.", "Search/sort interface"),
    ("docs/screenshots/5_waiting_queue.png", "v.", "Waiting queue"),
    ("docs/screenshots/6_reports.png", "vi.", "Reports"),
]

DIAGRAMS = [
    ("docs/UML_DIAGRAM.png", "Full class diagram (all layers)"),
    ("docs/uml/UML_1_model.png", "Model layer"),
    ("docs/uml/UML_2_controller.png", "Controller layer"),
    ("docs/uml/UML_3_screens.png", "GUI screens"),
    ("docs/uml/UML_4_utilities.png", "Utilities"),
]

UML_KEY = """
<table class="umlkey">
<tr><th style="width: 26%">What the brief asks for</th><th>Where to see it in the diagram</th></tr>
<tr>
  <td><strong>Inheritance</strong></td>
  <td>A solid line with a hollow arrow head runs from <code>GeneralPatient</code>,
  <code>EmergencyPatient</code> and <code>FollowUpPatient</code> up to
  <code>ScheduledPatient</code> and on to <code>PatientRecord</code>. The subclasses
  therefore inherit the common fields and the shared behaviour of their parents.</td>
</tr>
<tr>
  <td><strong>Abstract classes</strong></td>
  <td><code>PatientRecord</code> and <code>ScheduledPatient</code> are marked
  <code>&lt;&lt;abstract&gt;&gt;</code>. <code>PatientRecord</code> declares
  <code>getTypeLabel()</code>, <code>getPatientReport()</code>,
  <code>getSpecialDetail()</code> and <code>clone()</code> as pure virtual, so it can
  never be created on its own.</td>
</tr>
<tr>
  <td><strong>Polymorphism</strong></td>
  <td>The same three calls are answered by a different version in each subclass, and
  they are reached through a <code>PatientRecord*</code> in
  <code>main.cpp</code> and in <code>gui/PatientPanel.cpp</code>, so the version that
  runs is decided at run time. <code>Appointable</code> and <code>Billable</code> are
  marked <code>&lt;&lt;interface&gt;&gt;</code> and are joined to the classes that
  implement them by a dashed line with a filled arrow head.</td>
</tr>
<tr>
  <td><strong>Composition / association</strong></td>
  <td>A filled diamond joins <code>PatientRecord</code> to its
  <code>appointments</code> and <code>ClinicDatabase</code> to its <code>patients</code>.
  The owner creates and destroys what it holds, so a patient and its appointments are
  created and destroyed together, and every record is freed exactly once.</td>
</tr>
</table>
"""

STYLE = """
@page { size: A4 portrait; margin: 18mm 16mm 16mm 16mm; }
body { font-family: Calibri, "Segoe UI", sans-serif; font-size: 11pt; color: #111; margin: 0; }
h1 { font-size: 17pt; margin: 0 0 10pt 0; }
h2 { font-size: 14pt; margin: 18pt 0 8pt 0; }
h3 { font-size: 12pt; margin: 14pt 0 6pt 0; }
p { margin: 0 0 7pt 0; }

.cstitle { font-size: 17pt; font-weight: bold; margin-bottom: 16pt; }
.coverline { font-size: 11.5pt; margin-bottom: 4pt; }
.coverline b { font-weight: bold; }
.toc { margin-top: 26pt; }
.toc p { margin-bottom: 5pt; padding-left: 16pt; }
.toc h2 { margin-bottom: 10pt; }
.break { page-break-before: always; }

.fileindex { margin: 0 0 12pt 0; padding-left: 20pt; }
.fileindex li { margin-bottom: 3.5pt; }
.fileindex a { color: #1a6fd4; text-decoration: underline; }
.filelink { font-family: Consolas, monospace; font-size: 10.5pt; }
.repo a { color: #1a6fd4; }

pre.build { font-family: Consolas, "Courier New", monospace; font-size: 9.5pt;
            line-height: 1.3; background: #f4f4f6; border: 1px solid #c9ccd4;
            padding: 10pt 12pt; white-space: pre-wrap; margin: 0 0 12pt 0; }

.tile { margin: 0 0 15pt 0; page-break-inside: avoid; }
.tile .frame { border: 1px solid #b8bcc6; padding: 5px; }
.tile img { display: block; width: 100%; height: auto; }
.tile .caption { font-size: 10pt; color: #111111; margin-top: 5pt; }
.tile .chip { color: #1a6fd4; font-weight: bold; }

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

.report { font-size: 10.5pt; line-height: 1.24; }
.report h2 { font-size: 12.5pt; margin: 11pt 0 4pt 0; }
.report p { margin: 0 0 5pt 0; }
.report ul, .report ol { margin: 0 0 6pt 0; padding-left: 18pt; }
.report li { margin-bottom: 2pt; }
.report table { margin: 4pt 0 7pt 0; }
.report th, .report td { padding: 2pt 5pt; font-size: 10pt; }
table.umlkey { page-break-inside: avoid; }
"""


def file_uri(relative):
    return "file:///" + os.path.join(ROOT, relative).replace("\\", "/").replace(" ", "%20")


def blob_link(path):
    return "%s/blob/%s/%s" % (REPOSITORY, BRANCH, path)


def image_block(relative, caption, chip=None):
    """A labelled tile: the picture in a frame, and the caption under it."""
    if chip is None:
        label = html.escape(caption)
    else:
        label = ('<span class="chip">%s&nbsp;</span>%s'
                 % (html.escape(chip), html.escape(caption)))
    return (
        '<div class="tile"><div class="frame">'
        '<img src="%s" alt="%s"></div>'
        '<div class="caption">%s</div></div>'
        % (file_uri(relative), html.escape(caption), label)
    )


def build_html(path):
    parts = [
        "<!DOCTYPE html><html><head><meta charset='utf-8'><title>SCAPMS</title>",
        "<style>%s</style></head><body>" % STYLE,

        # ---------------------------------------------------------- the cover
        "<div class='cstitle'>COS202 - Computer Programming II - Lab Assessment</div>",
        "<div class='coverline'><b>Name:</b> %s</div>" % NAME,
        "<div class='coverline'><b>Email:</b> %s</div>" % EMAIL,
        "<div class='coverline'><b>Department:</b> %s</div>" % DEPARTMENT,
        "<div class='coverline'><b>Matric Number:</b> %s</div>" % MATRIC_NUMBER,
        "<div class='coverline'><b>Date:</b> %s</div>" % DATE,
        "<div class='toc'>",
        "<h2>Table of Content</h2>",
        "<p>Source Files/Code</p>",
        "<p>Project Report</p>",
        "<p>Screenshots of the program</p>",
        "<p>UML/Class Diagram</p>",
        "</div>",

        # -------------------------------------------- Part 1: the source code
        "<div class='break'><h1>Part 1: Source Files/Code</h1>",
        "<p>The complete C++ source is hosted on GitHub</p>",
        "<p class='repo'><a href='%s'>%s</a></p>" % (REPOSITORY, REPOSITORY),
    ]

    for heading, entries in SOURCE_INDEX:
        parts.append("<h2>%s</h2><ul class='fileindex'>" % heading)
        for file_path, description in entries:
            parts.append(
                '<li><a class="filelink" href="%s">%s</a> - %s</li>'
                % (blob_link(file_path), html.escape(file_path), html.escape(description))
            )
        parts.append("</ul>")

    parts.append("<h2>Build and run instruction</h2>")
    parts.append("<pre class='build'>%s</pre>" % html.escape(BUILD_INSTRUCTIONS))
    parts.append("</div>")

    # --------------------------------------------- Part 2: the project report
    with open(os.path.join(ROOT, "docs", "SUBMISSION_REPORT.md"), encoding="utf-8") as handle:
        report = handle.read()

    # The document already has the title and the details of the student on the
    # cover, so the heading and the lines under it are left out here.
    lines = []
    for line in report.split("\n"):
        if line.startswith("# "):
            continue
        if re.match(r"^\*\*(Course|Project|Student|Language[^:]*|Build Environment):\*\*", line):
            continue
        lines.append(line)
    report = "\n".join(lines)
    report_html = markdown.markdown(report, extensions=["tables", "sane_lists"])

    parts.append("<div class='break'><h1>Part 2: Project Report</h1>"
                 "<div class='report'>%s</div></div>" % report_html)

    # ------------------------------------------- Part 3: the screenshots
    parts.append("<div class='break'><h1>Part 3: Screenshots of the program</h1>")
    parts.append(
        "<p>The dashboard of the program is the Administration tab: it shows the clinic "
        "summary and the records that were used most recently, and it carries the undo, "
        "save, load and new-database actions.</p>"
    )
    for relative, chip, caption in SCREENSHOTS:
        parts.append(image_block(relative, caption, chip))
    parts.append("</div>")

    # ------------------------------------------------ Part 4: the diagram
    parts.append("<div class='break'><h1>Part 4: UML/Class Diagram</h1>")
    parts.append(
        "<p>The diagram below shows the major classes of every layer and the "
        "relationships between them. The key underneath it points at the part of the "
        "picture that answers each item of the brief.</p>"
    )
    for relative, caption in DIAGRAMS:
        parts.append(image_block(relative, caption))
    parts.append("<h2>How the diagram shows each required relationship</h2>")
    parts.append(UML_KEY)
    parts.append("</div>")

    parts.append("</body></html>")

    document = "\n".join(parts)
    with open(path, "w", encoding="utf-8") as handle:
        handle.write(document)

    return len(document)


def find_chrome():
    candidates = [
        r"C:\Program Files\Google\Chrome\Application\chrome.exe",
        r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
        os.path.expandvars(r"%LOCALAPPDATA%\Google\Chrome\Application\chrome.exe"),
        r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
        r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
    ]
    for candidate in candidates:
        if candidate and os.path.exists(candidate):
            return candidate
    return None


def main():
    chrome = find_chrome()
    if chrome is None:
        print("Chrome or Edge was not found, so the PDF cannot be printed.")
        return 1

    work = tempfile.mkdtemp(prefix="cos202_report_")
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
    if result.stdout:
        print(result.stdout.strip())
    if result.stderr:
        print(result.stderr.strip())
    if result.returncode != 0:
        print("chrome returned", result.returncode)

    if not os.path.exists(PDF_PATH):
        print("the PDF was not produced")
        return 1

    print("pdf written: %s  (%.1f MB)"
          % (PDF_PATH, os.path.getsize(PDF_PATH) / 1048576.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
