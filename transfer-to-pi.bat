@echo off
REM Transfer GymBike kiosk files to Raspberry Pi
REM Usage: transfer-to-pi.bat <raspberry_pi_ip>

setlocal

if "%~1"=="" (
    echo Usage: transfer-to-pi.bat ^<raspberry_pi_ip^>
    echo Example: transfer-to-pi.bat 192.168.1.50
    exit /b 1
)

set PI_IP=%~1
set PI_USER=pi

echo ========================================
echo GymBike File Transfer to Raspberry Pi
echo ========================================
echo.
echo Target: %PI_USER%@%PI_IP%
echo.

REM Check if we have scp (OpenSSH)
where scp >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: scp not found!
    echo.
    echo Please install OpenSSH Client from Windows Settings or:
    echo   Settings ^> Apps ^> Optional Features ^> Add OpenSSH Client
    echo Or use WinSCP or another SCP client to transfer these files:
    echo.
    echo   - setup-kiosk.sh
    echo   - test_esp32.py
    echo   - test_ui.html
    echo   - diagnostic.html
    echo   - minimal_test.html
    echo   - emergency.html
    echo   - troubleshoot-kiosk.sh
    echo   - quick-test.sh
    echo   - RASPBERRY_PI_KIOSK.md
    echo   - BLANK_PAGE_FIX.md
    echo.
    pause
    exit /b 1
)

echo Transferring files...
echo.

scp setup-kiosk.sh gymbike-kiosk-startup.sh test_esp32.py test_ui.html RASPBERRY_PI_KIOSK.md diagnostic.html minimal_test.html troubleshoot-kiosk.sh quick-test.sh emergency.html BLANK_PAGE_FIX.md START_HERE.md AUTOSTART_FIX.md %PI_USER%@%PI_IP%:~/

if %ERRORLEVEL% NEQ 0 goto :error

echo.
echo ========================================
echo Transfer Complete!
echo ========================================
echo.
echo Files transferred to /home/%PI_USER%/
echo.
echo Next steps on Raspberry Pi:
echo   1. SSH: ssh %PI_USER%@%PI_IP%
echo   2. Make scripts executable:
echo      chmod +x *.sh
echo   3. Run quick test:
echo      ./quick-test.sh ^<ESP32_IP^>
echo   4. Run troubleshooting:
echo      ./troubleshoot-kiosk.sh
echo.
pause
exit /b 0

:error
echo.
echo ERROR: File transfer failed!
echo.
echo Troubleshooting:
echo   - Check that Raspberry Pi is powered on
echo   - Verify IP address: %PI_IP%
echo   - Ensure SSH is enabled on Raspberry Pi
echo   - Check network connectivity
echo.
pause
exit /b 1
