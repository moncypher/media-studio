@echo off
REM Media Studio Installer Builder
REM Usage: build-installer.bat [version] [output-path]

setlocal enabledelayedexpansion

set "VERSION=%1"
if "!VERSION!"=="" set "VERSION=30.0.0"

set "OUTPUT_PATH=%2"
if "!OUTPUT_PATH!"=="" set "OUTPUT_PATH=..\build_x64\rundir\RelWithDebInfo"

set "NSIS_PATH=C:\Program Files (x86)\NSIS\makensis.exe"
set "INSTALLER_SCRIPT=%~dp0media-studio.nsi"

echo.
echo Media Studio Installer Builder
echo ==============================
echo.

REM Check NSIS
if not exist "!NSIS_PATH!" (
    echo ERROR: NSIS not found at !NSIS_PATH!
    echo Please install NSIS from: https://nsis.sourceforge.io/
    echo.
    pause
    exit /b 1
)
echo [OK] NSIS found

REM Check build output
if not exist "!OUTPUT_PATH!" (
    echo ERROR: Build path not found: !OUTPUT_PATH!
    echo Please build the project first
    echo.
    pause
    exit /b 1
)
echo [OK] Build output found at !OUTPUT_PATH!

REM Check installer script
if not exist "!INSTALLER_SCRIPT!" (
    echo ERROR: Installer script not found
    echo.
    pause
    exit /b 1
)
echo [OK] Installer script found

echo.
echo Building installer...
echo   Version: !VERSION!
echo   Output: !OUTPUT_PATH!
echo.

REM Run NSIS
"!NSIS_PATH!" "/DVERSION=!VERSION!" "/DOUTPUT_PATH=!OUTPUT_PATH!" "/V4" "!INSTALLER_SCRIPT!"

if %errorlevel% equ 0 (
    echo.
    echo [SUCCESS] Installer created at:
    echo   !OUTPUT_PATH!\Media-Studio-Installer.exe
    echo.
) else (
    echo.
    echo [ERROR] NSIS compilation failed!
    echo.
    pause
    exit /b 1
)
