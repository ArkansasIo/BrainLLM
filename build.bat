@echo off
REM Build script for BrainLLM on Windows

setlocal enabledelayedexpansion

echo ========================================
echo BrainLLM Build System
echo ========================================

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

REM Build the project
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% EQU 0 (
    cd ..
    if not exist output\build mkdir output\build
    echo Packaging runtime files into output\build...
    copy /Y build\Release\BrainLLM_API.exe output\build\BrainLLM_API.exe >nul
    copy /Y build\Release\BrainLLM_GUI.exe output\build\BrainLLM_GUI.exe >nul
    if exist build\Release\client xcopy /E /I /Y build\Release\client output\build\client >nul
    if exist build\Release\assets xcopy /E /I /Y build\Release\assets output\build\assets >nul
    if exist build\Release\data xcopy /E /I /Y build\Release\data output\build\data >nul
    if exist build\Release\scripts xcopy /E /I /Y build\Release\scripts output\build\scripts >nul
    if exist build\Release\tools xcopy /E /I /Y build\Release\tools output\build\tools >nul
    if exist build\Release\third_party robocopy build\Release\third_party output\build\third_party /MIR >nul
    if exist config.ini copy /Y config.ini output\build\config.ini >nul
    where windeployqt >nul 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo Deploying Qt runtime files...
        windeployqt --release --compiler-runtime output\build\BrainLLM_GUI.exe
        windeployqt --release --compiler-runtime output\build\BrainLLM_API.exe
    ) else if exist C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe (
        echo Deploying Qt runtime files...
        C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe --release --compiler-runtime output\build\BrainLLM_GUI.exe
        C:\Qt\6.7.3\msvc2019_64\bin\windeployqt.exe --release --compiler-runtime output\build\BrainLLM_API.exe
    ) else (
        echo Warning: windeployqt not found. Packaged executables may require Qt DLLs from your PATH.
    )
    echo.
    echo ========================================
    echo Build completed successfully!
    echo ========================================
    echo.
    echo Executables:
    echo   - GUI: .\build\Release\BrainLLM_GUI.exe
    echo   - API: .\build\Release\BrainLLM_API.exe
    echo   - Packaged API: .\output\build\BrainLLM_API.exe
    echo   - Web client: http://localhost:8080/client
    echo.
) else (
    cd ..
    echo.
    echo ========================================
    echo Build failed with errors!
    echo ========================================
)

pause
