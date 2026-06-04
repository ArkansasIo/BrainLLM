@echo off
REM Quick test script for BrainLLM

echo Testing BrainLLM API Server...
echo.

REM Start the API server
echo Starting API server on localhost:8080...
set API_EXE=.\output\build\BrainLLM_API.exe
if not exist "%API_EXE%" set API_EXE=.\build\Release\BrainLLM_API.exe
start /B "BrainLLM_API" "%API_EXE%"

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
curl -s -X POST http://localhost:8080/api/process -H "Content-Type: application/json" -d "{\"input\":\"Hello\"}"
echo.
echo.

echo 4. Testing /api/chat endpoint...
curl -s -X POST http://localhost:8080/api/chat -H "Content-Type: application/json" -d "{\"messages\":[{\"role\":\"user\",\"content\":\"summarize BrainLLM\"}],\"max_tokens\":120}"
echo.
echo.

echo 5. Testing /api/endpoints endpoint...
curl -s http://localhost:8080/api/endpoints
echo.
echo.

echo 6. Testing web client endpoint...
curl -s -I http://localhost:8080/client
echo.
echo.

echo Test completed!
pause
