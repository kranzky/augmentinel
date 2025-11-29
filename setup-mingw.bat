@echo off
REM Setup script for MinGW build dependencies
REM This script downloads SDL2_mixer, SDL2_ttf, and GLEW for MinGW

setlocal enabledelayedexpansion

set DEV_DIR=C:\Users\kranzky\dev
set SDL2_DIR=%DEV_DIR%\SDL2

echo ============================================
echo MinGW Dependencies Setup for Augmentinel
echo ============================================
echo.

REM Check for curl or wget
where curl >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: curl not found. Please install curl or download files manually.
    exit /b 1
)

REM Create temp directory
if not exist "%DEV_DIR%\temp" mkdir "%DEV_DIR%\temp"
cd /d "%DEV_DIR%\temp"

echo.
echo [1/3] Downloading SDL2_mixer...
if not exist "%SDL2_DIR%\i686-w64-mingw32\include\SDL2\SDL_mixer.h" (
    curl -L -o SDL2_mixer-2.6.3-mingw.tar.gz https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.6.3/SDL2_mixer-devel-2.6.3-mingw.tar.gz
    if %errorlevel% neq 0 (
        echo ERROR: Failed to download SDL2_mixer
        exit /b 1
    )
    tar -xf SDL2_mixer-2.6.3-mingw.tar.gz
    xcopy /E /Y "SDL2_mixer-2.6.3\i686-w64-mingw32\*" "%SDL2_DIR%\i686-w64-mingw32\"
    echo SDL2_mixer installed successfully.
) else (
    echo SDL2_mixer already installed.
)

echo.
echo [2/3] Downloading SDL2_ttf...
if not exist "%SDL2_DIR%\i686-w64-mingw32\include\SDL2\SDL_ttf.h" (
    curl -L -o SDL2_ttf-2.20.2-mingw.tar.gz https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.2/SDL2_ttf-devel-2.20.2-mingw.tar.gz
    if %errorlevel% neq 0 (
        echo ERROR: Failed to download SDL2_ttf
        exit /b 1
    )
    tar -xf SDL2_ttf-2.20.2-mingw.tar.gz
    xcopy /E /Y "SDL2_ttf-2.20.2\i686-w64-mingw32\*" "%SDL2_DIR%\i686-w64-mingw32\"
    echo SDL2_ttf installed successfully.
) else (
    echo SDL2_ttf already installed.
)

echo.
echo [3/3] Downloading GLEW...
if not exist "%DEV_DIR%\glew" (
    curl -L -o glew-2.2.0-mingw.zip https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip
    if %errorlevel% neq 0 (
        echo ERROR: Failed to download GLEW
        exit /b 1
    )
    tar -xf glew-2.2.0-mingw.zip
    move glew-2.2.0 "%DEV_DIR%\glew"
    echo GLEW installed successfully.
) else (
    echo GLEW already installed.
)

echo.
echo ============================================
echo Setup complete!
echo ============================================
echo.
echo SDL2 libraries are in: %SDL2_DIR%\i686-w64-mingw32
echo GLEW is in: %DEV_DIR%\glew
echo.
echo You can now build with:
echo   cd augmentinel
echo   build-mingw.bat
echo.

REM Cleanup temp
cd /d "%DEV_DIR%"
rmdir /S /Q "%DEV_DIR%\temp" 2>nul

endlocal
