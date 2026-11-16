# Re-captures the six screenshots of the submission.
#
# The window is captured with PrintWindow, so the whole window is in the
# picture even when it is wider than the screen.
#
#   powershell -ExecutionPolicy Bypass -File tools\capture_screenshots.ps1

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$exe  = Join-Path $root "build\bin\SCAPMS_GUI.exe"
$shotsDir = Join-Path $root "docs\screenshots"

if (-not (Test-Path $exe)) {
    throw "The program was not built. Run build_gui.bat first."
}

if (-not (Test-Path $shotsDir)) {
    New-Item -ItemType Directory -Force -Path $shotsDir | Out-Null
}

# A program that is still running would keep the binary locked, so any
# leftover instance is closed before the pictures are taken.
Get-Process SCAPMS_GUI -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 500

Add-Type -AssemblyName System.Drawing

Add-Type -ReferencedAssemblies System.Drawing @"
using System;
using System.Drawing;
using System.Runtime.InteropServices;

public class WindowCapture {
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint flags);

    [DllImport("user32.dll")]
    public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after, int x, int y, int cx, int cy, uint flags);

    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    public static void Capture(IntPtr handle, string path) {
        RECT rect;
        if (!GetWindowRect(handle, out rect)) {
            throw new Exception("The window could not be measured.");
        }

        int width = rect.Right - rect.Left;
        int height = rect.Bottom - rect.Top;

        using (Bitmap bitmap = new Bitmap(width, height))
        using (Graphics graphics = Graphics.FromImage(bitmap)) {
            IntPtr hdc = graphics.GetHdc();
            try {
                PrintWindow(handle, hdc, 2);
            } finally {
                graphics.ReleaseHdc(hdc);
            }
            bitmap.Save(path, System.Drawing.Imaging.ImageFormat.Png);
        }
    }
}
"@

$shots = @(
    @{ args = @("5");                 file = "1_main_dashboard.png" },
    @{ args = @("0");                 file = "2_patient_registration.png" },
    @{ args = @("1");                 file = "3_appointment_management.png" },
    @{ args = @("3", "GEN-101", "0"); file = "4_search_sort_interface.png" },
    @{ args = @("2", "GEN-101");      file = "5_waiting_queue.png" },
    @{ args = @("4");                 file = "6_reports.png" }
)

foreach ($shot in $shots) {
    $process = Start-Process -FilePath $exe -ArgumentList $shot.args -PassThru

    $handle = [IntPtr]::Zero
    for ($attempt = 0; $attempt -lt 60; $attempt++) {
        Start-Sleep -Milliseconds 250
        $process.Refresh()
        if ($process.HasExited) {
            throw "The program closed itself before the screenshot was taken."
        }
        if ($process.MainWindowHandle -ne [IntPtr]::Zero) {
            $handle = $process.MainWindowHandle
            break
        }
    }

    if ($handle -eq [IntPtr]::Zero) {
        $process.Kill()
        throw "No window appeared for $($shot.file)."
    }

    # The window is set to a size that fits on the screen and shows every
    # panel whole, and it is given a moment to lay itself out and to paint
    # the table before the picture is taken.
    [WindowCapture]::SetWindowPos($handle, [IntPtr]::Zero, 0, 0, 1500, 820, 0x0004) | Out-Null
    [WindowCapture]::SetForegroundWindow($handle) | Out-Null
    Start-Sleep -Milliseconds 2000

    $target = Join-Path $shotsDir $shot.file
    [WindowCapture]::Capture($handle, $target)

    $process.CloseMainWindow() | Out-Null
    if (-not $process.WaitForExit(3000)) {
        $process.Kill()
    }

    $info = Get-Item $target
    "{0,-26} {1,8:N0} bytes" -f $shot.file, $info.Length
}
