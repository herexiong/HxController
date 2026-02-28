@echo off
setlocal

:: Get the directory of this script
set "DIR=%~dp0"
:: Remove trailing backslash
set "DIR=%DIR:~0,-1%"

:: The actual registry path for Microsoft Edge Native Messaging Hosts
set "REG_KEY=HKCU\Software\Microsoft\Edge\NativeMessagingHosts\com.hxmonitor.proxy"

:: The absolute path to the manifest file
set "MANIFEST_PATH=%DIR%\manifest.json"

echo Installing Edge Native Messaging Host...
echo Path: %MANIFEST_PATH%

:: Add registry key pointing to manifest
reg add "%REG_KEY%" /ve /t REG_SZ /d "%MANIFEST_PATH%" /f

if %ERRORLEVEL% equ 0 (
    echo.
    echo Successfully registered com.hxmonitor.proxy for Microsoft Edge!
    echo Please make sure your Edge extension ID matches the allowed_origins in manifest.json.
) else (
    echo.
    echo Failed to register Native Messaging host.
)

pause
