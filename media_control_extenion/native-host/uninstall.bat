@echo off
REM ============================================================
REM  Uninstall Native Messaging Host for Media Control Hub
REM ============================================================

set HOST_NAME=com.hxmonitor.proxy

reg delete "HKCU\Software\Google\Chrome\NativeMessagingHosts\%HOST_NAME%" /f 2>nul
reg delete "HKCU\Software\Microsoft\Edge\NativeMessagingHosts\%HOST_NAME%" /f 2>nul

echo.
echo Native Messaging Host unregistered.
echo.
pause
