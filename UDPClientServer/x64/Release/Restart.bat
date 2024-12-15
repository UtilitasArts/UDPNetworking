@echo off

set "program=%1"

:: Get the folder 3 levels up from the current script
set "currentFolder=%~dp0"

:: Close the software
taskkill /IM %program% /F

cd /d "%currentFolder%"

:: Wait for 2 seconds before restarting
timeout /t 1 /nobreak > nul

:: Restart the software
start "" %program%
