@echo off
REM Quick test script for BrainLLM

echo Testing BrainLLM API Server...
echo.

REM Start the API server
echo Starting API server on localhost:8080...
start /B "BrainLLM_API" ".\build\Release\BrainLLM_API.exe"

REM Wait for server to start
timeout /t 2 /nobreak

REM Test endpoints
echo.
echo Testing endpoints...
echo.

echo 1. Testing /api/status endpoint...
curl -s http://localhost:8080/api/status
echo.
echo.

echo 2. Testing /api/config endpoint...
curl -s http://localhost:8080/api/config
echo.
echo.

echo 3. Testing /api/process endpoint...
curl -s -X POST http://localhost:8080/api/process -d "Hello"
echo.
echo.

echo Test completed!
pause
