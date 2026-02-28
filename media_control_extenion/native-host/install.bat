@echo off
REM ============================================================
REM  Install Native Messaging Host for Media Control Hub
REM  Run this script as Administrator
REM ============================================================

set MANIFEST_PATH=%~dp0com.hxmonitor.proxy.json
set HOST_NAME=com.hxmonitor.proxy

echo.
echo Installing Native Messaging Host: %HOST_NAME%
echo Manifest: %MANIFEST_PATH%
echo.

REM Register for Chrome (Current User)
reg add "HKCU\Software\Google\Chrome\NativeMessagingHosts\%HOST_NAME%" /ve /t REG_SZ /d "%MANIFEST_PATH%" /f

REM Register for Edge (Current User)
reg add "HKCU\Software\Microsoft\Edge\NativeMessagingHosts\%HOST_NAME%" /ve /t REG_SZ /d "%MANIFEST_PATH%" /f

echo.
echo ============================================================
echo  Done! Registered for both Chrome and Edge.
echo  Please restart your browser for changes to take effect.
echo ============================================================
echo.
pause
