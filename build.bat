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
    echo.
    echo ========================================
    echo Build completed successfully!
    echo ========================================
    echo.
    echo Executables:
    echo   - GUI: .\Release\BrainLLM_GUI.exe
    echo   - API: .\Release\BrainLLM_API.exe
    echo.
) else (
    echo.
    echo ========================================
    echo Build failed with errors!
    echo ========================================
)

cd ..
pause
