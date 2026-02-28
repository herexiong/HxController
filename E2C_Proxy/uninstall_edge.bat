@echo off
setlocal

set "REG_KEY=HKCU\Software\Microsoft\Edge\NativeMessagingHosts\com.hxmonitor.proxy"

echo Uninstalling Edge Native Messaging Host...
reg delete "%REG_KEY%" /f

if %ERRORLEVEL% equ 0 (
    echo.
    echo Successfully removed com.hxmonitor.proxy from Microsoft Edge!
) else (
    echo.
    echo Failed to remove registry key, or it didn't exist.
)

pause
