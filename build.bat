@echo off
setlocal EnableDelayedExpansion

:: Augmentinel Windows Build Script
:: Usage: build.bat [debug|release|clean|package]
:: Prerequisites: Visual Studio 2019+, CMake, vcpkg with SDL2, SDL2_mixer, SDL2_ttf, GLEW

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=release

set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%build
set RELEASE_DIR=%PROJECT_DIR%release
set APP_NAME=Augmentinel
set VERSION=1.6.0

:: Colors not available in basic batch, using labels instead
goto :main

:info
echo [INFO] %~1
goto :eof

:warn
echo [WARN] %~1
goto :eof

:error
echo [ERROR] %~1
exit /b 1

:main
if "%BUILD_TYPE%"=="debug" goto :build_debug
if "%BUILD_TYPE%"=="release" goto :build_release
if "%BUILD_TYPE%"=="clean" goto :clean
if "%BUILD_TYPE%"=="package" goto :package
goto :usage

:build_debug
call :info "Building Debug configuration..."
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ..
if errorlevel 1 goto :cmake_error
cmake --build . --config Debug --parallel
if errorlevel 1 goto :build_error
call :info "Debug build complete: %BUILD_DIR%\Debug\%APP_NAME%.exe"
goto :end

:build_release
call :info "Building Release configuration..."
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ..
if errorlevel 1 goto :cmake_error
cmake --build . --config Release --parallel
if errorlevel 1 goto :build_error
call :info "Release build complete: %BUILD_DIR%\Release\%APP_NAME%.exe"
goto :end

:clean
call :info "Cleaning build directories..."
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%RELEASE_DIR%" rmdir /s /q "%RELEASE_DIR%"
call :info "Clean complete"
goto :end

:package
call :info "Creating Windows distribution package..."

:: Build release first
call :build_release
if errorlevel 1 goto :end

:: Create package directory
set PACKAGE_DIR=%RELEASE_DIR%\%APP_NAME%-%VERSION%-windows
if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
mkdir "%PACKAGE_DIR%"

:: Copy executable
copy "%BUILD_DIR%\Release\%APP_NAME%.exe" "%PACKAGE_DIR%\"

:: Copy resources
copy "%BUILD_DIR%\48.rom" "%PACKAGE_DIR%\"
copy "%BUILD_DIR%\sentinel.sna" "%PACKAGE_DIR%\"
xcopy /s /i "%BUILD_DIR%\shaders" "%PACKAGE_DIR%\shaders"
xcopy /s /i "%BUILD_DIR%\sounds" "%PACKAGE_DIR%\sounds"

:: Copy SDL2 DLLs (from vcpkg or manual installation)
:: These paths may need adjustment based on your vcpkg setup
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\SDL2.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\SDL2.dll" "%PACKAGE_DIR%\"
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\SDL2_mixer.dll" "%PACKAGE_DIR%\"
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\SDL2_ttf.dll" "%PACKAGE_DIR%\"
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\glew32.dll" "%PACKAGE_DIR%\"
)

:: Copy any additional dependencies (codec DLLs for SDL2_mixer)
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\libFLAC.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\libFLAC.dll" "%PACKAGE_DIR%\"
)
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\libmpg123.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\libmpg123.dll" "%PACKAGE_DIR%\"
)
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\libogg.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\libogg.dll" "%PACKAGE_DIR%\"
)
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\libvorbis.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\libvorbis.dll" "%PACKAGE_DIR%\"
)
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\libvorbisfile.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\libvorbisfile.dll" "%PACKAGE_DIR%\"
)
if exist "%VCPKG_ROOT%\installed\x64-windows\bin\zlib1.dll" (
    copy "%VCPKG_ROOT%\installed\x64-windows\bin\zlib1.dll" "%PACKAGE_DIR%\"
)

call :info "Windows package created: %PACKAGE_DIR%"

:: Create ZIP file if 7-Zip is available
where 7z >nul 2>&1
if not errorlevel 1 (
    call :info "Creating ZIP archive..."
    cd "%RELEASE_DIR%"
    7z a -tzip "%APP_NAME%-%VERSION%-windows.zip" "%APP_NAME%-%VERSION%-windows"
    call :info "ZIP created: %RELEASE_DIR%\%APP_NAME%-%VERSION%-windows.zip"
)

goto :end

:cmake_error
call :error "CMake configuration failed"
goto :end

:build_error
call :error "Build failed"
goto :end

:usage
echo Usage: %~nx0 [debug^|release^|clean^|package]
echo.
echo Commands:
echo   debug   - Build debug configuration
echo   release - Build release configuration (default)
echo   clean   - Remove build directories
echo   package - Build release and create distributable package
echo.
echo Prerequisites:
echo   - Visual Studio 2019 or later (with C++ tools)
echo   - CMake 3.15+
echo   - vcpkg with packages: sdl2 sdl2-mixer sdl2-ttf glew
echo   - Set VCPKG_ROOT environment variable
echo.
echo To install vcpkg packages:
echo   vcpkg install sdl2:x64-windows sdl2-mixer:x64-windows sdl2-ttf:x64-windows glew:x64-windows
goto :end

:end
cd "%PROJECT_DIR%"
endlocal
