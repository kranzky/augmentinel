@echo off
REM Script to download and setup modern MinGW (GCC 13)
REM This replaces the old GCC 8.1.0 that has broken C++17 filesystem support

setlocal enabledelayedexpansion

set DEV_DIR=C:\Users\kranzky\dev
set MINGW_URL=https://github.com/niXman/mingw-builds-binaries/releases/download/13.2.0-rt_v11-rev1/i686-13.2.0-release-win32-dwarf-msvcrt-rt_v11-rev1.7z

echo ============================================
echo MinGW Upgrade Script
echo ============================================
echo.
echo This will download MinGW GCC 13.2.0 (32-bit)
echo Current: GCC 8.1.0 (has broken C++17 filesystem)
echo.
echo The new MinGW will be installed to: %DEV_DIR%\mingw32-new
echo.

REM Check for 7z
set SEVENZIP=7z
where 7z >nul 2>&1
if %errorlevel% neq 0 (
    if exist "C:\Program Files\7-Zip\7z.exe" (
        set SEVENZIP="C:\Program Files\7-Zip\7z.exe"
    ) else if exist "C:\Program Files (x86)\7-Zip\7z.exe" (
        set SEVENZIP="C:\Program Files (x86)\7-Zip\7z.exe"
    ) else (
        echo WARNING: 7-Zip not found.
        echo Please download and extract manually:
        echo   1. Download: %MINGW_URL%
        echo   2. Extract to: %DEV_DIR%\mingw32-new
        echo   3. Update your PATH to point to %DEV_DIR%\mingw32-new\bin
        echo.
        echo Or install 7-Zip from: https://www.7-zip.org/
        pause
        exit /b 1
    )
)

REM Create temp directory
if not exist "%DEV_DIR%\temp" mkdir "%DEV_DIR%\temp"
cd /d "%DEV_DIR%\temp"

echo Downloading MinGW GCC 13.2.0...
curl -L -o mingw32-13.7z "%MINGW_URL%"
if %errorlevel% neq 0 (
    echo ERROR: Download failed
    exit /b 1
)

echo.
echo Extracting...
%SEVENZIP% x -y mingw32-13.7z -o"%DEV_DIR%"
if %errorlevel% neq 0 (
    echo ERROR: Extraction failed
    exit /b 1
)

REM Rename the extracted folder
if exist "%DEV_DIR%\mingw32" (
    echo Backing up old MinGW to mingw32-old...
    if exist "%DEV_DIR%\mingw32-old" rmdir /S /Q "%DEV_DIR%\mingw32-old"
    move "%DEV_DIR%\mingw32" "%DEV_DIR%\mingw32-old"
)
move "%DEV_DIR%\i686-13.2.0-release-win32-dwarf-msvcrt-rt_v11-rev1" "%DEV_DIR%\mingw32"

echo.
echo Cleaning up...
del "%DEV_DIR%\temp\mingw32-13.7z"
rmdir /S /Q "%DEV_DIR%\temp" 2>nul

echo.
echo ============================================
echo MinGW upgraded successfully!
echo ============================================
echo.
echo New GCC version:
"%DEV_DIR%\mingw32\bin\gcc" --version
echo.
echo Your PATH should already include: %DEV_DIR%\mingw32\bin
echo If not, please update it.
echo.
echo You can now run: build-mingw.bat
echo.

endlocal
pause
