@echo off
REM Build script for MinGW
REM Usage: build-mingw.bat [debug|release|clean]

setlocal enabledelayedexpansion

set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%build-mingw
set BUILD_TYPE=Release

REM Parse command line argument
if "%1"=="debug" (
    set BUILD_TYPE=Debug
) else if "%1"=="release" (
    set BUILD_TYPE=Release
) else if "%1"=="clean" (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /S /Q "%BUILD_DIR%"
    echo Done.
    exit /b 0
) else if "%1"=="" (
    REM Default to Release
    set BUILD_TYPE=Release
) else (
    echo Usage: build-mingw.bat [debug^|release^|clean]
    exit /b 1
)

echo ============================================
echo Building Augmentinel with MinGW
echo Build type: %BUILD_TYPE%
echo ============================================
echo.

REM Check for MinGW
where gcc >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: gcc not found. Make sure MinGW is in your PATH.
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: cmake not found. Please install CMake.
    exit /b 1
)

REM Check for required libraries
if not exist "C:\Users\kranzky\dev\SDL2\i686-w64-mingw32\include\SDL2\SDL_mixer.h" (
    echo ERROR: SDL2_mixer not found. Run setup-mingw.bat first.
    exit /b 1
)
if not exist "C:\Users\kranzky\dev\SDL2\i686-w64-mingw32\include\SDL2\SDL_ttf.h" (
    echo ERROR: SDL2_ttf not found. Run setup-mingw.bat first.
    exit /b 1
)
if not exist "C:\Users\kranzky\dev\glew\include\GL\glew.h" (
    echo ERROR: GLEW not found. Run setup-mingw.bat first.
    exit /b 1
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Copy MinGW-specific CMakeLists.txt
copy /Y "%PROJECT_DIR%CMakeLists-mingw.txt" "%PROJECT_DIR%CMakeLists.txt.bak" >nul 2>&1
copy /Y "%PROJECT_DIR%CMakeLists.txt" "%PROJECT_DIR%CMakeLists-original.txt" >nul 2>&1
copy /Y "%PROJECT_DIR%CMakeLists-mingw.txt" "%PROJECT_DIR%CMakeLists.txt" >nul 2>&1

REM Run CMake with MinGW Makefiles generator
echo Running CMake...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ "%PROJECT_DIR%"

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    copy /Y "%PROJECT_DIR%CMakeLists-original.txt" "%PROJECT_DIR%CMakeLists.txt" >nul 2>&1
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE% -- -j%NUMBER_OF_PROCESSORS%

if %errorlevel% neq 0 (
    echo ERROR: Build failed!
    copy /Y "%PROJECT_DIR%CMakeLists-original.txt" "%PROJECT_DIR%CMakeLists.txt" >nul 2>&1
    exit /b 1
)

REM Restore original CMakeLists.txt
copy /Y "%PROJECT_DIR%CMakeLists-original.txt" "%PROJECT_DIR%CMakeLists.txt" >nul 2>&1
del "%PROJECT_DIR%CMakeLists-original.txt" >nul 2>&1
del "%PROJECT_DIR%CMakeLists.txt.bak" >nul 2>&1

echo.
echo ============================================
echo Build successful!
echo Executable: %BUILD_DIR%\Augmentinel.exe
echo ============================================

cd /d "%PROJECT_DIR%"
endlocal
