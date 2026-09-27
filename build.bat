@echo off
setlocal
:: Augmentinel build script for Windows (Visual Studio 2019+ and CMake 3.21+).
::
::   build.bat [debug|release|clean|package]
::
::   debug    Debug build: build\Debug\Augmentinel.exe
::   release  Release build: build\Release\Augmentinel.exe (default)
::   clean    Remove build output
::   package  Release build plus game data in dist\Augmentinel-windows-x64 (unsigned;
::            signed release builds come from CI, see packaging\windows\SIGNING.md)
::
:: SDL2 and SDL2_mixer are downloaded and built by CMake, and everything links
:: statically, so no other dependencies or DLLs are needed.

set ROOT=%~dp0
set BUILD=%ROOT%build
set CONFIG=Release
set COMMAND=%1
if "%COMMAND%"=="" set COMMAND=release

if /i "%COMMAND%"=="debug" set CONFIG=Debug& goto build
if /i "%COMMAND%"=="release" goto build
if /i "%COMMAND%"=="package" goto build
if /i "%COMMAND%"=="clean" goto clean
findstr /b "::" "%~f0"
exit /b 1

:build
cmake -S "%ROOT%." -B "%BUILD%" || exit /b 1
cmake --build "%BUILD%" --config %CONFIG% --parallel || exit /b 1
echo Built %BUILD%\%CONFIG%\Augmentinel.exe
if /i not "%COMMAND%"=="package" exit /b 0

set OUT=%ROOT%dist\Augmentinel-windows-x64
if exist "%OUT%" rmdir /s /q "%OUT%"
mkdir "%OUT%"
copy /y "%BUILD%\Release\Augmentinel.exe" "%OUT%" >nul
copy /y "%BUILD%\Release\48.rom" "%OUT%" >nul
copy /y "%BUILD%\Release\sentinel.sna" "%OUT%" >nul
xcopy /e /i /q /y "%BUILD%\Release\shaders" "%OUT%\shaders" >nul
xcopy /e /i /q /y "%BUILD%\Release\sounds" "%OUT%\sounds" >nul
copy /y "%ROOT%packaging\README.txt" "%OUT%" >nul
copy /y "%ROOT%COPYING" "%OUT%" >nul
echo Packaged %OUT%
exit /b 0

:clean
if exist "%BUILD%" rmdir /s /q "%BUILD%"
if exist "%ROOT%dist" rmdir /s /q "%ROOT%dist"
exit /b 0
